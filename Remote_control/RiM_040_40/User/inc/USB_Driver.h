//----------------------------------------------------------------------------//
#include <stdbool.h>
#include "common_types.h"
//----------------------------------------------------------------------------//
struct USB_Driver_t {
	void (*uartInit)(void);                    // Инициализация UART
	void (*configDMA)(u32 addr_buff, u16 cnt); // Настраиваем DMA для передачи
	bool (*isPktNotReadFlag)(void);            // Есть не прочитанный пакет
	void (*clearPktNotReadFlag)(void); // Сбросить флаг не прочитанного пакета
	bool (*getTrasferFlag)(void);      // Получить флаг передачи буфера
	void (*clearTrasferFlag)(void);    // Сбросить флаг передачи буфера
	void (*setPendingDataFlag)(
	    void); // Установить флаг ожидающих обработки данных
	bool (*isPendingDataFlag)(void);      // Есть ожидающие обработки данные
	bool (*getRxByteFlag)(void);          // Получить флаг принятого байта
	void (*clearRxByteFlag)(void);        // Сбросить флаг принятого байта
	bool (*getTransmitOkFlag)(void);      // Получить флаг успешной передачи
	void (*clearTransmitOkFlag)(void);    // Сбросить флаг успешной передачи
	bool (*getTransmitErrorFlag)(void);   // Получить флаг ошибки при передаче
	void (*clearTransmitErrorFlag)(void); // Сбросить флаг ошибки при передаче
	void (*startSendingData)(void);       // Начать отправку данных
};
extern const struct USB_Driver_t USB_Driver;
//----------------------------------------------------------------------------//
