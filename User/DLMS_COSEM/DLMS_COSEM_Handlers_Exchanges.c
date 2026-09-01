//----------------------------------------------------------------------------//
#include <string.h>
#include <inttypes.h>
#include "hdlc.h"
#include "dlms.h"
#include "log.h"
#include "globals.h"
#include "memory.h"
#include "utils.h"
#include "bootloader.h"
//----------------------------------------------------------------------------//
void Exchange_uint_8(void *data, u8 value) {
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_UINT8;
	buff[5] = value;

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 6;
}
//----------------------------------------------------------------------------//
void Exchange_int_8(void *data, u8 value) {
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_INT8;
	buff[5] = value;

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 6;
}
//----------------------------------------------------------------------------//
void Exchange_uint_16(void *data, u16 value) {
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_UINT16;
	buff[5] = BYTE_1(value);
	buff[6] = BYTE_0(value);

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 7;
}
//----------------------------------------------------------------------------//
void Exchange_int_16(void *data, u16 value) {
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_INT16;
	buff[5] = BYTE_1(value);
	buff[6] = BYTE_0(value);

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 7;
}
//----------------------------------------------------------------------------//
void Exchange_uint_32(void *data, u32 value) {
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_UINT32;
	buff[5] = BYTE_3(value);
	buff[6] = BYTE_2(value);
	buff[7] = BYTE_1(value);
	buff[8] = BYTE_0(value);

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 9;
}
//----------------------------------------------------------------------------//
void Exchange_uint_64(void *data, u64 value) {
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_UINT64;
	buff[5] = BYTE_7(value);
	buff[6] = BYTE_6(value);
	buff[7] = BYTE_5(value);
	buff[8] = BYTE_4(value);
	buff[9] = BYTE_3(value);
	buff[10] = BYTE_2(value);
	buff[11] = BYTE_1(value);
	buff[12] = BYTE_0(value);

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 13;
}
//----------------------------------------------------------------------------//
void Exchange_Device_Logical_Name(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Device_Logical_Name");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}

	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_VISIBLE_STRING;
	buff[5] = 16;

	char tmpStr[17];
	memset(tmpStr, 0x00, sizeof(tmpStr));
	u16 lenStr = snprintf(tmpStr, sizeof(tmpStr), "RIM04040%08" PRIu32,
	                      SystemInfo.serialNumber);
	memcpy((buff + 6), tmpStr, lenStr);

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 6 + lenStr;
}
//----------------------------------------------------------------------------//
void Exchange_Association(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Association");
	u16 k, x;
	u16 i;
	u16 j;
	u16 class_id;
	u8 obis[6];
	const struct attribute_desc_s *attrs;
	const struct method_desc_s *methods;

	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;
	u16 cnt = 0;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->start_addr = 0;

	if (0 == cosemLink->block_number) { // Текущий блок 0
		switch (cosemLink->access_selector) {
		case 0:
			break; // без селективного доступа
		case 1: {
			if ((TAG_NULLDATA != cosemLink->access_parameters[0]) ||
			    (1 != cosemLink->len_parameters)) {
				cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
				return;
			}
		} break;
		case 2: {
			x = cosemLink->len_parameters;
			if (TAG_ARRAY != cosemLink->access_parameters[0]) {
				cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
				return;
			}
			x -= 2;
			if ((0 == cosemLink->access_parameters[1]) && (0 == x)) {
				break;
			}
			for (u16 j = 0; j < cosemLink->access_parameters[1]; j++) {
				if (TAG_UINT16 != cosemLink->access_parameters[(j * 2) + 2]) {
					cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
					return;
				}
				x -= 3;
			}
			if (0 != x) {
				cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
				return;
			}
		} break;
		case 3: {
			x = cosemLink->len_parameters;
			if (TAG_ARRAY != cosemLink->access_parameters[0]) {
				cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
				return;
			}
			x -= 2;
			if ((0 == cosemLink->access_parameters[1]) && (0 == x)) {
				break;
			}
			for (u16 j = 0; j < cosemLink->access_parameters[1]; j++) {
				if (TAG_STRUCTURE != cosemLink->access_parameters[(j * 2) + 2]) {
					cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
					return;
				}
				if (2 != cosemLink->access_parameters[(j * 2) + 3]) {
					cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
					return;
				}
				if (TAG_UINT16 != cosemLink->access_parameters[(j * 2) + 4]) {
					cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
					return;
				}
				if (TAG_OCTET_STRING != cosemLink->access_parameters[j * 2 + 7]) {
					cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
					return;
				}
				if (6 != cosemLink->access_parameters[j * 2 + 8]) {
					cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
					return;
				}
				x -= 13;
			}
			if (0 != x) {
				cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
				return;
			}
		} break;
		case 4: {
			x = cosemLink->len_parameters;
			if (TAG_STRUCTURE != cosemLink->access_parameters[0]) {
				cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
				return;
			}
			if (2 != cosemLink->access_parameters[1]) {
				cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
				return;
			}
			if (TAG_UINT16 != cosemLink->access_parameters[2]) {
				cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
				return;
			}
			if (TAG_OCTET_STRING != cosemLink->access_parameters[5]) {
				cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
				return;
			}
			if (6 != cosemLink->access_parameters[6]) {
				cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
				return;
			}
			x -= 13;
			if (0 != x) {
				cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
				return;
			}
			break;
		}
		default: {
			cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
			return;
		}
		}
		for (i = 0; i < OBIS_ID_LIST_SIZE; i++) {
			cosemLink->maska_list_obj[i] =
			    0; // Сбрасывем маску прочитанных обхектов
		}
		cnt = 0;
		x = 0;

		for (i = 0;; i++) {
			if (0 == List_DLMS_Obj[i].attr_elements) {
				break; // Если нет аттрибутов, выходим из цикла
			}
			if ((List_DLMS_Obj[i].assoc & cosemLink->assoc) == 0) {
				continue; // Пропускаем объекты не доступные для текущей ассоциации
			}

			if (0 == cosemLink->access_selector) {
				cosemLink->maska_list_obj[i / 8] |= (1 << (i % 8));
				cnt += (21 + 7 * List_DLMS_Obj[i].attr_elements +
				        6 * List_DLMS_Obj[i].method_elements);
				if (CLASS_ID_PROFILE_GENERIC == List_DLMS_Obj[i].classID) {
					attrs = List_DLMS_Obj[i].attrs;
					switch (attrs[1].id & 0xC0) {
					case 0xC0:
						cnt += 5;
						break;
					case 0x80:
					case 0x40:
						cnt += 3;
						break;
					}
				}
				x++;
				continue;
			}
			if (1 == cosemLink->access_selector) {
				cosemLink->maska_list_obj[i / 8] |= (1 << (i % 8));
				cnt += 15;
				x++;
				continue;
			}
			if (2 == cosemLink->access_selector) {
				if (0 == cosemLink->access_parameters[1]) {
					continue;
				}
				for (j = 0; j < cosemLink->access_parameters[1]; j++) {
					class_id = cosemLink->access_parameters[j * 2 + 3];
					class_id <<= 8;
					class_id |= cosemLink->access_parameters[j * 2 + 4];
					if (class_id == List_DLMS_Obj[i].classID) {
						break;
					}
				}
				if (j != cosemLink->access_parameters[1]) {
					cosemLink->maska_list_obj[i / 8] |= (1 << (i % 8));
					cnt += 15;
					x++;
				}
				continue;
			}
			if (3 == cosemLink->access_selector) {
				if (0 == cosemLink->access_parameters[1]) {
					continue;
				}
				for (j = 0; j < cosemLink->access_parameters[1]; j++) {
					class_id = cosemLink->access_parameters[j * 13 + 5];
					class_id <<= 8;
					class_id |= cosemLink->access_parameters[j * 13 + 6];
					for (k = 0; k < 6; k++) {
						obis[k] = cosemLink->access_parameters[j * 13 + 9 + k];
					}
					if (class_id == List_DLMS_Obj[i].classID) {
						for (k = 0; k < 6; k++) {
							if (obis[k] != List_DLMS_Obj[i].OBIS[k]) {
								break;
							}
						}
						if (6 == k) {
							break;
						}
					}
				}
				if (j != cosemLink->access_parameters[1]) {
					cosemLink->maska_list_obj[i / 8] |= (1 << (i % 8));
					cnt += (21 + 7 * List_DLMS_Obj[i].attr_elements +
					        6 * List_DLMS_Obj[i].method_elements);
					if (CLASS_ID_PROFILE_GENERIC == List_DLMS_Obj[i].classID) {
						attrs = List_DLMS_Obj[i].attrs;
						switch (attrs[1].id & 0xC0) {
						case 0xC0:
							cnt += 5;
							break;
						case 0x80:
						case 0x40:
							cnt += 3;
							break;
						}
					}
					x++;
				}
				continue;
			}
			if (4 == cosemLink->access_selector) {
				class_id = cosemLink->access_parameters[3];
				class_id <<= 8;
				class_id |= cosemLink->access_parameters[4];
				for (k = 0; k < 6; k++) {
					obis[k] = cosemLink->access_parameters[7 + k];
				}
				if (class_id != List_DLMS_Obj[i].classID) {
					continue;
				}
				for (k = 0; k < 6; k++) {
					if (obis[k] != List_DLMS_Obj[i].OBIS[k]) {
						break;
					}
				}
				if (6 == k) {
					cosemLink->maska_list_obj[i / 8] |= (1 << (i % 8));
					cnt += (21 + 7 * List_DLMS_Obj[i].attr_elements +
					        6 * List_DLMS_Obj[i].method_elements);
					if (CLASS_ID_PROFILE_GENERIC == List_DLMS_Obj[i].classID) {
						attrs = List_DLMS_Obj[i].attrs;
						switch (attrs[1].id & 0xC0) {
						case 0xC0:
							cnt += 5;
							break;
						case 0x80:
						case 0x40:
							cnt += 3;
							break;
						}
					}
					x++;
				}
				continue;
			}
		}
		if (x < 128) {
			cnt += 2;
		} else {
			cnt += 4;
		}
		cosemLink->sa_rows = 0;
		cosemLink->sa_rest = x;

		x = 0;
		if ((cnt + 4) > (APDU_SIZE_MAX - x)) {
			buff[0] = APDU_GET_RESPONSE;
			buff[1] = GET_RESPONSE_WITH_DATABLOCK;
			buff[2] = cosemLink->invokeID;
			buff[3] = 0;
			cosemLink->block_number++;
			buff[4] = BYTE_3(cosemLink->block_number);
			buff[5] = BYTE_2(cosemLink->block_number);
			buff[6] = BYTE_1(cosemLink->block_number);
			buff[7] = BYTE_0(cosemLink->block_number);
			buff[8] = 0; // Data
			buff[9] = 0x82;
			buff[10] = 0;
			buff[11] = 0;
			cnt = 12;
		} else {
			buff[0] = APDU_GET_RESPONSE;
			buff[1] = RESPONSE_NORMAL;
			buff[2] = cosemLink->invokeID;
			buff[3] = 0; // Data
			cnt = 4;
		}
		buff[cnt++] = TAG_ARRAY;
		if (cosemLink->sa_rest < 128) {
			buff[cnt++] = cosemLink->sa_rest;
		} else {
			buff[cnt++] = 0x82;
			buff[cnt++] = BYTE_1(cosemLink->sa_rest);
			buff[cnt++] = BYTE_0(cosemLink->sa_rest);
		}
	} else {
		buff[0] = APDU_GET_RESPONSE;
		buff[1] = GET_RESPONSE_WITH_DATABLOCK;
		buff[2] = cosemLink->invokeID;
		buff[3] = 0;
		cosemLink->block_number++;
		buff[4] = BYTE_3(cosemLink->block_number);
		buff[5] = BYTE_2(cosemLink->block_number);
		buff[6] = BYTE_1(cosemLink->block_number);
		buff[7] = BYTE_0(cosemLink->block_number);
		buff[8] = 0; // Data
		buff[9] = 0x82;
		buff[10] = 0;
		buff[11] = 0;
		cnt = 12;
	}

	for (i = cosemLink->sa_rows;; i++) {
		if (0 == cosemLink->sa_rest) {
			cosemLink->sa_rows = i;
			break;
		}
		if (0 == List_DLMS_Obj[i].attr_elements) {
			cosemLink->sa_rest = 0;
			break;
		}
		if ((cosemLink->maska_list_obj[i / 8] & (1 << (i % 8))) == 0) {
			continue;
		}
		if ((1 == cosemLink->access_selector) ||
		    (2 == cosemLink->access_selector)) {
			k = 15;
		} else {
			k = (21 + 7 * List_DLMS_Obj[i].attr_elements +
			     6 * List_DLMS_Obj[i].method_elements);
			if (CLASS_ID_PROFILE_GENERIC == List_DLMS_Obj[i].classID) {
				attrs = List_DLMS_Obj[i].attrs;
				switch (attrs[1].id & 0xC0) {
				case 0xC0:
					k += 5;
					break;
				case 0x80:
				case 0x40:
					k += 3;
					break;
				}
			}
		}
		if ((cnt + k) > (APDU_SIZE_MAX)) {
			cosemLink->sa_rows = i;
			break;
		}
		cosemLink->sa_rest--;
		buff[cnt++] = TAG_STRUCTURE;
		buff[cnt++] = 4;
		buff[cnt++] = TAG_UINT16;
		buff[cnt++] = BYTE_1(List_DLMS_Obj[i].classID);
		buff[cnt++] = BYTE_0(List_DLMS_Obj[i].classID);
		buff[cnt++] = TAG_UINT8;
		buff[cnt++] = List_DLMS_Obj[i].version;
		buff[cnt++] = TAG_OCTET_STRING;
		buff[cnt++] = 6;
		for (j = 0; j < 6; j++) {
			buff[cnt++] = List_DLMS_Obj[i].OBIS[j];
		}
		if ((1 == cosemLink->access_selector) ||
		    (2 == cosemLink->access_selector)) {
			continue;
		}
		buff[cnt++] = TAG_STRUCTURE;
		buff[cnt++] = 2;
		buff[cnt++] = TAG_ARRAY;
		buff[cnt++] = List_DLMS_Obj[i].attr_elements;

		attrs = List_DLMS_Obj[i].attrs;
		for (j = 0; j < List_DLMS_Obj[i].attr_elements; j++) {
			buff[cnt++] = TAG_STRUCTURE;
			buff[cnt++] = 3;
			buff[cnt++] = TAG_INT8;
			buff[cnt++] = attrs[j].id & 0x3F;
			buff[cnt++] = TAG_ENUM;
			switch (cosemLink->assoc) {
			case ASSOC_MFR:
				buff[cnt++] = attrs[j].access.mfr;
				break;
			case ASSOC_USB:
				buff[cnt++] = attrs[j].access.usb;
				break;
			case ASSOC_SVC:
				buff[cnt++] = attrs[j].access.svc;
				break;
			default:
				buff[cnt++] = attrs[j].access.usb;
				break;
			}
			if (CLASS_ID_PROFILE_GENERIC == List_DLMS_Obj[i].classID) {
				if (2 == (attrs[j].id & 0x3F)) {
					switch (attrs[j].id & 0xC0) {
					case 0xC0: {
						buff[cnt++] = TAG_ARRAY;
						buff[cnt++] = 2;
						buff[cnt++] = TAG_INT8;
						buff[cnt++] = 1;
						buff[cnt++] = TAG_INT8;
						buff[cnt++] = 2;
					} break;
					case 0x80: {
						buff[cnt++] = TAG_ARRAY;
						buff[cnt++] = 1;
						buff[cnt++] = TAG_INT8;
						buff[cnt++] = 1;
					} break;
					case 0x40: {
						buff[cnt++] = TAG_ARRAY;
						buff[cnt++] = 1;
						buff[cnt++] = TAG_INT8;
						buff[cnt++] = 2;
					} break;
					default: {
						buff[cnt++] = TAG_NULLDATA;
					} break;
					}
				} else {
					buff[cnt++] = TAG_NULLDATA;
				}
			} else {
				buff[cnt++] = TAG_NULLDATA;
			}
		}

		buff[cnt++] = TAG_ARRAY;
		buff[cnt++] = List_DLMS_Obj[i].method_elements;

		if (0 != List_DLMS_Obj[i].method_elements) {
			methods = List_DLMS_Obj[i].methods;
			for (j = 0; j < List_DLMS_Obj[i].method_elements; j++) {
				buff[cnt++] = TAG_STRUCTURE;
				buff[cnt++] = 2;
				buff[cnt++] = TAG_INT8;
				buff[cnt++] = methods[j].id;
				buff[cnt++] = TAG_ENUM; // TAG_BOOLEAN для версии 0;
				switch (cosemLink->assoc) {
				case ASSOC_MFR:
					buff[cnt++] = methods[j].access.mfr;
					break;
				case ASSOC_USB:
					buff[cnt++] = methods[j].access.usb;
					break;
				case ASSOC_SVC:
					buff[cnt++] = methods[j].access.svc;
					break;
				default:
					buff[cnt++] = methods[j].access.usb;
					break;
				}
			}
		}
	}
	cosemLink->counter = cnt;
	if (0 != cosemLink->block_number) {
		cnt -= 12;
		if (cnt < 128) {
			buff[11] = cnt & 0xFF;
			buff[10] = buff[8];
			buff[9] = buff[7];
			buff[8] = buff[6];
			buff[7] = buff[5];
			buff[6] = buff[4];
			buff[5] = buff[3];
			buff[4] = buff[2];
			buff[3] = buff[1];
			buff[2] = buff[0];
			cosemLink->counter -= 2;
			cosemLink->start_addr = 2;
		} else {
			buff[10] = (cnt >> 8) & 0xFF;
			buff[11] = cnt & 0xFF;
		}
		if (0 == cosemLink->sa_rest) {
			cosemLink->block_number = 0;
			buff[3 + cosemLink->start_addr] = 0xFF;
		}
	}
}
//----------------------------------------------------------------------------//
void Exchange_Device_Serial_Number(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Device_Serial_Number");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff;

	if (access != ATTR_ACCESS_READ) {
		if (access != ATTR_ACCESS_WRITE) {
			cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
			return;
		}

		buff = cosemLink->access_parameters;
		if ((cosemLink->len_parameters < 3) || (cosemLink->len_parameters > 10)) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		if (buff[0] != TAG_VISIBLE_STRING) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		if ((buff[1] < 1) || (buff[1] > 8)) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		u32 serialNumb;
		char formatStr[20];
		char tmpStr[9];
		memset(tmpStr, 0, sizeof(tmpStr));
		memcpy(tmpStr, (char *)(buff + 2), 8);
		snprintf(formatStr, sizeof(formatStr), "%%%" PRIuPTR SCNu32,
		         cosemLink->len_parameters);
		if (1 != sscanf(tmpStr, formatStr, &serialNumb)) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		if (RESULT_ERROR ==
		    Memory_Write_And_Update_Status(MEMORY_SERIAL_NUMBER, &serialNumb,
		                                   sizeof(SystemInfo.serialNumber))) {
			cosemLink->result = ACCESS_RESULT_HARDWARE_FAULT;
			return;
		}
		if (RESULT_ERROR == memory.readTwoCopyesWithCRC(
		                        MEMORY_SERIAL_NUMBER, &SystemInfo.serialNumber,
		                        sizeof(SystemInfo.serialNumber))) {
			cosemLink->result = ACCESS_RESULT_HARDWARE_FAULT;
			return;
		}
		cosemLink->result = ACCESS_RESULT_SUCCESS;
	} else {
		buff = cosemLink->buff;
		buff[0] = APDU_GET_RESPONSE;
		buff[1] = RESPONSE_NORMAL;
		buff[2] = cosemLink->invokeID;
		buff[3] = 0; // Data
		buff[4] = TAG_VISIBLE_STRING;
		buff[5] = 8;

		char tmpStr[9];
		memset(tmpStr, 0x00, sizeof(tmpStr));
		u16 lenStr = snprintf(tmpStr, sizeof(tmpStr), "%08" PRIu32,
		                      SystemInfo.serialNumber);
		if (lenStr > 8) {
			cosemLink->result = ACCESS_RESULT_HARDWARE_FAULT;
			return;
		}
		memcpy((buff + 6), tmpStr, lenStr);

		cosemLink->result = ACCESS_RESULT_SUCCESS;
		cosemLink->counter = 6 + lenStr;
	}
}
//----------------------------------------------------------------------------//
void Exchange_Firmware_Version(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Firmware_Version");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}

	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_VISIBLE_STRING;

	u8 lenStr = (u8)strlen(SystemInfo.str.firmwareVersion);
	buff[5] = lenStr;
	memcpy((buff + 6), SystemInfo.str.firmwareVersion, lenStr);

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 6 + lenStr;
}
//----------------------------------------------------------------------------//
void Exchange_Date_Of_Production(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Date_Of_Production");

	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff;

	if (access != ATTR_ACCESS_READ) {
		if (access != ATTR_ACCESS_WRITE) {
			cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
			return;
		}

		buff = cosemLink->access_parameters;
		if (cosemLink->len_parameters != 7) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		if ((buff[0] != TAG_OCTET_STRING) ||
		    (buff[1] != 5)) { // Если не соответствует тип
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		date_t date;
		date.yearHiByte = buff[2];
		date.yearLowByte = buff[3];
		u16 year = WORD(date.yearHiByte, date.yearLowByte);
		if ((year < 2025) || (year > 2050)) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		date.month = (month_t)buff[4];
		if ((date.month < 1) || (date.month > 12)) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		date.dayOfMonth = buff[5];
		if ((date.dayOfMonth < 1) || (date.dayOfMonth > 31)) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		date.dayOfWeek = (dayWeek_t)buff[6];
		if ((date.dayOfWeek < 1) || (date.dayOfWeek > 7)) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}

		if (RESULT_ERROR == Memory_Write_And_Update_Status(
		                        MEMORY_DATE_OF_PRODUCTION, &date, sizeof(date))) {
			cosemLink->result = ACCESS_RESULT_HARDWARE_FAULT;
			return;
		}
		if (RESULT_ERROR ==
		    memory.readTwoCopyesWithCRC(MEMORY_DATE_OF_PRODUCTION,
		                                &SystemInfo.dateOfProduction,
		                                sizeof(SystemInfo.dateOfProduction))) {
			cosemLink->result = ACCESS_RESULT_HARDWARE_FAULT;
			return;
		}
		cosemLink->result = ACCESS_RESULT_SUCCESS;
	} else {
		buff = cosemLink->buff;

		buff[0] = APDU_GET_RESPONSE;
		buff[1] = RESPONSE_NORMAL;
		buff[2] = cosemLink->invokeID;
		buff[3] = 0; // Data
		buff[4] = TAG_OCTET_STRING;
		buff[5] = 5;
		buff[6] = SystemInfo.dateOfProduction.yearHiByte;
		buff[7] = SystemInfo.dateOfProduction.yearLowByte;
		buff[8] = SystemInfo.dateOfProduction.month;
		buff[9] = SystemInfo.dateOfProduction.dayOfMonth;
		buff[10] = SystemInfo.dateOfProduction.dayOfWeek;

		cosemLink->result = ACCESS_RESULT_SUCCESS;
		cosemLink->counter = 11;
	}
}
//----------------------------------------------------------------------------//
void Exchange_Device_Model_Id(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Device_Model_Id");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}
	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_VISIBLE_STRING;
	u8 lenStr = (u8)strlen(SystemInfo.str.modelIdDevice);
	buff[5] = lenStr;
	memcpy((buff + 6), SystemInfo.str.modelIdDevice, lenStr);

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 6 + lenStr;
}
//----------------------------------------------------------------------------//
accessResult_t mapResultToAccessResult(result_t result) {
	switch (result) {
	case RESULT_SUCCESS:
		return ACCESS_RESULT_SUCCESS;
	case RESULT_ERROR_OUT_OF_RANGE:
		return ACCESS_RESULT_TYPE_UNMATCHED;
	case RESULT_ERROR_UNMATCHED:
		return ACCESS_RESULT_TYPE_UNMATCHED;
	case RESULT_ERROR_MEM_WRITE:
		return ACCESS_RESULT_HARDWARE_FAULT;
	case RESULT_ERROR_MEM_READ:
		return ACCESS_RESULT_HARDWARE_FAULT;
	default:
		return ACCESS_RESULT_OTHER_REASON;
	}
}
//----------------------------------------------------------------------------//
void Exchange_Voltage_3V5(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Voltage_3V5");
	cosemLink_t *cosemLink;
	cosemLink = data;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}
	Exchange_uint_16(data, SystemInfo.Vcc.MCU);
}
//----------------------------------------------------------------------------//
void Exchange_Voltage_USB(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Voltage_USB");
	cosemLink_t *cosemLink;
	cosemLink = data;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}
	Exchange_uint_16(data, SystemInfo.Vcc.USB);
}
//----------------------------------------------------------------------------//
void Exchange_Voltage_Batt(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Voltage_Batt");
	cosemLink_t *cosemLink;
	cosemLink = data;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}
	Exchange_uint_16(data, SystemInfo.Vcc.Batt);
}
//----------------------------------------------------------------------------//
void Exchange_Temperature(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Temperature");
	cosemLink_t *cosemLink;
	cosemLink = data;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}
	Exchange_int_16(data, SystemInfo.temperature);
}
//----------------------------------------------------------------------------//
void Exchange_Operating_Time(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Operating_Time");
	cosemLink_t *cosemLink;
	cosemLink = data;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}
	Exchange_uint_32(data, (SystemInfo.cnt.operatingTime +
	                        SystemInfo.cnt.lastSessionOperatingTime));
}
//----------------------------------------------------------------------------//
void Exchange_Cnt_Starts(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Cnt_Starts");
	cosemLink_t *cosemLink;
	cosemLink = data;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}
	Exchange_uint_32(data, SystemInfo.cnt.starts);
}
//----------------------------------------------------------------------------//
void Exchange_Firmware_Identifier(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Firmware_Identifier");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}

	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_OCTET_STRING;
	buff[5] = 4;
	buff[6] = BYTE_0(*SystemInfo.firmwareID);
	buff[7] = BYTE_1(*SystemInfo.firmwareID);
	buff[8] = BYTE_2(*SystemInfo.firmwareID);
	buff[9] = BYTE_3(*SystemInfo.firmwareID);

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 10;
}
//----------------------------------------------------------------------------//
void Exchange_Firmware_Version_Identifier(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Firmware_Version_Identifier");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}

	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_OCTET_STRING;
	u8 v1, v2, v3;
	if (3 != sscanf(SystemInfo.str.firmwareVersion, "%hhu.%hhu.%hhu", &v1, &v2,
	                &v3)) {
		cosemLink->result = ACCESS_RESULT_OTHER_REASON;
		return;
	}
	buff[5] = 3;
	buff[6] = v1;
	buff[7] = v2;
	buff[8] = v3;
	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 9;
}
//----------------------------------------------------------------------------//
void Exchange_Firmware_Signature(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Firmware_Signature");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}

	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_OCTET_STRING;
	buff[5] = (u8)sizeof(SystemInfo.MD5.mainProgramm);
	memcpy((buff + 6), SystemInfo.MD5.mainProgramm,
	       sizeof(SystemInfo.MD5.mainProgramm));

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 6 + sizeof(SystemInfo.MD5);
}
//----------------------------------------------------------------------------//
void Exchange_CHK_Signature(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_CHK_Signature");
	cosemLink_t *cosemLink;
	u8 *buff;
	cosemLink = data;
	buff = cosemLink->buff;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}
	char md5_calc[MD5_STR_LEN];
	memset(md5_calc, 0, MD5_STR_LEN);
	MD5Init();
	MD5Update((char *)SystemInfo.MD5.range.start,
	          (SystemInfo.MD5.range.end - SystemInfo.MD5.range.start + 1));
	MD5Final(md5_calc);
	strncpy(SystemInfo.MD5.userRange, md5_calc, MD5_STR_LEN);

	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_OCTET_STRING;
	buff[5] = (u8)sizeof(SystemInfo.MD5.userRange);
	memcpy((buff + 6), SystemInfo.MD5.userRange,
	       sizeof(SystemInfo.MD5.userRange));

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 6 + sizeof(SystemInfo.MD5);
}
//----------------------------------------------------------------------------//
#define FMC_BASE 0x08000000
#define FMC_BASE_END (FMC_BASE + 0x3FFFF)
void Exchange_CHK_Address(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_CHK_Address");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff;

	if (access != ATTR_ACCESS_READ) {
		if (access != ATTR_ACCESS_WRITE) {
			cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
			return;
		}

		buff = cosemLink->access_parameters;
		if ((buff[0] != TAG_STRUCTURE) || (buff[1] != 2) ||
		    (cosemLink->len_parameters != 12)) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}

		if ((buff[2] != TAG_UINT32) || (buff[7] != TAG_UINT32)) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		u32 addrStart = DWORD(buff[3], buff[4], buff[5], buff[6]);
		u32 addrEnd = DWORD(buff[8], buff[9], buff[10], buff[11]);
		if ((addrStart < FMC_BASE) || (addrEnd < FMC_BASE) ||
		    (addrStart > FMC_BASE_END) || (addrEnd > FMC_BASE_END) ||
		    (addrEnd < addrStart)) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		SystemInfo.MD5.range.start = addrStart;
		SystemInfo.MD5.range.end = addrEnd;
		cosemLink->result = ACCESS_RESULT_SUCCESS;
	} else {
		u8 *buff = cosemLink->buff;
		buff = cosemLink->buff;

		buff[0] = APDU_GET_RESPONSE;
		buff[1] = RESPONSE_NORMAL;
		buff[2] = cosemLink->invokeID;
		buff[3] = 0; // Data
		buff[4] = TAG_STRUCTURE;
		buff[5] = 2;
		buff[6] = TAG_UINT32;
		buff[7] = BYTE_3(SystemInfo.MD5.range.start);
		buff[8] = BYTE_2(SystemInfo.MD5.range.start);
		buff[9] = BYTE_1(SystemInfo.MD5.range.start);
		buff[10] = BYTE_0(SystemInfo.MD5.range.start);
		buff[11] = TAG_UINT32;
		buff[12] = BYTE_3(SystemInfo.MD5.range.end);
		buff[13] = BYTE_2(SystemInfo.MD5.range.end);
		buff[14] = BYTE_1(SystemInfo.MD5.range.end);
		buff[15] = BYTE_0(SystemInfo.MD5.range.end);

		cosemLink->result = ACCESS_RESULT_SUCCESS;
		cosemLink->counter = 16;
	}
}
//----------------------------------------------------------------------------//
void Exchange_Image_Blocks_Status(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Image_Blocks_Status");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}

	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_BITSTRING;
	buff[5] = 0x82; // 2 следующие байта длины
	u16 size = sizeof(SystemInfo.image.transferredBlockStatus) * 8;
	buff[6] = BYTE_1(size);
	buff[7] = BYTE_0(size);
	for (u8 i = 0; i < sizeof(SystemInfo.image.transferredBlockStatus); i++) {
		buff[i + 8] = SystemInfo.image.transferredBlockStatus[i];
	}
	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = sizeof(SystemInfo.image.transferredBlockStatus) + 8;
}
//----------------------------------------------------------------------------//
void Exchange_Not_Transfer_Block_Number(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Not_Transfer_Block_Number");
	cosemLink_t *cosemLink;
	cosemLink = data;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}
	u8 i;
	for (i = 0; i < sizeof(SystemInfo.image.transferredBlockStatus); i++) {
		if (SystemInfo.image.transferredBlockStatus[i] != 0xFF) {
			break;
		}
	}
	u32 x = i * 8;
	if (i < sizeof(SystemInfo.image.transferredBlockStatus)) {
		u8 j;
		for (j = 0; j < 8; j++) {
			if (((1 << j) & SystemInfo.image.transferredBlockStatus[i]) == 0) {
				break;
			}
		}
		x += j;
	}
	Exchange_uint_32(data, x);
}
//----------------------------------------------------------------------------//
void Exchange_Image_Transfer_Enabled(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Image_Transfer_Enabled");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff;

	if (access != ATTR_ACCESS_READ) {
		if (access != ATTR_ACCESS_WRITE) {
			cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
			return;
		}

		buff = cosemLink->access_parameters;
		if ((buff[0] != TAG_BOOLEAN) || (cosemLink->len_parameters != 2)) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		SystemInfo.image.transferEnabled = (bool)buff[1];
		SystemInfo.image.transferStatus = IMAGE_TRANSFER_NOT_INITIATED;
		for (u8 i = 0; i < 4; i++) { // Стираем 4 блока по 64К
			memory.eraseBlock(MEMORY_UPDATE_DEVICE_BUFF_START +
			                  i * (MEMORY_BLOCK_SIZE / memory.getPageSize()));
		}
		cosemLink->result = ACCESS_RESULT_SUCCESS;
	} else {
		u8 *buff = cosemLink->buff;
		buff = cosemLink->buff;

		buff[0] = APDU_GET_RESPONSE;
		buff[1] = RESPONSE_NORMAL;
		buff[2] = cosemLink->invokeID;
		buff[3] = 0; // Data
		buff[4] = TAG_BOOLEAN;
		buff[5] = SystemInfo.image.transferEnabled;

		cosemLink->result = ACCESS_RESULT_SUCCESS;
		cosemLink->counter = 6;
	}
}
//----------------------------------------------------------------------------//
void Exchange_Image_Transfer_Status(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Image_Transfer_Status");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}

	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_ENUM;
	buff[5] = SystemInfo.image.transferStatus;

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 6;
}
//----------------------------------------------------------------------------//
void Exchange_Image_Info(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Image_Info");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}

	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_ARRAY;

	if (IMAGE_TRANSFER_VERIFICATION_SUCCESSFUL !=
	    SystemInfo.image.transferStatus) {
		buff[5] = 0;
		cosemLink->counter = 6;
	} else {
		buff[5] = 1;
		buff[6] = TAG_STRUCTURE;
		buff[7] = 3;
		buff[8] = TAG_UINT32;
		buff[9] = BYTE_3(BOOTLOADER_FLASH_MAIN_APP_SIZE);
		buff[10] = BYTE_2(BOOTLOADER_FLASH_MAIN_APP_SIZE);
		buff[11] = BYTE_1(BOOTLOADER_FLASH_MAIN_APP_SIZE);
		buff[12] = BYTE_0(BOOTLOADER_FLASH_MAIN_APP_SIZE);
		buff[13] = TAG_OCTET_STRING;
		buff[14] = 4;
		buff[15] = BYTE_3(SystemInfo.image.firmareID);
		buff[16] = BYTE_2(SystemInfo.image.firmareID);
		buff[17] = BYTE_1(SystemInfo.image.firmareID);
		buff[18] = BYTE_0(SystemInfo.image.firmareID);
		buff[19] = TAG_OCTET_STRING;

		buff[20] = (u8)sizeof(SystemInfo.image.calcMD5);
		memcpy((buff + 21), SystemInfo.image.calcMD5,
		       sizeof(SystemInfo.image.calcMD5));

		cosemLink->counter = 21 + sizeof(SystemInfo.image.calcMD5);
	}
	cosemLink->result = ACCESS_RESULT_SUCCESS;
}
//----------------------------------------------------------------------------//
void Exchange_Service_Mode_Password(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Service_Mode_Password");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff;

	if (access != ATTR_ACCESS_READ) {
		if (access != ATTR_ACCESS_WRITE) {
			cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
			return;
		}

		buff = cosemLink->access_parameters;
		if (cosemLink->len_parameters != 18) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		if (buff[0] != TAG_VISIBLE_STRING) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		if (buff[1] != 16) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}

		for (
		    u8 i = 0; i < 16;
		    i++) { // проверка на символы
			        // !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~
			if ((buff[i + 2] < 33) || (buff[i + 2] > 126)) {
				cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
				return;
			}
		}

		if (RESULT_ERROR == Memory_Write_And_Update_Status(
		                        MEMORY_SETTINGS_DEVICE_PASSWORD, (buff + 2),
		                        sizeof(SystemInfo.password))) {
			cosemLink->result = ACCESS_RESULT_HARDWARE_FAULT;
			return;
		}
		if (RESULT_ERROR ==
		    memory.readTwoCopyesWithCRC(MEMORY_SETTINGS_DEVICE_PASSWORD,
		                                &SystemInfo.password,
		                                sizeof(SystemInfo.password))) {
			cosemLink->result = ACCESS_RESULT_HARDWARE_FAULT;
			return;
		}
		cosemLink->result = ACCESS_RESULT_SUCCESS;
	} else {
		buff = cosemLink->buff;
		buff[0] = APDU_GET_RESPONSE;
		buff[1] = RESPONSE_NORMAL;
		buff[2] = cosemLink->invokeID;
		buff[3] = 0; // Data
		buff[4] = TAG_VISIBLE_STRING;
		buff[5] = 16;
		memcpy((buff + 6), SystemInfo.password, sizeof(SystemInfo.password));

		cosemLink->result = ACCESS_RESULT_SUCCESS;
		cosemLink->counter = 22;
	}
}
//----------------------------------------------------------------------------//
typedef struct {
	u8 yearHiByte;
	u8 yearLowByte;
	month_t month;
	u8 dayOfMonth;
	dayWeek_t dayOfWeek;
	u8 hour;
	u8 minute;
	u8 second;
	u8 hundredthsOfSecond;
	u8 deviationHighByte;
	u8 deviationLowByte;
	u8 clockStatus;
} dateTime_t;
//----------------------------------------------------------------------------//
void Exchange_DateTime_Compilation_FW(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_DateTime_Compilation_FW");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}
	dateTime_t dateTime;
	dateTime.clockStatus = 0;
	dateTime.yearHiByte = BYTE_1(2026);
	dateTime.yearLowByte = BYTE_0(2026);
	dateTime.month = APRIL;
	dateTime.dayOfMonth = 20;
	dateTime.dayOfWeek = MONDAY;

	dateTime.hour = 17;
	dateTime.minute = 00;
	dateTime.second = 00;
	dateTime.hundredthsOfSecond = 00;
	dateTime.deviationHighByte = 0x80;
	dateTime.deviationLowByte = 00;

	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_OCTET_STRING;
	buff[5] = sizeof(dateTime_t);
	memcpy((buff + 6), &dateTime, sizeof(dateTime_t));

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 6 + sizeof(dateTime_t);
}
//----------------------------------------------------------------------------//
void Exchange_Cnt_Btn_1(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Cnt_Btn_1");
	cosemLink_t *cosemLink;
	cosemLink = data;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}
	Exchange_uint_32(data, SystemInfo.cnt.btn1);
}
//----------------------------------------------------------------------------//
void Exchange_Cnt_Btn_2(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Cnt_Btn_2");
	cosemLink_t *cosemLink;
	cosemLink = data;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}
	Exchange_uint_32(data, SystemInfo.cnt.btn2);
}
//----------------------------------------------------------------------------//
void Exchange_LCD_Diag(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_LCD_Diag");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff;

	if (access != ATTR_ACCESS_READ) {
		if (access != ATTR_ACCESS_WRITE) {
			cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
			return;
		}
		buff = cosemLink->access_parameters;
		if ((cosemLink->len_parameters != 2) || (buff[0] != TAG_ENUM)) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		if ((buff[1] != 0) && buff[1] != 1) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		SystemInfo.status.lcdDiag = buff[1];
		cosemLink->result = ACCESS_RESULT_SUCCESS;
		return;
	} else {
		buff = cosemLink->buff;
		buff[0] = APDU_GET_RESPONSE;
		buff[1] = RESPONSE_NORMAL;
		buff[2] = cosemLink->invokeID;
		buff[3] = 0; // Data
		buff[4] = TAG_ENUM;
		buff[5] = (u8)SystemInfo.status.lcdDiag;

		cosemLink->result = ACCESS_RESULT_SUCCESS;
		cosemLink->counter = 6;
	}
}
//----------------------------------------------------------------------------//
void Exchange_BLE_Current_Status(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_BLE_Current_Status");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}
	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_ENUM;
	buff[5] = (u8)SystemInfo.BLE_State;

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 6;
}
//----------------------------------------------------------------------------//
extern s8 rssiBLE;
void Exchange_BLE_RSSI(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_BLE_RSSI");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}
	if (SystemInfo.BLE_State != BLE_STATE_CONNECT) {
		cosemLink->result = ACCESS_RESULT_OBJECT_UNAVAILABLE;
		return;
	}
	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_INT8;
	buff[5] = rssiBLE;
	;

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 6;
}
//----------------------------------------------------------------------------//
void Exchange_SP_Version(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_SP_Version");

	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	if (access != ATTR_ACCESS_READ) {
		cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
		return;
	}
	buff[0] = APDU_GET_RESPONSE;
	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;
	buff[3] = 0; // Data
	buff[4] = TAG_OCTET_STRING;
	buff[5] = 4;
	memcpy(buff + 6, "4.00", 4);

	cosemLink->result = ACCESS_RESULT_SUCCESS;
	cosemLink->counter = 10;
}
//----------------------------------------------------------------------------//
void Exchange_Meter_Number(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_Meter_Number");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff;

	if (access != ATTR_ACCESS_READ) {
		if (access != ATTR_ACCESS_WRITE) {
			cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
			return;
		}
		buff = cosemLink->access_parameters;
		if ((cosemLink->len_parameters != 5) || (buff[0] != TAG_UINT32)) {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		u32 meterNumber = DWORD(buff[1], buff[2], buff[3], buff[4]);
		if (meterNumber < 100000000) { // Проверка на диапазон
			if (RESULT_ERROR ==
			    Memory_Write_And_Update_Status(MEMORY_METER_NUMBER, &meterNumber,
			                                   sizeof(SystemInfo.numMeter))) {
				cosemLink->result = ACCESS_RESULT_HARDWARE_FAULT;
				return;
			}
			if (RESULT_ERROR == memory.readTwoCopyesWithCRC(
			                        MEMORY_METER_NUMBER, &SystemInfo.numMeter,
			                        sizeof(SystemInfo.numMeter))) {
				cosemLink->result = ACCESS_RESULT_HARDWARE_FAULT;
				return;
			}
			cosemLink->result = ACCESS_RESULT_SUCCESS;
			return;
		} else {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
	} else {
		if (SystemInfo.numMeter < 100000000) {
			Exchange_uint_32(data, (SystemInfo.numMeter));
		} else {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
		}
	}
}
//----------------------------------------------------------------------------//
void Exchange_PinCode(void *data, attrAccess_t access) {
	_SYSLOG_("[DLMS] Exchange_PinCode");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff;

	// [DIAG] что реально пришло на объект пина - снять по USB-логу
	_SYSLOG_("[DLMS] Pin: access=%d len=%d", access, cosemLink->len_parameters);
	if (access != ATTR_ACCESS_READ) {
		if (access != ATTR_ACCESS_WRITE) {
			_SYSLOG_("[DLMS] Pin: not WRITE -> SCOPE_VIOLATED");
			cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
			return;
		}
		buff = cosemLink->access_parameters;
		_SYSLOG_DATA_("[DLMS] Pin params", buff, cosemLink->len_parameters);
		if ((cosemLink->len_parameters != 5) || (buff[0] != TAG_UINT32)) {
			_SYSLOG_("[DLMS] Pin: bad type/len tag=%02X exp=%02X", buff[0],
			         TAG_UINT32);
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
		u32 pinCode = DWORD(buff[1], buff[2], buff[3], buff[4]);
		_SYSLOG_("[DLMS] Pin: value=%u (limit<1000000)", pinCode);
		if (pinCode < 1000000) { // Проверка на диапазон
			if (RESULT_ERROR ==
			    Memory_Write_And_Update_Status(MEMORY_PINCODE, &pinCode,
			                                   sizeof(SystemInfo.pinCode))) {
				cosemLink->result = ACCESS_RESULT_HARDWARE_FAULT;
				return;
			}
			if (RESULT_ERROR ==
			    memory.readTwoCopyesWithCRC(MEMORY_PINCODE, &SystemInfo.pinCode,
			                                sizeof(SystemInfo.pinCode))) {
				cosemLink->result = ACCESS_RESULT_HARDWARE_FAULT;
				return;
			}
			cosemLink->result = ACCESS_RESULT_SUCCESS;
			return;
		} else {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
			return;
		}
	} else {
		if (SystemInfo.pinCode < 1000000) {
			Exchange_uint_32(data, (SystemInfo.pinCode));
		} else {
			cosemLink->result = ACCESS_RESULT_TYPE_UNMATCHED;
		}
	}
}
//----------------------------------------------------------------------------//
