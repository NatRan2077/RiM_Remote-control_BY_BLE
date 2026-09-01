//----------------------------------------------------------------------------//
// threads_BLE.c
// Поток обслуживания BLE-стека HopeRF.
//
// Задачи потока:
//   1. Отложенный запуск стека по сообщению MSG_BLE_START (из меню).
//   2. Регулярная прокрутка планировщика rwip_schedule() + hp_sleep().
//   3. Отложенное подключение к выбранному устройству (флаг g_need_connect).
//   4. Передача исходящих кадров через транспортный модуль ble_transport.
//----------------------------------------------------------------------------//
#include "threads.h"
#include "common_types.h"
#include "messages.h"
#include "log.h"
#include "cmt453x.h"
#include "hp_sleep.h"
#include "app_ble.h"
#include "utils.h"
#include "lcd.h"
#include "ble_transport.h"
//----------------------------------------------------------------------------//
pt_t ble_pt;
//----------------------------------------------------------------------------//
bleData_t bleData;
//----------------------------------------------------------------------------//
// Переменные для ручного подключения (объявлены в app_ble.c)
extern bool g_need_connect;
extern uint8_t g_target_addr[6];
extern uint8_t g_target_type;
//----------------------------------------------------------------------------//
//  Диагностический порт радиотракта: вывод сигналов RxEn/TxEn на PB0/PB3
//  (методика из appnote HopeRF, используется для отладки радиочасти).
//----------------------------------------------------------------------------//
#define AFEC_CLK_REG REG32(0x40011000UL) // блок AFEC
#define AFEC_DBG_PORT_EN (0x01UL << 28)  // тактирование отладочного порта
#define DIAG_PORT0_ENABLE 0x80u          // включить dbg port 0
#define DIAG0_SEL_RXTX_EN 0x03u          // DIAG0[6:0]: сигналы RxEn/TxEn

void turnOnRxEnTxEn(void) {
	if (ip_diagcntl_get() != (DIAG_PORT0_ENABLE | DIAG0_SEL_RXTX_EN)) {
		RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
		GPIO_InitType GPIO_InitStructure;
		GPIO_InitStruct(&GPIO_InitStructure);
		GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_MODE_AF_PP;
		GPIO_InitStructure.GPIO_Pull = GPIO_PULL_DOWN;
		GPIO_InitStructure.GPIO_Alternate = GPIO_AF6;
		GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

		AFEC_CLK_REG |= AFEC_DBG_PORT_EN; // открыть dbg-порт
		ip_diagcntl_set(DIAG_PORT0_ENABLE | DIAG0_SEL_RXTX_EN); // выбрать сигналы
	}
}
//----------------------------------------------------------------------------//
PT_THREAD(thread_BLE(pt_t *pt)) {
	u32 numMeter;
	PT_BEGIN(pt);
	SystemInfo.BLE_State = BLE_STATE_DISCONNECT;
	PT_WAIT_UNTIL(pt, msg.getVal(MSG_BLE_START));

	// Серийный номер прибора - в строку для имени/фильтра сканирования
	numMeter = SystemInfo.numMeter;
	for (s8 i = 7; i >= 0; i--) {
		SystemInfo.numMeterStr[i] = (numMeter % 10) + '0';
		numMeter /= 10;
	}
	SystemInfo.numMeterStr[8] = 0;

	app_ble_init();
	hp_ble_radio_power_set(TX_POWER_Pos6_DBM);
	turnOnRxEnTxEn();

	while (1) {
		rwip_schedule();
#if 0 /* DIAG: sleep TEMPORARILY DISABLED - testing the                        \
         falls-asleep-and-never-wakes hypothesis.                              \
         RESTORE to #if 1 after the test! */
			hp_sleep();
#endif

		// Ручное подключение: вызываем BLE-функции ПОСЛЕ rwip_schedule()
		// (нельзя вызывать из коллбека scan_data_handler внутри rwip_schedule)
		if (g_need_connect) {
			g_need_connect = false;
			// stop, а не delete: scan_actv_idx сбрасывается в 0xFF только
			// в обработчике остановки активности. Если удалить напрямую,
			// индекс останется старым и повторный скан не запустится.
			hp_ble_stop_scan();
			hp_ble_start_init(g_target_addr, g_target_type);
		}

		// Исходящие кадры: транспорт сам проверит MSG_REQWEST и соединение
		ble_transport_process_tx();

		PT_YIELD(pt);
	}
	PT_END(pt);
}
//----------------------------------------------------------------------------//
