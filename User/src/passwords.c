/**
 * \file            passwords.c
 * \brief           Хранилище 10 паролей во флеше (описание - passwords.h)
 */
#include "passwords.h"
#include "memory.h"
#include "log.h"
#include <string.h>
//----------------------------------------------------------------------------//
//  Страница флеша под массив паролей (область настроек, свободный слот).
//  Вторая копия драйвер кладёт на page+1 автоматически.
//----------------------------------------------------------------------------//
#define MEMORY_PASSWORDS 0x0008
//----------------------------------------------------------------------------//
static passwordStore_t store;
//----------------------------------------------------------------------------//
//  Имя по умолчанию "ПАР N" (CP1251: П=0xCF А=0xC0 Р=0xD0), N - номер 1..10
//----------------------------------------------------------------------------//
//  Имя по умолчанию "P N" латиницей (ASCII, безопасно для 7/16-сегм. шрифта).
//  Русские буквы намеренно не используем: индекс шрифта - signed char,
//  байты 0xC0+ дали бы отрицательный индекс и порчу памяти.
static void default_name(char *dst, u8 slot) {
	u8 n = 0;
	dst[n++] = 'P';
	u8 num = (u8)(slot + 1);
	if (num >= 10) {
		dst[n++] = '1';
		dst[n++] = '0';
	} else {
		dst[n++] = (char)('0' + num);
	}
	dst[n] = 0;
}
//----------------------------------------------------------------------------//
static void set_defaults(void) {
	store.selected = 0;
	for (u8 i = 0; i < PWD_COUNT; i++) {
		strcpy(store.item[i].name, "\xCF\xC0\xD0\xCE\xCB\xDC"); // ПАРОЛЬ
		store.item[i].value = 0;
	}
	store.item[0].value = 123456;
}
//----------------------------------------------------------------------------//
void Passwords_Load(void) {
	if (RESULT_ERROR ==
	    memory.readTwoCopyesWithCRC(MEMORY_PASSWORDS, &store, sizeof(store))) {
		// первая загрузка или повреждение - ставим значения по умолчанию
		_SYSLOG_("[PWD] load failed -> defaults");
		set_defaults();
		Passwords_Save();
		return;
	}
	// страховка от мусора в индексе выбранного
	if (store.selected >= PWD_COUNT) {
		store.selected = 0;
	}
	_SYSLOG_("[PWD] loaded, selected=%d", store.selected);
}
//----------------------------------------------------------------------------//
bool Passwords_Save(void) {
	if (RESULT_ERROR ==
	    memory.writeTwoCopyesWithCRC(MEMORY_PASSWORDS, &store, sizeof(store))) {
		_SYSLOG_("[PWD] save failed");
		return false;
	}
	return true;
}
//----------------------------------------------------------------------------//
u8 Passwords_Count(void) {
	return PWD_COUNT;
}
u8 Passwords_Selected(void) {
	return store.selected;
}
//----------------------------------------------------------------------------//
void Passwords_Select(u8 index) {
	if (index >= PWD_COUNT) {
		return;
	}
	store.selected = index;
	Passwords_Save();
	_SYSLOG_("[PWD] select %d", index);
}
//----------------------------------------------------------------------------//
//  Быстрый выбор без записи флеша - для использования в интерактивном цикле,
//  где нельзя блокировать ядро на время стирания страницы (мешает BLE).
//----------------------------------------------------------------------------//
void Passwords_SelectRAM(u8 index) {
	if (index >= PWD_COUNT) {
		return;
	}
	store.selected = index;
}
//----------------------------------------------------------------------------//
//  Отложенная запись выбранного во флеш (вызывать вне критичных к BLE мест).
//----------------------------------------------------------------------------//
bool Passwords_SaveSelected(void) {
	return Passwords_Save();
}
//----------------------------------------------------------------------------//
const password_t *Passwords_Get(u8 index) {
	if (index >= PWD_COUNT) {
		return NULL;
	}
	return &store.item[index];
}
//----------------------------------------------------------------------------//
bool Passwords_Set(u8 index, const char *name, u32 value) {
	if (index >= PWD_COUNT) {
		return false;
	}

	if (name) {
		strncpy(store.item[index].name, name, PWD_NAME_LEN);
		store.item[index].name[PWD_NAME_LEN] = 0;
	}
	store.item[index].value = (value > PWD_VALUE_MAX) ? PWD_VALUE_MAX : value;

	return Passwords_Save();
}
//----------------------------------------------------------------------------//
u32 Passwords_ActiveValue(void) {
	return store.item[store.selected].value;
}
//----------------------------------------------------------------------------//
