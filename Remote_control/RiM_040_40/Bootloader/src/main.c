//----------------------------------------------------------------------------//
#include "utils.h"
#include "bootloader.h"
#include "lcd.h"
//----------------------------------------------------------------------------//
//uint32_t keyFlash __attribute__((at(0x01007FFC))) = 0xAABBCCDD;
int main(void){
	__disable_irq();
	SupplyEn();				//Включение питания
	Init_Core();
	__enable_irq();
	
	Bootloader_CheckUpdateFlag();					//Проверяем флаг обновления
	Bootloader_CheckMD5FirmwareInExtrnalMemory();	//Проверка MD5 прошивки во внешней памяти
	Bootloader_EraseMainApplication();				//Стереть из Flash основную программу
	Bootloader_ProgrammingPage();					//Программирование
	Bootloader_CheckMD5FirmwareInFlash();	//Проверка MD5 прошивки во Flash MCU
	NVIC_SystemReset();	//Если котрольная сумма не совпала, перезагружаемся
}
//----------------------------------------------------------------------------//
