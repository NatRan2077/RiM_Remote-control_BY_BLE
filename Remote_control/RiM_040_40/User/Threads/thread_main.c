//----------------------------------------------------------------------------//
#include "threads.h"
#include "lcd.h"
#include "dlmsListObj.h"
#include "messages.h"
#include "log.h"
#include "globals.h"
#include "HAL.h"
#include "utils.h"
#include "memory.h"
#include "math.h"
//----------------------------------------------------------------------------//
#define e0 1U
#define e1 10U
#define e2 100U
#define e3 1000U
#define e4 10000U
#define e5 100000UL
#define e6 1000000UL
#define e7 10000000UL
//----------------------------------------------------------------------------//
pt_t main_pt;
pt_t manualInput_pt;
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
#define TIME_INPUT_WAIT_BLINK_SHOW 500
#define TIME_INPUT_WAIT_BLINK_HIDE 1000
#define TIME_INPUT_BLINK_SHOW 150
#define TIME_INPUT_BLINK_HIDE 300
//----------------------------------------------------------------------------//
void decomposeToArray(u32 number, u8 array[], u8 digits) {
	// Заполняем массив с конца к началу
	for (int i = digits - 1; i >= 0; i--) {
		array[i] = number % 10;
		number /= 10;
	}
}
//----------------------------------------------------------------------------//
void clearBtnMsg(void) {
	msg.getVal(MSG_BTN_1_PUSH_SHORT);
	msg.getVal(MSG_BTN_1_PUSH_MID);
	msg.getVal(MSG_BTN_2_PUSH_SHORT);
	msg.getVal(MSG_BTN_2_PUSH_MID);
	msg.getVal(MSG_BTN_1_2_COMBINATION_MID);
}
//----------------------------------------------------------------------------//
PT_THREAD(child_manualInput(pt_t *pt, u32 *pNumber, u8 numbOfDigits,
                            bool fZeroOk)) {
	static u8 pos;
	static u16 tmp;
	static u8 numArray[8];
	static u8 allDigitsFilled = 0;
	static bool exitPT;
	PT_BEGIN(pt);
	pos = 0;
	allDigitsFilled = 0;
	exitPT = false;

	// Принудительная очистка ВСЕГО массива
	for (u8 i = 0; i < sizeof(numArray); i++) {
		numArray[i] = LINE;
	}

	for (u8 i = 0; i < sizeof(numArray); numArray[i++] = LINE)
		; // Заполняем массив пустыми значениями (LINE)
	if (pNumber && (*pNumber < (u32)pow(10, numbOfDigits))) {
		decomposeToArray(*pNumber, numArray, numbOfDigits);
	}
	tim.reset(TIMER_CHILD);
	clearBtnMsg();
	do {
		PT_YIELD_SPAWN(pt);
		tmp = tim.get(TIMER_CHILD);
		if ((TIME_INPUT_WAIT_BLINK_SHOW < tmp) &&
		    (TIME_INPUT_WAIT_BLINK_HIDE > tmp)) {
			for (u8 i = 0; i < numbOfDigits; i++) {
				lcd.show_val_pos(numArray[i], (i + 8));
			}
			if (msg.getVal(MSG_BTN_1_PUSH_SHORT)) {
				break;
			}
			if (msg.getVal(MSG_BTN_2_PUSH_MID)) {
				exitPT = true;
				break;
			}
		} else if (tmp > TIME_INPUT_WAIT_BLINK_HIDE) {
			tim.reset(TIMER_CHILD);
			lcd.clear_data();
		}
	} while (1);

	pos = 0;
	allDigitsFilled = 0;

	clearBtnMsg();
	while (!exitPT) {
		PT_YIELD_SPAWN(pt);
		if (msg.getVal(MSG_BTN_1_PUSH_SHORT)) {
			numArray[pos]++;
			if (10 == numArray[pos]) {
				numArray[pos] = 0;
			}
			tim.reset(
			    TIMER_CHILD); // Сбрасываем таймер мигания при изменении цифры
			lcd.show_val_pos(numArray[pos], pos + 8);
		}
		if (msg.getVal(MSG_BTN_2_PUSH_SHORT) && (numArray[pos] != LINE)) {
			lcd.show_val_pos(numArray[pos], pos + 8);
			pos++;
			if (pos == numbOfDigits) {
				pos = 0;
			}
			tim.reset(TIMER_CHILD);
		}

		// Обработка мигания курсора
		tmp = tim.get(TIMER_CHILD);
		if (pos < numbOfDigits) {
			if (tmp < TIME_INPUT_BLINK_SHOW) {
				lcd.show_val_pos(numArray[pos], pos + 8);
				// Кнопка 2 долго - завершение ввода
				if (msg.getVal(MSG_BTN_2_PUSH_MID)) {
					allDigitsFilled = 1; // Проверяем что все цифры заполнены
					for (u8 i = 0; i < numbOfDigits; i++) {
						if (numArray[i] == LINE) {
							allDigitsFilled = 0;
							break;
						}
					}
					if (allDigitsFilled) {
						if (fZeroOk) {
							break; // Все цифры заполнены - выходим
						} else {
							*pNumber = 0;
							for (u8 i = 0; i < numbOfDigits; i++) {
								*pNumber = *pNumber * 10 + numArray[i];
							}
							if (*pNumber != 0) {
								break;
							}
						}
					}
				}
			} else if (tmp < TIME_INPUT_BLINK_HIDE) {
				lcd.show_val_pos(SPACE, pos + 8); // Мигающий символ
			} else {
				tim.reset(TIMER_CHILD);
			}
		}
	}
	if (pNumber) {
		*pNumber = 0;
		for (u8 i = 0; i < numbOfDigits; i++) {
			*pNumber = *pNumber * 10 + numArray[i];
		}
	}
	PT_END(pt);
}
//----------------------------------------------------------------------------//
void lcd_out(u8 dlmsElement) { // Возвращает ошибку
	lcd.clear_OBIS_data();
	lcd.text(HIDE, 19, k, PP1, PP2, PP2_1, PP3, PP4, PP4_1, PP5, PP6, PP6_1, PP7,
	         COLON_1, COLON_2, V, t, A, r, h, Hz);
	lcd.clear_data();
	lcd.clearText();
	if (DLMS_NO_DATA == dlmsElement) {
		lcd.showText("NODATA");
		lcd.text(SHOW, 1, CP2);
	} else {
		ShowOBIS((objDLMS_t)dlmsElement);
		//		_SYSLOG_("DLMS - OBIS: %u.%u.%u.%u.%u.%u",
		//dlms.obj[dlmsElement].pConst->OBIS[0],
		//dlms.obj[dlmsElement].pConst->OBIS[1],
		//dlms.obj[dlmsElement].pConst->OBIS[2],
		//dlms.obj[dlmsElement].pConst->OBIS[3],
		//dlms.obj[dlmsElement].pConst->OBIS[4],
		//dlms.obj[dlmsElement].pConst->OBIS[5]);
		ShowData(dlms.obj[dlmsElement]);
	}
}

typedef struct {      // Структура для навигации
	u8 currentGroup;   // текущая группа
	u8 currentElement; // текущий элемент в группе
	u8 firstElement;   // первый доступный элемент в группе
	u8 lastElement;    // последний доступный элемент в группе
	u8 elementCount;   // количество доступных элементов в группе
} NavigationState_t;

NavigationState_t nav = {0};

u8 find_first_available_in_group(
    u8 group) { // Поиск первого доступного элемента в группе
	for (u8 i = 0; i < DLMS_MAX_OBJ; i++) {
		if (dlms.obj[i].fShow && dlms.obj[i].pConst->group == group) {
			return i;
		}
	}
	return 0xFF; // нет доступных элементов в группе
}

u8 find_last_available_in_group(
    u8 group) { // Поиск последнего доступного элемента в группе
	for (u8 i = DLMS_MAX_OBJ - 1; i > 0; i--) {
		if (dlms.obj[i].fShow && dlms.obj[i].pConst->group == group) {
			return i;
		}
	}
	return 0xFF; // нет доступных элементов в группе
}

u8 count_available_in_group(
    u8 group) { // Подсчет количества доступных элементов в группе
	u8 count = 0;
	for (u8 i = 0; i < DLMS_MAX_OBJ; i++) {
		if (dlms.obj[i].fShow && dlms.obj[i].pConst->group == group) {
			count++;
		}
	}
	return count;
}

void update_group_info(void) { // Обновление информации о текущей группе
	nav.firstElement = find_first_available_in_group(nav.currentGroup);
	nav.lastElement = find_last_available_in_group(nav.currentGroup);
	nav.elementCount = count_available_in_group(nav.currentGroup);

	// Если текущий элемент не входит в группу или не доступен, устанавливаем
	// первый
	if (nav.elementCount > 0) {
		if (!dlms.obj[nav.currentElement].fShow ||
		    dlms.obj[nav.currentElement].pConst->group != nav.currentGroup) {
			nav.currentElement = nav.firstElement;
		}
	} else {
		msg.set(MSG_BTN_2_PUSH_MID, NULL, 0);
	}
}

void init_navigation(void) { // Инициализация навигации
	// Начинаем с первой группы, в которой есть элементы
	for (u8 group = 1; group <= MAX_GROUP; group++) {
		if (count_available_in_group(group) > 0) {
			nav.currentGroup = group;
			break;
		}
	}
	update_group_info();
}

u8 get_element_index_in_group(
    void) { // Получить индекс элемента внутри группы (для отображения "X/Y")
	u8 index = 1;
	for (u8 i = nav.firstElement; i < nav.currentElement; i++) {
		if (dlms.obj[i].fShow && dlms.obj[i].pConst->group == nav.currentGroup) {
			index++;
		}
	}
	return index;
}

//----------------------------------------------------------------------------//
#define TYPE_DEVICE 04005
extern s8 rssiBLE;
//----------------------------------------------------------------------------//
PT_THREAD(thread_main(pt_t *pt)) {
	volatile static u32 tmp;
	static u32 tmp1;
	static bool showDiag = false;
	static bool settingMode = false;
	static bool fProcessConnectRelay = false;
	static s8 timerConnectRelay = 0;
	static char str[10];
	static u8 relayConnectResult = 0;

	PT_BEGIN(pt);
	PT_WAIT_UNTIL(
	    pt, ((V_BATT_STATUS_NONE != SystemInfo.status.vBatt) &&
	         (MODE_NONE !=
	          SystemInfo.status.manufactoryMode))); // Ждем измерения батарейки
	_SYSLOG_("[SYSTEM] - Battary Voltage: %.3f V",
	         ((float)(SystemInfo.Vcc.Batt) / 1000));
	if ((V_BATT_STATUS_LOWER == SystemInfo.status.vBatt) &&
	    (!SystemInfo.status.usbPwr) &&
	    (MODE_OFF ==
	     SystemInfo.status
	         .manufactoryMode)) { // Если батарейка разряжена и питание не от USB
		                           // и не режим калибровки
		PT_WAIT_UNTIL(pt, (IS_BUTTON_1_UNPRESSED &&
		                   IS_BUTTON_2_UNPRESSED)); // ждем пока отпутят кнопки
		PT_SPAWN(pt, &blinkingBatt_pt,
		         thread_blinkingBatt(
		             &blinkingBatt_pt)); // мигаем батарейкой 10 сек и выключаемся
	}
	tim.reset(TIMER_DELAY);
	msg.set(MSG_BLOCK_STATUS_THREAD, NULL, 0);
	do {
		if (tim.get(TIMER_DELAY) > 5000) {
			settingMode = true;
			lcd.all();
		}
		if (msg.getVal(
		        MSG_BTN_1_2_COMBINATION_MID)) { // Если обе кнопки были длительно
			                                     // нажаты, то выставляем флаг
			                                     // режима настройки
			showDiag = true;
		}
		PT_YIELD(pt);
	} while (IS_BUTTON_1_PRESSED &&
	         IS_BUTTON_2_PRESSED); // Ждем пока отпустят кнопки
	clearBtnMsg();

	if (settingMode || showDiag ||
	    (MODE_ON == SystemInfo.status.manufactoryMode)) { // Если режим настройки
		                                                   // или диагностики или
		                                                   // замкнута перемычка
		lcd.all(); // Отображаем все символы
		PT_DELAY_MS(TIMER_DELAY, WAIT_TIME);
		lcd.clear();
		PT_DELAY_MS(TIMER_DELAY, WAIT_TIME / 4);
	}
	msg.set(MSG_BLOCK_STATUS_THREAD, NULL, 0);
	if (settingMode || showDiag) {
		tmp1 = (FW_VERSION_MAJOR * 1000000) + (FW_VERSION_MINOR * 10000) +
		       (FW_VERSION_PATCH * 100) + FW_VERSION_BUILD;
		lcd.show_long(tmp1, false, false, 2); // номер прошивки
		lcd.text(SHOW, 3, PP2, PP4, PP6);
		lcd.showText("ПО ДД");
		if (strcmp(FW_VERSION_PRERELEASE, "-a") == 0) {
			lcd.show_val_pos(CHAR_A, 17);
			lcd.show_val_pos(CHAR_L, 18);
			lcd.show_val_pos(CHAR_F, 19);
			lcd.show_val_pos(CHAR_A, 20);
		} else if (strcmp(FW_VERSION_PRERELEASE, "-b") == 0) {
			lcd.show_val_pos(CHAR_B, 17);
			lcd.show_val_pos(CHAR_E, 18);
			lcd.show_val_pos(CHAR_T_1, 19);
			lcd.show_val_pos(CHAR_T_2, 20);
			lcd.show_val_pos(CHAR_A, 21);
		} else if (strcmp(FW_VERSION_PRERELEASE, "-rc") == 0) {
			lcd.show_val_pos(CHAR_r, 17);
			lcd.show_val_pos(CHAR_c, 18);
			lcd.text(SHOW, 2, OP1, OP2);
		}
		PT_DELAY_MS(TIMER_DELAY, WAIT_TIME);
		lcd.clear();
		PT_DELAY_MS(TIMER_DELAY, WAIT_TIME / 4);
	}

	if ((0 == SystemInfo.serialNumber) ||
	    (SystemInfo.serialNumber >=
	     100000000)) { // Если номер пульта 0, то входим в режим ввода
		lcd.showText("НOМ ДД");
		lcd.text(SHOW, 1, CP3);
		PT_DELAY_MS(TIMER_DELAY, 500);
		tmp1 = SystemInfo.serialNumber;
		PT_SPAWN(pt, &manualInput_pt,
		         child_manualInput(&manualInput_pt, &tmp1, 8, false));
		if (RESULT_ERROR ==
		    Memory_Write_And_Update_Status(MEMORY_SERIAL_NUMBER, &tmp1,
		                                   sizeof(SystemInfo.serialNumber))) {
			_SYSLOG_("Config - Error Write Meter Number");
			lcd.showText("MEMERR");
			lcd.text(SHOW, 1, CP3);
			PT_SPAWN(pt, &blinkingLines_pt,
			         thread_blinkingLines(&blinkingLines_pt)); // то отключаемся
		}
		if (RESULT_ERROR == memory.readTwoCopyesWithCRC(
		                        MEMORY_SERIAL_NUMBER, &SystemInfo.serialNumber,
		                        sizeof(SystemInfo.serialNumber))) {
			_SYSLOG_("Config - Error Read Meter Number");
			lcd.showText("MEMERR");
			lcd.text(SHOW, 1, CP3);
			PT_SPAWN(pt, &blinkingLines_pt,
			         thread_blinkingLines(&blinkingLines_pt)); // то отключаемся
		}
		_SYSLOG_("Config - Save Number Pult: %u", SystemInfo.serialNumber);
	}
	lcd.showText("НOМ ДД");
	lcd.text(SHOW, 1, CP3);
	lcd.show_long(SystemInfo.serialNumber, true, false,
	              0); // Показываем номер пульта
	PT_DELAY_MS(TIMER_DELAY, WAIT_TIME);
	lcd.clear();
	PT_DELAY_MS(TIMER_DELAY, WAIT_TIME / 4);

	// settingMode = true;
	if (settingMode || (0 == SystemInfo.numMeter) ||
	    (SystemInfo.numMeter >= 100000000) ||
	    (SystemInfo.pinCode >=
	     1000000)) { // Если режим настройки или номер счетчика равен 0
		lcd.showText("НOМ СЧ");
		lcd.text(SHOW, 1, CP3);
		_SYSLOG_("Config - Manual Input Meter Number");
		PT_DELAY_MS(TIMER_DELAY, 500);
		tmp1 = SystemInfo.numMeter;
		PT_SPAWN(pt, &manualInput_pt,
		         child_manualInput(&manualInput_pt, &tmp1, 8, false));
		if (RESULT_ERROR ==
		    Memory_Write_And_Update_Status(MEMORY_METER_NUMBER, &tmp1,
		                                   sizeof(SystemInfo.numMeter))) {
			_SYSLOG_("Config - Error Write Meter Number");
			lcd.showText("MEMERR");
			lcd.text(SHOW, 1, CP3);
			PT_SPAWN(pt, &blinkingLines_pt,
			         thread_blinkingLines(&blinkingLines_pt)); // то отключаемся
		}
		if (RESULT_ERROR ==
		    memory.readTwoCopyesWithCRC(MEMORY_METER_NUMBER, &SystemInfo.numMeter,
		                                sizeof(SystemInfo.numMeter))) {
			_SYSLOG_("Config - Error Read Meter Number");
			lcd.showText("MEMERR");
			lcd.text(SHOW, 1, CP3);
			PT_SPAWN(pt, &blinkingLines_pt,
			         thread_blinkingLines(&blinkingLines_pt)); // то отключаемся
		}
		_SYSLOG_("Config - Save Number Pult: %u", SystemInfo.numMeter);

		PT_DELAY_MS(TIMER_DELAY, 1000);
		PT_WAIT_UNTIL(pt, msg.getVal(MSG_BTN_1_PUSH_SHORT));

		lcd.clear();
		PT_DELAY_MS(TIMER_DELAY, 250);

		lcd.showText("ПИНКОД");
		_SYSLOG_("Config - Manual Input Meter Number");
		PT_DELAY_MS(TIMER_DELAY, 250);
		tmp1 = SystemInfo.pinCode;
		PT_SPAWN(pt, &manualInput_pt,
		         child_manualInput(&manualInput_pt, &tmp1, 6, true));
		if (RESULT_ERROR ==
		    Memory_Write_And_Update_Status(MEMORY_PINCODE, &tmp1,
		                                   sizeof(SystemInfo.pinCode))) {
			_SYSLOG_("Config - Error Write PinCode");
			lcd.showText("MEMERR");
			lcd.text(SHOW, 1, CP3);
			PT_SPAWN(pt, &blinkingLines_pt,
			         thread_blinkingLines(&blinkingLines_pt)); // то отключаемся
		}
		if (RESULT_ERROR ==
		    memory.readTwoCopyesWithCRC(MEMORY_PINCODE, &SystemInfo.pinCode,
		                                sizeof(SystemInfo.pinCode))) {
			_SYSLOG_("Config - Error Read PinCode");
			lcd.showText("MEMERR");
			lcd.text(SHOW, 1, CP3);
			PT_SPAWN(pt, &blinkingLines_pt,
			         thread_blinkingLines(&blinkingLines_pt)); // то отключаемся
		}
		_SYSLOG_("Config - Save PinCode: %u", SystemInfo.pinCode);
		PT_DELAY_MS(TIMER_DELAY, 1000);
		lcd.clear();
		PT_DELAY_MS(TIMER_DELAY, 250);
	}
	// Далее переходим к подключению BLE
	msg.set(MSG_BLE_START, NULL, 0);
	tim.reset(TIMER_DELAY);
	lcd.showText("ПОИСК ");
	static u8 cnt = 0;
	do {
		tim.reset(TIMER_RSSI);
		tmp = tim.get(TIMER_DELAY);
		if (tmp < 500) {
			rssiBLE = -100;
		} else {
			if (tmp >= 1000) {
				tim.reset(TIMER_DELAY);
				if (tim.get(TIMER_DISABLE) > 5000) {
					cnt++;
				}
				if (cnt >= 10) {
					rssiBLE = -100;
					lcd.clear();
					lcd.showText("НЕТ СВ");
					PT_SPAWN(pt, &blinkingLines_pt,
					         thread_blinkingLines(&blinkingLines_pt));
				}
			}
			rssiBLE = 0;
		}
		if (msg.getVal(MSG_BLE_CONNECT_SUCCESS)) {
			break;
		}
		PT_YIELD(pt);
	} while (1);
	lcd.clear();
	msg.set(MSG_PULT_CONNECT_TO_METER, NULL, 0);
	// thread_rim_protocol теперь владеет дисплеем и кнопками.
	// Ждём дисконнекта и уходим в начало (не крадём MSG_BTN, не рисуем НОМ СЧ).
	PT_WAIT_UNTIL(pt, BLE_STATE_CONNECT != SystemInfo.BLE_State);

	// (убрано: кража MSG_BTN у thread_rim_protocol)

	lcd.show_long(SystemInfo.numMeter, true, false,
	              0); // Показываем номер счетчика
	lcd.showText("НOМ СЧ");
	lcd.text(SHOW, 1, CP3);

	do {
		if (msg.getVal(MSG_NO_ANSWER)) { // нет ответа
			lcd.clear();
			lcd.text(SHOW, 1, CP3);
			lcd.showText("НЕТОТВ");
			PT_SPAWN(pt, &blinkingLines_pt,
			         thread_blinkingLines(&blinkingLines_pt));
		}
		if (msg.getVal(MSG_DATA_UPDATE)) {
			break;
		}
		PT_YIELD(pt);
	} while (1);

	init_navigation();

	PT_WAIT_UNTIL(pt, (msg.getVal(MSG_BTN_1_PUSH_SHORT) ||
	                   msg.getVal(MSG_BTN_2_PUSH_SHORT)));
	lcd.clear_data();
	lcd.clearText();
	clearBtnMsg();
	msg.set(MSG_UNBLOCK_STATUS, NULL, 0);
	while (1) {
		if (msg.getVal(MSG_DATA_UPDATE)) {
			update_group_info();
		}

		if (msg.getVal(MSG_BTN_2_PUSH_SHORT)) {
			if (nav.elementCount) {          // есть элементы
				u8 next = nav.currentElement; // Поиск следующего доступного
				                              // элемента
				do {
					next++;
					if (next >= DLMS_MAX_OBJ) {
						next = 0;
					}

					// Проверяем, принадлежит ли элемент текущей группе и доступен ли
					if (dlms.obj[next].fShow &&
					    dlms.obj[next].pConst->group == nav.currentGroup) {
						nav.currentElement = next;
						break;
					}
				} while (next != nav.currentElement); // вернулись к началу
				_SYSLOG_("Navigation: Group %d, Element %d/%d", nav.currentGroup,
				         get_element_index_in_group(), nav.elementCount);
			}
		}
		if (msg.getVal(MSG_BTN_1_PUSH_SHORT)) {
			if (nav.elementCount) { // есть элементы
				u8 previous =
				    nav.currentElement; // Поиск следующего доступного элемента
				do {
					previous--;
					if (previous > DLMS_MAX_OBJ) {
						previous = DLMS_MAX_OBJ - 1;
					}

					// Проверяем, принадлежит ли элемент текущей группе и доступен ли
					if (dlms.obj[previous].fShow &&
					    dlms.obj[previous].pConst->group == nav.currentGroup) {
						nav.currentElement = previous;
						break;
					}
				} while (previous != nav.currentElement); // вернулись к началу
				_SYSLOG_("Navigation: Group %d, Element %d/%d", nav.currentGroup,
				         get_element_index_in_group(), nav.elementCount);
			}
		}

		if (msg.getVal(MSG_BTN_2_PUSH_MID)) {
			u8 startGroup = nav.currentGroup;
			do { // Поиск следующей группы с доступными элементами
				nav.currentGroup++;
				if (nav.currentGroup > MAX_GROUP) {
					nav.currentGroup = 1; // зацикливаем
				}

				u8 firstInGroup = find_first_available_in_group(nav.currentGroup);
				if (firstInGroup != 0xFF) { // нашли группу с элементами
					update_group_info();
					nav.currentElement = nav.firstElement;

					_SYSLOG_("Navigation: Switched to Group %d, %d elements",
					         nav.currentGroup, nav.elementCount);
					break;
				}
			} while (nav.currentGroup != startGroup); // обошли все группы

			_SYSLOG_("Navigation: No other groups with available elements");
		}

		if (msg.getVal(MSG_BTN_1_PUSH_MID)) {
			u8 startGroup = nav.currentGroup;
			do { // Поиск следующей группы с доступными элементами
				nav.currentGroup--;
				if (nav.currentGroup == 0) {
					nav.currentGroup = MAX_GROUP; // зацикливаем
				}

				u8 firstInGroup = find_first_available_in_group(nav.currentGroup);
				if (firstInGroup != 0xFF) { // нашли группу с элементами
					update_group_info();
					nav.currentElement = nav.firstElement;

					_SYSLOG_("Navigation: Switched to Group %d, %d elements",
					         nav.currentGroup, nav.elementCount);
					break;
				}
			} while (nav.currentGroup != startGroup); // обошли все группы

			_SYSLOG_("Navigation: No other groups with available elements");
		}

		if (DLMS_LCD_DIAG == nav.currentElement) { // если диагностика
			msg.set(MSG_SWITCH_BLOCK_STATUS_THREAD, NULL, 0);
			do {
				PT_WAIT_UNTIL(pt, IS_BUTTON_1_UNPRESSED && IS_BUTTON_2_UNPRESSED);
				if (msg.getVal(MSG_BTN_1_PUSH_SHORT)) {
					msg.set(MSG_BTN_1_PUSH_SHORT, NULL, 0);
					break;
				}
				if (msg.getVal(MSG_BTN_2_PUSH_SHORT)) {
					msg.set(MSG_BTN_2_PUSH_SHORT, NULL, 0);
					break;
				}
				if (msg.getVal(MSG_BTN_1_PUSH_MID) &&
				    msg.getVal(MSG_BTN_2_PUSH_MID)) {
					msg.set(MSG_SET_DIAGNOSIS, NULL, 0);
					PT_WAIT_UNTIL(pt, msg.getVal(MSG_RECEIVE_SET_DIAGNOSIS) ||
					                      (TIME_WAIT_TO_ANSWER <
					                       tim.get(TIMER_DIAGNOSIS_REQ)));
				}
				PT_YIELD(pt);
			} while (1);
			msg.set(MSG_SWITCH_BLOCK_STATUS_THREAD, NULL, 0);

		} else {
			lcd_out(nav.currentElement);
		}

		if ((msg.getVal(MSG_BTN_1_2_COMBINATION_MID)) &&
		    (READY_FOR_RECONNECTION == dlms.status.relay)) {
			if (fProcessConnectRelay) {
				timerConnectRelay = 66;
			} else {
				_SYSLOG_("Relay - Go Countdown To Connect");
				relayConnectResult = 0;
				fProcessConnectRelay = true;
				timerConnectRelay = 64;
			}
		}

		if (fProcessConnectRelay) {
			tim.reset(TIMER_DISABLE);
			if (66 == timerConnectRelay) {
				lcd.showText("ОТМЕНА");
			} else if (65 == timerConnectRelay) {
				fProcessConnectRelay = false;
				lcd.showText("      ");
			} else if (63 == timerConnectRelay) {
				lcd.showText("РЕЛЕ  ");
			} else if (62 == timerConnectRelay) {
				lcd.showText("ВКЛЮЧ ");
			} else if (61 == timerConnectRelay) {
				lcd.showText("ЧЕРЕЗ ");
			} else if (timerConnectRelay >= 0) { // Если таймер не вышел, то
				                                  // отображаем время до включения
				snprintf(str, 7, "ОСТ %02dс", timerConnectRelay);
				lcd.text(SHOW, 1, CP3);
				lcd.showText(str);
			} else if (0 > timerConnectRelay) {
				lcd.text(HIDE, 1, CP3);
				if (relayConnectResult == 0) {
					lcd.showText("ЖДЕМ  ");
					lcd.text(SHOW, 2, CP4, CP5);
				} else if (relayConnectResult == 1) {
					lcd.showText("ОШИБКА");
					lcd.text(HIDE, 2, CP4, CP5);
				} else if (relayConnectResult == 2) {
					lcd.showText("УСПЕХ ");
					lcd.text(HIDE, 2, CP4, CP5);
				}
				if (relayConnectResult && (timerConnectRelay < -4)) {
					fProcessConnectRelay = false;
					lcd.showText("      ");
					lcd.text(HIDE, 2, CP4, CP5);
				}
				if ((relayConnectResult == 0) && (timerConnectRelay < -5)) {
					timerConnectRelay = -1;
					tim.reset(TIMER_RELAY_CONNECT);
					relayConnectResult = 1;
				}
			}
			if (tim.get(TIMER_RELAY_CONNECT) > 1000) {
				if (timerConnectRelay == 0) {
					msg.set(MSG_REQWEST_RELAY_CONNECT, NULL, 0);
				}
				tim.reset(TIMER_RELAY_CONNECT);
				timerConnectRelay--;
			}
			if (msg.getVal(MSG_RELAY_CONNECT_ERROR)) {
				relayConnectResult = 1;
				timerConnectRelay = -1;
				tim.reset(TIMER_RELAY_CONNECT);
			}
			if (msg.getVal(MSG_RELAY_CONNECT_SUCCESS)) {
				relayConnectResult = 2;
				timerConnectRelay = -1;
				tim.reset(TIMER_RELAY_CONNECT);
			}
		}
		PT_YIELD(pt);
	}
	PT_END(pt);
}
//----------------------------------------------------------------------------//
