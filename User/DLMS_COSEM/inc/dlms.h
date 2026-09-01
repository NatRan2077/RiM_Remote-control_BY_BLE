//----------------------------------------------------------------------------//
#ifndef INCDLMSLISTOBJ_H
#define INCDLMSLISTOBJ_H
//----------------------------------------------------------------------------//
#include "obis.h"
#include "cosem.h"
//----------------------------------------------------------------------------//
typedef enum {
	OBIS_ID_DEVICE_LOGICAL_NAME,
	OBIS_ID_ASSOCIATION_MFR,
	OBIS_ID_ASSOCIATION_USB,
	OBIS_ID_ASSOCIATION_SVC,
	OBIS_ID_DIVICE_SERIAL_NUMBER,
	OBIS_ID_DIVICE_TYPE,
	OBIS_ID_FIRMWARE_VERSION,
	OBIS_ID_MANUFACTURER_NAME,
	OBIS_ID_DATA_OF_PRODUCTION,
	OBIS_ID_IPUE_MS_PROTOCOL_VERSION,
	OBIS_ID_DEVICE_MODEL_ID,
	OBIS_ID_VOLTAGE_3V3,
	OBIS_ID_VOLTAGE_USB,
	OBIS_ID_VOLTAGE_BATT,
	OBIS_ID_TEMPERATURE,
	OBIS_ID_OPERATING_TIME,
	OBIS_ID_CNT_STARTS,
	OBIS_ID_FIRMWARE_IDENTIFIER,
	OBIS_ID_FIRMWARE_VERSION_IDENTIFIER,
	OBIS_ID_FIRMWARE_SIGNATURE,
	OBIS_ID_CHK_SIGNATURE,
	OBIS_ID_CHK_ADDRESS,
	OBIS_OD_IMAGE_TRANSFER,

	OBIS_ID_DEVICE_REBOOT,
	OBIS_ID_SERVICE_MODE_PASSWORD,
	OBIS_ID_DATETIME_COMPILATION_FW,
	OBIS_ID_CNT_BTN_1,
	OBIS_ID_CNT_BTN_2,
	OBIS_ID_LCD_DIAG,
	OBIS_ID_SETTINGS_RESET,
	OBIS_ID_BLE_CURRENT_STATUS,

	OBIS_SP_VERSION,
	OBIS_PINCODE,
	OBIS_METER_NUMBER,
	OBIS_ID_LIST_SIZE,
	OBIS_ID_UNAVALIBLE = 0xFE,
	OBIS_ID_END = 0xFF,
} OBIS_Id_t;
//----------------------------------------------------------------------------//
typedef void (*exchange_data_t)(void *data, attrAccess_t access);
typedef void (*method_t)(void *data);
//----------------------------------------------------------------------------//
// Уровни доступа
typedef enum {
	N = 0,     // No access
	R = 0x01,  // Read
	W = 0x02,  // Write
	RW = 0x03, // Read + Write
} accessLevel_t;
//----------------------------------------------------------------------------//
// Права доступа
typedef struct {
	accessLevel_t mfr;
	accessLevel_t usb;
	accessLevel_t svc;
} accessPermissions_t;
//----------------------------------------------------------------------------//
struct attribute_desc_s {
	u8 id;                      // Номер аттрибута
	accessPermissions_t access; // Права доступа
	cosemTag_t type;            // Тип данных
	u8 *addr;                   // Указатель на данные
	exchange_data_t callback;   // Указатель на функцию обработки
};
//----------------------------------------------------------------------------//
struct method_desc_s {
	u8 id;                      // Номер метода
	accessPermissions_t access; // Права доступа
	method_t callback;          // Указатель на функцию обработки
};
//----------------------------------------------------------------------------//
typedef struct {
	assoc_t assoc;                        // Ассоциация
	class_id_t classID;                   // Класс объекта
	u8 version;                           // Версия
	u8 OBIS[6];                           // OBIS-код
	u8 attr_elements;                     // Число аттрибутов
	const struct attribute_desc_s *attrs; // Указатель на струтуру с аттрибутами
	uint8_t method_elements;              // Число методов
	const struct method_desc_s *methods;  // Указатель на структуру с методами
} DLMS_Obj_t;                            // Объект DLSM
extern const DLMS_Obj_t List_DLMS_Obj[];
//----------------------------------------------------------------------------//
#endif
