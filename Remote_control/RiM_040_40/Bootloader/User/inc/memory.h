//----------------------------------------------------------------------------//
#ifndef INCMEMORY_H
#define INCMEMORY_H
//----------------------------------------------------------------------------//
#include "common_types.h"
#include <stdbool.h>
//----------------------------------------------------------------------------//
//Настройки 0x0000 - 0x00FF						Номер страницы
//Вторая копия хранится по нечетным адресам
#define	MEMORY_SERIAL_NUMBER				0x0000	//Заводской номер
#define	MEMORY_DATE_OF_PRODUCTION			0x0002	//Дата производства
#define	MEMORY_METER_NUMBER					0x0004	//Номер счетчика
#define	MEMORY_PINCODE						0x0006	//Пин-код


//Настройка моудя связи 0x0100 - 0x01FF
#define	MEMORY_SETTINGS_DEVICE_PASSWORD		0x0100	//Пароль для доступа к настройкам
#define MEMORY_SETTINGS_TIMEOUT_RESTART		0x0102	//Таймаут безусловной перезагрузки устройства (0 отключен)
#define MEMORY_SETTINGS_SMS_REMOTE_CONTROL	0x0104	//Параметры доступа к настройкам через СМС(пароль, белый список)

//Данные о работе устройства 0x0300 - 0x03FF	номер страницы
#define	MEMORY_OPERATING_TIME						0x0300	//Время наработки
#define	MEMORY_LAST_SESSION_OPERATING_TIME			0x0302	//Время наработки последнего сеанса (периода)
#define	MEMORY_CNT_STARTS							0x0304	//Cчетчик стартов программы
#define	MEMORY_CNT_IWDG								0x0306	//Cчетчик рестартов программы по IWDG
#define	MEMORY_CNT_HARDFAULT						0x0308	//Cчетчик аппаратных сбоев
#define	MEMORY_CNT_CLOCK_ERROR						0x030A	//Общее число ошибок тактирования

//----------------------------------------------------------------------------//
//Для удаленного обновления 0x0400 - 0x07FF		номер страницы
#define	MEMORY_UPDATE_DEVICE_BUFF_START				0x0400	//Буфер начало
#define	MEMORY_UPDATE_DEVICE_BUFF_END				0x077F	//Буфер конец(224 KB) 896 страницы
#define	MEMORY_UPDATE_DEVICE_CRC16					0x07F8	//Контрольная сумма записанной прошивки CRC-16
#define	MEMORY_UPDATE_DEVICE_MD5					0x07FA	//Хэш-сумма записанной прошивки MD5
#define	MEMORY_UPDATE_DEVICE_FLAG					0x07FC	//Флаг для удаленного обновления
#define	MEMORY_UPDATE_DEVICE_LAST_WRITE_BLOCK		0x07FE	//Последний записанный блок (по сути длина прошивки)

#define MEMORY_UPDATE_DATA_BLOCK_SIZE	256
#define	MEMORY_BLOCK_SIZE	0x10000	//64 КБ
#define MEMORY_UPDATE_NUM_PAGE						(MEMORY_UPDATE_DEVICE_BUFF_END - MEMORY_UPDATE_DEVICE_BUFF_START + 1)
#define MEMORY_UPDATE_SIZE							MEMORY_UPDATE_NUM_PAGE * MEMORY_UPDATE_DATA_BLOCK_SIZE

#define MEMORY_LAST_NUMBER_PAGE	0x1FFF
//----------------------------------------------------------------------------//
//Журналы 0x0800 - 0x1FFF
//----------------------------------------------------------------------------//
struct MEMORY_driver{
	bool (*init)();
	result_t (*writeTwoCopyesWithCRC)(u32 page, void* const buffer_ptr, u8 len);
	result_t (*readTwoCopyesWithCRC)(u32 page, void* buffer_ptr, u8 len);
	void (*readBytes)(const u32 address, u8* const buffer_ptr, const u16 len);
	void (*writeBytes)(const u32 address, u8* const buffer_ptr, const u8 len);
	u16 (*getPageSize)(void);
	void (*eraseTwoCopyes)(u32 page);
	void (*eraseBlock)(u32 page);
};
extern const struct MEMORY_driver memory;
//----------------------------------------------------------------------------//
#endif
