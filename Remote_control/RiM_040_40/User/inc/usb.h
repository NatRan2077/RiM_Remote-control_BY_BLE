//----------------------------------------------------------------------------//
#include "common_types.h"
//----------------------------------------------------------------------------//
struct usb_t {
	void (*setTransferFlag)(void);   // Установить флаг для передачи буфера
	bool (*isPendingDataFlag)(void); // Есть ожидающие обработки данные
	void (*clearPendingDataFlag)(
	    void); // Сбросить флаг ожидающих обработки данных
	void (*writeToBufTx)(u8 *buff, u16 len); // Записать данные в буфер передачи
	u16 (*getCntPendingData)(void);          // Получить число байт  данных
	u8 *(*getBufPendingData)(void); // Получить указатель на буфер с данными
	void (*irqHandler)(void);       // Обработчик прерываний
	void (*DMAirqHandler)(void);    // Обработчик прерываний DMA
};
extern const struct usb_t usb;
//----------------------------------------------------------------------------//
