//----------------------------------------------------------------------------//
#ifndef INCMESSAGES_H
#define INCMESSAGES_H
//----------------------------------------------------------------------------//
#include <stdbool.h>
#include "common_types.h"
#include <string.h>
//----------------------------------------------------------------------------//
typedef enum {
	TIMER_GENERAL, // Нельзя сбрасывать
	TIMER_TX_TIMEOUT,
	TIMER_RX_TIMEOUT,
	TIMER_RF_ANSWER,
	TIMER_RX_PREAMBLE,
	TIMER_WAIT_BETWEEN_REQ,
	TIMER_UART,
	TIMER_LOG,
	TIMER_BTN,
	TIMER_CLEAR_MSG_BTN,
	TIMER_RELAY_CONNECT,
	TIMER_DIAGNOSIS_REQ,
	TIMER_DELAY,
	TIMER_ERROR,
	TIMER_CHILD_BLK,
	TIMER_CHILD,
	TIMER_STATUS,
	TIMER_RSSI,
	TIMER_DISABLE,
	TIMER_DELAY_REQUEST,
	TIMER_BACKLIGHT,
	TIMER_USB_RX_PKT,
	TIMER_DELAY_LOG,
	TIMER_SUSPENSION_LOG,
	TIMER_LCD,
	TIMER_ADC,
	TIMER_SYSTEM,
	TIMER_FORCED_REBOOT_DEVICE,
	TIMER_BLE,
	TIMER_SERVICE_MODE_ON,
	TIMER_LOG_ON,
	MAX_TIMERS,
} Timer;
//----------------------------------------------------------------------------//
typedef enum {
	MSG_BTN_1_PUSH_SHORT,
	MSG_BTN_1_PUSH_MID,
	MSG_BTN_2_PUSH_SHORT,
	MSG_BTN_2_PUSH_MID,
	MSG_BTN_1_CLAMPED, // Сообщение о зажатой конпки при старте
	MSG_BTN_2_CLAMPED, // Сообщение о зажатой конпки при старте
	MSG_BTN_1_2_COMBINATION_MID,
	MSG_BLE_START,                 // Сообщение для запуска модуля BLE
	MSG_BLE_CONNECT_SUCCESS,       // Успешнео подключено
	MSG_BLE_CONNECT_ERROR_PINCODE, // Не верный пинкод
	MSG_BLE_CONNECT_NOT_FOUND,     // Устройство не найдено
	MSG_PULT_CONNECT_TO_METER,
	MSG_REQWEST,
	MSG_RADIO_ANSWER_OK,

	MSG_REQWEST_RELAY_CONNECT,
	MSG_REQWEST_DIAGNOSIS,
	MSG_SET_DIAGNOSIS,
	MSG_RECEIVE_SET_DIAGNOSIS,
	MSG_RECEIVE_DIAGNOSIS,
	MSG_RELAY_CONNECT_SUCCESS,
	MSG_RELAY_CONNECT_ERROR,

	MSG_USART_RX,
	MSG_SWITCH_BLOCK_STATUS_THREAD,
	MSG_BLOCK_STATUS_THREAD,
	MSG_UNBLOCK_STATUS,

	MSG_NO_ANSWER,

	MSG_DATA_UPDATE,
	MAX_MSG,
} Message_t;
//----------------------------------------------------------------------------//
#define PT_DELAY_MS(timer, ms)                                                 \
	do {                                                                        \
		tim.reset(timer);                                                        \
		PT_WAIT_WHILE(pt, tim.get(timer) < ms);                                  \
	} while (0)
//----------------------------------------------------------------------------//
#define WAIT_EVENT_TIME(timer, time)                                           \
	tim.reset(timer);                                                           \
	while (tim.get(timer) < time)
//----------------------------------------------------------------------------//
struct Timer_driver { // Timer_driver
	u32 (*get)(Timer timer);
	void (*reset)(Timer timer);
	void (*init)();
	void (*set)(Timer timer, u32 val);
};
extern const struct Timer_driver tim;
//----------------------------------------------------------------------------//
struct Msg_driver { // Структура дравера MSG.
	void (*init)();
	u8 (*set)(const Message_t imsg, void *iparamPtr, u16 iparamSize);
	bool (*getVal)(const Message_t imsg);
	void *(*getPtr)(const u8 imsg);
	u16 (*getSize)(const u8 imsg);
};
extern const struct Msg_driver msg;
//----------------------------------------------------------------------------//
#endif
