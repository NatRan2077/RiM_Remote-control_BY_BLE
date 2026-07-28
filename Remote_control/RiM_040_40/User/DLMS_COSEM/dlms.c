//----------------------------------------------------------------------------//
#include <stdio.h>
#include <string.h>
#include "common_types.h"
#include "dlms.h"
#include "obis.h"
#include "globals.h"
#include "DLMS_COSEM_Handlers_Exchanges.h"
#include "DLMS_COSEM_Handlers_Methods.h"
//----------------------------------------------------------------------------//
#define ACCESS_RIGHTS(...) ((accessPermissions_t){__VA_ARGS__})
//----------------------------------------------------------------------------//
#define SCALER_TEMPERATURE -2
#define SCALER_VOLTAGE -3
#define SCALER_TIME_SEC 0

#define INJECT8(x) ((uint16_t)(x) & 0xFF)
#define INJECT16(x) (((uint16_t)(x) >> 8) & 0xFF), ((uint16_t)(x) & 0xFF)
#define INJECT16X(x) ((uint16_t)(x) & 0xFF), (((uint16_t)(x) >> 8) & 0xFF)
/* Client SAP */ /* Server SAP */
const uint8_t partner_HS[] = {INJECT16X(6), 2,          TAG_INT8,
                              0x30,         TAG_UINT16, INJECT16(1)};
static const uint8_t security_setup_none[] = {6, 0, 0, 43, 0, 0, 255};
//----------------------------------------------------------------------------//
#define DLMS_PROTOCOL_VERSION 6
//----------------------------------------------------------------------------//
// Битовая маска для всех возможных функций (Conformance Block)
typedef enum {
	// Байт 1 (0-7 биты)
	IEC62056_RESERVED0 = (1 << 7),
	IEC62056_RESERVED1 = (1 << 6),
	IEC62056_RESERVED2 = (1 << 5),
	IEC62056_READ = (1 << 4),
	IEC62056_WRITE = (1 << 3),
	IEC62056_UNCONFIRMED_WRITE = (1 << 2),
	IEC62056_RESERVED3 = (1 << 1),
	IEC62056_RESERVED4 = (1 << 0),

	// Байт 2 (8-15 биты)
	IEC62056_ATTR0_SET_SUPPORT = (1 << 15),
	IEC62056_PRIORITY_MGMT = (1 << 14),
	IEC62056_ATTR0_GET_SUPPORT = (1 << 13),
	IEC62056_BLOCK_GET = (1 << 12),
	IEC62056_BLOCK_SET = (1 << 11),
	IEC62056_BLOCK_ACTION = (1 << 10),
	IEC62056_MULTIPLE_REFERENCES = (1 << 9),
	IEC62056_INFO_REPORT = (1 << 8),

	// Байт 3 (16-23 биты)
	IEC62056_RESERVED5 = (1 << 23),
	IEC62056_RESERVED6 = (1 << 22),
	IEC62056_PARAMETERIZED_ACCESS = (1 << 21),
	IEC62056_GET = (1 << 20),
	IEC62056_SET = (1 << 19),
	IEC62056_SELECTIVE_ACCESS = (1 << 18),
	IEC62056_EVENT_NOTIFICATION = (1 << 17),
	IEC62056_ACTION = (1 << 16),

	// Шифрование (если нужно)
	IEC62056_GLOBAL_CIPHER = (1 << 30),
	IEC62056_DEDICATED_CIPHER = (int)0x80000000 //(1 << 31)
} IEC62056_ConformanceFlags;
//----------------------------------------------------------------------------//
#define CONFIG_A0 (IEC62056_BLOCK_GET | IEC62056_GET)
#define CONFIG_A1                                                              \
	(IEC62056_BLOCK_GET | IEC62056_GET | IEC62056_SET |                         \
	 IEC62056_SELECTIVE_ACCESS | IEC62056_ACTION)
#define CONFIG_A2                                                              \
	(IEC62056_BLOCK_GET | IEC62056_GET | IEC62056_SET |                         \
	 IEC62056_SELECTIVE_ACCESS | IEC62056_EVENT_NOTIFICATION | IEC62056_ACTION)
#define CONFIG_A3                                                              \
	(IEC62056_BLOCK_GET | IEC62056_GET | IEC62056_SET |                         \
	 IEC62056_SELECTIVE_ACCESS | IEC62056_ACTION)
//----------------------------------------------------------------------------//
// Массив acceptable_conformance[4][3] (4 ассоциации ? 3 байта)
const uint8_t acceptable_conformance[4][3] = {
    {BYTE_2(CONFIG_A0), BYTE_1(CONFIG_A0), BYTE_0(CONFIG_A0)}, // A0
    {BYTE_2(CONFIG_A1), BYTE_1(CONFIG_A1), BYTE_0(CONFIG_A1)}, // A1
    {BYTE_2(CONFIG_A2), BYTE_1(CONFIG_A2), BYTE_0(CONFIG_A2)}, // A2
    {BYTE_2(CONFIG_A3), BYTE_1(CONFIG_A3), BYTE_0(CONFIG_A3)}  // A3
};
//----------------------------------------------------------------------------//
// Пример использования в context_info_HS (для A2)
static const uint8_t context_info_HS[] = {
    INJECT16X(18), // Длина данных
    6,             // Количество элементов (?)

    TAG_BITSTRING, 24, // Conformance block (3 байта)

    // Байты конфигурации для A2
    BYTE_2(CONFIG_A2), // Байт 1 (16-23 биты)
    BYTE_1(CONFIG_A2), // Байт 2 (8-15 биты)
    BYTE_0(CONFIG_A2), // Байт 3 (0-7 биты)

    TAG_UINT16, INJECT16(APDU_SIZE_MAX), // Max Rx PDU
    TAG_UINT16, INJECT16(APDU_SIZE_MAX), // Max Tx PDU
    TAG_UINT8, DLMS_PROTOCOL_VERSION,    // Версия DLMS
    TAG_INT8, 0,                         // QoS
    TAG_OCTET_STRING, 0x00               // Шифрование (нет)
};
//----------------------------------------------------------------------------//
enum { NON_ASSOCIATED, ASSOCIATION_PENDING, ASSOCIATED };
//----------------------------------------------------------------------------//
const uint8_t curr_assoc_status = ASSOCIATED;
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Device_Logical_Name[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_DEVICE_LOGICAL_NAME].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING, NULL,
     Exchange_Device_Logical_Name},
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Association_MFR[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = N), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_ASSOCIATION_MFR].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = N), TAG_OCTET_STRING, NULL,
     Exchange_Association},
    {3, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = N), TAG_STRUCTURE,
     (void *)partner_HS, NULL},
    {4, ACCESS_RIGHTS(.mfr = N, .usb = N, .svc = N), TAG_OCTET_STRING,
     (void *)security_setup_none, NULL},
    {5, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = N), TAG_STRUCTURE,
     (void *)&context_info_HS, NULL},
    {6, ACCESS_RIGHTS(.mfr = N, .usb = N, .svc = N), TAG_OCTET_STRING,
     (void *)security_setup_none, NULL},
    {7, ACCESS_RIGHTS(.mfr = N, .usb = N, .svc = N), TAG_OCTET_STRING,
     (void *)security_setup_none, NULL},
    {8, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = N), TAG_ENUM,
     (void *)&curr_assoc_status, NULL},
    {9, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = N), TAG_OCTET_STRING,
     (void *)security_setup_none, NULL},
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Association_USB[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_ASSOCIATION_USB].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING, NULL,
     Exchange_Association},
    {3, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_STRUCTURE,
     (void *)partner_HS, NULL},
    {4, ACCESS_RIGHTS(.mfr = N, .usb = N, .svc = R), TAG_OCTET_STRING,
     (void *)security_setup_none, NULL},
    {5, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_STRUCTURE,
     (void *)&context_info_HS, NULL},
    {6, ACCESS_RIGHTS(.mfr = N, .usb = N, .svc = R), TAG_OCTET_STRING,
     (void *)security_setup_none, NULL},
    {7, ACCESS_RIGHTS(.mfr = N, .usb = N, .svc = R), TAG_OCTET_STRING,
     (void *)security_setup_none, NULL},
    {8, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_ENUM,
     (void *)&curr_assoc_status, NULL},
    {9, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)security_setup_none, NULL},
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Association_SVC[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_ASSOCIATION_SVC].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_OCTET_STRING, NULL,
     Exchange_Association},
    {3, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_STRUCTURE,
     (void *)partner_HS, NULL},
    {4, ACCESS_RIGHTS(.mfr = N, .usb = N, .svc = R), TAG_OCTET_STRING,
     (void *)security_setup_none, NULL},
    {5, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_STRUCTURE,
     (void *)&context_info_HS, NULL},
    {6, ACCESS_RIGHTS(.mfr = N, .usb = N, .svc = R), TAG_OCTET_STRING,
     (void *)security_setup_none, NULL},
    {7, ACCESS_RIGHTS(.mfr = N, .usb = N, .svc = R), TAG_OCTET_STRING,
     (void *)security_setup_none, NULL},
    {8, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_ENUM,
     (void *)&curr_assoc_status, NULL},
    {9, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_OCTET_STRING,
     (void *)security_setup_none, NULL},
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Device_Serial_Number[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_DIVICE_SERIAL_NUMBER].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = RW, .usb = R, .svc = R), TAG_VISIBLE_STRING, NULL,
     Exchange_Device_Serial_Number},
};
//----------------------------------------------------------------------------//
const uint8_t Device_Type_Const[] = {14,  'R', 'E', 'M', 'O', 'T', 'E', '_',
                                     'D', 'I', 'S', 'P', 'L', 'A', 'Y'};
static const struct attribute_desc_s Obj_Device_Type[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_DIVICE_TYPE].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_VISIBLE_STRING,
     (void *)Device_Type_Const, NULL},
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Firmware_Version[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_FIRMWARE_VERSION].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_VISIBLE_STRING, NULL,
     Exchange_Firmware_Version},
};
//----------------------------------------------------------------------------//
const uint8_t Manufacturer_Name_Const[] = {6, 'A', 'O', ' ', 'R', 'i', 'M'};
static const struct attribute_desc_s Obj_Manufacturer_Name[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_MANUFACTURER_NAME].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_VISIBLE_STRING,
     (void *)Manufacturer_Name_Const, NULL},
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Date_Of_Production[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_DATA_OF_PRODUCTION].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = RW, .usb = R, .svc = R), TAG_DATE, NULL,
     Exchange_Date_Of_Production},
};
//----------------------------------------------------------------------------//
const uint8_t IPUE_MS_Protocol_Version_Const[] = {4, '1', '.', '0', '0'};
static const struct attribute_desc_s Obj_IPUE_MS_Protocol_Version[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_IPUE_MS_PROTOCOL_VERSION].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_VISIBLE_STRING,
     (void *)IPUE_MS_Protocol_Version_Const, NULL},
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Device_Model_Id[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_DEVICE_MODEL_ID].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_VISIBLE_STRING, NULL,
     Exchange_Device_Model_Id},
};
//----------------------------------------------------------------------------//
const u32 Profile_Capture_Period_0 = 0;
const u8 Profile_Sort_Method_FIFO = 1;
const u8 Profile_No_Sort_Object[] = {INJECT16X(16 + 1),
                                     4,
                                     TAG_UINT16,
                                     INJECT16(0),
                                     TAG_OCTET_STRING,
                                     6,
                                     0,
                                     0,
                                     0,
                                     0,
                                     0,
                                     0,
                                     TAG_INT8,
                                     0,
                                     TAG_UINT16,
                                     INJECT16(0)};
//----------------------------------------------------------------------------//
#define OBIS_GROUP_A_ABSTRACT_OBJECTS 0
#define OBIS_ABSTRACT_GROUP_C_GENERAL_SERVICE_ENTRIES 96
#define OBIS_ABSTRACT_SERVICE_ENTRIES_GROUP_D_OPERATING_TIME 8
const u8 Work_Profile_Capture_Objects[] = {
    INJECT16X(3 * 18 + 1),
    /*TAG_ARRAY,*/ 3,
    TAG_STRUCTURE,
    4,
    TAG_UINT16,
    INJECT16(CLASS_ID_CLOCK),
    TAG_OCTET_STRING,
    6,
    OBIS_GROUP_A_ABSTRACT_OBJECTS,
    0,
    1,
    0,
    0,
    255, // Date & Time
    TAG_INT8,
    2,
    TAG_UINT16,
    INJECT16(0),
    TAG_STRUCTURE,
    4,
    TAG_UINT16,
    INJECT16(CLASS_ID_DATA),
    TAG_OCTET_STRING,
    6,
    OBIS_GROUP_A_ABSTRACT_OBJECTS,
    0,
    OBIS_ABSTRACT_GROUP_C_GENERAL_SERVICE_ENTRIES,
    11,
    2,
    255, // Event code object 2
    TAG_INT8,
    2,
    TAG_UINT16,
    INJECT16(1),
    TAG_STRUCTURE,
    4,
    TAG_UINT16,
    INJECT16(CLASS_ID_REGISTER),
    TAG_OCTET_STRING,
    6,
    OBIS_GROUP_A_ABSTRACT_OBJECTS,
    0,
    OBIS_ABSTRACT_GROUP_C_GENERAL_SERVICE_ENTRIES,
    OBIS_ABSTRACT_SERVICE_ENTRIES_GROUP_D_OPERATING_TIME,
    0,
    255,
    TAG_INT8,
    2,
    TAG_UINT16,
    INJECT16(0)};
//----------------------------------------------------------------------------//
const u8 scalar_unit_voltage[] = {INJECT16X(5), 2,
                                  TAG_INT8,     INJECT8(SCALER_VOLTAGE),
                                  TAG_ENUM,     OBIS_UNIT_VOLTAGE_VOLT};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Voltage_3V5[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_VOLTAGE_3V3].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_UINT16, NULL,
     Exchange_Voltage_3V5},
    {3, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_STRUCTURE,
     (void *)&scalar_unit_voltage, NULL}};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Voltage_USB[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_VOLTAGE_USB].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_UINT16, NULL,
     Exchange_Voltage_USB},
    {3, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_STRUCTURE,
     (void *)&scalar_unit_voltage, NULL}};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Voltage_Batt[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_VOLTAGE_BATT].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_UINT16, NULL,
     Exchange_Voltage_Batt},
    {3, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_STRUCTURE,
     (void *)&scalar_unit_voltage, NULL}};
//----------------------------------------------------------------------------//
const u8 scalar_unit_temperature[] = {
    INJECT16X(5), 2,
    TAG_INT8,     INJECT8(SCALER_TEMPERATURE),
    TAG_ENUM,     OBIS_UNIT_TEMPERATURE_DEGREE_CELSIUS};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Temperature[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_TEMPERATURE].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_INT16, NULL,
     Exchange_Temperature},
    {3, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_STRUCTURE,
     (void *)&scalar_unit_temperature, NULL}};
//----------------------------------------------------------------------------//
const uint8_t scalar_unit_time_sec[] = {INJECT16X(5), 2,
                                        TAG_INT8,     INJECT8(SCALER_TIME_SEC),
                                        TAG_ENUM,     OBIS_UNIT_TIME_SECOND};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Operating_Time[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_OPERATING_TIME].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_UINT32, NULL,
     Exchange_Operating_Time},
    {3, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_STRUCTURE,
     (void *)&scalar_unit_time_sec, NULL}};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Cnt_Starts[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_CNT_STARTS].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_UINT32, NULL,
     Exchange_Cnt_Starts},
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Firmware_Identifier[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_FIRMWARE_IDENTIFIER].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING, NULL,
     Exchange_Firmware_Identifier},
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Firmware_Version_Identifier[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_FIRMWARE_VERSION_IDENTIFIER].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING, NULL,
     Exchange_Firmware_Version_Identifier},
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Firmware_Signature[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_FIRMWARE_SIGNATURE].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING, NULL,
     Exchange_Firmware_Signature},
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_CHK_Signature[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_CHK_SIGNATURE].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING, NULL,
     Exchange_CHK_Signature},
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_CHK_Address[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_CHK_ADDRESS].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = RW, .usb = RW, .svc = RW), TAG_STRUCTURE, NULL,
     Exchange_CHK_Address},
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Image_Transfer[] = {
    {1, ACCESS_RIGHTS(.usb = R, .svc = R, .mfr = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_OD_IMAGE_TRANSFER].OBIS, NULL},
    {2, ACCESS_RIGHTS(.usb = R, .svc = R, .mfr = R), TAG_UINT32,
     (void *)&(SystemInfo.image.blockSize), NULL},
    {3, ACCESS_RIGHTS(.usb = R, .svc = R, .mfr = R), TAG_BITSTRING, NULL,
     Exchange_Image_Blocks_Status},
    {4, ACCESS_RIGHTS(.usb = R, .svc = R, .mfr = R), TAG_UINT32, NULL,
     Exchange_Not_Transfer_Block_Number},
    {5, ACCESS_RIGHTS(.usb = RW, .svc = RW, .mfr = RW), TAG_BOOLEAN, NULL,
     Exchange_Image_Transfer_Enabled},
    {6, ACCESS_RIGHTS(.usb = R, .svc = R, .mfr = R), TAG_ENUM, NULL,
     Exchange_Image_Transfer_Status},
    {7, ACCESS_RIGHTS(.usb = R, .svc = R, .mfr = R), TAG_ARRAY, NULL,
     Exchange_Image_Info}};
//----------------------------------------------------------------------------//
static const struct method_desc_s Obj_Image_Transfer_Methods[] = {
    {1, ACCESS_RIGHTS(.usb = R, .svc = R, .mfr = R),
     Method_Image_Transfer_Initiate},
    {2, ACCESS_RIGHTS(.usb = R, .svc = R, .mfr = R),
     Method_Image_Block_Transfer},
    {3, ACCESS_RIGHTS(.usb = R, .svc = R, .mfr = R), Method_Image_Verify},
    {4, ACCESS_RIGHTS(.usb = R, .svc = R, .mfr = R), Method_Image_Activate}};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Device_Reboot[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_DEVICE_REBOOT].OBIS, NULL},
};
//----------------------------------------------------------------------------//
static const struct method_desc_s Obj_Device_Reboot_Method[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), Method_Device_Reboot}};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Service_Mode_Password[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_SERVICE_MODE_PASSWORD].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = RW, .usb = N, .svc = N), TAG_VISIBLE_STRING, NULL,
     Exchange_Service_Mode_Password},
};
//----------------------------------------------------------------------------//
static const struct method_desc_s Obj_Service_Mode_Entering_Method[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R),
     Method_Service_Mode_Entering},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), Method_Service_Mode_Exit},
    {3, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R),
     Method_Service_Mode_Log_On},
    {4, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R),
     Method_Service_Mode_Log_Off}};

//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_DateTime_Compilation_FW[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_DATETIME_COMPILATION_FW].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_DATETIME, NULL,
     Exchange_DateTime_Compilation_FW},
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Cnt_Btn_1[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_CNT_BTN_1].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_UINT32, NULL,
     Exchange_Cnt_Btn_1},
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Cnt_Btn_2[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_CNT_BTN_2].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_UINT32, NULL,
     Exchange_Cnt_Btn_2},
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_LCD_Diag[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_LCD_DIAG].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = RW, .usb = N, .svc = RW), TAG_ENUM, NULL,
     Exchange_LCD_Diag},
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Settings_Reset[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_SETTINGS_RESET].OBIS, NULL},
};
//----------------------------------------------------------------------------//
static const struct method_desc_s Obj_Settings_Reset_Method[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), Method_Settings_Reset}};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_BLE_Current_Status[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_ID_BLE_CURRENT_STATUS].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_ENUM, NULL,
     Exchange_BLE_Current_Status},
    {3, ACCESS_RIGHTS(.mfr = R, .usb = N, .svc = R), TAG_INT8, NULL,
     Exchange_BLE_RSSI},
};
//----------------------------------------------------------------------------//
const u8 Service_Profile_Capture_Objects[] = {
    INJECT16X(4 * 18 + 1),
    /*TAG_ARRAY,*/ 4,
    TAG_STRUCTURE,
    4,
    TAG_UINT16,
    INJECT16(CLASS_ID_CLOCK),
    TAG_OCTET_STRING,
    6,
    OBIS_GROUP_A_ABSTRACT_OBJECTS,
    0,
    1,
    0,
    0,
    255, // Date & Time
    TAG_INT8,
    2,
    TAG_UINT16,
    INJECT16(0),
    TAG_STRUCTURE,
    4,
    TAG_UINT16,
    INJECT16(CLASS_ID_DATA),
    TAG_OCTET_STRING,
    6,
    OBIS_GROUP_A_ABSTRACT_OBJECTS,
    0,
    OBIS_ABSTRACT_GROUP_C_GENERAL_SERVICE_ENTRIES,
    11,
    3,
    255, // Event code object 3
    TAG_INT8,
    2,
    TAG_UINT16,
    INJECT16(1),
    TAG_STRUCTURE,
    4,
    TAG_UINT16,
    INJECT16(CLASS_ID_DATA),
    TAG_OCTET_STRING,
    6,
    OBIS_GROUP_A_ABSTRACT_OBJECTS,
    0,
    OBIS_ABSTRACT_GROUP_C_GENERAL_SERVICE_ENTRIES,
    12,
    4,
    255, // Channel
    TAG_INT8,
    2,
    TAG_UINT16,
    INJECT16(0),
    TAG_STRUCTURE,
    4,
    TAG_UINT16,
    INJECT16(CLASS_ID_REGISTER),
    TAG_OCTET_STRING,
    6,
    OBIS_GROUP_A_ABSTRACT_OBJECTS,
    0,
    OBIS_ABSTRACT_GROUP_C_GENERAL_SERVICE_ENTRIES,
    OBIS_ABSTRACT_SERVICE_ENTRIES_GROUP_D_OPERATING_TIME,
    0,
    255,
    TAG_INT8,
    2,
    TAG_UINT16,
    INJECT16(0)};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_SP_Version[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_SP_VERSION].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING, NULL,
     Exchange_SP_Version}, // Версия протокола СПОДЭС
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_Meter_Number[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_METER_NUMBER].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = RW, .usb = RW, .svc = RW), TAG_UINT32, NULL,
     Exchange_Meter_Number}, // Номер счетчика
};
//----------------------------------------------------------------------------//
static const struct attribute_desc_s Obj_PinCode[] = {
    {1, ACCESS_RIGHTS(.mfr = R, .usb = R, .svc = R), TAG_OCTET_STRING,
     (void *)List_DLMS_Obj[OBIS_PINCODE].OBIS, NULL},
    {2, ACCESS_RIGHTS(.mfr = RW, .usb = RW, .svc = RW), TAG_UINT32, NULL,
     Exchange_PinCode}, // Пин код
};
//----------------------------------------------------------------------------//
#define ASSOC(x) ((assoc_t)(x))
#define MFR ASSOC_MFR
#define USB ASSOC_USB
#define SVC ASSOC_SVC
//----------------------------------------------------------------------------//
const DLMS_Obj_t List_DLMS_Obj[] = {
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 42, 0, 0, 255},
     2,
     Obj_Device_Logical_Name,
     0,
     NULL},
    {ASSOC(MFR),
     CLASS_ID_ASSOCIATION_LN,
     1,
     {0, 0, 40, 0, 0, 255},
     9,
     Obj_Association_MFR,
     0,
     NULL}, // Ассоциация MFR
    {ASSOC(USB),
     CLASS_ID_ASSOCIATION_LN,
     1,
     {0, 0, 40, 0, 0, 255},
     9,
     Obj_Association_USB,
     0,
     NULL}, // Ассоциация USB
    {ASSOC(SVC),
     CLASS_ID_ASSOCIATION_LN,
     1,
     {0, 0, 40, 0, 0, 255},
     9,
     Obj_Association_SVC,
     0,
     NULL}, // Ассоциация SVC
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 1, 150, 255},
     2,
     Obj_Device_Serial_Number,
     0,
     NULL}, // Серийный номер
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 1, 151, 255},
     2,
     Obj_Device_Type,
     0,
     NULL}, // Тип модуля
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 1, 152, 255},
     2,
     Obj_Firmware_Version,
     0,
     NULL}, // Версия ПО
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 1, 153, 255},
     2,
     Obj_Manufacturer_Name,
     0,
     NULL}, // Наимениание производетеля
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 1, 154, 255},
     2,
     Obj_Date_Of_Production,
     0,
     NULL}, // Дата выпуска
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 1, 156, 255},
     2,
     Obj_IPUE_MS_Protocol_Version,
     0,
     NULL}, // Версия протокола ИПУЭ-МС
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 1, 159, 255},
     2,
     Obj_Device_Model_Id,
     0,
     NULL}, // Идентификатор исполнения
    {ASSOC(MFR | SVC),
     CLASS_ID_REGISTER,
     0,
     {0, 0, 96, 60, 2, 255},
     3,
     Obj_Voltage_3V5,
     0,
     NULL}, // Напряжение (+3.5В)
    {ASSOC(MFR | SVC),
     CLASS_ID_REGISTER,
     0,
     {0, 0, 96, 60, 5, 255},
     3,
     Obj_Voltage_USB,
     0,
     NULL}, // Напряжение (USB)
    {ASSOC(MFR | SVC),
     CLASS_ID_REGISTER,
     0,
     {0, 0, 96, 60, 6, 255},
     3,
     Obj_Voltage_Batt,
     0,
     NULL}, // Напряжение батарейки

    {ASSOC(MFR | USB | SVC),
     CLASS_ID_REGISTER,
     0,
     {0, 0, 96, 9, 0, 255},
     3,
     Obj_Temperature,
     0,
     NULL}, // Температура
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_REGISTER,
     0,
     {0, 0, 96, 8, 0, 255},
     3,
     Obj_Operating_Time,
     0,
     NULL}, // Время наработки
    {ASSOC(MFR | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 70, 0, 255},
     2,
     Obj_Cnt_Starts,
     0,
     NULL}, // Счетчик стартов программы
    // Удаленное обновление
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 0, 2, 0, 255},
     2,
     Obj_Firmware_Identifier,
     0,
     NULL}, // Идентификатор ПО
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 0, 2, 1, 255},
     2,
     Obj_Firmware_Version_Identifier,
     0,
     NULL}, // Идентификатор версии ПО(версия)
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 0, 2, 8, 255},
     2,
     Obj_Firmware_Signature,
     0,
     NULL}, // MD5 Основной прошивки
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 0, 2, 164, 255},
     2,
     Obj_CHK_Signature,
     0,
     NULL}, // MD5 по диапазону
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 0, 2, 165, 255},
     2,
     Obj_CHK_Address,
     0,
     NULL}, // Диапазон адресов для MD5
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_IMAGE_TRANSFER,
     0,
     {0, 0, 44, 0, 0, 255},
     7,
     Obj_Image_Transfer,
     4,
     Obj_Image_Transfer_Methods},

    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 61, 0, 255},
     1,
     Obj_Device_Reboot,
     1,
     Obj_Device_Reboot_Method}, // Перезагрузка устройства (Таймаут перезагрузки
                                // устройства)
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 61, 1, 255},
     2,
     Obj_Service_Mode_Password,
     4,
     Obj_Service_Mode_Entering_Method}, // Сервисный режим
    {ASSOC(MFR | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 61, 3, 255},
     2,
     Obj_DateTime_Compilation_FW,
     0,
     NULL}, // Время и дата компиляции ПО
    {ASSOC(MFR | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 61, 7, 255},
     2,
     Obj_Cnt_Btn_1,
     0,
     NULL}, // Счетчик нажатия кнопки 1
    {ASSOC(MFR | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 61, 8, 255},
     2,
     Obj_Cnt_Btn_2,
     0,
     NULL}, // Счетчик нажатия кнопки 2
    {ASSOC(MFR | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 61, 9, 255},
     2,
     Obj_LCD_Diag,
     0,
     NULL}, // Диагностика LCD
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 61, 10, 255},
     1,
     Obj_Settings_Reset,
     1,
     Obj_Settings_Reset_Method}, // Сброс настроек
    {ASSOC(MFR | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 61, 11, 255},
     3,
     Obj_BLE_Current_Status,
     0,
     NULL}, // Статус BLE

    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 1, 6, 255},
     2,
     Obj_SP_Version,
     0,
     NULL}, // Версия протокола СПОДЭС

    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 1, 170, 255},
     2,
     Obj_PinCode,
     0,
     NULL}, // Пинкод
    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 96, 1, 171, 255},
     2,
     Obj_Meter_Number,
     0,
     NULL}, // Номер счетчика

    {ASSOC(MFR | USB | SVC),
     CLASS_ID_DATA,
     0,
     {0, 0, 0, 0, 0, 0},
     0,
     NULL,
     0,
     NULL} // Последний объект
};
//----------------------------------------------------------------------------//
