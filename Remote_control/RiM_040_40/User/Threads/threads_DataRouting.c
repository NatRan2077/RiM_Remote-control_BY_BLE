//----------------------------------------------------------------------------//
#include "messages.h"
#include "pt.h"
#include "log.h"
#include "calc_fsc.h"
#include "usb.h"
#include "hdlc.h"
#include "cosem.h"
#include "globals.h"
#include "service_cmd.h"
//----------------------------------------------------------------------------//
pt_t dataRouting_pt;
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
#define HDLC_FLAG 0x7E
#define HDLC_MIN_LEN 9
#define HDLC_MAX_LEN (APDU_SIZE_MAX + 5)
#define HDLC_FRAME_FORMAT 0xA0
#define HDLC_FRAME_FORMAT_MASK 0xF0
#define HDLC_FRAME_LEN_MASK 0x7FF
#define HDLC_HCS_OFFSET 6
//----------------------------------------------------------------------------//
bool Check_HDLC_Frame_Preset(u8 *buff, u16 len) {
	if ((len < HDLC_MIN_LEN) || (len > HDLC_MAX_LEN) || (buff[0] != HDLC_FLAG) ||
	    (buff[len - 1] != HDLC_FLAG)) {
		return false;
	}
	if ((buff[1] & HDLC_FRAME_FORMAT_MASK) != HDLC_FRAME_FORMAT) {
		return false;
	}
	u16 frameLen = WORD(buff[1], buff[2]) & HDLC_FRAME_LEN_MASK;
	if ((frameLen + 2) != len) {
		return false;
	}

	u16 HCS, FCS;
	HCS = pppfcs16(PPPINITFCS16, (buff + 1), 5) ^ 0xffff;
	FCS = pppfcs16(PPPINITFCS16, (buff + 1), (frameLen - 2)) ^ 0xffff;

	if ((HCS != WORD(buff[HDLC_HCS_OFFSET + 1], buff[HDLC_HCS_OFFSET]) ||
	     (FCS != WORD(buff[frameLen], buff[frameLen - 1])))) {
		return false;
	}
	return true;
}
//----------------------------------------------------------------------------//
#define HDLC_DEST_NUMB_BYTE 3
#define HDLC_SRC_NUMB_BYTE 4

#define HDLC_CHAN_ID_MASK 0xF0
#define HDLC_CHAN_ID_OFFSET 4
#define HDLC_DEV_ID_MASK 0x0E
#define HDLC_DEV_ID_OFFSET 1
#define HDLC_ZERO_BIT_MASK 0x01

#define HDLC_FRAME_FORMAT_UI 0x13
#define HDLC_FRAME_FORMAT 0xA0
#define HDLC_FLAG 0x7E
//----------------------------------------------------------------------------//
bool Get_Chan_and_Dev(
    u8 *buff, u16 len, chanID_t *destChanID, devID_t *destDevID,
    chanID_t *srcChanID,
    devID_t *srcDevID) { // Получить канал и id устройства из пакета
	if ((buff[HDLC_DEST_NUMB_BYTE] & HDLC_ZERO_BIT_MASK) != HDLC_ZERO_BIT_MASK) {
		return false; // Проверяем однобайтовую адресацию
	}
	if ((buff[HDLC_SRC_NUMB_BYTE] & HDLC_ZERO_BIT_MASK) != HDLC_ZERO_BIT_MASK) {
		return false; // Проверяем однобайтовую адресацию
	}
	*destChanID = (chanID_t)((buff[HDLC_DEST_NUMB_BYTE] & HDLC_CHAN_ID_MASK) >>
	                         HDLC_CHAN_ID_OFFSET);
	*destDevID = (devID_t)((buff[HDLC_DEST_NUMB_BYTE] & HDLC_DEV_ID_MASK) >>
	                       HDLC_DEV_ID_OFFSET);
	*srcChanID = (chanID_t)((buff[HDLC_SRC_NUMB_BYTE] & HDLC_CHAN_ID_MASK) >>
	                        HDLC_CHAN_ID_OFFSET);
	*srcDevID = (devID_t)((buff[HDLC_SRC_NUMB_BYTE] & HDLC_DEV_ID_MASK) >>
	                      HDLC_DEV_ID_OFFSET);
	if ((*destChanID == 0) || (*srcChanID == 0)) {
		return false; // ChanId = 0 запрещен
	}
	return true;
}
//----------------------------------------------------------------------------//
bool HDLC_Wrapper_Preset(cosemLink_t *cosemLink, chanID_t destChan,
                         devID_t destDev, chanID_t srcChan, devID_t srcDev) {
	if ((cosemLink->counter + 11) < 2048) {
		if (0 == cosemLink->counter) {
			cosemLink->counter = cosemLink->counter + 7;
			cosemLink->buff[0] = HDLC_FLAG;
			cosemLink->buff[1] = BYTE_1(cosemLink->counter) | HDLC_FRAME_FORMAT;
			cosemLink->buff[2] = BYTE_0(cosemLink->counter);
			cosemLink->buff[HDLC_DEST_NUMB_BYTE] =
			    (srcChan << HDLC_CHAN_ID_OFFSET) | (srcDev << HDLC_DEV_ID_OFFSET) |
			    HDLC_ZERO_BIT_MASK;
			cosemLink->buff[HDLC_SRC_NUMB_BYTE] =
			    (destChan << HDLC_CHAN_ID_OFFSET) |
			    (destDev << HDLC_DEV_ID_OFFSET) | HDLC_ZERO_BIT_MASK;
			cosemLink->buff[5] = HDLC_FRAME_FORMAT_UI;
			u16 FCS = pppfcs16(PPPINITFCS16, (cosemLink->buff + 1),
			                   (cosemLink->counter - 2)) ^
			          0xffff;
			cosemLink->buff[6] = BYTE_0(FCS);
			cosemLink->buff[7] = BYTE_1(FCS);
			cosemLink->buff[8] = HDLC_FLAG;
			cosemLink->counter = 9;
			return true;
		} else {
			memmove((cosemLink->buff + 8), cosemLink->buff, cosemLink->counter);
			cosemLink->counter = cosemLink->counter + 9;
			cosemLink->buff[0] = HDLC_FLAG;
			cosemLink->buff[1] = BYTE_1(cosemLink->counter) | HDLC_FRAME_FORMAT;
			cosemLink->buff[2] = BYTE_0(cosemLink->counter);
			cosemLink->buff[HDLC_DEST_NUMB_BYTE] =
			    (srcChan << HDLC_CHAN_ID_OFFSET) | (srcDev << HDLC_DEV_ID_OFFSET) |
			    HDLC_ZERO_BIT_MASK;
			cosemLink->buff[HDLC_SRC_NUMB_BYTE] =
			    (destChan << HDLC_CHAN_ID_OFFSET) |
			    (destDev << HDLC_DEV_ID_OFFSET) | HDLC_ZERO_BIT_MASK;
			cosemLink->buff[5] = HDLC_FRAME_FORMAT_UI;
			u16 HCS = pppfcs16(PPPINITFCS16, (cosemLink->buff + 1), 5) ^ 0xffff;
			cosemLink->buff[6] = BYTE_0(HCS);
			cosemLink->buff[7] = BYTE_1(HCS);
			u16 FCS = pppfcs16(PPPINITFCS16, (cosemLink->buff + 1),
			                   (cosemLink->counter - 2)) ^
			          0xffff;
			cosemLink->buff[cosemLink->counter - 1] = BYTE_0(FCS);
			cosemLink->buff[cosemLink->counter] = BYTE_1(FCS);
			cosemLink->buff[cosemLink->counter + 1] = HDLC_FLAG;
			cosemLink->counter += 2;
			return true;
		}
	}
	return false;
}
//----------------------------------------------------------------------------//
bool CosemProcessing(u8 *buff, u16 len, cosemLink_t *cosemLink);
//----------------------------------------------------------------------------//
PT_THREAD(thread_dataRouting(pt_t *pt)) {
	static devID_t srcDev, destDev;
	static chanID_t srcChan, destChan;
	static cosemLink_t cosemLinkUSB;
	PT_BEGIN(pt);
	while (1) {
		if (usb.isPendingDataFlag()) { // Данные пришли по USB
			// Сервисные текстовые команды (PIN=..., PIN?) - до HDLC/DLMS
			if (ServiceCmd_Process(usb.getBufPendingData(),
			                       usb.getCntPendingData())) {
				// команда обработана, ответ уже поставлен в TX
			} else if (Check_HDLC_Frame_Preset(usb.getBufPendingData(),
			                                   usb.getCntPendingData())) {
				if (Get_Chan_and_Dev(
				        usb.getBufPendingData(), usb.getCntPendingData(), &destChan,
				        &destDev, &srcChan, &srcDev)) { // если корректно получены
					if (DEV_ID_MC == destDev) {          // Пришел запрос для МС
						tim.reset(TIMER_SUSPENSION_LOG);
						tim.reset(TIMER_DISABLE);
						u8 tmpBuff[2048];
						cosemLinkUSB.buff = tmpBuff;
						cosemLinkUSB.interface = INTERFACE_USB;
						if (SystemInfo.flag.serviceMode) {
							cosemLinkUSB.interface = INTERFACE_SVC;
						}
						if (MODE_ON == SystemInfo.status.manufactoryMode) {
							cosemLinkUSB.interface = INTERFACE_MFR;
						}
						memset(&tmpBuff, 0, sizeof(tmpBuff));
						if (CosemProcessing((usb.getBufPendingData() + 8),
						                    (usb.getCntPendingData() - 11),
						                    &cosemLinkUSB)) {
							if (HDLC_Wrapper_Preset(&cosemLinkUSB, destChan, destDev,
							                        srcChan, srcDev)) {
								usb.writeToBufTx(cosemLinkUSB.buff,
								                 cosemLinkUSB.counter);
								usb.setTransferFlag();
							}
						}
					}
				}
			}
			usb.clearPendingDataFlag();
		}
		PT_YIELD(pt);
	}
	PT_END(pt);
}
//----------------------------------------------------------------------------//
