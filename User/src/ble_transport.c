/**
 * \file            ble_transport.c
 * \brief           Транспортный уровень обмена по BLE (API - ble_transport.h)
 */
#include "ble_transport.h"
#include "messages.h"
#include "globals.h"
#include "log.h"
#include <string.h>
//----------------------------------------------------------------------------//
//  Буферы транспорта. Доступ извне - только через функции модуля.
//----------------------------------------------------------------------------//
static u8 txBuff[BLE_TX_BUFF_SIZE];
static u8 txCnt = 0;

static u8 rxBuff[BLE_RX_BUFF_SIZE];
static u8 rxCnt = 0;
//----------------------------------------------------------------------------//
//  Точка входа в стек (app_rdtsc.c): упаковывает данные в керн-сообщение
//  RDTSC_DATA_TX_REQ для задачи RDTS-клиента
//----------------------------------------------------------------------------//
void user_send_ble_data(const uint8_t *data, uint16_t length);
//----------------------------------------------------------------------------//
//  Постановка кадра на отправку. Физическая передача произойдёт
//  в thread_BLE после очередного rwip_schedule().
//----------------------------------------------------------------------------//
bleTxResult_t ble_transport_send(const u8 *data, u8 len) {
	if ((NULL == data) || (0 == len) || (len > BLE_TX_BUFF_SIZE)) {
		_SYSLOG_("[BLE TX] bad frame len=%d", len);
		return BLE_TX_ERR_SIZE;
	}
	memcpy(txBuff, data, len);
	txCnt = len;
	msg.set(MSG_REQWEST, NULL, 0);
	return BLE_TX_OK;
}
//----------------------------------------------------------------------------//
//  Вызывается из thread_BLE на каждой итерации: если есть запрос
//  на передачу и соединение активно - отдаём кадр стеку
//----------------------------------------------------------------------------//
void ble_transport_process_tx(void) {
	if (!msg.getVal(MSG_REQWEST)) {
		return;
	}
	if (BLE_STATE_CONNECT != SystemInfo.BLE_State) {
		return;
	}

	_SYSLOG_DATA_("[Data BLE TX]", txBuff, txCnt);
	user_send_ble_data(txBuff, txCnt);
}
//----------------------------------------------------------------------------//
//  Вызывается из обработчика приёма стека (rdtsc_data_rx_ind_handler).
//  Фрагменты накапливаются до тех пор, пока потребитель не разберёт
//  полный кадр и не очистит буфер.
//----------------------------------------------------------------------------//
void ble_transport_on_rx(const u8 *data, u16 len) {
	if ((NULL == data) || (0 == len)) {
		return;
	}

	if (((u16)rxCnt + len) > BLE_RX_BUFF_SIZE) {
		// Переполнение: старое содержимое отбрасываем, но не молча -
		// это признак рассинхронизации с блоком, фиксируем в логе
		_SYSLOG_("[BLE RX] overflow: drop %d bytes", rxCnt);
		rxCnt = 0;
		if (len > BLE_RX_BUFF_SIZE) {
			return; // фрагмент больше буфера
		}
	}
	memcpy(&rxBuff[rxCnt], data, len);
	rxCnt += (u8)len;

	msg.set(MSG_RADIO_ANSWER_OK, NULL, 0);
}
//----------------------------------------------------------------------------//
//  Доступ к накопленным данным (без копирования)
//----------------------------------------------------------------------------//
const u8 *ble_transport_rx_data(u8 *len) {
	if (len) {
		*len = rxCnt;
	}
	return rxBuff;
}
//----------------------------------------------------------------------------//
void ble_transport_rx_clear(void) {
	rxCnt = 0;
}
//----------------------------------------------------------------------------//
