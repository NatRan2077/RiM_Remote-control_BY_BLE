//----------------------------------------------------------------------------//
#include "utils.h"
#include "cmt453x_gpio.h"
#include "cmt453x_rcc.h"
#include "memory.h"
#include "HAL.h"
#include "lcd.h"
#include "MD5.h"
#include "bootloader.h"
#include "USB_Driver.h"
#include <string.h>
//----------------------------------------------------------------------------//
//POWER Pin
#define POWER_GPIO_CLK	RCC_APB2_PERIPH_GPIOB
#define POWER_GPIO		GPIOB
#define POWER_PIN		GPIO_PIN_13
//----------------------------------------------------------------------------//
void SupplyEn(void){	//Включение питания
	GPIO_InitType	GPIO_InitStruct;
	
	volatile u32 i = 100000;	//Задержка для исключения неправильного включения
	while(i--);

	RCC_EnableAPB2PeriphClk(POWER_GPIO_CLK, ENABLE);
	
	GPIO_InitStruct.Pin				= POWER_PIN;
    GPIO_InitStruct.GPIO_Speed		= GPIO_SPEED_HIGH; 
    GPIO_InitStruct.GPIO_Mode		= GPIO_MODE_OUTPUT_PP;            
    GPIO_InitStruct.GPIO_Alternate	= 0;
    GPIO_InitStruct.GPIO_Pull		= GPIO_NO_PULL;
    GPIO_InitPeripheral(POWER_GPIO, &GPIO_InitStruct);
	GPIO_SetBits(POWER_GPIO, POWER_PIN);
}
//----------------------------------------------------------------------------//
void SupplyDis(void){	//Выключение питания
	GPIO_ResetBits(POWER_GPIO, POWER_PIN);
}
//----------------------------------------------------------------------------//
static void IWDG_Init(void){	//Настройка WatchDog
#if IWDG_ON == 1
	IWDG_WriteConfig(IWDG_WRITE_ENABLE);
	IWDG_SetPrescalerDiv(IWDG_PRESCALER_DIV32);
	IWDG_CntReload(4095);
	IWDG_ReloadKey();
	while(RESET == IWDG_GetStatus(IWDG_PVU_FLAG));
	while(RESET == IWDG_GetStatus(IWDG_CRVU_FLAG));
	IWDG_Enable();
#endif
}
//----------------------------------------------------------------------------//
void IWDG_Refresh(void){	//Сброс IWDG
	IWDG_ReloadKey();
}
//----------------------------------------------------------------------------//
void Init_Core(void){	//Инициализация основных модулей
	char md5_calc[MD5_STR_LEN];
	
	IWDG_Init();		//Настройка WatchDog
	memory.init();		//Инициализация flash-памяти
	USB_Driver.uartInit();	//Инициализация служебного интерфейса
	lcd.init();
	lcd.all();
	lcd.update();
	
	_SYSLOG_("********************************");
	_SYSLOG_("----- RiM 040.40_BOOTLOADER-----");
	_SYSLOG_("Software version: %x.%2.2x", VERSION, BUILD);
	_SYSLOG_("Build Date And Time: %s %s", __DATE__, __TIME__);
	
	MD5Init();
	MD5Update((char*)BOOTLOADER_FLASH_BOOTLOADER_START_ADDRESS, BOOTLOADER_FLASH_BOOTLOADER_SIZE);
	MD5Final(md5_calc);
	char MD5_Str[MD5_STR_LEN*2];
	MD5_ToHexStr(md5_calc, MD5_Str);
	_SYSLOG_("MD5: %s", MD5_Str);
}
//----------------------------------------------------------------------------//
#ifdef USE_FULL_ASSERT
void assert_failed(const uint8_t* expr, const uint8_t* file, uint32_t line)
{
    while (1)
    {
    }
}
#endif // USE_FULL_ASSERT
//----------------------------------------------------------------------------//
