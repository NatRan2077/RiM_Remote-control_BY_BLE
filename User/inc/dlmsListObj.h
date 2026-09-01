//----------------------------------------------------------------------------//
#ifndef INCDLMSLISTOBJ_H
#define INCDLMSLISTOBJ_H
//----------------------------------------------------------------------------//
#include "common_types.h"
#include "cosem.h"
#include <stdbool.h>
//----------------------------------------------------------------------------//
typedef enum { // Все возможные объекты DLMS
	DLMS_ENERGY_I_IV,
	DLMS_ENERGY_II_III,
	DLMS_ENERGY_I_II,
	DLMS_ENERGY_III_IV,

	DLMS_ACTIVE_ENERGY_T1,
	DLMS_ACTIVE_ENERGY_T2,
	DLMS_ACTIVE_ENERGY_T3,
	DLMS_ACTIVE_ENERGY_T4,
	DLMS_ACTIVE_ENERGY_T5,
	DLMS_ACTIVE_ENERGY_T6,
	DLMS_ACTIVE_ENERGY_T7,
	DLMS_ACTIVE_ENERGY_T8,

	DLMS_ENERGY_II_III_T1,
	DLMS_ENERGY_II_III_T2,
	DLMS_ENERGY_II_III_T3,
	DLMS_ENERGY_II_III_T4,
	DLMS_ENERGY_II_III_T5,
	DLMS_ENERGY_II_III_T6,
	DLMS_ENERGY_II_III_T7,
	DLMS_ENERGY_II_III_T8,

	DLMS_ENERGY_I_II_T1,
	DLMS_ENERGY_I_II_T2,
	DLMS_ENERGY_I_II_T3,
	DLMS_ENERGY_I_II_T4,
	DLMS_ENERGY_I_II_T5,
	DLMS_ENERGY_I_II_T6,
	DLMS_ENERGY_I_II_T7,
	DLMS_ENERGY_I_II_T8,

	DLMS_ENERGY_III_IV_T1,
	DLMS_ENERGY_III_IV_T2,
	DLMS_ENERGY_III_IV_T3,
	DLMS_ENERGY_III_IV_T4,
	DLMS_ENERGY_III_IV_T5,
	DLMS_ENERGY_III_IV_T6,
	DLMS_ENERGY_III_IV_T7,
	DLMS_ENERGY_III_IV_T8,

	DLMS_TIME,
	DLMS_DATE,

	DLMS_ACTIVE_POWER_PLUS,
	DLMS_L1_ACTIVE_POWER_PLUS,
	DLMS_L2_ACTIVE_POWER_PLUS,
	DLMS_L3_ACTIVE_POWER_PLUS,
	DLMS_REACTIVE_POWER_PLUS,
	DLMS_L1_REACTIVE_POWER_PLUS,
	DLMS_L2_REACTIVE_POWER_PLUS,
	DLMS_L3_REACTIVE_POWER_PLUS,
	DLMS_APPARENT_POWER_PLUS,
	DLMS_L1_APPARENT_POWER_PLUS,
	DLMS_L2_APPARENT_POWER_PLUS,
	DLMS_L3_APPARENT_POWER_PLUS,

	DLMS_VOLTAGE,
	DLMS_L1_VOLTAGE,
	DLMS_L2_VOLTAGE,
	DLMS_L3_VOLTAGE,
	DLMS_L1_L2_VOLTAGE_OLD,
	DLMS_L2_L3_VOLTAGE_OLD,
	DLMS_L1_L3_VOLTAGE_OLD,
	DLMS_L1_L2_VOLTAGE,
	DLMS_L2_L3_VOLTAGE,
	DLMS_L1_L3_VOLTAGE,
	DLMS_CURRENT,
	DLMS_L1_CURRENT,
	DLMS_L2_CURRENT,
	DLMS_L3_CURRENT,
	DLMS_NEUTRAL_CURRENT,
	DLMS_SUPPLY_FREQUENCY,
	DLMS_L1_COS,
	DLMS_L2_COS,
	DLMS_L3_COS,
	DLMS_COS,

	DLMS_RDCH_ENERGY_I_IV_OLD,
	DLMS_RDCH_ENERGY_II_III_OLD,
	DLMS_RDCH_ENERGY_I_II_OLD,
	DLMS_RDCH_ENERGY_III_IV_OLD,
	DLMS_RDCH_ENERGY_I_IV,
	DLMS_RDCH_ENERGY_II_III,
	DLMS_RDCH_ENERGY_I_II,
	DLMS_RDCH_ENERGY_III_IV,

	DLMS_RDCH_ACTIVE_ENERGY_T1_OLD,
	DLMS_RDCH_ACTIVE_ENERGY_T2_OLD,
	DLMS_RDCH_ACTIVE_ENERGY_T3_OLD,
	DLMS_RDCH_ACTIVE_ENERGY_T4_OLD,
	DLMS_RDCH_ACTIVE_ENERGY_T5_OLD,
	DLMS_RDCH_ACTIVE_ENERGY_T6_OLD,
	DLMS_RDCH_ACTIVE_ENERGY_T7_OLD,
	DLMS_RDCH_ACTIVE_ENERGY_T8_OLD,
	DLMS_RDCH_ACTIVE_ENERGY_T1,
	DLMS_RDCH_ACTIVE_ENERGY_T2,
	DLMS_RDCH_ACTIVE_ENERGY_T3,
	DLMS_RDCH_ACTIVE_ENERGY_T4,
	DLMS_RDCH_ACTIVE_ENERGY_T5,
	DLMS_RDCH_ACTIVE_ENERGY_T6,
	DLMS_RDCH_ACTIVE_ENERGY_T7,
	DLMS_RDCH_ACTIVE_ENERGY_T8,

	DLMS_RDCH_ENERGY_II_III_T1_OLD,
	DLMS_RDCH_ENERGY_II_III_T2_OLD,
	DLMS_RDCH_ENERGY_II_III_T3_OLD,
	DLMS_RDCH_ENERGY_II_III_T4_OLD,
	DLMS_RDCH_ENERGY_II_III_T5_OLD,
	DLMS_RDCH_ENERGY_II_III_T6_OLD,
	DLMS_RDCH_ENERGY_II_III_T7_OLD,
	DLMS_RDCH_ENERGY_II_III_T8_OLD,
	DLMS_RDCH_ENERGY_II_III_T1,
	DLMS_RDCH_ENERGY_II_III_T2,
	DLMS_RDCH_ENERGY_II_III_T3,
	DLMS_RDCH_ENERGY_II_III_T4,
	DLMS_RDCH_ENERGY_II_III_T5,
	DLMS_RDCH_ENERGY_II_III_T6,
	DLMS_RDCH_ENERGY_II_III_T7,
	DLMS_RDCH_ENERGY_II_III_T8,

	DLMS_RDCH_ENERGY_I_II_T1_OLD,
	DLMS_RDCH_ENERGY_I_II_T2_OLD,
	DLMS_RDCH_ENERGY_I_II_T3_OLD,
	DLMS_RDCH_ENERGY_I_II_T4_OLD,
	DLMS_RDCH_ENERGY_I_II_T5_OLD,
	DLMS_RDCH_ENERGY_I_II_T6_OLD,
	DLMS_RDCH_ENERGY_I_II_T7_OLD,
	DLMS_RDCH_ENERGY_I_II_T8_OLD,
	DLMS_RDCH_ENERGY_I_II_T1,
	DLMS_RDCH_ENERGY_I_II_T2,
	DLMS_RDCH_ENERGY_I_II_T3,
	DLMS_RDCH_ENERGY_I_II_T4,
	DLMS_RDCH_ENERGY_I_II_T5,
	DLMS_RDCH_ENERGY_I_II_T6,
	DLMS_RDCH_ENERGY_I_II_T7,
	DLMS_RDCH_ENERGY_I_II_T8,

	DLMS_RDCH_ENERGY_III_IV_T1_OLD,
	DLMS_RDCH_ENERGY_III_IV_T2_OLD,
	DLMS_RDCH_ENERGY_III_IV_T3_OLD,
	DLMS_RDCH_ENERGY_III_IV_T4_OLD,
	DLMS_RDCH_ENERGY_III_IV_T5_OLD,
	DLMS_RDCH_ENERGY_III_IV_T6_OLD,
	DLMS_RDCH_ENERGY_III_IV_T7_OLD,
	DLMS_RDCH_ENERGY_III_IV_T8_OLD,
	DLMS_RDCH_ENERGY_III_IV_T1,
	DLMS_RDCH_ENERGY_III_IV_T2,
	DLMS_RDCH_ENERGY_III_IV_T3,
	DLMS_RDCH_ENERGY_III_IV_T4,
	DLMS_RDCH_ENERGY_III_IV_T5,
	DLMS_RDCH_ENERGY_III_IV_T6,
	DLMS_RDCH_ENERGY_III_IV_T7,
	DLMS_RDCH_ENERGY_III_IV_T8,

	DLMS_RDCH_TIME,
	DLMS_RDCH_DATE,

	DLMS_SERIAL_NUMBER_METER,
	DLMS_TYPE_METER,
	DLMS_FW_METER,
	DLMS_OPERATING_TIME,
	DLMS_BATTERY_VOLTAGE,
	DLMS_TEMPERATURE,

	DLMS_SERIAL_NUMBER_DISPLAY,
	DLMS_TYPE_DISPLAY,
	DLMS_FW_DISPLAY,

	DLMS_LCD_DIAG,
	DLMS_NO_DATA,
	DLMS_MAX_OBJ,
} objDLMS_t;
#define MAX_GROUP 4
//----------------------------------------------------------------------------//
typedef enum {
	DLMS_ERROR_NONE,
	DLMS_NUMB_ELEMENTS_LESS_5,
	DLMS_ERROR_HEADER,
	DLMS_ERROR_HDLC,
	DLMS_ERROR_STATUS_PKT,
	DLMS_ERROR_DATA_PKT,
	DLMS_ERROR_HEADER_RELAY_CONNECT,
	DLMS_ERROR_HEADER_REQ_DIAGNOSIS,
	DLMS_ERROR_HEADER_SET_DIAGNOSIS,
	DLMS_ERROR_OBIS_UNKNOWN,
	DLMS_ERROR_STRUCT_DATA,

} dlmsError_t; // Ошибки связанные с информационной частью
//----------------------------------------------------------------------------//
typedef enum {
	ACC_2,
	ACC_3,
	ACC_4,
} accuracy_t; // Точность параметра
//----------------------------------------------------------------------------//
typedef enum {
	UNIT_TEMPERATURE = 9,
	UNIT_ACTIVE_POWER = 27,
	UNIT_APPARENT_POWER = 28,
	UNIT_REACTIVE_POWER = 29,
	UNIT_ACTIVE_ENERGY = 30,
	UNIT_REACTIVE_ENERGY = 32,
	UNIT_CURRENT = 33,
	UNIT_VOLTAGE = 35,
	UNIT_FREQUENCY = 44,
	UNIT_NO_UNIT = 255,
} unit_t; // Единицы измерения
//----------------------------------------------------------------------------//
typedef enum {
	STATUS_OK,
	STATUS_ERROR,
} status_t;
//----------------------------------------------------------------------------//
typedef enum {
	STATUS_TAMPER_OK = 0,
	STATUS_TAMPER_FLAG = 1,
	STATUS_TAMPER_ACTIVE = 2,
} statusTamper_t;
//----------------------------------------------------------------------------//
typedef enum {
	NONE,
	DISCONNECTED,
	CONNECTED,
	READY_FOR_RECONNECTION,
} RelayStatus_t; // Состояние реле
//----------------------------------------------------------------------------//
typedef struct {
	statusTamper_t Eqi;       // ПКЭ
	statusTamper_t Cover;     // Пломба клеммной крышки
	statusTamper_t Meter;     // Пломба корпуса
	statusTamper_t Magnet;    // Магнитрое поле
	statusTamper_t Battery;   // Пломба отсека батарейки
	statusTamper_t Uhf;       // СВЧ
	statusTamper_t LimPower;  // Превышение порога мощности
	statusTamper_t Diag;      // Статус диагностики
	statusTamper_t Unbalance; // Небаланс

	status_t Rtc;        // ЧРВ
	u8 Phase;            // Статус фаз
	RelayStatus_t relay; // Состояние реле
	status_t DiagLCD;    // Диагностика ЖКИ
} tamper_t;
//----------------------------------------------------------------------------//
typedef enum {
	ENERGY_DIR_NONE = 0,
	ENERGY_DIR_P_PLUS = 1,
	ENERGY_DIR_P_MINUS = 2,
	ENERGY_DIR_Q_PLUS = 4,
	ENERGY_DIR_Q_MINUS = 8,
	ENERGY_DIR_P_PLUS_Q_PLUS = 5,
	ENERGY_DIR_P_MINUS_Q_PLUS = 6,
	ENERGY_DIR_P_MINUS_Q_MINUS = 10,
	ENERGY_DIR_P_PLUS_Q_MINUS = 9,
} energyDir_t;
//----------------------------------------------------------------------------//
typedef struct {
	u8 OBIS[6];
	accuracy_t acc;
	bool fKilo;
	u8 group;
} list_dlms_OBIS_codes_t;
//----------------------------------------------------------------------------//
typedef struct {
	const list_dlms_OBIS_codes_t *pConst; // Указатель на константные данные

	u64 value;       // Значение
	cosemTag_t type; // Тип данных
	s8 scale;        // Степень 10 (множитель)
	unit_t unit;     // Единица измерения

	bool fShow; // Флаг, что элемент доступен для отображения (сбрасываем при
	            // отображении, выставляем при считывании информации со счетчика)
} obj_t;       // Структура для хранения данных объектов
//----------------------------------------------------------------------------//
typedef struct {
	//	u32 lastReadElement;		//Последний прочитанный элемент
	//	u32 firstElementForDislpay;
	//	u8 NumbElements;	//Число элементов
	//	u8 dontReadElements;	//Число не прочитанных элементов
	//	u8 curElementOnDisplay;	//номер текущего элемента на дисплее
	dlmsError_t error;
	obj_t obj[DLMS_MAX_OBJ];
	tamper_t status;
	energyDir_t energyDir;
	u8 verLCD; // Версия ЖКИ
	u8 curTariff;
	bool fHz;
	bool fVolt;
} dlms_t; // Структура для информационного обмена
extern dlms_t dlms;
//----------------------------------------------------------------------------//
void ShowOBIS(objDLMS_t OBIS);
void ShowData(obj_t element);
//----------------------------------------------------------------------------//
#endif
