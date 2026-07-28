//----------------------------------------------------------------------------//
#define BOOTLOADER_FLASH_BOOTLOADER_START_ADDRESS                              \
	(uint32_t)0x01000000 // Адрес начала основной прошивки (Загрузчик до 32 КБ)
#define BOOTLOADER_FLASH_BOOTLOADER_END_ADDRESS (uint32_t)0x01007FFF //
#define BOOTLOADER_FLASH_BOOTLOADER_SIZE                                       \
	(BOOTLOADER_FLASH_BOOTLOADER_END_ADDRESS -                                  \
	 BOOTLOADER_FLASH_BOOTLOADER_START_ADDRESS + 1)

#define BOOTLOADER_FLASH_PAGE_SIZE 0x800
#define BOOTLOADER_FLASH_MAIN_APP_START_ADDRESS                                \
	(uint32_t)0x01008000 // Адрес начала основной прошивки (Загрузчик до 32 КБ)
#define BOOTLOADER_FLASH_MAIN_APP_END_ADDRESS (uint32_t)0x0103FFFF //
#define BOOTLOADER_FLASH_MAIN_APP_SIZE                                         \
	(BOOTLOADER_FLASH_MAIN_APP_END_ADDRESS -                                    \
	 BOOTLOADER_FLASH_MAIN_APP_START_ADDRESS + 1)
#define BOOTLOADER_FLASH_MAIN_APP_NUM_PAGE                                     \
	(BOOTLOADER_FLASH_MAIN_APP_SIZE / BOOTLOADER_FLASH_PAGE_SIZE)
//----------------------------------------------------------------------------//
#define BOOTLOADER_UPDATE_DEVICE_KEY "Update Device Flag Set"
#define BOOTLOADER_UPDATE_DEVICE_FLAG_SIZE_MAX 100
//----------------------------------------------------------------------------//
