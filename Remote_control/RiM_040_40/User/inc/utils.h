//----------------------------------------------------------------------------//
#include <stdio.h>
#include "common_types.h"
#include "globals.h"
//----------------------------------------------------------------------------//
#define TIMEOUT_FORCED_REBOOT_DEVICE_MAX 60 // 60 сек
//----------------------------------------------------------------------------//
extern char tmpBuf[100];
//----------------------------------------------------------------------------//
#define _SYSLOG_START_(...)                                                    \
	do {                                                                        \
		snprintf(tmpBuf, sizeof(tmpBuf), __VA_ARGS__);                           \
		SysLogSend(tmpBuf);                                                      \
	} while (0)
//----------------------------------------------------------------------------//
void SupplyEn(void);                   // Включение питания
void SupplyDis(void);                  // Выключение питания
void Clear_Buffer(u8 *buff, u16 size); // Заполнить буфер нулями
//----------------------------------------------------------------------------//
void remap_vector_table(void);
void SystemClock_Config(void); // Настройка тактирования
void Init_Core(void);          // Инициализация основных модулей
void ReadSystemInfoAndSettings(
    void);               // Считываем системную инфромацию и настройки
void IWDG_Refresh(void); // Сброс IWDG
void Threads_Init(void); // Инициализация потоков
result_t Memory_Write_And_Update_Status(u32 page, void *const buffer_ptr,
                                        u8 len);
u16 adc_init_and_get_res(void); // Получить значение АЦП
//----------------------------------------------------------------------------//
typedef enum {
	REQ_DATA = 1,
	CONNECT_RELAY = 2,
	REQ_DIAGNOSIS = 3,
	SET_DIAGNOSIS = 4,
} request_t; // “ип запроса
//----------------------------------------------------------------------------//
typedef struct {
	//	request_t requestType;		//Тип запроса
	u8 elementStart; // Начальный элемент для вычитывания
	u8 elementStop;  // Конечный элемент для вычитывания
} bleData_t;
extern bleData_t bleData;
//----------------------------------------------------------------------------//
