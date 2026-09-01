//----------------------------------------------------------------------------//
#define BOOTLOADER_FLASH_BOOTLOADER_START_ADDRESS	(uint32_t)0x01000000	//Адрес начала основной прошивки (Загрузчик до 32 КБ)
#define BOOTLOADER_FLASH_BOOTLOADER_END_ADDRESS		(uint32_t)0x01007FFF	//
#define BOOTLOADER_FLASH_BOOTLOADER_SIZE			(BOOTLOADER_FLASH_BOOTLOADER_END_ADDRESS - BOOTLOADER_FLASH_BOOTLOADER_START_ADDRESS + 1)

#define BOOTLOADER_FLASH_PAGE_SIZE				0x100
#define BOOTLOADER_FLASH_MAIN_APP_START_ADDRESS	(uint32_t)0x01008000	//Адрес начала основной прошивки (Загрузчик до 32 КБ)
#define BOOTLOADER_FLASH_MAIN_APP_END_ADDRESS	(uint32_t)0x0103FFFF	//
#define BOOTLOADER_FLASH_MAIN_APP_SIZE			(BOOTLOADER_FLASH_MAIN_APP_END_ADDRESS - BOOTLOADER_FLASH_MAIN_APP_START_ADDRESS + 1)
#define BOOTLOADER_FLASH_MAIN_APP_NUM_PAGE		(BOOTLOADER_FLASH_MAIN_APP_SIZE/BOOTLOADER_FLASH_PAGE_SIZE)

#define BOOTLOADER_FLASH_SECTOR_SIZE			0x1000
#define BOOTLOADER_FLASH_MAIN_APP_NUM_SECTOR	(BOOTLOADER_FLASH_MAIN_APP_SIZE/BOOTLOADER_FLASH_SECTOR_SIZE)
//----------------------------------------------------------------------------//
#define BOOTLOADER_UPDATE_DEVICE_KEY			"Update Device Flag Set"
#define BOOTLOADER_UPDATE_DEVICE_FLAG_SIZE_MAX	100
//----------------------------------------------------------------------------//
void Bootloader_CheckUpdateFlag(void);					//Проверка флага обновления
void Bootloader_CheckMD5FirmwareInExtrnalMemory(void);	//Проверка MD5 прошивки во внешней памяти
void Bootloader_EraseMainApplication(void);				//Стереть из Flash основную программу
void Bootloader_ProgrammingPage(void);					//Программирование одной страницы Flash
void Bootloader_CheckMD5FirmwareInFlash(void);			//Проверка MD5 прошивки во Flash MCU
//----------------------------------------------------------------------------//
