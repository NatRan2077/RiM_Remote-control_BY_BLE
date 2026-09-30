//----------------------------------------------------------------------------//
// thread_rim_protocol.c
// Управление БУ ВВ серии 6х через BLE-шлюз RiM_BY61 + отрисовка экрана.
//
// Кнопки:
//   ЛЕВАЯ  (коротко)   - смена команды в меню
//   ПРАВАЯ (удержание) - отправка выбранной команды
//   ОБЕ кнопки вместе  - аварийная отправка ОТКЛ
//
// Разметка ЖКИ:
//   1-6    - текст команды; при отправке: ОТПР. -> УСПЕХ / ОШИБКА
//   8-15   - код аварии (пусто = нет), при ОШИБКА - код отказа
//   17-23  - заводской номер блока; при аварии - "ErrOr" (мигает)
//   S39    - состояние ВВ: горит = включён (ВКЛ), мигает = идёт операция
//   S46    - заряд конденсаторов
//   S50    - блокировка
//   S53    - авария
//----------------------------------------------------------------------------//
#include "threads.h"
#include "messages.h"
#include "globals.h"
#include "lcd.h"
#include "log.h"
#include "ble_transport.h"
#include "passwords.h"
#include <string.h>
//----------------------------------------------------------------------------//
pt_t rim_pt;
//----------------------------------------------------------------------------//
extern s8 rssiBLE; // текущий RSSI (threads_other.c)
void ShowRSSI(s8 rssi);
void hp_ble_disconnect(void); // разрыв BLE-соединения (SDK)
extern bool g_connecting;     // идёт попытка подключения (app_ble.c)
//----------------------------------------------------------------------------//
//  Клавиатура пульта (алиасы событий BTN_1/BTN_2, чтобы семантика читалась)
//----------------------------------------------------------------------------//
#define MSG_SELECT_CMD MSG_BTN_1_PUSH_SHORT           // ЛЕВАЯ  - выбор
#define MSG_SEND_CMD MSG_BTN_2_PUSH_MID               // ПРАВАЯ - отправить
#define MSG_EMERGENCY_OFF MSG_BTN_1_2_COMBINATION_MID // ОБЕ    - аварийное ОТКЛ
//----------------------------------------------------------------------------//
//  Протокол
//----------------------------------------------------------------------------//
#define RIM_ID 0x96        // ID устройства в кадрах РиМ
#define COP_OFF_VV 0x12    // ОТКЛ ВВ по BLE
#define COP_ON_VV 0x13     // ВКЛ  ВВ по BLE
#define COP_TELEMETRY 0x40 // телеметрия блока (матрица сост. и счётчики)
#define COP_SERVICE 0x41   // служебная информация (заводской номер)
#define COP_CNT_RESET 0x22 // сброс локального счётчика циклов ВО
#define COP_ERR_FLAG 0x80  // COP+0x80 = ответ с кодом ошибки
#define RIM_ERR_NO_CHANGE                                                      \
	16 // отказ "состояние ВВ уже такое"
	   // (ВКЛ включённому / ОТКЛ отключённому)

// Смещения полей внутри секции данных DATA телеметрии 0x40
// (DATA[0..3] - версия прошивки, DATA[4..7] - матрица, Uint32 LE)
#define TLM_MATRIX_OFFSET 4
#define TLM_CNT_GLOBAL 25 // DATA[25..28] - глобальный счётчик циклов ВО
#define TLM_CNT_LOCAL 29  // DATA[29..32] - локальный счётчик циклов ВО

// Биты матрицы текущих состояний
#define M_PWR_RES 0x00000004UL    // резервное питание
#define M_PWR_OP 0x00000008UL     // оперативное питание
#define M_BUS48 0x00000010UL      // шина 48 В
#define M_CAP 0x00000020UL        // конденсаторы заряжены
#define M_VV 0x00000040UL         // состояние ВВ (1 = включён)
#define M_BLOCK 0x00000080UL      // блокировка              -> код 1
#define M_CTRL_FAIL 0x00000100UL  // неисправность управления-> код 2
#define M_READY 0x00000200UL      // готовность блока
#define M_KZ_CMD 0x00040000UL     // КЗ в ЦУ при команде     -> код 3
#define M_NO_CHANGE 0x00080000UL  // состояние ВВ не изменилось -> код 4
#define M_KZ_POLL 0x00100000UL    // КЗ в ЦУ при опросе      -> код 5
#define M_BREAK_POLL 0x00200000UL // обрыв цепи ЦУ при опросе-> код 6
#define M_BUS48_ERR 0x00400000UL  // неисправна шина 48 В    -> код 7
#define M_OP_FAIL 0x00800000UL    // неисправность ОП        -> код 8
#define M_FAULT 0x01000000UL      // общий отказ оборудования-> код 9
#define M_EXEC_OFF 0x02000000UL   // выполняется ОТКЛ
#define M_EXEC_ON 0x04000000UL    // выполняется ВКЛ
#define M_EXEC_ANY 0x40000000UL   // выполняется любая команда
//----------------------------------------------------------------------------//
#define POLL_PERIOD_MS 2000  // период опроса телеметрии
#define SEND_TIMEOUT_MS 3000 // ожидание ответа на команду
#define ACK_SHOW_MS 3000     // сколько держать УСПЕХ/ОШИБКА
#define BLINK_MS 400         // период мигания
//----------------------------------------------------------------------------//
//  Экран поиска: "устр<n>" - количество найденных БУ (1 или 2 цифры), CP1251.
//  Байты - буквы У,С,Т,Р; на сегментном ЖКИ строчные и заглавные совпадают.
#define TXT_SCAN_PREFIX "\xD3\xD1\xD2\xD0" // "устр"
//----------------------------------------------------------------------------//
//  Тексты состояний отправки (CP1251, не более 6 знакомест)
//----------------------------------------------------------------------------//
#define TXT_SENDING "\xCE\xD2\xCF\xD0. "        // "ОТПР. " - запрос ушёл
#define TXT_SEND_OK "\xD3\xD1\xCF\xC5\xD5 "     // "УСПЕХ " - подтверждено
#define TXT_SEND_ERR "\xCE\xD8\xC8\xC1\xCA\xC0" // "ОШИБКА" - отказ/нет ответа
//----------------------------------------------------------------------------//
static u8 devSel = 0; // выбранное устройство в списке
//----------------------------------------------------------------------------//
//  Меню команд (текст в CP1251, ровно 6 символов)
//----------------------------------------------------------------------------//
typedef struct {
	u8 cop;
	char text[7];
} rimCmd_t;

//  cop = 0 -> у пункта меню отдельная от отправки семантика
static const rimCmd_t MENU[] = {
    {COP_ON_VV, "\xC2\xCA\xCB   "},     // ВКЛ
    {COP_OFF_VV, "\xCE\xD2\xCA\xCB  "}, // ОТКЛ
    {0, "\xD1\xD7.\xC2\xCE "}, // СЧ.ВО (удержание = сброс лок. счётчика)
    {COP_SERVICE, "\xD2\xC8\xCF \xC0\xC2"}, // ТИП АВ (повторный опрос 0x41)
    {0, "\xC2\xDB\xD5\xCE\xC4 "},           // ВЫХОД (разрыв соединения)
};
#define MENU_CNT_VIEW 2 // индекс пункта "СЧ.ВО"
#define MENU_EXIT 4     // индекс пункта "ВЫХОД"
#define MENU_SIZE (sizeof(MENU) / sizeof(MENU[0]))
//----------------------------------------------------------------------------//
//  Состояние
//----------------------------------------------------------------------------//
static u8 curCmd = 0;     // выбранный пункт меню
static u32 matrix = 0;    // матрица текущих состояний
static u32 serialBU = 0;  // заводской номер блока
static u32 cntLocal = 0;  // локальный счётчик циклов ВО
static u32 cntGlobal = 0; // глобальный счётчик циклов ВО
//  Индикация отправки команды
typedef enum {
	SEND_IDLE = 0, // команда не отправлялась - обычный экран
	SEND_WAIT,     // запрос ушёл, ждём ответ      ("ОТПР.")
	SEND_OK,       // получено подтверждение       ("УСПЕХ")
	SEND_ERR,      // отказ или ответа нет         ("ОШИБКА")
} sendState_t;
static sendState_t sendState = SEND_IDLE;
static u8 lastErr = 0; // код ошибки последнего ответа
//----------------------------------------------------------------------------//
//  CRC16 MODBUS
//----------------------------------------------------------------------------//
static u16 rim_crc16(const u8 *d, u16 len) {
	u16 crc = 0xFFFF;
	for (u16 i = 0; i < len; i++) {
		crc ^= d[i];
		for (u8 b = 0; b < 8; b++) {
			if (crc & 1) {
				crc = (crc >> 1) ^ 0xA001;
			} else {
				crc >>= 1;
			}
		}
	}
	return crc;
}
//----------------------------------------------------------------------------//
//  Сборка пакета:  67 00 LEN 5C 26 | 96 COP CNT [DATA] CRC_L CRC_H
//  (67 - COP_GATEWAY BLE-шлюза, 5C 26 - фиксированные параметры шлюза,
//   LEN - длина внутреннего пакета РиМ)
//----------------------------------------------------------------------------//
static u8 rim_build(u8 *out, u8 cop, const u8 *data, u8 dlen) {
	u8 inner[32];
	u8 n = 0;

	inner[n++] = RIM_ID;
	inner[n++] = cop;
	inner[n++] = dlen + 2; // CNT = DATA + 2 байта CRC
	if (dlen && data) {
		memcpy(&inner[n], data, dlen);
		n += dlen;
	}
	u16 crc = rim_crc16(inner, n);
	inner[n++] = (u8)(crc & 0xFF);
	inner[n++] = (u8)(crc >> 8);

	out[0] = 0x67; // COP_GATEWAY
	out[1] = 0x00;
	out[2] = n; // длина внутреннего пакета
	out[3] = 0x5C;
	out[4] = 0x26;
	memcpy(&out[5], inner, n);
	return (u8)(5 + n);
}
//----------------------------------------------------------------------------//
static void rim_send(u8 cop) {
	u8 frame[BLE_TX_BUFF_SIZE];
	u8 len = rim_build(frame, cop, NULL, 0);

	ble_transport_rx_clear(); // готовимся принять новый ответ
	if (BLE_TX_OK == ble_transport_send(frame, len)) {
		_SYSLOG_DATA_("[RIM TX]", frame, len);
	}
}
//----------------------------------------------------------------------------//
//  Разбор ответа: ищем в буфере внутренний пакет РиМ (96 COP CNT ... CRC)
//  и проверяем CRC. Мусор перед ним пропускаем - это байты обёртки шлюза.
//----------------------------------------------------------------------------//
static bool rim_parse(const u8 *buf, u8 len) {
	for (u8 i = 0; (u8)(i + 5) <= len; i++) {
		if (buf[i] != RIM_ID) {
			continue;
		}

		u8 cnt = buf[i + 2];
		u16 total = (u16)(3 + cnt); // ID COP CNT + CNT байт
		if ((u16)(i + total) > len) {
			continue;
		}
		if (cnt < 2 || cnt > 64) {
			continue;
		}

		u16 crcCalc = rim_crc16(&buf[i], (u16)(total - 2));
		u16 crcPkt = (u16)buf[i + total - 2] | ((u16)buf[i + total - 1] << 8);
		if (crcCalc != crcPkt) {
			continue;
		}

		u8 cop = buf[i + 1];
		const u8 *data = &buf[i + 3];
		u8 dlen = (u8)(cnt - 2); // размер полезных DATA

		_SYSLOG_("[RIM RX] COP=%02X LEN=%d", cop, dlen);
		if (dlen) {
			_SYSLOG_DATA_("[RIM DATA]", (u8 *)data, dlen);
		}

		//--- ответ с ошибкой: COP | 0x80 --------------------------------//
		if (cop & COP_ERR_FLAG) {
			const u8 errCop = (u8)(cop & 0x7F);
			lastErr = dlen ? data[0] : 0xFF;
			_SYSLOG_("[RIM ERR] cop=%02X err=%02X", errCop, lastErr);

			// ВКЛ уже включённому (или ОТКЛ отключённому): блок отвечает
			// отказом 16 "состояние не изменилось". Для оператора это не
			// ошибка - выключатель и так в нужном состоянии: показываем УСПЕХ
			if (((COP_ON_VV == errCop) || (COP_OFF_VV == errCop)) &&
			    (RIM_ERR_NO_CHANGE == lastErr)) {
				lastErr = 0;
				if (SEND_WAIT == sendState) {
					sendState = SEND_OK;
					tim.reset(TIMER_WAIT_BETWEEN_REQ);
				}
				return true;
			}

			// Отказ на служебный запрос (ТИП АВ / фоновый 0x41):
			// на экран не выводим, индикацию не трогаем
			if (COP_SERVICE == errCop) {
				lastErr = 0;
				if (SEND_WAIT == sendState) {
					sendState = SEND_IDLE;
				}
				return true;
			}

			sendState = SEND_ERR;
			tim.reset(TIMER_WAIT_BETWEEN_REQ);
			return true;
		}

		switch (cop) {
		//--- телеметрия: матрица текущих состояний -------------------//
		case COP_TELEMETRY:
			if (dlen >= TLM_MATRIX_OFFSET + 4) {
				matrix = ((u32)data[TLM_MATRIX_OFFSET]) |
				         ((u32)data[TLM_MATRIX_OFFSET + 1] << 8) |
				         ((u32)data[TLM_MATRIX_OFFSET + 2] << 16) |
				         ((u32)data[TLM_MATRIX_OFFSET + 3] << 24);
				_SYSLOG_("[RIM MATRIX] %08X", matrix);
			}
			if (dlen >= TLM_CNT_LOCAL + 4) {
				cntGlobal = ((u32)data[TLM_CNT_GLOBAL]) |
				            ((u32)data[TLM_CNT_GLOBAL + 1] << 8) |
				            ((u32)data[TLM_CNT_GLOBAL + 2] << 16) |
				            ((u32)data[TLM_CNT_GLOBAL + 3] << 24);
				cntLocal = ((u32)data[TLM_CNT_LOCAL]) |
				           ((u32)data[TLM_CNT_LOCAL + 1] << 8) |
				           ((u32)data[TLM_CNT_LOCAL + 2] << 16) |
				           ((u32)data[TLM_CNT_LOCAL + 3] << 24);
				_SYSLOG_("[RIM CNT] glob=%u loc=%u", cntGlobal, cntLocal);
			}
			break;

		//--- служебная информация 0x41 ------------------------------//
		//  Формат DATA дан по наблюдениям: первые 4 байта содержат
		//  заводской номер (Uint32 LE), далее версия платы и ПО.
		case COP_SERVICE:
			if (dlen >= 4) {
				serialBU = ((u32)data[0]) | ((u32)data[1] << 8) |
				           ((u32)data[2] << 16) | ((u32)data[3] << 24);
				_SYSLOG_("[RIM SERVICE] SN=%u", serialBU);
			}
			if (SEND_WAIT == sendState) { // ответ на команду ТИП АВ
				lastErr = 0;
				sendState = SEND_OK;
				tim.reset(TIMER_WAIT_BETWEEN_REQ);
			}
			break;

		//--- эхо команды ВКЛ/ОТКЛ = команда принята -----------------//
		case COP_ON_VV:
		case COP_OFF_VV:
		case COP_CNT_RESET:
			lastErr = 0;
			if (SEND_WAIT == sendState) {
				sendState = SEND_OK;
				tim.reset(TIMER_WAIT_BETWEEN_REQ);
			}
			break;

		default:
			break;
		}
		return true;
	}
	return false;
}
//----------------------------------------------------------------------------//
//  Код аварии 1..9 (0 = аварии нет)
//----------------------------------------------------------------------------//
static u8 rim_alarm_code(u32 m) {
	if (m & M_BLOCK) {
		return 1; // сработала блокировка
	}
	if (m & M_CTRL_FAIL) {
		return 2; // неисправность управления
	}
	if (m & M_KZ_CMD) {
		return 3; // КЗ в ЦУ при команде
	}
	if (m & M_NO_CHANGE) {
		return 4; // состояние ВВ не изменилось
	}
	if (m & M_KZ_POLL) {
		return 5; // КЗ в ЦУ при опросе
	}
	if (m & M_BREAK_POLL) {
		return 6; // обрыв цепи в ЦУ при опросе
	}
	if (m & M_BUS48_ERR) {
		return 7; // неисправна шина 48 В
	}
	if (m & M_OP_FAIL) {
		return 8; // неисправность ОП
	}
	if (m & M_FAULT) {
		return 9; // общий отказ оборудования
	}
	return 0;
}
//----------------------------------------------------------------------------//
//  Разряды ЖКИ 17..23
//----------------------------------------------------------------------------//
static void rim_show_serial(u32 sn) {
	for (s8 p = 23; p >= 17; p--) {
		lcd.show_val_pos((u8)(sn % 10), (u8)p);
		sn /= 10;
	}
}

//  Вывод числа с подавлением ведущих нулей в разрядах [from..to]
static void rim_show_num_range(u32 val, u8 from, u8 to) {
	for (s8 p = (s8)to; p >= (s8)from; p--) {
		if ((val == 0) && (p != (s8)to)) {
			lcd.show_val_pos(SPACE, (u8)p);
		} else {
			lcd.show_val_pos((u8)(val % 10), (u8)p);
		}
		val /= 10;
	}
}
//----------------------------------------------------------------------------//
//  Надпись ошибки на разрядах 17..23: "Error" (код ошибки/отказа - на 8-15)
static void rim_show_alarm_top(void) {
	lcd.show_val_pos(SPACE, 17);
	lcd.show_val_pos(SPACE, 18);
	lcd.show_val_pos(CHAR_E, 19);
	lcd.show_val_pos(CHAR_r, 20);
	lcd.show_val_pos(CHAR_r, 21);
	lcd.show_val_pos(CHAR_o, 22); // строчная 'o' - чтобы не путать с нулём
	lcd.show_val_pos(CHAR_r, 23);
}
//----------------------------------------------------------------------------//
//  Батарейка: значок виден всегда (кроме питания от USB),
//  при разряде мигает нижним сегментом
//----------------------------------------------------------------------------//
static void rim_show_batt(bool blink) {
	if (SystemInfo.status.usbPwr) {
		lcd.text(HIDE, 2, BATT, BATT_1);
		return;
	}
	lcd.text(SHOW, 1, BATT);
	switch (SystemInfo.status.vBatt) {
	case V_BATT_STATUS_LOW:
	case V_BATT_STATUS_LOWER:
		lcd.text(blink ? SHOW : HIDE, 1, BATT_1); // разряжена - мигаем
		break;
	default:
		lcd.text(SHOW, 1, BATT_1);
		break;
	}
}
//----------------------------------------------------------------------------//
//  Вывод части серийника после последнего '_' ("RiM_BY61_0000000" -> "0000000")
//----------------------------------------------------------------------------//
static void rim_show_dev_name(const char *name) {
	const char *tail = name;
	for (const char *p = name; *p; p++) {
		if ('_' == *p) {
			tail = p + 1;
		}
	}

	u8 len = 0;
	while (tail[len] && (len < 8)) {
		len++;
	}

	// выравнивание вправо на разрядах 8..15
	for (s8 pos = 15; pos >= 8; pos--) {
		s8 idx = (s8)(len - 1 - (15 - pos));
		if (idx < 0) {
			lcd.show_val_pos(SPACE, (u8)pos);
		} else if ((tail[idx] >= '0') && (tail[idx] <= '9')) {
			lcd.show_val_pos((u8)(tail[idx] - '0'), (u8)pos);
		} else {
			lcd.show_val_pos(LINE, (u8)pos);
		}
	}
}
//----------------------------------------------------------------------------//
//  Экран поиска: "КОЛ У<n>" слева, серийник выбранного БУ на 8-15,
//  при неудаче подключения мигает S39
//----------------------------------------------------------------------------//
static void rim_draw_scan(void) {
	const bool blink = (((tim.get(TIMER_GENERAL) / BLINK_MS) & 1) != 0);
	char txt[7];

	// 1-6: "устр" + количество найденных БУ.
	// До 10 - одна цифра ("устр 7"), от 10 - две ("устр10"), максимум 99.
	{
		const u8 n = (bleDevCnt <= 99) ? bleDevCnt : 99;
		memcpy(txt, TXT_SCAN_PREFIX, 4); // "устр"
		if (n < 10) {
			txt[4] = ' ';
			txt[5] = (char)('0' + n);
		} else {
			txt[4] = (char)('0' + n / 10);
			txt[5] = (char)('0' + n % 10);
		}
		txt[6] = 0;
	}
	lcd.showText(txt);

	// 8-15: серийник выбранного устройства (пробелы, если не найдены)
	if (bleDevCnt) {
		if (devSel >= bleDevCnt) {
			devSel = 0;
		}
		rim_show_dev_name(bleDevList[devSel].name);
	} else {
		for (u8 p = 8; p <= 15; p++) {
			lcd.show_val_pos(SPACE, p);
		}
	}

	lcd.text(g_ble_fail && blink ? SHOW : HIDE, 1, S39);
	rim_show_batt(blink);
	ShowRSSI(rssiBLE);
}
//----------------------------------------------------------------------------//
//  Экран выбора пароля: 1-6 имя, 8-15 продолжение имени, 17-23 значение.
//  Листание левой, выбор правой. S46 горит на текущем активном слоте.
//----------------------------------------------------------------------------//
static u8 pwdSel = 0; // подсвеченный слот на экране выбора

//  Продолжение имени на разрядах from..to (символы имени начиная с skip):
//  цифры рисуем, прочие символы - прочерком (7-сегментник букв почти не умеет).
static void pwd_show_name_range(const char *name, u8 from, u8 to, u8 skip) {
	u8 len = (u8)strlen(name);
	u8 pos = from;
	for (u8 i = skip; (i < len) && (pos <= to); i++, pos++) {
		char c = name[i];
		if (c >= '0' && c <= '9') {
			lcd.show_val_pos((u8)(c - '0'), pos);
		} else {
			lcd.show_val_pos(LINE, pos);
		}
	}
	while (pos <= to) {
		lcd.show_val_pos(SPACE, pos++);
	}
}

static void rim_draw_pwd_select(void) {
	const password_t *p = Passwords_Get(pwdSel);
	if (!p) {
		return;
	}
	const bool blink = (((tim.get(TIMER_GENERAL) / BLINK_MS) & 1) != 0);

	// 1-6: имя пароля буквами (верхний ряд умеет A-Z, А-Я, цифры)
	char txt[7];
	u8 n = 0;
	for (u8 i = 0; (i < 6) && p->name[i]; i++) {
		txt[n++] = p->name[i];
	}
	while (n < 6) {
		txt[n++] = ' ';
	}
	txt[6] = 0;
	lcd.showText(txt);

	// 8-15: сам пароль (значение), без ведущих нулей
	{
		u32 v = p->value;
		for (s8 pos = 15; pos >= 8; pos--) {
			if ((v == 0) && (pos != 15)) {
				lcd.show_val_pos(SPACE, (u8)pos);
			} else {
				lcd.show_val_pos((u8)(v % 10), (u8)pos);
			}
			v /= 10;
		}
	}

	// 17-23: какой пароль по счёту - "N.10" (номер.всего), N от 1
	{
		const u8 num = (u8)(pwdSel + 1);  // 1..10
		const u8 tot = Passwords_Count(); // 10
		for (u8 pos = 17; pos <= 23; pos++) {
			lcd.show_val_pos(SPACE, pos);
		}
		// номер справа от 17-18, всего на 21-22, между ними прочерк на 20
		if (num >= 10) {
			lcd.show_val_pos(1, 17);
			lcd.show_val_pos(0, 18);
		} else {
			lcd.show_val_pos(num, 18);
		}
		lcd.show_val_pos(LINE, 20); // разделитель
		if (tot >= 10) {
			lcd.show_val_pos(1, 22);
			lcd.show_val_pos(0, 23);
		} else {
			lcd.show_val_pos(tot, 23);
		}
	}

	// S46 отмечает текущий активный (сохранённый) слот
	lcd.text((pwdSel == Passwords_Selected()) ? SHOW : HIDE, 1, S46);
	rim_show_batt(blink);
	ShowRSSI(rssiBLE);
}
//----------------------------------------------------------------------------//
//  Основной экран
//----------------------------------------------------------------------------//
static void rim_draw(void) {
	const bool blink = (((tim.get(TIMER_GENERAL) / BLINK_MS) & 1) != 0);
	const u8 alarm = rim_alarm_code(matrix);
	const bool exec = (matrix & (M_EXEC_ANY | M_EXEC_ON | M_EXEC_OFF)) != 0;

	// 1-6: текст выбранной команды или состояние отправки
	switch (sendState) {
	case SEND_WAIT:
		lcd.showText(TXT_SENDING);
		break;
	case SEND_OK:
		lcd.showText(TXT_SEND_OK);
		break;
	case SEND_ERR:
		lcd.showText(TXT_SEND_ERR);
		break;
	default:
		lcd.showText((char *)MENU[curCmd].text);
		break;
	}

	// Состояние ВВ:
	//   разомкнут (отключён) -> горит стрелка S42
	//   замкнут  (включён)   -> горит стрелка S44
	// Во время выполнения команды активная стрелка мигает.
	{
		const bool vvOn = (matrix & M_VV) != 0;
		const bool on = (!exec || blink);
		lcd.text((!vvOn && on) ? SHOW : HIDE, 1, S42);
		lcd.text((vvOn && on) ? SHOW : HIDE, 1, S44);
		lcd.text(HIDE, 1, S43);
	}

	// S46: заряд конденсаторов
	lcd.text((matrix & M_CAP) ? SHOW : HIDE, 1, S46);
	// S50: блокировка
	lcd.text((matrix & M_BLOCK) ? SHOW : HIDE, 1, S50);
	// S53: авария
	lcd.text(alarm ? SHOW : HIDE, 1, S53);

	// батарея и уровень сигнала
	rim_show_batt(blink);
	ShowRSSI(rssiBLE);

	// пункт "СЧ.ВО": локальный счётчик всегда на 8-15, глобальный на 17-23
	// (результат сброса виден по тексту УСПЕХ и обнулению самого счётчика)
	if (MENU_CNT_VIEW == curCmd) {
		lcd.show_long((s32)cntLocal, false, false, 0);
		rim_show_serial(cntGlobal);
		return;
	}

	// 8-15: код отказа при ОШИБКА; иначе код аварии; без аварии - пусто
	if ((SEND_ERR == sendState) && lastErr) {
		lcd.show_long(lastErr, false, false, 0);
	} else if (alarm) {
		lcd.show_long(alarm, false, false, 0);
	} else {
		for (u8 p = 8; p <= 15; p++) {
			lcd.show_val_pos(SPACE, p);
		}
	}

	// 17-23: заводской номер, при аварии - мигает "Error"
	if (alarm && blink) {
		rim_show_alarm_top();
	} else {
		rim_show_serial(serialBU);
	}
}
//----------------------------------------------------------------------------//
// Флаг запуска: ставит thread_main после первичной настройки
// (номер пульта / счётчика / пин). До него поток не трогает ЖКИ
// и не съедает сообщения кнопок у экрана ввода номера.
volatile bool g_rim_enable = false;

PT_THREAD(thread_rim_protocol(pt_t *pt)) {
	PT_BEGIN(pt);
	PT_WAIT_UNTIL(pt, g_rim_enable);
	msg.set(MSG_BLOCK_STATUS_THREAD, NULL, 0); // штатный индикатор статуса гасим
	while (1) {
		//--- ЭКРАН 1: выбор пароля (до поиска устройства) -----------------//
		lcd.clear();
		pwdSel = Passwords_Selected(); // начинаем с текущего активного
		while (BLE_STATE_CONNECT != SystemInfo.BLE_State) {
			// ЛЕВАЯ - листаем пароли
			if (msg.getVal(MSG_SELECT_CMD)) {
				pwdSel = (u8)((pwdSel + 1) % Passwords_Count());
				tim.reset(TIMER_DISABLE);
			}
			// ПРАВАЯ - зафиксировать пароль (только в ОЗУ, без флеша!) и дальше
			if (msg.getVal(MSG_SEND_CMD)) {
				Passwords_SelectRAM(pwdSel); // мгновенно, не блокируем ядро/BLE
				tim.reset(TIMER_DISABLE);
				break;
			}
			rim_draw_pwd_select();
			PT_YIELD(pt);
		}
		if (BLE_STATE_CONNECT == SystemInfo.BLE_State) {
			goto rim_connected;
		}

		//--- ЭКРАН 2: поиск и выбор устройства ----------------------------//
		lcd.clear();
		devSel = 0;
		while (BLE_STATE_CONNECT != SystemInfo.BLE_State) {
			// ЛЕВАЯ кнопка - листаем список найденных БУ
			if (msg.getVal(MSG_SELECT_CMD) && bleDevCnt) {
				devSel = (u8)((devSel + 1) % bleDevCnt);
				g_ble_fail = false;
				tim.reset(TIMER_DISABLE); // активность: таймер отключения
			}
			// ПРАВАЯ кнопка (удержание) - подключаемся к выбранному
			if (msg.getVal(MSG_SEND_CMD) && bleDevCnt) {
				app_ble_connect_to(devSel);
				tim.reset(TIMER_DISABLE);
				_SYSLOG_("password ver");
			}
			// пока идёт попытка подключения - не даём пульту уснуть
			if (g_connecting) {
				tim.reset(TIMER_DISABLE);
			}
			_SYSLOG_("[RIM dont't sleep]");

			rim_draw_scan();
			PT_YIELD(pt);
		}

	rim_connected:
		g_ble_fail = false;
		_SYSLOG_("[RIM false]");

		PT_DELAY_MS(TIMER_DELAY, 100);
		Passwords_SaveSelected(); // теперь BLE установлен - пишем флеш
		lcd.clear();
		curCmd = 0;
		matrix = 0;
		sendState = SEND_IDLE;
		_SYSLOG_("RIM READY");

		rim_send(COP_SERVICE); // сразу запросим заводской номер
		tim.reset(TIMER_DELAY_REQUEST);

		while (BLE_STATE_CONNECT == SystemInfo.BLE_State) {
			//--- приём (ответ склеивается транспортом из фрагментов) ------//
			if (msg.getVal(MSG_RADIO_ANSWER_OK)) {
				u8 rxLen;
				const u8 *rx = ble_transport_rx_data(&rxLen);
				if (rim_parse(rx, rxLen)) {
					ble_transport_rx_clear(); // кадр разобран
				}
			}

			//--- ЛЕВАЯ кнопка: смена команды ------------------------------//
			if (msg.getVal(MSG_SELECT_CMD)) {
				curCmd = (u8)((curCmd + 1) % MENU_SIZE);
				sendState = SEND_IDLE;
				_SYSLOG_("RIM MENU %d", curCmd);
			}

			//--- ПРАВАЯ кнопка (удержание): отправка ---------------------//
			if (msg.getVal(MSG_SEND_CMD)) {
				if (MENU[curCmd].cop) {
					rim_send(MENU[curCmd].cop);
					sendState = SEND_WAIT;
					tim.reset(TIMER_WAIT_BETWEEN_REQ);
				} else if (MENU_CNT_VIEW == curCmd) {
					rim_send(COP_CNT_RESET); // сброс локального счётчика
					sendState = SEND_WAIT;
					tim.reset(TIMER_WAIT_BETWEEN_REQ);
					_SYSLOG_("RIM RESET LOCAL CNT");
				} else if (MENU_EXIT == curCmd) {
					_SYSLOG_("RIM EXIT - disconnect");
					hp_ble_disconnect(); // штатное отключение
				}
			}

			//--- ОБЕ кнопки: аварийное ОТКЛ ------------------------------//
			if (msg.getVal(MSG_EMERGENCY_OFF)) {
				curCmd = 1; // ОТКЛ
				rim_send(COP_OFF_VV);
				sendState = SEND_WAIT;
				tim.reset(TIMER_WAIT_BETWEEN_REQ);
				_SYSLOG_("RIM EMERGENCY OFF");
			}

			//--- периодика телеметрии ------------------------------------//
			if (tim.get(TIMER_DELAY_REQUEST) > POLL_PERIOD_MS) {
				tim.reset(TIMER_DELAY_REQUEST);
				rim_send(COP_TELEMETRY);
			}

			//--- таймауты индикации отправки -----------------------------//
			if ((SEND_WAIT == sendState) &&
			    (tim.get(TIMER_WAIT_BETWEEN_REQ) > SEND_TIMEOUT_MS)) {
				sendState = SEND_ERR; // ответа нет
				lastErr = 0;
				tim.reset(TIMER_WAIT_BETWEEN_REQ);
				_SYSLOG_("RIM TIMEOUT");
			}
			if (((SEND_OK == sendState) || (SEND_ERR == sendState)) &&
			    (tim.get(TIMER_WAIT_BETWEEN_REQ) > ACK_SHOW_MS)) {
				sendState = SEND_IDLE;
			}

			tim.reset(TIMER_DISABLE); // пока есть связь - пульт не засыпает

			rim_draw();
			PT_YIELD(pt);
		}
		lcd.clear();
		tim.reset(TIMER_DISABLE); // после разрыва отсчёт бездействия заново
	}
	PT_END(pt);
}
//----------------------------------------------------------------------------//