//----------------------------------------------------------------------------//
#include "utils.h"
#include "log.h"
#include "cmt453x_rcc.h"
#include "HAL.h"
#include "threads.h"
#include "memory.h"
//----------------------------------------------------------------------------//
int main(void) {
	remap_vector_table();
	__disable_irq();

	SupplyEn(); //????????? ???????
	Init_Core();

	ReadSystemInfoAndSettings();
	Threads_Init();

	_SYSLOG_("START");
	/* DIAG: last reset cause (IWDG? software/hardfault?) */
	_SYSLOG_("RST por=%d pin=%d sft=%d iwdg=%d wwdg=%d",
	         (RCC_GetFlagStatus(RCC_CTRLSTS_FLAG_PORRSTF) == SET),
	         (RCC_GetFlagStatus(RCC_CTRLSTS_FLAG_PINRSTF) == SET),
	         (RCC_GetFlagStatus(RCC_CTRLSTS_FLAG_SFTRSTF) == SET),
	         (RCC_GetFlagStatus(RCC_CTRLSTS_FLAG_IWDGRSTF) == SET),
	         (RCC_GetFlagStatus(RCC_CTRLSTS_FLAG_WWDGRSTF) == SET));
	RCC_ClrFlag();

	__enable_irq();

	while (1) {
		thread_systemInfo(&systemInfo_pt);
		thread_lcd(&lcd_pt);
		thread_btn(&btn_pt);
		thread_main(&main_pt);
		thread_status(&status_pt);
		thread_usbUart(&usbUart_pt);
		thread_dataRouting(&dataRouting_pt);

		thread_BLE(&ble_pt);
		thread_rim_protocol(&rim_pt);

		thread_disabling(&disabling_pt);
		IWDG_Refresh();
	}
}
//----------------------------------------------------------------------------//
