//----------------------------------------------------------------------------//
#include "messages.h"
#include "cmt453x.h"
//----------------------------------------------------------------------------//
u32 timers[MAX_TIMERS];
//----------------------------------------------------------------------------//
void SysTick_Handler(void) { // Обработчик таймера
	for (u8 i = 0; i < MAX_TIMERS; i++) {
		timers[i]++;
	}
}
//----------------------------------------------------------------------------//
#define SYSTICK_1MS ((uint32_t)1000)
//----------------------------------------------------------------------------//
static void osTimerInit() { // Инициализация таймера
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / SYSTICK_1MS);
}
//----------------------------------------------------------------------------//
static void resetTimer(Timer timer) { // Сбросить таймер
	timers[timer] = 0;
}
//----------------------------------------------------------------------------//
static void setTimer(Timer timer, u32 val) { // Установить значение таймера
	timers[timer] = val;
}
//----------------------------------------------------------------------------//
static u32 getTimer(Timer timer) { // Получить значение таймера
	return timers[timer];
}
//----------------------------------------------------------------------------//
struct {
	u8 val;
	u8 *paramPtr;
	u16 paramSize;
} messages[MAX_MSG];
//----------------------------------------------------------------------------//
static void MSG_Init() { // Инициализация сообщений
	for (u8 i = 0; i < MAX_MSG; i++) {
		messages[i].val = 0;
		messages[i].paramPtr = NULL;
		messages[i].paramSize = 0;
	}
}
//----------------------------------------------------------------------------//
static u8 MSG_Set(const Message_t imsg, void *iparamPtr,
                  u16 iparamSize) { // Установить сообщение
	if (imsg < MAX_MSG) {
		messages[imsg].val = 1;
		messages[imsg].paramPtr = iparamPtr;
		messages[imsg].paramSize = iparamSize;
		return (u8)0;
	}
	return (u8)-1;
}
//----------------------------------------------------------------------------//
static bool getMsgVal(const Message_t imsg) { // Получить сообщение
	if (1 == messages[imsg].val) {
		messages[imsg].val = 0;
		return true;
	}
	return false;
}
//----------------------------------------------------------------------------//
static void *getMsgPtr(const u8 imsg) { // Получить указатель на сообщение
	return messages[imsg].paramPtr;
}
//----------------------------------------------------------------------------//
static u16 getMsgSize(const u8 imsg) { // Получить размер сообщения
	return messages[imsg].paramSize;
}
//----------------------------------------------------------------------------//
const struct Timer_driver tim = {
    // Структура дравера TIM.
    .get = getTimer,
    .reset = resetTimer,
    .init = osTimerInit,
    .set = setTimer,
};
//----------------------------------------------------------------------------//
const struct Msg_driver msg = {
    // Структура дравера MSG.
    .init = MSG_Init,    .set = MSG_Set,        .getVal = getMsgVal,
    .getPtr = getMsgPtr, .getSize = getMsgSize,
};
//----------------------------------------------------------------------------//
