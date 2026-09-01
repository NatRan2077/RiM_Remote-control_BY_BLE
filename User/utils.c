//----------------------------------------------------------------------------//
#include "utils.h"
#include "cmt453x_gpio.h"
#include "cmt453x_rcc.h"
#include "messages.h"
#include "memory.h"
#include "passwords.h"
#include "log.h"
#include "HAL.h"
#include "MD5.h"
#include "bootloader.h"
#include "pt.h"
#include "threads.h"
//----------------------------------------------------------------------------//
// POWER Pin
#define POWER_GPIO_CLK RCC_APB2_PERIPH_GPIOB
#define POWER_GPIO GPIOB
#define POWER_PIN GPIO_PIN_13
//----------------------------------------------------------------------------//
#define MAIN_APP_START_ADDRESS (FLASH_BASE + 0x8000UL) // 0x01008000
void remap_vector_table(void) {
	__set_PRIMASK(1);
	PWR->VTOR_REG =
	    0x80000000 |
	    MAIN_APP_START_ADDRESS; // SCB->VTOR = FMC_BASE | MAIN_APP_START_ADDRESS;
	__set_PRIMASK(0);
}
//----------------------------------------------------------------------------//
void SupplyEn(void) { // Включение питания
	GPIO_InitType GPIO_InitStruct;

	volatile u32 i = 100000; // Задержка для исключения неправильного включения
	while (i--)
		;

	RCC_EnableAPB2PeriphClk(POWER_GPIO_CLK, ENABLE);

	GPIO_InitStruct.Pin = POWER_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.GPIO_Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.GPIO_Alternate = 0;
	GPIO_InitStruct.GPIO_Pull = GPIO_NO_PULL;
	GPIO_InitPeripheral(POWER_GPIO, &GPIO_InitStruct);
	GPIO_SetBits(POWER_GPIO, POWER_PIN);

	tim.init();

	SystemInfo.flag.logOn = true;
}
//----------------------------------------------------------------------------//
void SupplyDis(void) { // Выключение питания
	GPIO_ResetBits(POWER_GPIO, POWER_PIN);
}
//----------------------------------------------------------------------------//
static void IWDG_Init(void) { // Настройка WatchDog
#if IWDG_ON == 1
	IWDG_WriteConfig(IWDG_WRITE_ENABLE);
	IWDG_SetPrescalerDiv(IWDG_PRESCALER_DIV32);
	IWDG_CntReload(4095);
	IWDG_ReloadKey();
	while (RESET == IWDG_GetStatus(IWDG_PVU_FLAG))
		;
	while (RESET == IWDG_GetStatus(IWDG_CRVU_FLAG))
		;
	IWDG_Enable();
#endif
}
//----------------------------------------------------------------------------//
void IWDG_Refresh(void) { // Сброс IWDG
	IWDG_ReloadKey();
}
//----------------------------------------------------------------------------//
#define MODEL_ID_DEVICE_STR_LEN_MAX 10 + 1
#define FIRMWARE_VERSION_STR_LEN 17
const char STR_MODEL_ID_DEVICE[MODEL_ID_DEVICE_STR_LEN_MAX] =
    "RiM 040.40"; // Тип модуля (идентификатор)
static const u32 FIRMWARE_ID = 0x01404000;
const char STR_FIRMWARE_VERSION[FIRMWARE_VERSION_STR_LEN] = FW_VERSION;

void Init_Core(void) { // Инициализация основных модулей
	IWDG_Init();        // Настройка WatchDog
	tim.init();
	if (false == memory.init()) { // Инициализация flash-памяти
		_SYSLOG_("UNKNOWN MEMORY!!!");
	}
	SystemInfo.str.modelIdDevice = STR_MODEL_ID_DEVICE;
	SystemInfo.str.firmwareVersion = STR_FIRMWARE_VERSION;
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
void ReadSystemInfoAndSettings(
    void) { // Считываем системную инфромацию и настройки
	char md5_calc[MD5_STR_LEN];

	SystemInfo.firmwareID = (u32 *)&FIRMWARE_ID; // Индентификатор ПО
	SystemInfo.str.modelIdDevice = STR_MODEL_ID_DEVICE;
	SystemInfo.str.firmwareVersion = STR_FIRMWARE_VERSION;

	SystemInfo.image.blockSize = 256;

	MD5Init();
	MD5Update((char *)BOOTLOADER_FLASH_MAIN_APP_START_ADDRESS,
	          BOOTLOADER_FLASH_MAIN_APP_SIZE);
	MD5Final(md5_calc);
	strncpy(SystemInfo.MD5.mainProgramm, md5_calc, MD5_STR_LEN);
	char MD5_Str[MD5_STR_LEN * 2];
	MD5_ToHexStr(md5_calc, MD5_Str);

	SystemInfo.MD5.range.start = BOOTLOADER_FLASH_MAIN_APP_START_ADDRESS;
	SystemInfo.MD5.range.end = BOOTLOADER_FLASH_MAIN_APP_END_ADDRESS;
	MD5Init();
	MD5Update((char *)SystemInfo.MD5.range.start,
	          (SystemInfo.MD5.range.end - SystemInfo.MD5.range.start + 1));
	MD5Final(md5_calc);
	strncpy(SystemInfo.MD5.userRange, md5_calc, MD5_STR_LEN);

	if (RESULT_ERROR == memory.readTwoCopyesWithCRC(
	                        MEMORY_SERIAL_NUMBER, &SystemInfo.serialNumber,
	                        sizeof(SystemInfo.serialNumber))) {
		SystemInfo.serialNumber = 0xFFFFFFFF;
		_SYSLOG_("Config - Error Read Number Pult or No Number In Memory");
	}
	if (RESULT_ERROR ==
	    memory.readTwoCopyesWithCRC(MEMORY_METER_NUMBER, &SystemInfo.numMeter,
	                                sizeof(SystemInfo.numMeter))) {
		SystemInfo.numMeter = 0xFFFFFFFF;
		_SYSLOG_("Config - Error Read Meter Number or No Number In Memory");
	}
	if (RESULT_ERROR ==
	    memory.readTwoCopyesWithCRC(MEMORY_PINCODE, &SystemInfo.pinCode,
	                                sizeof(SystemInfo.pinCode))) {
		SystemInfo.pinCode = 0xFFFFFFFF;
		_SYSLOG_("Config - Error Read PinCode or No PinCode In Memory");
	}
	if (RESULT_ERROR == memory.readTwoCopyesWithCRC(
	                        MEMORY_SETTINGS_DEVICE_PASSWORD,
	                        &SystemInfo.password, sizeof(SystemInfo.password))) {
		memset(SystemInfo.password, 0x00, sizeof(SystemInfo.password));
		_SYSLOG_("Config - Error Read Password or No Password In Memory");
	}
	if (RESULT_ERROR ==
	    memory.readTwoCopyesWithCRC(MEMORY_DATE_OF_PRODUCTION,
	                                &SystemInfo.dateOfProduction,
	                                sizeof(SystemInfo.dateOfProduction))) {
		SystemInfo.dateOfProduction.yearHiByte = BYTE_1(2026);
		SystemInfo.dateOfProduction.yearLowByte = BYTE_0(2026);
		SystemInfo.dateOfProduction.month = APRIL;
		SystemInfo.dateOfProduction.dayOfMonth = 27;
		SystemInfo.dateOfProduction.dayOfWeek = MONDAY;
		_SYSLOG_("Config - Error Read Data Of Production or No Data In Memory");
	}

	_SYSLOG_("**********************");
	_SYSLOG_("----- RiM 040.40 -----");
	_SYSLOG_("Firmware version: %s", SystemInfo.str.firmwareVersion);
	_SYSLOG_("Number: %u", SystemInfo.serialNumber);
	_SYSLOG_("Type: %s", SystemInfo.str.modelIdDevice);
	//	_SYSLOG_("MD5: %s", MD5_Str);

	memory.readTwoCopyesWithCRC(MEMORY_OPERATING_TIME,
	                            &SystemInfo.cnt.operatingTime,
	                            sizeof(SystemInfo.cnt.operatingTime));
	memory.readTwoCopyesWithCRC(MEMORY_LAST_SESSION_OPERATING_TIME,
	                            &SystemInfo.cnt.lastSessionOperatingTime,
	                            sizeof(SystemInfo.cnt.lastSessionOperatingTime));
	memory.readTwoCopyesWithCRC(MEMORY_CNT_STARTS, &SystemInfo.cnt.starts,
	                            sizeof(SystemInfo.cnt.starts));

	SystemInfo.cnt.starts++; // увеличиваем счетчик включений
	Memory_Write_And_Update_Status(MEMORY_CNT_STARTS, &SystemInfo.cnt.starts,
	                               sizeof(SystemInfo.cnt.starts));

	_SYSLOG_("");
	_SYSLOG_("Operating time: %d s", (SystemInfo.cnt.operatingTime +
	                                  SystemInfo.cnt.lastSessionOperatingTime));
	_SYSLOG_("Cnt Starts: %d", SystemInfo.cnt.starts);
	_SYSLOG_("");
	Passwords_Load();
	_SYSLOG_("**********************");
}
//----------------------------------------------------------------------------//
void Threads_Init(void) { // Инициализация потоков
	PT_INIT(&lcd_pt);
	PT_INIT(&btn_pt);
	PT_INIT(&main_pt);
	PT_INIT(&status_pt);
	PT_INIT(&usbUart_pt);
	PT_INIT(&ble_pt);
	PT_INIT(&rim_pt);
}
//----------------------------------------------------------------------------//
result_t Memory_Write_And_Update_Status(u32 page, void *const buffer_ptr,
                                        u8 len) {
	if (RESULT_ERROR == memory.writeTwoCopyesWithCRC(page, buffer_ptr, len)) {
		// Добавить запись в журнал
		return RESULT_ERROR;
	} else {
		return RESULT_SUCCESS;
	}
}
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
void Clear_Buffer(u8 *buff, u16 size) { // Заполнить буфер нулями
	memset(buff, 0, size);
}

// char tmpBuf[100];
////----------------------------------------------------------------------------//
// void SysLogSend(char* str){
//	if(1 == PMU_GetModeStatus()){	//Отправляем только в нормальном режиме
//		u8 len = strlen(str);
//		for(u8 i = 0; i < len; i++){
//			UART4->DATA = str[i];
//			while(0 == UART_GetFlag(UART4, UART_INTSTS_TXDONE));	//ожидаем
// передачи байта 			UART_ClearFlag(UART4, UART_INTSTS_TXDONE);
//		}
//		UART4->DATA = '\r';
//		while(0 == UART_GetFlag(UART4, UART_INTSTS_TXDONE));	//ожидаем передачи
// байта 		UART_ClearFlag(UART4, UART_INTSTS_TXDONE); 		UART4->DATA =
// '\n'; 		while(0
//== UART_GetFlag(UART4, UART_INTSTS_TXDONE));	//ожидаем передачи байта
//		UART_ClearFlag(UART4, UART_INTSTS_TXDONE);
//		u32 tt = 100000;
//		while(tt--);
//	}
// }

#ifdef USE_FULL_ASSERT
void assert_failed(const uint8_t *expr, const uint8_t *file, uint32_t line) {
	while (1) {}
}
#endif // USE_FULL_ASSERT
