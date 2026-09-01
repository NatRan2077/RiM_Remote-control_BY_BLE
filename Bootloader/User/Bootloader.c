//----------------------------------------------------------------------------//
#include "bootloader.h"
#include "utils.h"
#include "memory.h"
#include "lcd.h"
#include "USB_Driver.h"
#include "cmt453x_rcc.h"
#include "cmt453x_qflash.h"
#include <string.h>
#include "MD5.h"
//----------------------------------------------------------------------------//
#define UPDATE_DEVICE_KEY							"Update Device Flag Set"
#define UPDATE_DEVICE_ERROR_MD5_IN_EXTERNAL_MEMORY	"Update Device Error MD5 In External Memory"
#define UPDATE_DEVICE_SUCCESS						"Update Device Success"
//----------------------------------------------------------------------------//
#define BOOTLOADER_UPDATE_DEVICE_FLAG_SIZE_MAX	100
//----------------------------------------------------------------------------//
typedef void (*pFunction)(void);
#define MAIN_APP_START_ADDRESS  0x01008000UL
//----------------------------------------------------------------------------//
void Bootloader_JumpToApplication(void){	//Переход на основную программу
	_SYSLOG_("Jump To Main Application!!!");
	_SYSLOG_("********************************");
	
	lcd.all();
	lcd.update();
	while(!USB_Driver.getTransmissionCompleteFlag());	//Ждем пока не передадим все байты
	
	
	for(volatile int delay = 0; delay < 10000; delay++);	//Без задержки основная программа не стартует
	
	uint32_t  JumpAddress = *(__IO uint32_t*)(BOOTLOADER_FLASH_MAIN_APP_START_ADDRESS + 4);
	pFunction Jump = (pFunction)JumpAddress;
	__disable_irq();
	
	//Деинициализируем всю периферию
//	RCC_DeInit();
//	
//	RCC_EnableAHBPeriphReset((RCC_AHB_PERIPH_ADC | RCC_AHB_PERIPH_IRC), ENABLE);
//	RCC_EnableAHBPeriphReset((RCC_AHB_PERIPH_ADC | RCC_AHB_PERIPH_IRC), DISABLE);
//    
//	RCC_EnableAPB2PeriphReset((	RCC_APB2_PERIPH_AFIO | RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOB | 
//								RCC_APB2_PERIPH_SPI1 | RCC_APB2_PERIPH_SPI2 | RCC_APB2_PERIPH_TIM1 |  RCC_APB2_PERIPH_USART1), ENABLE);
//	RCC_EnableAPB2PeriphReset((	RCC_APB2_PERIPH_AFIO | RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOB | 
//								RCC_APB2_PERIPH_SPI1 | RCC_APB2_PERIPH_SPI2 | RCC_APB2_PERIPH_TIM1 |  RCC_APB2_PERIPH_USART1), DISABLE);

//	RCC_EnableAPB1PeriphReset((RCC_APB1_PERIPH_TIM3 | RCC_APB1_PERIPH_TIM6 | RCC_APB1_PERIPH_PWR), ENABLE);
//	RCC_EnableAPB1PeriphReset((RCC_APB1_PERIPH_TIM3 | RCC_APB1_PERIPH_TIM6 | RCC_APB1_PERIPH_PWR), DISABLE);

//	

//	 __DSB();
//    __ISB();
	
	USART_Enable(USART2, DISABLE);
	I2C_Enable(I2C1, DISABLE);
	
	RCC_EnableAPB2PeriphReset((RCC_APB2_PERIPH_AFIO | RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOB | RCC_APB2_PERIPH_SPI1), ENABLE);
	RCC_EnableAPB2PeriphReset((RCC_APB2_PERIPH_AFIO | RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_GPIOB | RCC_APB2_PERIPH_SPI1), DISABLE);

	RCC_EnableAPB1PeriphReset((RCC_APB1_PERIPH_USART2 | RCC_APB1_PERIPH_I2C1), ENABLE);
	RCC_EnableAPB1PeriphReset((RCC_APB1_PERIPH_USART2 | RCC_APB1_PERIPH_I2C1), DISABLE);
	
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL  = 0;
	
	__set_MSP(*(__IO uint32_t*)BOOTLOADER_FLASH_MAIN_APP_START_ADDRESS);
	Jump();	//Переход на основную программу
}
//----------------------------------------------------------------------------//
void Bootloader_CheckUpdateFlag(void){	//Проверка флага обновления
	_SYSLOG_("Check Update Flag");
	
	char updKey[BOOTLOADER_UPDATE_DEVICE_FLAG_SIZE_MAX];
	memset(updKey, 0, sizeof(updKey));
	
	memory.readTwoCopyesWithCRC(MEMORY_UPDATE_DEVICE_FLAG, &updKey, sizeof(updKey));	//Читаем из памяти флаг
	_SYSLOG_("Read Update Flag From Memory: %s", updKey);
	
	if(0 != strncmp(UPDATE_DEVICE_KEY, updKey, BOOTLOADER_UPDATE_DEVICE_FLAG_SIZE_MAX)){	//Если ключ не совпал
		_SYSLOG_("Update Key Does Not Match!");
		Bootloader_JumpToApplication();		//То переходим на основную программу
	}
}
//----------------------------------------------------------------------------//
void Bootloader_CheckMD5FirmwareInExtrnalMemory(void){	//Проверка MD5 прошивки во внешней памяти
	_SYSLOG_("Check MD5 Firmware In External Memory");
	
	char md5_save[MD5_STR_LEN], md5_calc[MD5_STR_LEN];
	u8 tmpBuf[MEMORY_UPDATE_DATA_BLOCK_SIZE];
	
	memory.readTwoCopyesWithCRC(MEMORY_UPDATE_DEVICE_MD5, &md5_save, sizeof(md5_save));
	
	_SYSLOG_("Save MD5: %s", md5_save);
	
	MD5Init();
	for(u16 i = 0; i < MEMORY_UPDATE_NUM_PAGE; i++){
		memory.readBytes((i + MEMORY_UPDATE_DEVICE_BUFF_START)*memory.getPageSize(), tmpBuf, MEMORY_UPDATE_DATA_BLOCK_SIZE);
		MD5Update((char*)tmpBuf, sizeof(tmpBuf));
	}
	MD5Final(md5_calc);
	char MD5_Str[MD5_STR_LEN*2];
	MD5_ToHexStr(md5_calc, MD5_Str);
	_SYSLOG_("MD5: %s", MD5_Str);
	
	if((memcmp(md5_save, md5_calc, sizeof(md5_calc)))){	//Если контрольная сумма не совпадает
		_SYSLOG_("MD5 Does Not Match!");
		char str[BOOTLOADER_UPDATE_DEVICE_FLAG_SIZE_MAX];
		memset(str, 0, BOOTLOADER_UPDATE_DEVICE_FLAG_SIZE_MAX);
		strncpy(str, UPDATE_DEVICE_ERROR_MD5_IN_EXTERNAL_MEMORY, BOOTLOADER_UPDATE_DEVICE_FLAG_SIZE_MAX);
		memory.writeTwoCopyesWithCRC(MEMORY_UPDATE_DEVICE_FLAG, str, BOOTLOADER_UPDATE_DEVICE_FLAG_SIZE_MAX);	//Записываем флаг ошибки
		_SYSLOG_("Write Update Flag: %s", str);
		Bootloader_JumpToApplication();	//То переходим на основную программу
	}
}
//----------------------------------------------------------------------------//
void Bootloader_EraseMainApplication(void){	//Стереть из Flash основную программу
	_SYSLOG_("Erase Main Applacation From Flash");
	
	u32 address = 0;
	Qflash_Init();
	lcd.clear();
    lcd.showText("SECTOR");
	lcd.show_val_pos(ERASE, 7);
	for(u8 i = 0; i < BOOTLOADER_FLASH_MAIN_APP_NUM_SECTOR; i++){
		lcd.show_long(i, false, false, 0);
		lcd.update();
		address = BOOTLOADER_FLASH_MAIN_APP_START_ADDRESS + i*BOOTLOADER_FLASH_SECTOR_SIZE;
		Qflash_Erase_Sector(address);
	}
	lcd.clear();
	lcd.update();
	
	_SYSLOG_("Starting Device Flashing!");
}
//----------------------------------------------------------------------------//

void Bootloader_ProgrammingPage(void){	//Программирование одной страницы Flash
	u8 buff_page[BOOTLOADER_FLASH_PAGE_SIZE];
	
	lcd.clear();
    lcd.showText("PAGE");
	lcd.show_val_pos(PROGRAMM, 7);
	
	for(u16 page = 0; page < BOOTLOADER_FLASH_MAIN_APP_NUM_PAGE; page++){
		lcd.show_long(page, false, false, 0);
		lcd.update();
		
		memory.readBytes((MEMORY_UPDATE_DEVICE_BUFF_START + page)*memory.getPageSize(), buff_page, BOOTLOADER_FLASH_PAGE_SIZE);
		if(FlashOperationSuccess != Qflash_Write(BOOTLOADER_FLASH_MAIN_APP_START_ADDRESS + page*BOOTLOADER_FLASH_PAGE_SIZE, buff_page, BOOTLOADER_FLASH_PAGE_SIZE)){
			_SYSLOG_("Programming Page:%d	Address: %.8X - %.8X	- ERROR!", page, (BOOTLOADER_FLASH_MAIN_APP_START_ADDRESS + page*BOOTLOADER_FLASH_PAGE_SIZE), (BOOTLOADER_FLASH_MAIN_APP_START_ADDRESS + (page + 1)*BOOTLOADER_FLASH_PAGE_SIZE - 1));
			while(1);
		}
		_SYSLOG_("Programming Page:%d	Address: %.8X - %.8X	- Success!", page, (BOOTLOADER_FLASH_MAIN_APP_START_ADDRESS + page*BOOTLOADER_FLASH_PAGE_SIZE), (BOOTLOADER_FLASH_MAIN_APP_START_ADDRESS + (page + 1)*BOOTLOADER_FLASH_PAGE_SIZE - 1));
		IWDG_Refresh();
	}
}
//----------------------------------------------------------------------------//
void Bootloader_CheckMD5FirmwareInFlash(void){	//Проверка MD5 прошивки во Flash MCU
	_SYSLOG_("Check MD5 Firmware In Flash MCU");
	
	char md5_save[MD5_STR_LEN], md5_calc[MD5_STR_LEN];
	char MD5_Str[MD5_STR_LEN*2 + 1];
	
	memory.readTwoCopyesWithCRC(MEMORY_UPDATE_DEVICE_MD5, &md5_save, sizeof(md5_save));
	MD5_ToHexStr(md5_save, MD5_Str);
	_SYSLOG_("Save MD5: %s", MD5_Str);
	
	
	MD5Init();
	MD5Update((char*)BOOTLOADER_FLASH_MAIN_APP_START_ADDRESS, BOOTLOADER_FLASH_MAIN_APP_SIZE);
	MD5Final(md5_calc);
	
	MD5_ToHexStr(md5_calc, MD5_Str);
	_SYSLOG_("Calc MD5: %s", MD5_Str);
	
	if(!(memcmp(md5_save, md5_calc, sizeof(md5_calc)))){	//Если контрольная сумма совпадает
		char str[BOOTLOADER_UPDATE_DEVICE_FLAG_SIZE_MAX];
		memset(str, 0, BOOTLOADER_UPDATE_DEVICE_FLAG_SIZE_MAX);
		strncpy(str, UPDATE_DEVICE_SUCCESS, BOOTLOADER_UPDATE_DEVICE_FLAG_SIZE_MAX);
		memory.writeTwoCopyesWithCRC(MEMORY_UPDATE_DEVICE_FLAG, str, BOOTLOADER_UPDATE_DEVICE_FLAG_SIZE_MAX);	//Записываем флаг ошибки
		_SYSLOG_("Write Update Flag: %s", str);
		
		Bootloader_JumpToApplication();	//То переходим на основную программу
	}
	_SYSLOG_("MD5 Does Not Match!");
}
//----------------------------------------------------------------------------//
