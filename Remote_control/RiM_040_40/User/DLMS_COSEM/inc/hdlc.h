//----------------------------------------------------------------------------//
#ifndef INCHDLC_H
#define INCHDLC_H
//----------------------------------------------------------------------------//
#include <stdbool.h>
#include "common_types.h"
#include "cosem.h"
//----------------------------------------------------------------------------//
typedef enum {
	ACCESS_RESULT_SUCCESS = 0,
	ACCESS_RESULT_HARDWARE_FAULT = 1,
	ACCESS_RESULT_TEMPORARY_FAILURE = 2,
	ACCESS_RESULT_READ_WRITE_DENIED = 3,
	ACCESS_RESULT_OBJECT_UNDEFINED = 4,
	ACCESS_RESULT_OBJECT_CLASS_INCONSISTENT = 9,
	ACCESS_RESULT_OBJECT_UNAVAILABLE = 11,
	ACCESS_RESULT_TYPE_UNMATCHED = 12,
	ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED = 13,
	ACCESS_RESULT_DATA_BLOCK_UNAVAILABLE = 14,
	ACCESS_RESULT_LONG_GET_ABORTED = 15,
	ACCESS_RESULT_NO_LONG_GET_IN_PROGRESS = 16,
	ACCESS_RESULT_LONG_SET_ABORTED = 17,
	ACCESS_RESULT_NO_LONG_SET_IN_PROGRESS = 18,
	ACCESS_RESULT_OTHER_REASON = 250
} accessResult_t;

typedef enum {
	APDU_RLRQ = 98,
	APDU_GET_REQUEST = 192,     // C0
	APDU_SET_REQUEST = 193,     // C1
	APDU_ACTION_REQUEST = 195,  // C3
	APDU_GET_RESPONSE = 196,    // C4
	APDU_SET_RESPONSE = 197,    // C5
	APDU_ACTION_RESPONSE = 199, // C7
	APDU_EXCEPTION_RESPONSE = 216
} APDU_t;

typedef enum {
	TYPE_REQUEST_NORMAL = 1,
	TYPE_REQUEST_BLOCK = 2,
} TypeRequest_t;

typedef enum {
	ASSOC_MFR = 0x01, // «аводской доступ(только с калибровочной перемычкой)
	ASSOC_USB = 0x04, // USB
	ASSOC_SVC = 0x08, // —ервисный доступ(только после ввода сервисного парол€)
	ASSOC_MFR_USB = ASSOC_MFR | ASSOC_USB,
	ASSOC_MFR_SVC = ASSOC_MFR | ASSOC_SVC,
	ASSOC_USB_SVC = ASSOC_USB | ASSOC_SVC,
	ASSOC_MFR_USB_SVC = ASSOC_MFR | ASSOC_USB | ASSOC_SVC,
} assoc_t;

typedef enum {
	INTERFACE_USB,
	INTERFACE_SVC,
	INTERFACE_MFR,
} interface_t;

typedef struct {
	accessResult_t result;
	APDU_t APDU;
	TypeRequest_t typeRequest;
	assoc_t assoc;
	interface_t interface;
	u8 invokeID;
	u8 objID;
	u8 attribute;
	u8 *access_parameters;
	u16 len_parameters;
	u32 block_number;
	u32 start_addr;
	u8 maska_list_obj[100]; // u8 maska_list_obj[OBIS_LIST_ID_SIZE];
	u8 access_selector;     // тип селективного доступа
	u32 sa_from_range;
	u32 sa_to_range;
	u32 sa_from_entry;
	u16 sa_rows;
	u16 sa_rest;
	u16 counter;
	u8 *buff;
} cosemLink_t;

typedef enum {
	DEV_ID_IVK = 0x01,
	DEV_ID_IPUE = 0x02,
	DEV_ID_MC = 0x03,
} devID_t;

typedef enum {
	CHAN_ID_LOGIC_1 = 0x01,
	CHAN_ID_LOGIC_2 = 0x02,
	CHAN_ID_LOGIC_3 = 0x03,
	CHAN_ID_LOGIC_4 = 0x04,
	CHAN_ID_LOGIC_5 = 0x05,
	CHAN_ID_LOGIC_6 = 0x06,
	CHAN_ID_UART = 0x07,
	CHAN_ID_OPTO = 0x08,
	CHAN_ID_PULT = 0x09,
	CHAN_ID_PUSH = 0x0A,
	CHAN_ID_INT_1 = 0x0B,
	CHAN_ID_INT_2 = 0x0C,
	CHAN_ID_INT_3 = 0x0D,
	CHAN_ID_INT_4 = 0x0E,
} chanID_t;

void Wrap_Pkt_From_IPUE(u8 *buff, u16 len, u8 *tmpBuff, u16 *tmpCnt,
                        chanID_t srcChan); // ќборачиваем пакет в обертку HDLC
                                           // дл€ передачи его счетчику
bool Get_Chan_and_Dev(
    u8 *buff, u16 len, chanID_t *destChan, devID_t *destDev, chanID_t *srcChan,
    devID_t *srcDev); // ѕолучить канал и id устройства из пакета
//----------------------------------------------------------------------------//
#endif // INCHDLC_H
