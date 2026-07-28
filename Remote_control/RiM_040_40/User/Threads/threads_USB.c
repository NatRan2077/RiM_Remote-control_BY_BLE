//----------------------------------------------------------------------------//
#include "USB_Driver.h"
#include "pt.h"
#include "log.h"
#include "messages.h"
#include "utils.h"
#include <string.h>
//----------------------------------------------------------------------------//
pt_t usbUart_pt;
//----------------------------------------------------------------------------//
#define USB_WAIT_TIME(ms)                                                      \
	logLastTimeOut = ms;                                                        \
	WAIT_EVENT_TIME(TIMER_DELAY_LOG, ms)
#define USB_WAS_TIMEOUT() (tim.get(TIMER_DELAY_LOG) > logLastTimeOut)
//----------------------------------------------------------------------------//
#define TIMEOUT_USB_TRANSMIT 500
//----------------------------------------------------------------------------//
PT_THREAD(thread_usbUart(pt_t *pt)) {
	static u8 stringLog[SYS_LOG_BUFF_SIZE];
	static u16 cntLog;
	static u32 logLastTimeOut;

	PT_BEGIN(pt);
	USB_Driver.uartInit();
	tim.set(TIMER_SUSPENSION_LOG, 5001);
	while (1) {
		if (USB_Driver.getTrasferFlag()) {
			USB_Driver.startSendingData();
			USB_WAIT_TIME(TIMEOUT_USB_TRANSMIT) {
				if (USB_Driver.getTransmitOkFlag()) {
					USB_Driver.clearTransmitOkFlag();
					break;
				}
				PT_YIELD(pt);
			}
			if (USB_WAS_TIMEOUT()) { // Был таймаут или ошибка
			}
			USB_Driver.clearTrasferFlag();
		}
		if (USB_Driver.isPktNotReadFlag()) {
			USB_Driver.clearPktNotReadFlag();
			USB_Driver.setPendingDataFlag();
		}
		PT_YIELD(pt);
		if (FIFO_COUNT(sysLog) &&
		    (SystemInfo.flag.logOn || SystemInfo.status.manufactoryMode) &&
		    (tim.get(TIMER_SUSPENSION_LOG) >
		     5000)) { // если буфер не пустой, то отправляем сообщение в уарт
			cntLog = FIFO_COUNT(sysLog);
			for (u16 i = 0; i < cntLog; i++) {
				stringLog[i] = FIFO_GET(sysLog);
				FIFO_DROP(sysLog);
			}
			USB_Driver.configDMA((u32)&stringLog, cntLog);

			USB_WAIT_TIME(1000) {
				if (USB_Driver.getTransmitOkFlag()) {
					USB_Driver.clearTransmitOkFlag();
					break;
				}
				PT_YIELD(pt);
			}
			if (USB_WAS_TIMEOUT() ||
			    USB_Driver.getTransmitErrorFlag()) { // Был таймаут или ошибка
				USB_Driver.clearTransmitErrorFlag();
				PT_RESTART(pt);
			}
		}
		PT_YIELD(pt);
	}
	PT_END(pt);
}
//----------------------------------------------------------------------------//
