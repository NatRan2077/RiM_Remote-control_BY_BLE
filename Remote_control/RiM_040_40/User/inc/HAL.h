//----------------------------------------------------------------------------//
#define FW_VERSION_MAJOR 1 // меняется при изменении схемотехники
#define FW_VERSION_MINOR 1 // меняется при изменении функционала
#define FW_VERSION_PATCH 4 // текущая версия
#define FW_VERSION_PRERELEASE                                                  \
	"" // флаги (-a(alpha), -b(beta), -rc(release condidate))
#define FW_VERSION_BUILD 1 // сборка

#define STRINGIFY(x) #x
#define EXPAND_AND_STRINGIFY(x) STRINGIFY(x)

#define FW_VERSION                                                             \
	EXPAND_AND_STRINGIFY(FW_VERSION_MAJOR)                                      \
	"." EXPAND_AND_STRINGIFY(FW_VERSION_MINOR) "." EXPAND_AND_STRINGIFY(        \
	    FW_VERSION_PATCH) FW_VERSION_PRERELEASE                                 \
	    "+" EXPAND_AND_STRINGIFY(FW_VERSION_BUILD)
//----------------------------------------------------------------------------//
// #define DEBUG
#define IWDG_ON 1
//----------------------------------------------------------------------------//
#ifdef DEBUG
#else
#define RELEASE
#endif
//----------------------------------------------------------------------------//
#define TIME_DISABLE 60000 // 60 сек
#define TIME_ERROR 10000   // 10 сек
//----------------------------------------------------------------------------//
#define TIMEOUT_RADIO_ANSWER 3000 // Таймаут ответа на запрос
//----------------------------------------------------------------------------//
#define TIME_WAIT_TO_ANSWER 5000
//----------------------------------------------------------------------------//
#define TIME_REQ_DLMS_STATUS 30000 // Таймаут запроса статусов
//----------------------------------------------------------------------------//
#ifdef DEBUG
#define WAIT_TIME 200 // Время смены экранов
#else
#define WAIT_TIME 2000 // Время смены экранов
#endif
#define DELAY_TIME 1000 // Задержка между отображением номера и показаниями
//----------------------------------------------------------------------------//
// Напряжение батарейки
#define BAT_LOWER_VOLT_LIM 1900 // При меньшем напряжении не работает кнопка
#define BAT_LOW_VOLT_LIM 2200 // Отображаем значек батарейки
#define BAT_MID_VOLT_LIM 2500 // Отображаем значек батарейки
//----------------------------------------------------------------------------//
// FLASH Место хранения заводского номера
#define I_EE_NUMPULT (u32)0x3FC00 // Предпоследняя страница
#define MEM_BLOCK_SIZE 512        // Размер страницы
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
// BUTTON
#define BTN_GPIO GPIOB
#define BTN_1_Pin GPIO_PIN_1
#define BTN_2_Pin GPIO_PIN_2
//----------------------------------------------------------------------------//
#define IS_BUTTON_1_PRESSED (BTN_1_Pin == (BTN_GPIO->PID & BTN_1_Pin))
#define IS_BUTTON_1_UNPRESSED (!IS_BUTTON_1_PRESSED)
#define IS_BUTTON_2_PRESSED (BTN_2_Pin == (BTN_GPIO->PID & BTN_2_Pin))
#define IS_BUTTON_2_UNPRESSED (!IS_BUTTON_2_PRESSED)
//----------------------------------------------------------------------------//
