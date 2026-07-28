//----------------------------------------------------------------------------//
#include <string.h>
#include "hdlc.h"
#include "log.h"
#include "globals.h"
#include "messages.h"
#include "utils.h"
#include "memory.h"
#include "bootloader.h"
//----------------------------------------------------------------------------//
void Method_Push_Ch1(void *data) {
	_SYSLOG_("[DLMS] Method_Push_Ch1");
}
//----------------------------------------------------------------------------//
void Method_Push_Ch2(void *data) {
	_SYSLOG_("[DLMS] Method_Push_Ch2");
}
//----------------------------------------------------------------------------//
void Method_Push_Ch3(void *data) {
	_SYSLOG_("[DLMS] Method_Push_Ch3");
}
//----------------------------------------------------------------------------//
void Method_Push_Ch4(void *data) {
	_SYSLOG_("[DLMS] Method_Push_Ch4");
}
//----------------------------------------------------------------------------//
void Method_Work_Profile_Reset(void *data) {
	//	Obj_Profile_Reset(data, Reset_Work_Profile, 38);
}
//----------------------------------------------------------------------------//
void Method_Change_Profile_Reset(void *data) {
	//	Obj_Profile_Reset(data, Reset_Change_Profile, 255);
}
//----------------------------------------------------------------------------//
void Method_Diagnostic_Profile_Reset(void *data) {
	//	Obj_Profile_Reset(data, Reset_Diagnostic_Profile, 255);
}
//----------------------------------------------------------------------------//
void Method_Image_Transfer_Initiate(void *data) {
	_SYSLOG_("[DLMS] Method_Image_Transfer_Initiate");
	u32 tmp_u32;
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->access_parameters;

	cosemLink->counter = 0;

	if (0 == SystemInfo.image.transferEnabled) {
		cosemLink->result = ACCESS_RESULT_READ_WRITE_DENIED;
		return;
	}
	cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;

	if ((1 != cosemLink->access_selector) || (TAG_STRUCTURE != buff[0]) ||
	    (2 != buff[1]) || (TAG_OCTET_STRING != buff[2]) || (4 != buff[3]) ||
	    (13 != cosemLink->len_parameters)) {
		return;
	}

	tmp_u32 = DWORD(buff[4], buff[5], buff[6], buff[7]); // Тип железа
	if (tmp_u32 != *SystemInfo.firmwareID) {
		return;
	}

	if (TAG_UINT32 != buff[8]) {
		return;
	}
	tmp_u32 = DWORD(buff[9], buff[10], buff[11], buff[12]); // Размер прошивки
	if (tmp_u32 != MEMORY_UPDATE_SIZE) {
		return;
	}

	memset(SystemInfo.image.transferredBlockStatus, 0,
	       sizeof(SystemInfo.image.transferredBlockStatus));
	SystemInfo.image.transferStatus = IMAGE_TRANSFER_INITIATED;

	cosemLink->result = ACCESS_RESULT_SUCCESS;
}
//----------------------------------------------------------------------------//
void Method_Image_Block_Transfer(void *data) {
	_SYSLOG_("[DLMS] Method_Image_Block_Transfer");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->access_parameters;

	if (0 == SystemInfo.image.transferEnabled) {
		cosemLink->result = ACCESS_RESULT_READ_WRITE_DENIED;
		return;
	}
	cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;

	if ((1 != cosemLink->access_selector) || (TAG_STRUCTURE != buff[0]) ||
	    (2 != buff[1]) || (TAG_UINT32 != buff[2]) ||
	    (TAG_OCTET_STRING != buff[7]) || (0x82 != buff[8]) ||
	    ((SystemInfo.image.blockSize + 11) != cosemLink->len_parameters)) {
		return;
	}
	u32 blockNumber = DWORD(buff[3], buff[4], buff[5], buff[6]); // Номер блока
	if (blockNumber > MEMORY_UPDATE_NUM_PAGE) {
		return;
	}
	u16 len = WORD(buff[9], buff[10]); // длина
	if (len != SystemInfo.image.blockSize) {
		return;
	}

	_SYSLOG_("[DLMS] Method_Image_Block_Transfer - №%u", blockNumber);

	u8 tmpData[MEMORY_UPDATE_DATA_BLOCK_SIZE];
	memory.writeBytes((blockNumber + MEMORY_UPDATE_DEVICE_BUFF_START) *
	                      memory.getPageSize(),
	                  &buff[11], SystemInfo.image.blockSize);
	memory.readBytes((blockNumber + MEMORY_UPDATE_DEVICE_BUFF_START) *
	                     memory.getPageSize(),
	                 tmpData, SystemInfo.image.blockSize);
	if (0 != memcmp(&buff[11], tmpData,
	                MEMORY_UPDATE_DATA_BLOCK_SIZE)) { // Ошибка записи данных
		cosemLink->result = ACCESS_RESULT_HARDWARE_FAULT;
		return;
	}

	u8 byteIndex = blockNumber / 8;
	u8 bitPos = blockNumber % 8;
	if (byteIndex > sizeof(SystemInfo.image.transferredBlockStatus)) {
		return;
	}
	SystemInfo.image.transferredBlockStatus[byteIndex] |= (1 << bitPos);
	if (blockNumber == (MEMORY_UPDATE_NUM_PAGE - 1)) {
		memset(SystemInfo.image.transferredBlockStatus, 0xFF,
		       sizeof(SystemInfo.image.transferredBlockStatus));
	}
	cosemLink->result = ACCESS_RESULT_SUCCESS;
}
//----------------------------------------------------------------------------//
result_t Verify_Image_Header(void) {
	_SYSLOG_("[DLMS] Verify_Image_Header");
	for (u16 i = 0; i < sizeof(SystemInfo.image.transferredBlockStatus);
	     i++) { // Проверка записи всех блоков
		if (0xFF != SystemInfo.image.transferredBlockStatus[i]) {
			return RESULT_ERROR;
		}
	}
	u8 tmpData[256];
	memory.readBytes((MEMORY_UPDATE_DEVICE_BUFF_END)*memory.getPageSize(),
	                 tmpData,
	                 SystemInfo.image.blockSize); // Читаем последний блок

	SystemInfo.image.firmareID =
	    DWORD(tmpData[251], tmpData[250], tmpData[249], tmpData[248]);

	if (SystemInfo.image.firmareID != *SystemInfo.firmwareID) {
		return RESULT_ERROR;
	}
	return RESULT_SUCCESS;
}
//----------------------------------------------------------------------------//
void Method_Image_Verify(void *data) {
	_SYSLOG_("[DLMS] Method_Image_Verify");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->access_parameters;

	if (0 == SystemInfo.image.transferEnabled) {
		cosemLink->result = ACCESS_RESULT_READ_WRITE_DENIED;
		return;
	}
	cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;

	if ((1 != cosemLink->access_selector) || (TAG_INT8 != buff[0]) ||
	    (0 != buff[1]) || (2 != cosemLink->len_parameters)) {
		return;
	}

	if (Verify_Image_Header() != RESULT_SUCCESS) {
		cosemLink->result = ACCESS_RESULT_TEMPORARY_FAILURE;
		SystemInfo.image.transferStatus = IMAGE_TRANSFER_VERIFICATION_FAILED;
	} else {
		cosemLink->result = ACCESS_RESULT_SUCCESS;
		SystemInfo.image.transferStatus = IMAGE_TRANSFER_VERIFICATION_INITIATED;
		SystemInfo.image.fStartVerified =
		    sizeof(SystemInfo.image.transferredBlockStatus) * 8;
	}
}
//----------------------------------------------------------------------------//
void Method_Image_Activate(void *data) {
	_SYSLOG_("[DLMS] Method_Image_Activate");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->access_parameters;
	cosemLink->counter = 0;

	if (0 == SystemInfo.image.transferEnabled) {
		cosemLink->result = ACCESS_RESULT_READ_WRITE_DENIED;
		return;
	}
	cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;

	if ((1 != cosemLink->access_selector) || (2 != cosemLink->len_parameters) ||
	    (TAG_INT8 != buff[0]) || (0 != buff[1])) {
		return;
	}

	if (IMAGE_TRANSFER_VERIFICATION_SUCCESSFUL ==
	    SystemInfo.image.transferStatus) {
		if (RESULT_SUCCESS != Verify_Image_Header()) {
			cosemLink->result = ACCESS_RESULT_TEMPORARY_FAILURE;
			SystemInfo.image.transferStatus = IMAGE_TRANSFER_VERIFICATION_FAILED;
		} else {
			char keyUpd[BOOTLOADER_UPDATE_DEVICE_FLAG_SIZE_MAX];
			memset(keyUpd, 0, BOOTLOADER_UPDATE_DEVICE_FLAG_SIZE_MAX);
			strncpy(keyUpd, BOOTLOADER_UPDATE_DEVICE_KEY,
			        BOOTLOADER_UPDATE_DEVICE_FLAG_SIZE_MAX);
			if (RESULT_SUCCESS !=
			    Memory_Write_And_Update_Status(MEMORY_UPDATE_DEVICE_MD5,
			                                   SystemInfo.image.calcMD5,
			                                   sizeof(SystemInfo.image.calcMD5))) {
				cosemLink->result = ACCESS_RESULT_HARDWARE_FAULT;
				SystemInfo.image.transferStatus = IMAGE_TRANSFER_ACTIVATEION_FAILED;
				return;
			}
			if (RESULT_SUCCESS != Memory_Write_And_Update_Status(
			                          MEMORY_UPDATE_DEVICE_FLAG, keyUpd,
			                          BOOTLOADER_UPDATE_DEVICE_FLAG_SIZE_MAX)) {
				cosemLink->result = ACCESS_RESULT_HARDWARE_FAULT;
				SystemInfo.image.transferStatus = IMAGE_TRANSFER_ACTIVATEION_FAILED;
				return;
			} else {
				cosemLink->result = ACCESS_RESULT_SUCCESS;
				SystemInfo.image.transferStatus =
				    IMAGE_TRANSFER_ACTIVATEION_INITIATED;
				SystemInfo.timeout.forcedReboot = 1;
				tim.reset(TIMER_FORCED_REBOOT_DEVICE);
			}
		}
	} else {
		return;
	}
}
//----------------------------------------------------------------------------//
void Method_Device_Reboot(void *data) {
	_SYSLOG_("[DLMS] Method_Device_Reboot");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	buff = cosemLink->access_parameters;
	cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
	cosemLink->counter = 0;

	if (5 != cosemLink->len_parameters) {
		return;
	}
	if (TAG_UINT32 != buff[0]) {
		return;
	}
	u32 tmp = DWORD(buff[1], buff[2], buff[3], buff[4]);
	if (tmp > TIMEOUT_FORCED_REBOOT_DEVICE_MAX) {
		return;
	}
	SystemInfo.timeout.forcedReboot = tmp * 1000;
	tim.reset(TIMER_FORCED_REBOOT_DEVICE);
	cosemLink->result = ACCESS_RESULT_SUCCESS;
}
//----------------------------------------------------------------------------//
void Method_Settings_Reset(void *data) {
	_SYSLOG_("[DLMS] Method_Settings_Reset");
	cosemLink_t *cosemLink;
	cosemLink = data;

	cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;
	cosemLink->counter = 0;

	if (0 != cosemLink->len_parameters) {
		return;
	}
	u32 tmp = 0xFFFFFFFF;
	u8 err = 0;
	if (RESULT_ERROR ==
	    Memory_Write_And_Update_Status(MEMORY_METER_NUMBER, &tmp,
	                                   sizeof(SystemInfo.numMeter))) {
		_SYSLOG_("Config - Error Write Meter Number");
		err++;
	}
	if (RESULT_ERROR ==
	    memory.readTwoCopyesWithCRC(MEMORY_METER_NUMBER, &SystemInfo.numMeter,
	                                sizeof(SystemInfo.numMeter))) {
		_SYSLOG_("Config - Error Read Meter Number");
		err++;
	}
	if (RESULT_ERROR == Memory_Write_And_Update_Status(
	                        MEMORY_PINCODE, &tmp, sizeof(SystemInfo.pinCode))) {
		_SYSLOG_("Config - Error Write Meter Number");
		err++;
	}
	if (RESULT_ERROR ==
	    memory.readTwoCopyesWithCRC(MEMORY_PINCODE, &SystemInfo.pinCode,
	                                sizeof(SystemInfo.pinCode))) {
		_SYSLOG_("Config - Error Read Meter Number");
		err++;
	}
	if (err++) {
		cosemLink->result = ACCESS_RESULT_HARDWARE_FAULT;
	} else {
		cosemLink->result = ACCESS_RESULT_SUCCESS;
	}
}

//----------------------------------------------------------------------------//
#define TIMEOUT_SERVICE_MODE_ON_MAX 49 * 60 * 60 // 49 суток

void Method_Service_Mode_Entering(void *data) {
	_SYSLOG_("[DLMS] Method_Service_Mode_Entering");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	buff = cosemLink->access_parameters;
	cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;

	cosemLink->counter = 0;

	if (25 != cosemLink->len_parameters) {
		return;
	}
	if ((TAG_STRUCTURE != buff[0]) || (2 != buff[1]) ||
	    (TAG_VISIBLE_STRING != buff[2]) || (16 != buff[3]) ||
	    (TAG_UINT32 != buff[20])) {
		return;
	}
	if (0 !=
	    memcmp((buff + 4), SystemInfo.password, sizeof(SystemInfo.password))) {
		return; // Проверяем пароль
	}
	u32 tmp = DWORD(buff[21], buff[22], buff[23], buff[24]);
	if (tmp > TIMEOUT_SERVICE_MODE_ON_MAX) {
		return;
	}

	SystemInfo.timeout.serviceModeOn = tmp * 1000;
	SystemInfo.flag.serviceMode = true; // Переход в сервисный режим
	tim.reset(TIMER_SERVICE_MODE_ON);
	cosemLink->result = ACCESS_RESULT_SUCCESS;
}
//----------------------------------------------------------------------------//
void Method_Service_Mode_Exit(void *data) {
	_SYSLOG_("[DLMS] Method_Service_Mode_Exit");
	cosemLink_t *cosemLink;
	cosemLink = data;

	SystemInfo.timeout.serviceModeOn = 0;
	SystemInfo.flag.serviceMode = false; // Выход из сервисного режима
	cosemLink->result = ACCESS_RESULT_SUCCESS;
}
//----------------------------------------------------------------------------//
#define TIMEOUT_LOG_ON_MAX 49 * 60 * 60 // 49 суток
void Method_Service_Mode_Log_On(void *data) {
	_SYSLOG_("[DLMS] Method_Service_Mode_Log_On");
	cosemLink_t *cosemLink;
	cosemLink = data;
	u8 *buff = cosemLink->buff;

	buff = cosemLink->access_parameters;
	cosemLink->result = ACCESS_RESULT_SCOPE_OF_ACCESS_VIOLATED;

	if (5 != cosemLink->len_parameters) {
		return;
	}
	if (TAG_UINT32 != buff[0]) {
		return;
	}
	u32 tmp = DWORD(buff[1], buff[2], buff[3], buff[4]);
	if (tmp > TIMEOUT_LOG_ON_MAX) {
		return;
	}

	SystemInfo.timeout.logOn = tmp * 1000;
	tim.reset(TIMER_LOG_ON);

	SystemInfo.flag.logOn = true; // Включение лога
	cosemLink->result = ACCESS_RESULT_SUCCESS;
}
//----------------------------------------------------------------------------//
void Method_Service_Mode_Log_Off(void *data) {
	_SYSLOG_("[DLMS] Method_Service_Mode_Log_Off");
	cosemLink_t *cosemLink;
	cosemLink = data;

	SystemInfo.timeout.logOn = 0;
	SystemInfo.flag.logOn = false; // Выключение лога
	cosemLink->result = ACCESS_RESULT_SUCCESS;
}
//----------------------------------------------------------------------------//
