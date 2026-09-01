//----------------------------------------------------------------------------//
#include "threads.h"
#include "common_types.h"
#include "cmt453x_gpio.h"
#include "cmt453x_rcc.h"
#include "messages.h"
#include "log.h"
#include "HAL.h"
#include "globals.h"
//----------------------------------------------------------------------------//
pt_t btn_pt;
//----------------------------------------------------------------------------//
typedef enum {
	BTN_RELEASE = (u8)0x00,
	BTN_MAY_PUSH = (u8)0x01,
	BTN_PUSH = (u8)0x02,
	BTN_MAY_RELEASE = (u8)0x03,
} BTN_STATE_TypeDef;
//----------------------------------------------------------------------------//
// BUTTON
#define BTN_GPIO GPIOB
#define BTN_1_Pin GPIO_PIN_1
#define BTN_2_Pin GPIO_PIN_2
//----------------------------------------------------------------------------//
#define IS_BUTTON_1_PRESSED (BTN_1_Pin == (BTN_GPIO->PID & BTN_1_Pin))
#define IS_BUTTON_1_UNPRESSED (!IS_BUTTON_1_PRESSED)
#define IS_BUTTON_2_PRESSED (BTN_2_Pin == (BTN_GPIO->PID & BTN_2_Pin))
#define IS_BUTTON_2_UNPRESSED (!IS_BUTTON_2_PRESSED)
//----------------------------------------------------------------------------//
void BTN_Init(void) { // ������������� ������
	GPIO_InitType GPIO_InitStructure;
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);

	GPIO_InitStruct(&GPIO_InitStructure);
	GPIO_InitStructure.Pin = BTN_1_Pin | BTN_2_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.GPIO_Pull = GPIO_PULL_DOWN;
	GPIO_InitPeripheral(BTN_GPIO, &GPIO_InitStructure);
}
//----------------------------------------------------------------------------//
#define CALIBR_Pin GPIO_PIN_11
#define CALIBR_GPIO_Port GPIOB
#define IS_CALIBR_ON (CALIBR_Pin != (CALIBR_GPIO_Port->PID & CALIBR_Pin))

#define USB_CHECK_Pin GPIO_PIN_8
#define USB_CHECK_GPIO_Port GPIOB
#define IS_USB_CHECK_ON                                                        \
	(USB_CHECK_Pin == (USB_CHECK_GPIO_Port->PID & USB_CHECK_Pin))

typedef enum {
	PIN_BUTTON_CALIBR,
	PIN_BUTTON_USB_CHECK,
} pin_t;

void GPIO_Init_CALIBR_USB_Check(void) {
	GPIO_InitType GPIO_InitStructure;
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);

	GPIO_InitStruct(&GPIO_InitStructure);
	GPIO_InitStructure.Pin = CALIBR_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.GPIO_Pull = GPIO_PULL_UP;
	GPIO_InitPeripheral(CALIBR_GPIO_Port, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = USB_CHECK_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.GPIO_Pull = GPIO_PULL_DOWN;
	GPIO_InitPeripheral(CALIBR_GPIO_Port, &GPIO_InitStructure);
}
//----------------------------------------------------------------------------//
// BackLight Pin
#define BACKLIGHT_GPIO GPIOB
#define BACKLIGHT_PIN GPIO_PIN_12

#define POWERED_BY_USB_GPIO GPIOF
#define POWERED_BY_USB_PIN GPIO_Pin_0
//----------------------------------------------------------------------------//
#define BACKLIGHT_TIMEOUT 15000
//----------------------------------------------------------------------------//
#define BACKLIGHT_ON GPIO_SetBits(BACKLIGHT_GPIO, BACKLIGHT_PIN);
#define BACKLIGHT_OFF GPIO_ResetBits(BACKLIGHT_GPIO, BACKLIGHT_PIN);
//----------------------------------------------------------------------------//
#define POWERED_BY_USB (POWERED_BY_USB_GPIO->STS & POWERED_BY_USB_PIN)
//----------------------------------------------------------------------------//
void Init_BackLight(void) { // ������������� ��������� ���
	GPIO_InitType GPIO_InitStructure;
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);

	GPIO_InitStruct(&GPIO_InitStructure);
	GPIO_InitStructure.Pin = BACKLIGHT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitPeripheral(BACKLIGHT_GPIO, &GPIO_InitStructure);
	GPIO_ResetBits(BACKLIGHT_GPIO, BACKLIGHT_PIN);
}
//----------------------------------------------------------------------------//
#define TIME_DEBOUNCE 30
//----------------------------------------------------------------------------//
// ������������ �������
#define DEBOUNCE_TIME_MS 30    // ����� ������������
#define SHORT_PRESS_MIN_MS 50  // ����������� ����� ��������� �������
#define SHORT_PRESS_MAX_MS 500 // ������������ ����� ��������� �������
#define MEDIUM_PRESS_MS 1500   // ����� �������� �������
//----------------------------------------------------------------------------//
typedef enum { // ��������� ������
	BTN_STATE_RELEASED,
	BTN_STATE_PRESSED,
	BTN_STATE_DEBOUNCE_PRESS,  // ����������� ��� �������
	BTN_STATE_DEBOUNCE_RELEASE // ����������� ��� ����������
} button_state_t;
//----------------------------------------------------------------------------//
typedef enum { // ���� �������
	PRESS_NONE,
	PRESS_SHORT,
	PRESS_MEDIUM,
	PRESS_LONG
} press_type_t;
//----------------------------------------------------------------------------//
typedef struct { // ��������� ��� ���������� �������
	button_state_t state;
	u32 press_start_time;
	u32 debounce_start_time;
	bool last_physical_state;
	bool stable_physical_state;
} button_t;
//----------------------------------------------------------------------------//
// ������� ��� ������ ����������� ��������� ������
// ���������� true ���� ������ ������, false ���� ��������
bool read_button_physical_state(u8 nBtn) {
	switch (nBtn) {
	case 0:
		return IS_BUTTON_1_PRESSED;
	case 1:
		return IS_BUTTON_2_PRESSED;
	case 2:
		return IS_USB_CHECK_ON;
	}
	return false;
}
//----------------------------------------------------------------------------//
void setMessageBtnPressShort(u8 nBtn, u32 press_duration) {
	if ((press_duration > SHORT_PRESS_MIN_MS) &&
	    (press_duration < SHORT_PRESS_MAX_MS)) {
		tim.reset(TIMER_DISABLE); // any press = activity, reset disable timer
		                          // regardless of BLE_State
		msg.set((Message_t)(MSG_BTN_1_PUSH_SHORT + (2 * nBtn)), NULL, 0);
		_SYSLOG_("Button %u Press Short", nBtn);
		if (nBtn == 0) {
			SystemInfo.cnt.btn1++;
		} else if (nBtn == 1) {
			SystemInfo.cnt.btn2++;
		}
	}
}
u8 btnPressMidState[2];
//----------------------------------------------------------------------------//
void setMessageBtnPressMid(u8 nBtn, u32 press_duration) {
	if (press_duration > MEDIUM_PRESS_MS) {
		tim.reset(TIMER_DISABLE); // any press = activity, reset disable timer
		                          // regardless of BLE_State
		if (0 == btnPressMidState[nBtn]) {
			btnPressMidState[nBtn] = 1;
			msg.set((Message_t)(MSG_BTN_1_PUSH_MID + (2 * nBtn)), NULL, 0);
			_SYSLOG_("Button %u Press Mid", nBtn);
		}
	}
}
//----------------------------------------------------------------------------//
// ��������� ��� ������������ ���������� ������
typedef struct {
	bool btn1_pressed;
	bool btn2_pressed;
	u32 combination_start_time;
	bool combination_processed;
} combination_t;
static combination_t combination = {false, false, 0, false};
#define COMBINATION_PRESS_MS 2000 // ����� ��� ����������� ���������� ������
//----------------------------------------------------------------------------//
// ������� ��� ��������� �������������� ������� ���� ������
bool process_combination_press(u32 current_time) {
	// ���������, ������ �� ��� ������
	bool both_pressed =
	    read_button_physical_state(0) && read_button_physical_state(1);
	static bool twoBtnPress = false;
	if (both_pressed) {
		if (!combination.btn1_pressed && !combination.btn2_pressed) {
			// ��� ������ ������ ��� ���� ������ ������������
			combination.btn1_pressed = true;
			combination.btn2_pressed = true;
			combination.combination_start_time = current_time;
			combination.combination_processed = false;
			_SYSLOG_("Combination press detected, waiting for duration");
		} else if (!combination.combination_processed &&
		           (current_time - combination.combination_start_time) >=
		               COMBINATION_PRESS_MS) {
			// ������ ���������� ������� ��� ����������
			tim.reset(TIMER_DISABLE); // activity - reset disable timer regardless
			                          // of BLE_State
			msg.set(MSG_BTN_1_2_COMBINATION_MID, NULL, 0);
			_SYSLOG_("Buttons 1 and 2 combination press (medium)");
			combination.combination_processed = true;
		}
		twoBtnPress = true;
	} else {
		// ���� ���� �� ���� ������ ��������, ���������� ��������� ����������
		if (combination.btn1_pressed || combination.btn2_pressed) {
			_SYSLOG_("Combination press released, duration: %u ms",
			         current_time - combination.combination_start_time);
		}
		combination.btn1_pressed = false;
		combination.btn2_pressed = false;
		combination.combination_processed = false;
	}
	if (0 == (read_button_physical_state(0) || read_button_physical_state(1))) {
		twoBtnPress = false;
	}
	if (twoBtnPress) {
		return true;
	} else {
		return both_pressed;
	}
}
//----------------------------------------------------------------------------//
void button_process(button_t *btn, u8 nBtn) {
	bool current_physical_state = read_button_physical_state(nBtn);
	u32 current_time = tim.get(TIMER_BTN);

	if (current_physical_state !=
	    btn->last_physical_state) { // ����������� ��������� ���������
		btn->debounce_start_time = current_time;
		btn->last_physical_state = current_physical_state;
	}
	switch (btn->state) {
	case BTN_STATE_RELEASED: {
		if (current_physical_state != btn->stable_physical_state) {
			if ((current_time - btn->debounce_start_time) >
			    DEBOUNCE_TIME_MS) { // ���������� ������� ����� ������������
				btn->state = BTN_STATE_PRESSED;
				btn->stable_physical_state = current_physical_state;
				btn->press_start_time = current_time;
				//_SYSLOG_("������ %u ��������� ������ (����� ������������)\n",
				// nBtn);
			} else { // � �������� ������������ �������
				btn->state = BTN_STATE_DEBOUNCE_PRESS;
			}
		}
	} break;
	case BTN_STATE_PRESSED: {
		if (current_physical_state != btn->stable_physical_state) {
			if ((current_time - btn->debounce_start_time) >
			    DEBOUNCE_TIME_MS) { // ���������� ���������� ����� ������������
				// u32 press_duration = current_time - btn->press_start_time;
				//_SYSLOG_("������ %u ��������, ������������: %u ��\n", nBtn,
				// press_duration);
				btn->state = BTN_STATE_RELEASED;
				btn->stable_physical_state = current_physical_state;
			} else { // � �������� ������������ ����������
				btn->state = BTN_STATE_DEBOUNCE_RELEASE;
			}
		} else {
			if (nBtn != 2) {
				uint32_t press_duration = current_time - btn->press_start_time;
				setMessageBtnPressMid(nBtn, press_duration);
			}
		}
	} break;
	case BTN_STATE_DEBOUNCE_PRESS: {
		if ((current_time - btn->debounce_start_time) >
		    DEBOUNCE_TIME_MS) { // ���������� ������������ �������
			if (current_physical_state) {
				btn->state = BTN_STATE_PRESSED;
				btn->stable_physical_state = true;
				btn->press_start_time = current_time;
				if (2 == nBtn) {
					SystemInfo.status.usbPwr = true;
				}
				//_SYSLOG_("������ %u ��������� ������\n", nBtn);
			} else { // ������� - ������� � �������� ���������
				btn->state = BTN_STATE_RELEASED;
				//_SYSLOG_("������� ������� ������ %u ��������������\n", nBtn);
			}
		}
	} break;

	case BTN_STATE_DEBOUNCE_RELEASE: {
		if ((current_time - btn->debounce_start_time) >
		    DEBOUNCE_TIME_MS) { // ���������� ������������ ����������
			if (!current_physical_state) {
				uint32_t press_duration = current_time - btn->press_start_time;
				if (2 == nBtn) {
					SystemInfo.status.usbPwr = false;
				}
				//_SYSLOG_("������ %u ��������, ������������: %u ��\n", nBtn,
				// press_duration);
				msg.getVal((Message_t)(MSG_BTN_1_PUSH_MID + (2 * nBtn)));
				setMessageBtnPressShort(nBtn, press_duration);
				btn->state = BTN_STATE_RELEASED;
				btn->stable_physical_state = false;
				btnPressMidState[nBtn] = 0;
			} else { // ������� - ������� � ��������� �������
				btn->state = BTN_STATE_PRESSED;
				//_SYSLOG_("������� ���������� ������ %u ��������������\n", nBtn);
			}
		}
	} break;
	}
}
//----------------------------------------------------------------------------//
PT_THREAD(thread_btn(pt_t *pt)) {
	static button_t button[3];
	static u32 calibrTime;
	static u32 current_time;
	PT_BEGIN(pt);
	GPIO_Init_CALIBR_USB_Check();
	BTN_Init();
	Init_BackLight();
	while (1) {
		if (MODE_NONE ==
		    SystemInfo.status.manufactoryMode) { // ���� ������ �� �����������
			if (IS_CALIBR_ON) {
				if (!calibrTime) {
					calibrTime = tim.get(TIMER_GENERAL); // ���������� �����
				} else if ((tim.get(TIMER_GENERAL) - calibrTime) >= 50) {
					SystemInfo.status.manufactoryMode = MODE_ON;
				}
			} else {
				calibrTime = 0; // ���������� ������
			}
			if ((MODE_NONE == SystemInfo.status.manufactoryMode) &&
			    (tim.get(TIMER_GENERAL) >
			     1000)) { // ���� ������ ������� � ������ �� ��� ���������
				SystemInfo.status.manufactoryMode = MODE_OFF;
			}
		}
		current_time = tim.get(TIMER_BTN);
		if (0 == process_combination_press(current_time)) {
			button_process(&button[0], 0);
			button_process(&button[1], 1);
		}
		button_process(&button[2], 2); // USB_CHECK

		// ��������� �������������� ������� ������ 0 � 1

		PT_YIELD(pt);
	}
	PT_END(pt);
}
//----------------------------------------------------------------------------//
