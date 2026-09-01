/**
 * \file            passwords.h
 * \brief           Хранилище до 10 паролей (BLE passkey): имя + значение
 *
 * Значение - BLE passkey, по стандарту 6 цифр (0..999999).
 * Имя - короткий текст в CP1251, максимум PWD_NAME_LEN символов.
 * Массив во флеше (страница MEMORY_PASSWORDS) с двойной копией и CRC;
 * в ОЗУ - зеркало, грузится при старте, меняется через USB.
 */
#ifndef PASSWORDS_H
#define PASSWORDS_H

#include "common_types.h"
#include <stdbool.h>

#define PWD_COUNT 10          // фиксированное число слотов
#define PWD_NAME_LEN 11       // символов имени (+1 на '\0' = 12 байт)
#define PWD_VALUE_MAX 999999u // предел BLE passkey (6 цифр)

//----------------------------------------------------------------------------//
//  Один пароль. Размер: 12 + 4 = 16 байт; 10 шт = 160 байт (< 255).
//----------------------------------------------------------------------------//
typedef struct {
	char name[PWD_NAME_LEN + 1]; // имя, CP1251, '\0'-терминированное
	u32 value;                   // BLE passkey
} password_t;

//----------------------------------------------------------------------------//
//  Всё хранилище (то, что пишется во флеш одним куском).
//----------------------------------------------------------------------------//
typedef struct {
	u8 selected; // индекс выбранного (0..PWD_COUNT-1)
	password_t item[PWD_COUNT];
} passwordStore_t;

//----------------------------------------------------------------------------//
//  API
//----------------------------------------------------------------------------//
void Passwords_Load(void); // загрузить из флеша при старте
bool Passwords_Save(void); // сохранить весь массив во флеш

u8 Passwords_Count(void);           // всегда PWD_COUNT (слоты фикс.)
u8 Passwords_Selected(void);        // индекс выбранного
void Passwords_Select(u8 index);    // выбрать активный + сразу во флеш
void Passwords_SelectRAM(u8 index); // выбрать активный только в ОЗУ (быстро)
bool Passwords_SaveSelected(void);  // дописать выбранный во флеш позже
const password_t *
Passwords_Get(u8 index); // доступ к слоту (NULL если вне диапазона)

// Задать имя+значение слота (из USB). value обрезается до PWD_VALUE_MAX.
bool Passwords_Set(u8 index, const char *name, u32 value);

// Значение активного пароля - его берёт BLE при сопряжении.
u32 Passwords_ActiveValue(void);

#endif // PASSWORDS_H
//----------------------------------------------------------------------------//
