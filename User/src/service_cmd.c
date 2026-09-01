/**
 * \file            service_cmd.c
 * \brief           Сервисные команды по USB для списка из 10 паролей (протокол
 * - service_cmd.h)
 */
#include "service_cmd.h"
#include "passwords.h"
#include "usb.h"
#include "log.h"
#include <string.h>
//----------------------------------------------------------------------------//
static void reply(const char *s) {
	usb.writeToBufTx((u8 *)s, (u16)strlen(s));
	usb.setTransferFlag();
}
//----------------------------------------------------------------------------//
//  Число u32 -> строку (без printf). Возвращает длину.
//----------------------------------------------------------------------------//
static u8 u32_to_str(u32 v, char *out) {
	char tmp[10];
	u8 d = 0, n = 0;
	do {
		tmp[d++] = (char)('0' + (v % 10));
		v /= 10;
	} while (v && d < sizeof(tmp));
	while (d) {
		out[n++] = tmp[--d];
	}
	out[n] = 0;
	return n;
}
//----------------------------------------------------------------------------//
//  Строка описания слота: "PWD i=<idx> sel=<0/1> val=<v> name=<name>\r\n"
//----------------------------------------------------------------------------//
static void reply_slot(u8 idx) {
	const password_t *p = Passwords_Get(idx);
	if (!p) {
		reply("PWD ERR FORMAT\r\n");
		return;
	}

	char out[64];
	u8 n = 0;
	const char *pre = "PWD i=";
	memcpy(&out[n], pre, 6);
	n += 6;
	n += u32_to_str(idx, &out[n]);
	memcpy(&out[n], " sel=", 5);
	n += 5;
	out[n++] = (idx == Passwords_Selected()) ? '1' : '0';
	memcpy(&out[n], " val=", 5);
	n += 5;
	n += u32_to_str(p->value, &out[n]);
	memcpy(&out[n], " name=", 6);
	n += 6;
	u8 nl = (u8)strlen(p->name);
	if (nl > PWD_NAME_LEN) {
		nl = PWD_NAME_LEN;
	}
	memcpy(&out[n], p->name, nl);
	n += nl;
	out[n++] = '\r';
	out[n++] = '\n';
	out[n] = 0;
	reply(out);
}
//----------------------------------------------------------------------------//
//  Разбор беззнакового числа из буфера [i0..i1). digits - счётчик цифр.
//----------------------------------------------------------------------------//
static bool parse_uint(const u8 *b, u16 i0, u16 i1, u32 *out, u8 max_digits) {
	u32 v = 0;
	u8 d = 0;
	if (i0 >= i1) {
		return false;
	}
	for (u16 i = i0; i < i1; i++) {
		if (b[i] < '0' || b[i] > '9' || d >= max_digits) {
			return false;
		}
		v = v * 10u + (u32)(b[i] - '0');
		d++;
	}
	*out = v;
	return true;
}
//----------------------------------------------------------------------------//
bool ServiceCmd_Process(u8 *buf, u16 len) {
	if (!buf || len < 4) {
		return false;
	}

	// отсечь завершающие CR/LF/NUL
	while (len &&
	       (buf[len - 1] == '\r' || buf[len - 1] == '\n' || buf[len - 1] == 0)) {
		len--;
	}
	if (len < 4) {
		return false;
	}

	//================= блок PWD ==========================================//
	if (0 == memcmp(buf, "PWD", 3)) {
		// "PWD?" - весь список
		if (len == 4 && buf[3] == '?') {
			reply("PWD LIST\r\n");
			for (u8 i = 0; i < Passwords_Count(); i++) {
				reply_slot(i);
			}
			reply("PWD END\r\n");
			return true;
		}
		// ожидаем "PWD <i>..." - пробел + индекс
		if (buf[3] != ' ') {
			reply("PWD ERR FORMAT\r\n");
			return true;
		}
		u16 pos = 4;
		u32 idx;
		u16 idx_end = pos;
		while (idx_end < len && buf[idx_end] >= '0' && buf[idx_end] <= '9') {
			idx_end++;
		}
		if (!parse_uint(buf, pos, idx_end, &idx, 2) || idx >= Passwords_Count()) {
			reply("PWD ERR FORMAT\r\n");
			return true;
		}
		// "PWD i?" - один слот
		if (idx_end < len && buf[idx_end] == '?' && idx_end + 1 == len) {
			reply_slot((u8)idx);
			return true;
		}
		// "PWD i=VAL,NAME" - запись
		if (idx_end < len && buf[idx_end] == '=') {
			u16 vpos = (u16)(idx_end + 1);
			u16 comma = vpos;
			while (comma < len && buf[comma] != ',') {
				comma++;
			}
			u32 val;
			if (!parse_uint(buf, vpos, comma, &val, 6)) { // до 6 цифр
				reply("PWD ERR FORMAT\r\n");
				return true;
			}
			char name[PWD_NAME_LEN + 1];
			u8 nl = 0;
			if (comma < len) { // есть ,NAME
				for (u16 i = comma + 1; i < len && nl < PWD_NAME_LEN; i++) {
					name[nl++] = (char)buf[i];
				}
			}
			name[nl] = 0;
			const char *pname = (nl > 0) ? name : NULL; // пусто = имя не менять
			if (Passwords_Set((u8)idx, pname, val)) {
				reply("PWD OK\r\n");
			} else {
				reply("PWD ERR MEM\r\n");
			}
			return true;
		}
		reply("PWD ERR FORMAT\r\n");
		return true;
	}

	//================= блок SEL ==========================================//
	if (0 == memcmp(buf, "SEL", 3)) {
		if (len == 4 && buf[3] == '?') {
			char out[16];
			u8 n = 0;
			memcpy(out, "SEL=", 4);
			n = 4;
			n += u32_to_str(Passwords_Selected(), &out[n]);
			out[n++] = '\r';
			out[n++] = '\n';
			out[n] = 0;
			reply(out);
			return true;
		}
		if (buf[3] == '=') {
			u32 idx;
			if (parse_uint(buf, 4, len, &idx, 2) && idx < Passwords_Count()) {
				Passwords_Select((u8)idx);
				reply("SEL OK\r\n");
			} else {
				reply("SEL ERR\r\n");
			}
			return true;
		}
		reply("SEL ERR\r\n");
		return true;
	}

	//================= совместимость: PIN ================================//
	if (0 == memcmp(buf, "PIN", 3)) {
		u8 sel = Passwords_Selected();
		if (len == 4 && buf[3] == '?') {
			const password_t *p = Passwords_Get(sel);
			char out[16];
			u8 n = 0;
			memcpy(out, "PIN=", 4);
			n = 4;
			n += u32_to_str(p ? p->value : 0, &out[n]);
			out[n++] = '\r';
			out[n++] = '\n';
			out[n] = 0;
			reply(out);
			return true;
		}
		if (buf[3] == '=') {
			u32 val;
			if (!parse_uint(buf, 4, len, &val, 6)) {
				reply("PIN ERR FORMAT\r\n");
				return true;
			}
			if (Passwords_Set(sel, NULL, val)) {
				reply("PIN OK\r\n");
			} else {
				reply("PIN ERR MEM\r\n");
			}
			return true;
		}
		reply("PIN ERR FORMAT\r\n");
		return true;
	}

	return false; // не наша команда
}
//----------------------------------------------------------------------------//
