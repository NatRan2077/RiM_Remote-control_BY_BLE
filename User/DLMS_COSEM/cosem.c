//----------------------------------------------------------------------------//
#include <stdbool.h>
#include <string.h>
#include "common_types.h"
#include "log.h"
#include "dlms.h"
#include "cosem.h"
//----------------------------------------------------------------------------//
typedef enum {
	EXCEPTION_RESPONSE_CODE_NONE = 0,
	EXCEPTION_RESPONSE_CODE_STATE_ERROR_SERVICE_NOT_ALLOWED,
	EXCEPTION_RESPONSE_CODE_STATE_ERROR_SERVICE_UNKNOWN,
	EXCEPTION_RESPONSE_CODE_SERVICE_ERROR_OPERATION_NOT_POSSIBLE,
	EXCEPTION_RESPONSE_CODE_SERVICE_ERROR_SERVICE_NOT_SUPPORTED,
	EXCEPTION_RESPONSE_CODE_SERVICE_ERROR_OTHER_REASON,
} ExceptionResponseCode_t;
//----------------------------------------------------------------------------//
u8 PrepareExceptionResponse(cosemLink_t *cosemLink,
                            ExceptionResponseCode_t code) {
	u8 *buff = cosemLink->buff;
	u16 cnt = 0;
	buff[cnt++] = APDU_EXCEPTION_RESPONSE;
	switch (code) {
	case EXCEPTION_RESPONSE_CODE_STATE_ERROR_SERVICE_NOT_ALLOWED: {
		buff[cnt++] = 0x00;
		buff[cnt++] = 0x01;
	} break;
	case EXCEPTION_RESPONSE_CODE_STATE_ERROR_SERVICE_UNKNOWN: {
		buff[cnt++] = 0x00;
		buff[cnt++] = 0x02;
	} break;
	case EXCEPTION_RESPONSE_CODE_SERVICE_ERROR_OPERATION_NOT_POSSIBLE: {
		buff[cnt++] = 0x01;
		buff[cnt++] = 0x01;
	} break;
	case EXCEPTION_RESPONSE_CODE_SERVICE_ERROR_SERVICE_NOT_SUPPORTED: {
		buff[cnt++] = 0x01;
		buff[cnt++] = 0x02;
	} break;
	case EXCEPTION_RESPONSE_CODE_SERVICE_ERROR_OTHER_REASON: {
		buff[cnt++] = 0x01;
		buff[cnt++] = 0x03;
	} break;
	default: {
		buff[cnt++] = 0x01;
		buff[cnt++] = 0x03;
	}
	}
	cosemLink->counter = cnt;
	return true;
}
//----------------------------------------------------------------------------//
bool PrepareResult(cosemLink_t *cosemLink, accessResult_t result) {
	u8 *buff = cosemLink->buff;

	buff[1] = RESPONSE_NORMAL;
	buff[2] = cosemLink->invokeID;

	switch (cosemLink->APDU) {
	case APDU_GET_REQUEST: {
		buff[0] = APDU_GET_RESPONSE;
		buff[3] = 0x01;
		buff[4] = result;
		cosemLink->counter = 5;
		return true;
	}
	case APDU_SET_REQUEST: {
		buff[0] = APDU_SET_RESPONSE;
		buff[3] = result;
		cosemLink->counter = 4;
		return true;
	}
	case APDU_ACTION_REQUEST: {
		buff[0] = APDU_ACTION_RESPONSE;
		buff[3] = result;
		buff[4] = 0x00; // not return-parameters
		cosemLink->counter = 5;
		return true;
	}
	default: {
		return false;
	}
	}
}

bool checkAccessLevel(assoc_t requesterAssoc,
                      const accessPermissions_t *attrPerms,
                      attrAccess_t requiredAccess) {
	accessLevel_t allowedAccess;
	switch (requesterAssoc) { // Получаем разрешенный уровень доступа для
		                       // запрашивающей ассоциации
	case ASSOC_USB:
		allowedAccess = attrPerms->usb;
		break;
	case ASSOC_SVC:
		allowedAccess = attrPerms->svc;
		break;
	case ASSOC_MFR:
		allowedAccess = attrPerms->mfr;
		break;
	default:
		return false; // Неизвестная ассоциация - доступ запрещен
	}

	switch (requiredAccess) { // Проверяем соответствие требуемого доступа с
		                       // разрешенным
	case ATTR_ACCESS_READ:
		return (allowedAccess & R) == R; // Для чтения достаточно бита R (0x01)
	case ATTR_ACCESS_WRITE:
		return (allowedAccess & W) == W; // Для записи достаточно бита W (0x02)
	default:
		return false; // Неизвестный тип доступа - доступ запрещен
	}
}
//----------------------------------------------------------------------------//
bool PrepareGetBlock(cosemLink_t *cosemLink) {
	const struct attribute_desc_s *attrs;
	void (*callback)(void *data, attrAccess_t access);
	uint8_t attrID;

	uint8_t *buff;

	attrs = List_DLMS_Obj[cosemLink->objID].attrs;
	u16 cnt = 0;

	for (attrID = 0; attrID < List_DLMS_Obj[cosemLink->objID].attr_elements;
	     attrID++) {
		if ((attrs[attrID].id & 0x3F) == cosemLink->attribute) {
			break;
		}
	}
	if (attrID == List_DLMS_Obj[cosemLink->objID].attr_elements) {
		if (PrepareResult(cosemLink, ACCESS_RESULT_HARDWARE_FAULT)) {
			return true;
		} else {
			return false;
		}
	}

	if (NULL == attrs[attrID].callback) {
		switch (attrs[attrID].type) {
		case TAG_ARRAY:
		case TAG_STRUCTURE: {
			u16 data_u16;
			u16 dataOffset;
			buff = cosemLink->buff;
			memcpy(&data_u16, attrs[attrID].addr,
			       2); // Вычитываем общую длинну данных
			dataOffset = cosemLink->block_number *
			             (APDU_SIZE_MAX - 12); // Умножаем число переданных блоков
			                                   // на размер блока(без заголовка)
			dataOffset--;
			data_u16 -= dataOffset; // Вычитаем смещенеие(колличество переданных
			                        // байт) и получаем остаток

			buff[cnt++] = APDU_GET_RESPONSE;
			buff[cnt++] = GET_RESPONSE_WITH_DATABLOCK;
			buff[cnt++] = cosemLink->invokeID;
			buff[cnt++] = 0;
			cosemLink->block_number++;
			buff[cnt++] = BYTE_3(cosemLink->block_number);
			buff[cnt++] = BYTE_2(cosemLink->block_number);
			buff[cnt++] = BYTE_1(cosemLink->block_number);
			buff[cnt++] = BYTE_0(cosemLink->block_number);
			buff[cnt++] = 0; // Data
			if ((data_u16 + 12) >
			    APDU_SIZE_MAX) { // Если длинна данных плюс заголовок(12 байт)
				                  // больше размера APDU
				data_u16 = APDU_SIZE_MAX; // Передаем блок целиком
				data_u16 -= 12;
			} else { // Иначе выставляем флаг последнего блока и обнуляем счетчик
				      // блоков
				buff[3] = LAST_BLOCK_FLAG;
				cosemLink->block_number = 0;
			}
			if (data_u16 < 128) {
				buff[cnt++] = data_u16 & 0xFF; // Если длинна данных менее 128 байт,
				                               // то передаем сразу байт длинны
			}
			// else if()
			else {
				buff[cnt++] = 0x82;
				buff[cnt++] = BYTE_1(data_u16);
				buff[cnt++] = BYTE_0(data_u16);
			}
			memcpy((buff + cnt), (attrs[attrID].addr + 2 + dataOffset), data_u16);
			cnt += data_u16;
			cosemLink->counter = cnt;
		} break;
		default: {
			if (PrepareResult(cosemLink, ACCESS_RESULT_HARDWARE_FAULT)) {
				return true;
			} else {
				return false;
			}
		}
		}
	} else {
		callback = *attrs[attrID].callback;
		callback(cosemLink, ATTR_ACCESS_READ);

		if (ACCESS_RESULT_SUCCESS != cosemLink->result) {
			if (PrepareResult(cosemLink, cosemLink->result)) {
				return true;
			} else {
				return false;
			}
		} else {
			return true;
		}
	}
	return false;
}
//----------------------------------------------------------------------------//
bool PrepareGet(cosemLink_t *cosemLink) {
	const struct attribute_desc_s *attrs;
	void (*callback)(void *data, attrAccess_t access);
	u8 attrID, size;
	u8 *buff = cosemLink->buff;
	attrs = List_DLMS_Obj[cosemLink->objID].attrs;
	u16 cnt = 0;

	for (attrID = 0; attrID < List_DLMS_Obj[cosemLink->objID].attr_elements;
	     attrID++) {
		if ((attrs[attrID].id & 0x3F) == cosemLink->attribute) {
			break;
		}
	}
	if (attrID == List_DLMS_Obj[cosemLink->objID].attr_elements) {
		if (PrepareResult(cosemLink, ACCESS_RESULT_HARDWARE_FAULT)) {
			return true;
		} else {
			return false;
		}
	}
	if (!(checkAccessLevel(cosemLink->assoc, &attrs[attrID].access,
	                       ATTR_ACCESS_READ))) {
		if (PrepareResult(cosemLink, ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED)) {
			return true;
		} else {
			return false;
		}
	}
	if (NULL == attrs[attrID].callback) {
		buff[cnt++] = APDU_GET_RESPONSE;
		buff[cnt++] = RESPONSE_NORMAL;
		buff[cnt++] = cosemLink->invokeID;
		buff[cnt++] = 0; // Data
		buff[cnt++] = attrs[attrID].type;

		switch (attrs[attrID].type) {
		case TAG_NULLDATA:
			break;
		case TAG_BOOLEAN:
		case TAG_INT8:
		case TAG_UINT8:
		case TAG_ENUM: {
			memcpy(buff + cnt, attrs[attrID].addr, 1);
			cnt++;
		} break;
		case TAG_INT16:
		case TAG_UINT16: {
			u16 data_u16; // Переменная для временного корирования данных
			memcpy(&data_u16, attrs[attrID].addr, sizeof(data_u16));
			buff[cnt++] = BYTE_1(data_u16);
			buff[cnt++] = BYTE_0(data_u16);
		} break;
		case TAG_INT32:
		case TAG_UINT32:
		case TAG_FLOAT32: {
			u32 data_u32; // Переменная для временного корирования данных
			memcpy(&data_u32, attrs[attrID].addr, sizeof(data_u32));
			buff[cnt++] = BYTE_3(data_u32);
			buff[cnt++] = BYTE_2(data_u32);
			buff[cnt++] = BYTE_1(data_u32);
			buff[cnt++] = BYTE_0(data_u32);
		} break;
		case TAG_INT64:
		case TAG_UINT64:
		case TAG_FLOAT64: {
			u64 data_u64; // Переменная для временного корирования данных
			memcpy(&data_u64, attrs[attrID].addr, sizeof(data_u64));
			buff[cnt++] = BYTE_7(data_u64);
			buff[cnt++] = BYTE_6(data_u64);
			buff[cnt++] = BYTE_5(data_u64);
			buff[cnt++] = BYTE_4(data_u64);
			buff[cnt++] = BYTE_3(data_u64);
			buff[cnt++] = BYTE_2(data_u64);
			buff[cnt++] = BYTE_1(data_u64);
			buff[cnt++] = BYTE_0(data_u64);
		} break;
		case TAG_ARRAY:
		case TAG_STRUCTURE: {
			u16 data_u16; // Переменная для временного корирования данных
			memcpy(&data_u16, attrs[attrID].addr,
			       sizeof(data_u16)); // Копируем длину

			if ((data_u16 + 5) >
			    (APDU_SIZE_MAX)) { // Если длинна данных плюс заголовок(5 байт)
				                    // больше размера APDU
				cnt -= 5;           // Перемещаем указатель на начало заголовка и
				                    // перезаписываем его для блочной передачи
				buff[cnt++] = APDU_GET_RESPONSE;
				buff[cnt++] = GET_RESPONSE_WITH_DATABLOCK;
				buff[cnt++] = cosemLink->invokeID;
				buff[cnt++] = 0;
				cosemLink->block_number++; // Номер блока
				buff[cnt++] = BYTE_3(cosemLink->block_number);
				buff[cnt++] = BYTE_2(cosemLink->block_number);
				buff[cnt++] = BYTE_1(cosemLink->block_number);
				buff[cnt++] = BYTE_0(cosemLink->block_number);
				buff[cnt++] = 0; // Data
				buff[cnt++] = 0x82;
				data_u16 =
				    APDU_SIZE_MAX - 12; // Длина данных APDU_SIZE_MAX - заголовок
				                        // (вместе с длинной 12 байт)
				buff[cnt++] = BYTE_1(data_u16);
				buff[cnt++] = BYTE_0(data_u16);
				buff[cnt++] = attrs[attrID].type; // Тип аттрибута записывается
				                                  // 13-м, и входит в длинну данных
				data_u16--;                       // Поэтому вычитаем 1
				memcpy((buff + cnt), (attrs[attrID].addr + 2),
				       data_u16); // Копируем данные в буфер
				cnt += data_u16;  // Увеличиваем счетчик на длинну данных
			} else {
				memcpy((buff + cnt), (attrs[attrID].addr + 2),
				       data_u16); // Просто копируем данные
				cnt += data_u16;
			}
		} break;
		case TAG_OCTET_STRING:
		case TAG_VISIBLE_STRING: {
			if (attrID != 0) {
				memcpy(&size, attrs[attrID].addr, 1);
				memcpy(buff + cnt, attrs[attrID].addr, ++size);
				cnt += size;
			} else {
				buff[cnt++] = 6;
				memcpy(buff + cnt, attrs[attrID].addr, 6);
				cnt += 6;
			}
		} break;
		default: {
			if (PrepareResult(cosemLink, ACCESS_RESULT_OTHER_REASON)) {
				return true;
			} else {
				return false;
			}
		}
		}
		cosemLink->counter = cnt;
		return true;
	} else {
		callback = *attrs[attrID].callback;
		callback(cosemLink, ATTR_ACCESS_READ);

		if (ACCESS_RESULT_SUCCESS != cosemLink->result) {
			if (PrepareResult(cosemLink, cosemLink->result)) {
				return true;
			} else {
				return false;
			}
		} else {
			return true;
		}
	}
}
//----------------------------------------------------------------------------//
bool PrepareAction(cosemLink_t *cosemLink) {
	const struct method_desc_s *methods;
	void (*callback)(void *data);
	u8 methodID;
	u8 *buff = cosemLink->buff;
	methods = List_DLMS_Obj[cosemLink->objID].methods;

	for (methodID = 0;
	     methodID < List_DLMS_Obj[cosemLink->objID].method_elements;
	     methodID++) {
		if (methods[methodID].id == cosemLink->attribute) {
			break;
		}
	}
	if (methodID == List_DLMS_Obj[cosemLink->objID].method_elements) {
		if (PrepareResult(cosemLink, ACCESS_RESULT_HARDWARE_FAULT)) {
			return true;
		} else {
			return false;
		}
	}

	if (!(checkAccessLevel(cosemLink->assoc, &methods[methodID].access,
	                       ATTR_ACCESS_READ))) {
		if (PrepareResult(cosemLink, ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED)) {
			return true;
		} else {
			return false;
		}
	}

	if (NULL == methods[methodID].callback) {
		if (PrepareResult(cosemLink, ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED)) {
			return true;
		} else {
			return false;
		}
	} else {
		callback = *methods[methodID].callback;
		callback(cosemLink);
		if ((ACCESS_RESULT_SUCCESS != cosemLink->result) ||
		    (0 == cosemLink->counter)) {
			if (PrepareResult(cosemLink, cosemLink->result)) {
				return true;
			} else {
				return false;
			}
		} else {
			buff = cosemLink->buff;
			u16 cnt = cosemLink->counter;
			while (cnt != 0) {
				cnt--;
				buff[cnt + 6] = buff[cnt];
			}
			buff[0] = APDU_ACTION_RESPONSE;
			buff[1] = RESPONSE_NORMAL;
			buff[2] = cosemLink->invokeID;
			buff[3] = cosemLink->result;
			buff[4] = 1; // return-parameters
			buff[5] = 0; // Data
			cosemLink->counter += 6;
			return true;
		}
	}
}
//----------------------------------------------------------------------------//
bool PrepareSet(cosemLink_t *cosemLink) {
	const struct attribute_desc_s *attrs;
	void (*callback)(void *data, attrAccess_t access);
	u8 attrID;
	attrs = List_DLMS_Obj[cosemLink->objID].attrs;

	for (attrID = 0; attrID < List_DLMS_Obj[cosemLink->objID].attr_elements;
	     attrID++) {
		if ((attrs[attrID].id & 0x3F) == cosemLink->attribute) {
			break;
		}
	}
	if (attrID == List_DLMS_Obj[cosemLink->objID].attr_elements) {
		if (PrepareResult(cosemLink, ACCESS_RESULT_HARDWARE_FAULT)) {
			return true;
		} else {
			return false;
		}
	}
	if (!(checkAccessLevel(cosemLink->assoc, &attrs[attrID].access,
	                       ATTR_ACCESS_WRITE))) {
		if (PrepareResult(cosemLink, ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED)) {
			return true;
		} else {
			return false;
		}
	}
	if (NULL == attrs[attrID].callback) {
		if (PrepareResult(cosemLink, ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED)) {
			return true;
		} else {
			return false;
		}
	} else {
		callback = *attrs[attrID].callback;
		callback(cosemLink, ATTR_ACCESS_WRITE);
		if (PrepareResult(cosemLink, cosemLink->result)) {
			return true;
		} else {
			return false;
		}
	}
}
//----------------------------------------------------------------------------//
OBIS_Id_t FindObj(assoc_t assoc, u8 *obis) { // Поиск объекта DLSM в списке
	OBIS_Id_t OBIS_num = OBIS_ID_END;

	for (u8 i = 0;; i++) { // Поиск OBIS кода
		if (0 == List_DLMS_Obj[i].attr_elements) {
			break; // Если нет аттрибутов(конец списка) выходим
		}
		if (0 == memcmp(List_DLMS_Obj[i].OBIS, obis, 6)) {
			if (0 == (List_DLMS_Obj[i].assoc &
			          assoc)) { // Если не для текущей ассоциации
				OBIS_num = OBIS_ID_UNAVALIBLE;
				continue;
			}
			OBIS_num = (OBIS_Id_t)i;
			break;
		}
	}
	return OBIS_num;
}
//----------------------------------------------------------------------------//
void Start_Cosem_Link(cosemLink_t *cosemLink, assoc_t assoc, u8 select,
                      u8 *parameters, u16 paramLen, u8 obj, u8 attr) {
	cosemLink->assoc = assoc;
	cosemLink->objID = obj;
	cosemLink->attribute = attr;
	cosemLink->access_selector = select;
	cosemLink->access_parameters = parameters;
	cosemLink->len_parameters = paramLen;
	cosemLink->block_number = 0;
	cosemLink->start_addr = 0;
	cosemLink->counter = 0;
	cosemLink->result = ACCESS_RESULT_SUCCESS;
}
//----------------------------------------------------------------------------//
bool CosemProcessing(
    u8 *buff, u16 len,
    cosemLink_t *cosemLink) { // передаем в функцию буфер уже без HDLC оболочки
	u16 cnt = 0;
	assoc_t assoc;
	u8 attr, select, obj;
	u8 invokeID;
	cosemLink->APDU = (APDU_t)buff[cnt++];
	len--;
	switch (cosemLink->APDU) {
	case APDU_GET_REQUEST: {
		cosemLink->typeRequest = (TypeRequest_t)buff[cnt++];
		len--;
		invokeID = buff[cnt++];
		len--;
		switch (cosemLink->typeRequest) {
		case TYPE_REQUEST_NORMAL: {
			cosemLink->invokeID = invokeID;
			u16 ClassID = buff[cnt++];
			ClassID <<= 8;
			ClassID |= buff[cnt++];
			len -= 2;
			u8 obis[6];
			for (u8 i = 0; i < 6; i++) {
				obis[i] = buff[cnt++];
			}
			len -= 6;
			attr = buff[cnt++];
			len--;
			select = buff[cnt++];
			len--;
			switch (select) {
			case 0: {
				if (len != 0) {
					if (PrepareResult(cosemLink,
					                  ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED)) {
						return true;
					} else {
						return false;
					}
				}
			} break;
			case 1:
			case 2: {
				if (len <= 0) {
					if (PrepareResult(cosemLink,
					                  ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED)) {
						return true;
					} else {
						return false;
					}
				}
			} break;
			case 3:
			case 4: {
				if (len <= 0) {
					if (PrepareResult(cosemLink,
					                  ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED)) {
						return true;
					}
					return false;
				}
				if (15 != ClassID) {
					if (PrepareResult(cosemLink,
					                  ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED)) {
						return true;
					} else {
						return false;
					}
				}
			} break;
			default: {
				if (PrepareResult(cosemLink,
				                  ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED)) {
					return true;
				} else {
					return false;
				}
			}
			}
			switch (cosemLink->interface) {
			case INTERFACE_USB:
				assoc = ASSOC_USB;
				break;
			case INTERFACE_SVC:
				assoc = ASSOC_SVC;
				break;
			case INTERFACE_MFR:
				assoc = ASSOC_MFR;
				break;
			default:
				assoc = ASSOC_USB;
				break;
			}
			obj = FindObj(assoc, obis);
			_SYSLOG_("%d,%d,%d,%d,%d,%d", obis[0], obis[1], obis[2], obis[3],
			         obis[4], obis[5]);
			if (obj == 0xFF) {
				if (PrepareResult(cosemLink, ACCESS_RESULT_OBJECT_UNDEFINED)) {
					return true;
				} else {
					return false;
				}
			} else if (obj == 0xFE) {
				if (PrepareResult(cosemLink, ACCESS_RESULT_OBJECT_UNAVAILABLE)) {
					return true;
				} else {
					return false;
				}
			}
			if (List_DLMS_Obj[obj].classID != ClassID) {
				if (PrepareResult(cosemLink,
				                  ACCESS_RESULT_OBJECT_CLASS_INCONSISTENT)) {
					return true;
				} else {
					return false;
				}
			}
			if (List_DLMS_Obj[obj].attr_elements < attr) {
				if (PrepareResult(cosemLink, ACCESS_RESULT_OBJECT_UNDEFINED)) {
					return true;
				} else {
					return false;
				}
			}
			Start_Cosem_Link(cosemLink, assoc, select, (buff + cnt), len, obj,
			                 attr);
			if (PrepareGet(cosemLink)) {
				return true;
			} else {
				return false;
			}
		}
		case TYPE_REQUEST_BLOCK: {
			if (0 == cosemLink->block_number) { // Если номер блока равен 0
				if (PrepareResult(cosemLink,
				                  ACCESS_RESULT_NO_LONG_GET_IN_PROGRESS)) {
					return true; // Отправляем ошибку
				} else {
					return false;
				}
			}
			u32 numberRequestedBlock =
			    DWORD(buff[cnt], buff[cnt + 1], buff[cnt + 2], buff[cnt + 3]);
			len -= 4;
			if ((len != 0) || (cosemLink->invokeID != invokeID) ||
			    (0 == numberRequestedBlock)) {
				if (PrepareResult(cosemLink, ACCESS_RESULT_LONG_GET_ABORTED)) {
					return true;
				} else {
					return false;
				}
			}
			if (numberRequestedBlock !=
			    cosemLink
			        ->block_number) { // Если запрашиваемы блок не равен текущему
				if (PrepareResult(cosemLink, ACCESS_RESULT_LONG_GET_ABORTED)) {
					return true;
				} else {
					return false;
				}
			}
			if (PrepareGetBlock(cosemLink)) {
				return true;
			} else {
				return false;
			}
		}
		default: {
			if (PrepareResult(cosemLink, ACCESS_RESULT_TYPE_UNMATCHED)) {
				return true;
			} else {
				return false;
			}
		}
		}
	}
	case APDU_SET_REQUEST: {
		cosemLink->typeRequest = (TypeRequest_t)buff[cnt++];
		len--;
		invokeID = buff[cnt++];
		len--;
		switch (cosemLink->typeRequest) {
		case TYPE_REQUEST_NORMAL: {
			cosemLink->invokeID = invokeID;
			u16 ClassID = buff[cnt++];
			ClassID <<= 8;
			ClassID |= buff[cnt++];
			len -= 2;
			u8 obis[6];
			for (u8 i = 0; i < 6; i++) {
				obis[i] = buff[cnt++];
			}
			len -= 6;
			attr = buff[cnt++];
			len--;
			select = buff[cnt++];
			len--;
			switch (select) {
			case 0:
				break;
			default: {
				// event_access(cosem_link->comm_port,cosem_link->far_msap, 2); надо
				// узнать что это
				if (PrepareResult(cosemLink,
				                  ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED)) {
					return true;
				} else {
					return false;
				}
			}
			}
			switch (cosemLink->interface) {
			case INTERFACE_USB:
				assoc = ASSOC_USB;
				break;
			case INTERFACE_SVC:
				assoc = ASSOC_SVC;
				break;
			case INTERFACE_MFR:
				assoc = ASSOC_MFR;
				break;
			default:
				assoc = ASSOC_USB;
				break;
			}
			obj = FindObj(assoc, obis);
			_SYSLOG_("%d,%d,%d,%d,%d,%d", obis[0], obis[1], obis[2], obis[3],
			         obis[4], obis[5]);
			if (obj == 0xFF) {
				if (PrepareResult(cosemLink, ACCESS_RESULT_OBJECT_UNDEFINED)) {
					return true;
				} else {
					return false;
				}
			} else if (obj == 0xFE) {
				if (PrepareResult(cosemLink, ACCESS_RESULT_OBJECT_UNAVAILABLE)) {
					return true;
				} else {
					return false;
				}
			}
			if (List_DLMS_Obj[obj].classID != ClassID) {
				if (PrepareResult(cosemLink,
				                  ACCESS_RESULT_OBJECT_CLASS_INCONSISTENT)) {
					return true;
				} else {
					return false;
				}
			}
			if (List_DLMS_Obj[obj].attr_elements < attr) {
				if (PrepareResult(cosemLink, ACCESS_RESULT_OBJECT_UNDEFINED)) {
					return true;
				} else {
					return false;
				}
			}
			Start_Cosem_Link(cosemLink, assoc, select, (buff + cnt), len, obj,
			                 attr);
			if (PrepareSet(cosemLink)) {
				return true;
			} else {
				return false;
			}
		}
		default: {
			if (PrepareResult(cosemLink, ACCESS_RESULT_TYPE_UNMATCHED)) {
				return true;
			} else {
				return false;
			}
		}
		}
	}
	case APDU_ACTION_REQUEST: {
		cosemLink->typeRequest = (TypeRequest_t)buff[cnt++];
		len--;
		invokeID = buff[cnt++];
		len--;
		switch (cosemLink->typeRequest) {
		case TYPE_REQUEST_NORMAL: {
			cosemLink->invokeID = invokeID;
			u16 ClassID = buff[cnt++];
			ClassID <<= 8;
			ClassID |= buff[cnt++];
			len -= 2;
			u8 obis[6];
			for (u8 i = 0; i < 6; i++) {
				obis[i] = buff[cnt++];
			}
			len -= 6;
			attr = buff[cnt++];
			len--;
			select = buff[cnt++];
			len--;
			switch (select) {
			case 0:
			case 1:
				break;
			default: {
				// event_access(cosem_link->comm_port,cosem_link->far_msap, 2);
				if (PrepareResult(cosemLink,
				                  ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED)) {
					return true;
				} else {
					return false;
				}
			}
			}
			switch (cosemLink->interface) {
			case INTERFACE_USB:
				assoc = ASSOC_USB;
				break;
			case INTERFACE_SVC:
				assoc = ASSOC_SVC;
				break;
			case INTERFACE_MFR:
				assoc = ASSOC_MFR;
				break;
			default:
				assoc = ASSOC_USB;
				break;
			}
			obj = FindObj(assoc, obis);
			_SYSLOG_("%d,%d,%d,%d,%d,%d", obis[0], obis[1], obis[2], obis[3],
			         obis[4], obis[5]);
			if (obj == 0xFF) {
				if (PrepareResult(cosemLink, ACCESS_RESULT_OBJECT_UNDEFINED)) {
					return true;
				} else {
					return false;
				}
			} else if (obj == 0xFE) {
				if (PrepareResult(cosemLink, ACCESS_RESULT_OBJECT_UNAVAILABLE)) {
					return true;
				} else {
					return false;
				}
			}
			if (List_DLMS_Obj[obj].classID != ClassID) {
				if (PrepareResult(cosemLink,
				                  ACCESS_RESULT_OBJECT_CLASS_INCONSISTENT)) {
					return true;
				} else {
					return false;
				}
			}
			if (List_DLMS_Obj[obj].method_elements < attr) {
				if (PrepareResult(cosemLink, ACCESS_RESULT_OBJECT_UNDEFINED)) {
					return true;
				} else {
					return false;
				}
			}
			Start_Cosem_Link(cosemLink, assoc, select, (buff + cnt), len, obj,
			                 attr);
			if (PrepareAction(cosemLink)) {
				return true;
			} else {
				return false;
			}
		}
		default: {
			if (PrepareResult(cosemLink, ACCESS_RESULT_TYPE_UNMATCHED)) {
				return true;
			} else {
				return false;
			}
		}
		}
	}
	case APDU_RLRQ: {
		if (PrepareExceptionResponse(
		        cosemLink,
		        EXCEPTION_RESPONSE_CODE_SERVICE_ERROR_SERVICE_NOT_SUPPORTED)) {
			return true;
		} else {
			return false;
		}
	}
	default: {
		if (PrepareExceptionResponse(
		        cosemLink,
		        EXCEPTION_RESPONSE_CODE_STATE_ERROR_SERVICE_NOT_ALLOWED)) {
			return true;
		} else {
			return false;
		}
	}
	}
}
//----------------------------------------------------------------------------//
