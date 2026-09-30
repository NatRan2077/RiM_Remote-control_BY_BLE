//----------------------------------------------------------------------------//
#include <stdbool.h>
#include "pt.h"
#include "common_types.h"
//----------------------------------------------------------------------------//
extern pt_t systemInfo_pt;
PT_THREAD(thread_systemInfo(pt_t *pt));
//----------------------------------------------------------------------------//
extern pt_t lcd_pt;
PT_THREAD(thread_lcd(pt_t *pt));
//----------------------------------------------------------------------------//
extern pt_t btn_pt;
PT_THREAD(thread_btn(pt_t *pt));
//----------------------------------------------------------------------------//
extern pt_t main_pt;
PT_THREAD(thread_main(pt_t *pt));
//----------------------------------------------------------------------------//
extern pt_t status_pt;
PT_THREAD(thread_status(pt_t *pt));
//----------------------------------------------------------------------------//
extern pt_t usbUart_pt;
PT_THREAD(thread_usbUart(pt_t *pt));
//----------------------------------------------------------------------------//
extern pt_t ble_pt;
PT_THREAD(thread_BLE(pt_t *pt));
//----------------------------------------------------------------------------//
extern pt_t dataRouting_pt;
PT_THREAD(thread_dataRouting(pt_t *pt));
//----------------------------------------------------------------------------//
extern pt_t disabling_pt;
PT_THREAD(thread_disabling(pt_t *pt));
//----------------------------------------------------------------------------//
extern pt_t blinkingBatt_pt;
PT_THREAD(thread_blinkingBatt(pt_t *pt));
//----------------------------------------------------------------------------//
extern pt_t blinkingLines_pt;
PT_THREAD(thread_blinkingLines(pt_t *pt));
//----------------------------------------------------------------------------//
extern pt_t rim_pt;
PT_THREAD(thread_rim_protocol(pt_t *pt));
extern volatile bool g_rim_enable;
//----------------------------------------------------------------------------//