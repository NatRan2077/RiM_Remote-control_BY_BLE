//----------------------------------------------------------------------------//
#include "threads.h"
#include "dlmsListObj.h"
#include "messages.h"
#include "lcd.h"
#include "globals.h"
#include "HAL.h"
#include "log.h"
#include "utils.h"
#include "memory.h"
#include "cmt453x_gpio.h"
// #include "cmt453x_rcc.h"
#include "cmt453x_dma.h"
#include "cmt453x_adc.h"
#include "lcd.h"
//----------------------------------------------------------------------------//
pt_t other_pt;
//----------------------------------------------------------------------------//
PT_THREAD(thread_other(pt_t *pt)) {
	PT_BEGIN(pt);
	while (1) {
		PT_YIELD(pt);
	}
	PT_END(pt);
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
pt_t blinkingLines_pt;
pt_t blinkingBatt_pt;
pt_t disabling_pt;
pt_t status_pt;
//----------------------------------------------------------------------------//
void mcu_reset(void) { // Функция ресета, через вочдог
	SupplyDis();
	NVIC_SystemReset();
}
//----------------------------------------------------------------------------//
#define TIME_STATUS_BLINK_SHOW 250
#define TIME_STATUS_BLINK_HIDE 500
//----------------------------------------------------------------------------//
PT_THREAD(thread_blinkingBatt(pt_t *pt)) {
	static u16 tmp;
	PT_BEGIN(pt);
	tim.reset(TIMER_ERROR);
	tim.reset(TIMER_DELAY);
	lcd.clear();
	lcd.showText("LOWBAT");
	lcd.text(SHOW, 3, CP3, BATT, V);
	lcd.show_long(SystemInfo.Vcc.Batt, false, false, 3);
	do {
		PT_YIELD_SPAWN(pt);
		tmp = tim.get(TIMER_DELAY);
		if ((TIME_STATUS_BLINK_SHOW < tmp) && (TIME_STATUS_BLINK_HIDE > tmp)) {
			lcd.text(SHOW, 1, BATT_1);
		}
		if ((TIME_STATUS_BLINK_HIDE < tmp)) {
			tim.reset(TIMER_DELAY);
			lcd.text(HIDE, 1, BATT_1);
		}
	} while (tim.get(TIMER_ERROR) <= TIME_ERROR);
	lcd.clear();
	PT_DELAY_MS(TIMER_DELAY, 100);
	mcu_reset();
	PT_END(pt);
}
//----------------------------------------------------------------------------//
PT_THREAD(thread_blinkingLines(pt_t *pt)) {
	static u16 tmp;
	PT_BEGIN(pt);
	// lcd.clear();
	tim.reset(TIMER_ERROR);
	tim.reset(TIMER_DELAY);
	do {
		PT_YIELD_SPAWN(pt);
		tmp = tim.get(TIMER_DELAY);
		if ((TIME_STATUS_BLINK_SHOW < tmp) && (TIME_STATUS_BLINK_HIDE > tmp)) {
			lcd.show_lines();
		}
		if ((TIME_STATUS_BLINK_HIDE < tmp)) {
			tim.reset(TIMER_DELAY);
			lcd.clear_data();
		}
		IWDG_Refresh();
	} while (tim.get(TIMER_ERROR) <= TIME_ERROR);
	lcd.clear();
	PT_DELAY_MS(TIMER_DELAY, 100);
	mcu_reset();
	PT_END(pt);
}
//----------------------------------------------------------------------------//
PT_THREAD(thread_disabling(pt_t *pt)) {
	PT_BEGIN(pt);
	do {
		PT_YIELD(pt);
	} while (tim.get(TIMER_DISABLE) <= TIME_DISABLE);
	_SYSLOG_("Disabling");
	lcd.clear();
	PT_DELAY_MS(TIMER_DELAY, 100);
	mcu_reset();
	PT_END(pt);
}
//----------------------------------------------------------------------------//
void ShowCurTarif(u8 tarif) { // Показать текущий тариф
	if ((tarif == 0) || (tarif > 8)) {
		lcd.text(HIDE, 1, TARIF);
		lcd.show_val_pos(SPACE, 7);
	} else {
		lcd.text(SHOW, 1, TARIF);
		lcd.show_val_pos(tarif, 7);
	}
}
//----------------------------------------------------------------------------//
void ShowEnergyDirection(u8 Dir) { // Показать направление энергии
	switch (Dir) {
	case 0:
		lcd.text(HIDE, 4, P_PLUS, Q_PLUS, P_MINUS, Q_MINUS);
		break;

	case 1:
		lcd.text(SHOW, 1, P_PLUS); // АЭ+
		lcd.text(HIDE, 3, Q_PLUS, P_MINUS, Q_MINUS);
		break;

	case 2:
		lcd.text(SHOW, 1, P_MINUS); // АЭ-
		lcd.text(HIDE, 3, P_PLUS, Q_PLUS, Q_MINUS);
		break;

	case 4:
		lcd.text(SHOW, 1, Q_PLUS); // РЭ+
		lcd.text(HIDE, 3, P_PLUS, P_MINUS, Q_MINUS);
		break;

	case 8:
		lcd.text(SHOW, 1, Q_MINUS); // РЭ-
		lcd.text(HIDE, 3, P_PLUS, P_MINUS, Q_PLUS);
		break;

	case 5:
		lcd.text(SHOW, 2, P_PLUS, Q_PLUS); // АЭ+ РЭ+
		lcd.text(HIDE, 2, P_MINUS, Q_MINUS);
		break;

	case 6:
		lcd.text(SHOW, 2, P_MINUS, Q_PLUS); // АЭ- РЭ+
		lcd.text(HIDE, 2, P_PLUS, Q_MINUS);
		break;

	case 10:
		lcd.text(SHOW, 2, P_MINUS, Q_MINUS); // АЭ- РЭ-
		lcd.text(HIDE, 2, P_PLUS, Q_PLUS);
		break;

	case 9:
		lcd.text(SHOW, 2, P_PLUS, Q_MINUS); // АЭ+ РЭ-
		lcd.text(HIDE, 2, P_MINUS, Q_PLUS);
		break;
	}
}
//----------------------------------------------------------------------------//
void ShowRSSI(s8 rssiBLE) {
	if (rssiBLE > -60) {
		lcd.text(SHOW, 3, RSSI_1, RSSI_2, RSSI_3);
	} else if (rssiBLE > -75) {
		lcd.text(SHOW, 2, RSSI_1, RSSI_2);
		lcd.text(HIDE, 1, RSSI_3);
	} else if (rssiBLE > -85) {
		lcd.text(SHOW, 1, RSSI_1);
		lcd.text(HIDE, 2, RSSI_2, RSSI_3);
	} else {
		lcd.text(HIDE, 3, RSSI_1, RSSI_2, RSSI_3);
	}
}
//----------------------------------------------------------------------------//
s8 rssiBLE = -127;
bool rssiNew = false;
PT_THREAD(thread_status(pt_t *pt)) {
	static u16 tmp;
	static bool fUnblockStatus = false;
	PT_BEGIN(pt);
	while (1) {
		if (msg.getVal(MSG_UNBLOCK_STATUS)) {
			fUnblockStatus = true;
		}
		if (msg.getVal(MSG_BLOCK_STATUS_THREAD)) {
			while (1) {
				if (msg.getVal(MSG_BLOCK_STATUS_THREAD)) {
					break;
				}
				PT_YIELD(pt);
			}
		}
		if (msg.getVal(MSG_SWITCH_BLOCK_STATUS_THREAD)) { // Блокируем поток для
			                                               // диагностики дисплея
			while (1) {
				tmp = tim.get(TIMER_STATUS);
				if ((TIME_STATUS_BLINK_SHOW < tmp) &&
				    (TIME_STATUS_BLINK_HIDE > tmp)) {
					lcd.all();
				}
				if ((TIME_STATUS_BLINK_HIDE < tmp)) {
					tim.reset(TIMER_STATUS);
					if (dlms.status.DiagLCD == 1) {
						lcd.clear();
					}
				}
				if (msg.getVal(MSG_SWITCH_BLOCK_STATUS_THREAD)) {
					lcd.clear();
					break;
				}
				PT_YIELD(pt);
			}
		}
		if (SystemInfo.status.lcdDiag) {
			while (1) {
				lcd.all();
				if (!SystemInfo.status.lcdDiag) {
					lcd.clear();
					break;
				}
				PT_YIELD(pt);
			}
		}
		if (rssiNew) {
			tim.reset(TIMER_RSSI);
		} else if (tim.get(TIMER_RSSI) > 5000) {
			rssiBLE = -127;
		}
		rssiNew = false;
		ShowRSSI(rssiBLE);

		if (fUnblockStatus) {
			ShowCurTarif(dlms.curTariff);
			ShowEnergyDirection(dlms.energyDir);
		}

		tmp = tim.get(TIMER_STATUS);
		if ((TIME_STATUS_BLINK_SHOW < tmp) && (TIME_STATUS_BLINK_HIDE > tmp)) {
			if (fUnblockStatus) {
				if (dlms.status.Phase & 0x80) { // если трехфазник
					if (0x08 == (dlms.status.Phase & 0x08)) {
						lcd.text(SHOW, 3, PHASE_A, PHASE_B, PHASE_C); // перефазировка
					} else {
						if (0 != (dlms.status.Phase &
						          0x77)) { // отсутствие фазы или переток
							if (!(dlms.status.Phase & 0x01)) {
								lcd.text(SHOW, 1, PHASE_A); // присутствует фаза 1
							} else if (!((dlms.status.Phase & 0x10) >> 4)) {
								lcd.text(HIDE, 1, PHASE_A);
							}

							if (!((dlms.status.Phase & 0x02) >> 1)) {
								lcd.text(SHOW, 1, PHASE_B); // присутствует фаза 2
							} else if (!((dlms.status.Phase & 0x20) >> 5)) {
								lcd.text(HIDE, 1, PHASE_B);
							}

							if (!((dlms.status.Phase & 0x04) >> 2)) {
								lcd.text(SHOW, 1, PHASE_C); // присутствует фаза 3
							} else if (!((dlms.status.Phase & 0x40) >> 6)) {
								lcd.text(HIDE, 1, PHASE_C);
							}

							if ((dlms.status.Phase & 0x10) >> 4) {
								lcd.text(SHOW, 1, PHASE_A); // если переток фазы 1
							}
							if ((dlms.status.Phase & 0x20) >> 5) {
								lcd.text(SHOW, 1, PHASE_B); // если переток фазы 2
							}
							if ((dlms.status.Phase & 0x40) >> 6) {
								lcd.text(SHOW, 1, PHASE_C); // если переток фазы 3
							}
						} else {
							lcd.text(SHOW, 3, PHASE_A, PHASE_B, PHASE_C);
						}
					}
				} else {
					lcd.text(HIDE, 3, PHASE_A, PHASE_B, PHASE_C);
				}
			}
			if (SystemInfo.status.usbPwr) {
				lcd.text(HIDE, 2, BATT, BATT_1);
			} else {
				switch (SystemInfo.status.vBatt) {
				case V_BATT_STATUS_NONE:
				case V_BATT_STATUS_FULL:
					lcd.text(HIDE, 2, BATT, BATT_1);
					break;
				case V_BATT_STATUS_MID:
				case V_BATT_STATUS_LOW:
				case V_BATT_STATUS_LOWER:
					lcd.text(SHOW, 2, BATT, BATT_1);
					break;
				}
			}
			if (fUnblockStatus) {
				switch (dlms.status.relay) {
				case DISCONNECTED: {
					lcd.text(SHOW, 1, RELAY);
					lcd.text(HIDE, 1, RELAY_ARR);
				} break;
				case NONE:
				case CONNECTED: {
					lcd.text(HIDE, 1, RELAY);
					lcd.text(HIDE, 1, RELAY_ARR);
				} break;
				case READY_FOR_RECONNECTION: {
					lcd.text(SHOW, 1, RELAY);
					lcd.text(SHOW, 1, RELAY_ARR);
				} break;
				}
				switch (dlms.status.Rtc) {
				case 0:
					lcd.text(HIDE, 1, CLOCK);
					break;
				case 1:
					lcd.text(SHOW, 1, CLOCK);
					break;
				}
				if (0 == dlms.verLCD) {
					if (dlms.fHz) { // если отображается частота
						switch (dlms.status.Eqi) {
						case STATUS_TAMPER_OK:
							lcd.text(HIDE, 1, WARNING);
							break;
						case STATUS_TAMPER_ACTIVE:
							lcd.text(SHOW, 1, WARNING);
							break;
						default:
							break;
						}
					} else if (dlms.fVolt) { // если отображается напряжение
						switch (dlms.status.Eqi) {
						case STATUS_TAMPER_OK:
							lcd.text(HIDE, 1, WARNING);
							break;
						case STATUS_TAMPER_ACTIVE:
							lcd.text(SHOW, 1, WARNING);
							break;
						default:
							break;
						}
					} else {
						switch (dlms.status.Magnet) {
						case STATUS_TAMPER_OK:
							lcd.text(HIDE, 1, WARNING);
							break;
						case STATUS_TAMPER_ACTIVE:
							lcd.text(SHOW, 1, WARNING);
							break;
						default:
							break;
						}
					}
				} else if (1 == dlms.verLCD) {
					switch (dlms.status.Uhf) {
					case STATUS_TAMPER_OK:
						lcd.text(HIDE, 1, UHF);
						break;
					case STATUS_TAMPER_FLAG:
						lcd.text(SHOW, 1, UHF);
						break;
					case STATUS_TAMPER_ACTIVE:
						lcd.text(SHOW, 1, UHF);
						break;
					}
					switch (dlms.status.Cover) {
					case STATUS_TAMPER_OK:
						lcd.text(HIDE, 1, TAMPER_COVER);
						break;
					case STATUS_TAMPER_FLAG:
						lcd.text(SHOW, 1, TAMPER_COVER);
						break;
					case STATUS_TAMPER_ACTIVE:
						lcd.text(SHOW, 1, TAMPER_COVER);
						break;
					}
					switch (dlms.status.Meter) {
					case STATUS_TAMPER_OK:
						lcd.text(HIDE, 1, TAMPER_METER);
						break;
					case STATUS_TAMPER_FLAG:
						lcd.text(SHOW, 1, TAMPER_METER);
						break;
					case STATUS_TAMPER_ACTIVE:
						lcd.text(SHOW, 1, TAMPER_METER);
						break;
					}
					switch (dlms.status.Battery) {
					case STATUS_TAMPER_OK:
						lcd.text(HIDE, 1, TAMPER_BATTARY);
						break;
					case STATUS_TAMPER_FLAG:
						lcd.text(SHOW, 1, TAMPER_BATTARY);
						break;
					case STATUS_TAMPER_ACTIVE:
						lcd.text(SHOW, 1, TAMPER_BATTARY);
						break;
					}
					switch (dlms.status.Magnet) {
					case STATUS_TAMPER_OK:
						lcd.text(HIDE, 1, MAG);
						break;
					case STATUS_TAMPER_FLAG:
						lcd.text(SHOW, 1, MAG);
						break;
					case STATUS_TAMPER_ACTIVE:
						lcd.text(SHOW, 1, MAG);
						break;
					}
					switch (dlms.status.Diag) {
					case STATUS_TAMPER_OK:
						lcd.text(HIDE, 1, WARNING);
						break;
					case STATUS_TAMPER_FLAG:
						lcd.text(SHOW, 1, WARNING);
						break;
					case STATUS_TAMPER_ACTIVE:
						lcd.text(SHOW, 1, WARNING);
						break;
					}
					switch (dlms.status.Eqi) {
					case STATUS_TAMPER_OK:
						lcd.text(HIDE, 1, EQI);
						break;
					case STATUS_TAMPER_FLAG:
						lcd.text(SHOW, 1, EQI);
						break;
					case STATUS_TAMPER_ACTIVE:
						lcd.text(SHOW, 1, EQI);
						break;
					}
					switch (dlms.status.Unbalance) {
					case STATUS_TAMPER_OK:
						lcd.text(HIDE, 1, S41);
						break;
					case STATUS_TAMPER_FLAG:
						lcd.text(SHOW, 1, S41);
						break;
					case STATUS_TAMPER_ACTIVE:
						lcd.text(SHOW, 1, S41);
						break;
					}
					//					switch(dlms.status.LimPower){
					//						case STATUS_TAMPER_OK:		lcd.text(HIDE, 1,
					//S10);break; 						case STATUS_TAMPER_FLAG:	lcd.text(SHOW, 1,
					//S10);break; 						case STATUS_TAMPER_ACTIVE:	lcd.text(SHOW, 1,
					//S10);break;
					//					}
				}
			}
		}
		if ((TIME_STATUS_BLINK_HIDE < tmp)) {
			tim.reset(TIMER_STATUS);
			if (fUnblockStatus) {
				if (dlms.status.Phase & 0x80) {              // если трехфазник
					if (0x08 == (dlms.status.Phase & 0x08)) { // перефазировка
						lcd.text(HIDE, 3, PHASE_A, PHASE_B, PHASE_C);
					} else {
						if (0 != (dlms.status.Phase &
						          0x77)) { // отсутствие фазы или переток
							if (!(dlms.status.Phase & 0x01)) { // если нет фазы 1
								lcd.text(SHOW, 1, PHASE_A);
							}
							if (!((dlms.status.Phase & 0x02) >> 1)) {
								lcd.text(SHOW, 1, PHASE_B); // если нет фазы 2
							}
							if (!((dlms.status.Phase & 0x04) >> 2)) {
								lcd.text(SHOW, 1, PHASE_C); // если нет фазы 3
							}
							if ((dlms.status.Phase & 0x10) >> 4) {
								lcd.text(HIDE, 1, PHASE_A); // если переток фазы 1
							}
							if ((dlms.status.Phase & 0x20) >> 5) {
								lcd.text(HIDE, 1, PHASE_B); // если переток фазы 2
							}
							if ((dlms.status.Phase & 0x40) >> 6) {
								lcd.text(HIDE, 1, PHASE_C); // если переток фазы 3
							}
						} else {
							lcd.text(SHOW, 3, PHASE_A, PHASE_B, PHASE_C);
						}
					}
				} else {
					lcd.text(HIDE, 3, PHASE_A, PHASE_B, PHASE_C);
				}
			}
			if (SystemInfo.status.usbPwr) {
				lcd.text(HIDE, 2, BATT, BATT_1);
			} else {
				switch (SystemInfo.status.vBatt) {
				case V_BATT_STATUS_NONE:
				case V_BATT_STATUS_FULL:
					lcd.text(HIDE, 2, BATT, BATT_1);
					break;
				case V_BATT_STATUS_MID:
					lcd.text(SHOW, 2, BATT, BATT_1);
					break;
				case V_BATT_STATUS_LOW:
				case V_BATT_STATUS_LOWER:
					lcd.text(SHOW, 1, BATT);
					lcd.text(HIDE, 1, BATT_1);
					break;
				}
			}
			if (fUnblockStatus) {
				switch (dlms.status.relay) {
				case DISCONNECTED: {
					lcd.text(SHOW, 1, RELAY);
					lcd.text(HIDE, 1, RELAY_ARR);
				} break;
				case NONE:
				case CONNECTED: {
					lcd.text(HIDE, 1, RELAY);
					lcd.text(HIDE, 1, RELAY_ARR);
				} break;
				case READY_FOR_RECONNECTION: {
					lcd.text(SHOW, 1, RELAY);
					lcd.text(HIDE, 1, RELAY_ARR);
				} break;
				}
				lcd.text(HIDE, 1, CLOCK);
				if (0 == dlms.verLCD) {
					lcd.text(HIDE, 1, WARNING);
				} else if (1 == dlms.verLCD) {
					switch (dlms.status.Uhf) {
					case STATUS_TAMPER_OK:
						lcd.text(HIDE, 1, UHF);
						break;
					case STATUS_TAMPER_FLAG:
						lcd.text(SHOW, 1, UHF);
						break;
					case STATUS_TAMPER_ACTIVE:
						lcd.text(HIDE, 1, UHF);
						break;
					}
					switch (dlms.status.Cover) {
					case STATUS_TAMPER_OK:
						lcd.text(HIDE, 1, TAMPER_COVER);
						break;
					case STATUS_TAMPER_FLAG:
						lcd.text(SHOW, 1, TAMPER_COVER);
						break;
					case STATUS_TAMPER_ACTIVE:
						lcd.text(HIDE, 1, TAMPER_COVER);
						break;
					}
					switch (dlms.status.Meter) {
					case STATUS_TAMPER_OK:
						lcd.text(HIDE, 1, TAMPER_METER);
						break;
					case STATUS_TAMPER_FLAG:
						lcd.text(SHOW, 1, TAMPER_METER);
						break;
					case STATUS_TAMPER_ACTIVE:
						lcd.text(HIDE, 1, TAMPER_METER);
						break;
					}
					switch (dlms.status.Battery) {
					case STATUS_TAMPER_OK:
						lcd.text(HIDE, 1, TAMPER_BATTARY);
						break;
					case STATUS_TAMPER_FLAG:
						lcd.text(SHOW, 1, TAMPER_BATTARY);
						break;
					case STATUS_TAMPER_ACTIVE:
						lcd.text(HIDE, 1, TAMPER_BATTARY);
						break;
					}
					switch (dlms.status.Magnet) {
					case STATUS_TAMPER_OK:
						lcd.text(HIDE, 1, MAG);
						break;
					case STATUS_TAMPER_FLAG:
						lcd.text(SHOW, 1, MAG);
						break;
					case STATUS_TAMPER_ACTIVE:
						lcd.text(HIDE, 1, MAG);
						break;
					}
					switch (dlms.status.Diag) {
					case STATUS_TAMPER_OK:
						lcd.text(HIDE, 1, WARNING);
						break;
					case STATUS_TAMPER_FLAG:
						lcd.text(SHOW, 1, WARNING);
						break;
					case STATUS_TAMPER_ACTIVE:
						lcd.text(HIDE, 1, WARNING);
						break;
					}
					switch (dlms.status.Eqi) {
					case STATUS_TAMPER_OK:
						lcd.text(HIDE, 1, EQI);
						break;
					case STATUS_TAMPER_FLAG:
						lcd.text(SHOW, 1, EQI);
						break;
					case STATUS_TAMPER_ACTIVE:
						lcd.text(HIDE, 1, EQI);
						break;
					}
					switch (dlms.status.Unbalance) {
					case STATUS_TAMPER_OK:
						lcd.text(HIDE, 1, S41);
						break;
					case STATUS_TAMPER_FLAG:
						lcd.text(SHOW, 1, S41);
						break;
					case STATUS_TAMPER_ACTIVE:
						lcd.text(HIDE, 1, S41);
						break;
					}
					//					switch(dlms.status.LimPower){
					//						case STATUS_TAMPER_OK:		lcd.text(HIDE, 1,
					//S10);break; 						case STATUS_TAMPER_FLAG:	lcd.text(SHOW, 1,
					//S10);break; 						case STATUS_TAMPER_ACTIVE:	lcd.text(HIDE, 1,
					//S10);break;
					//					}
				}
			}
		}
		PT_YIELD(pt);
	}
	PT_END(pt);
}
//----------------------------------------------------------------------------//
pt_t lcd_pt;
//----------------------------------------------------------------------------//
PT_THREAD(thread_lcd(pt_t *pt)) {
	PT_BEGIN(pt);
	lcd.init();
	while (1) {
		if (tim.get(TIMER_LCD) > 100) {
			tim.reset(TIMER_LCD);
			if (lcd.update()) {
				PT_RESTART(pt); // Если ошибка, перезапускаем поток
			}
		}
		PT_YIELD(pt);
	}
	PT_END(pt);
}

//----------------------------------------------------------------------------//
#define ADC_VCC_GPIO_Port GPIOB
#define ADC_VCC_BATT_Pin GPIO_PIN_10
#define ADC_VCC_USB_Pin GPIO_PIN_9
//----------------------------------------------------------------------------//
#define ADC_BATT ADC_CTRL_CH_1
#define ADC_USB ADC_CTRL_CH_2
#define ADC_VCC ADC_CTRL_CH_6
#define ADC_TS ADC_CTRL_CH_7
//----------------------------------------------------------------------------//
#define ADC_USB_UP 360.0
#define ADC_USB_DOWN 75.0
#define ADC_USB_K (ADC_USB_UP + ADC_USB_DOWN) / ADC_USB_DOWN
//----------------------------------------------------------------------------//
#define ADC_BATT_UP 360.0
#define ADC_BATT_DOWN 75.0
#define ADC_BATT_K (ADC_BATT_UP + ADC_BATT_DOWN) / ADC_BATT_DOWN
//----------------------------------------------------------------------------//
static void ACD_Init(u32 addr, u8 size) { // Настройка АЦП
	GPIO_InitType GPIO_InitStructure;
	DMA_InitType DMA_InitStructure;

	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA, ENABLE);
	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);
	RCC_ConfigAdcClk(RCC_ADCCLK_SRC_AUDIOPLL);
	RCC_Enable_ADC_CLK_SRC_AUDIOPLL(ENABLE); /* enable ADC 4M clock */

	GPIO_InitStruct(&GPIO_InitStructure);
	GPIO_InitStructure.Pin = ADC_VCC_BATT_Pin | ADC_VCC_USB_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_MODE_ANALOG;
	GPIO_InitPeripheral(ADC_VCC_GPIO_Port, &GPIO_InitStructure);

	DMA_DeInit(DMA_CH1);
	DMA_InitStructure.PeriphAddr = (uint32_t)&ADC->DAT;
	DMA_InitStructure.MemAddr = (uint32_t)addr;
	DMA_InitStructure.Direction = DMA_DIR_PERIPH_SRC;
	DMA_InitStructure.BufSize = size;
	DMA_InitStructure.PeriphInc = DMA_PERIPH_INC_DISABLE;
	DMA_InitStructure.DMA_MemoryInc = DMA_MEM_INC_ENABLE;
	DMA_InitStructure.PeriphDataSize = DMA_PERIPH_DATA_SIZE_HALFWORD;
	DMA_InitStructure.MemDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.CircularMode = DMA_MODE_CIRCULAR;
	DMA_InitStructure.Priority = DMA_PRIORITY_HIGH;
	DMA_InitStructure.Mem2Mem = DMA_M2M_DISABLE;
	DMA_Init(DMA_CH1, &DMA_InitStructure);
	DMA_RequestRemap(DMA_REMAP_ADC, DMA, DMA_CH1, ENABLE);

	DMA_EnableChannel(DMA_CH1, ENABLE);
	ADC_SetOverSampleCounter(ADC, 3);
	ADC_EnableBypassFilter(ADC, ENABLE);
	ADC_ConfigContinuousMode(ADC, ENABLE);
	ADC_EnableDMA(ADC, ENABLE);
	ADC_EnableTS(ADC, ENABLE);
}

pt_t systemInfo_pt;
//----------------------------------------------------------------------------//
#define ADC_BUFF_SIZE 16
//----------------------------------------------------------------------------//
PT_THREAD(thread_systemInfo(pt_t *pt)) {
	//	static bool fRequestReboot = false;

	static u32 adcSum = 0;
	static u8 adcCurCh = 0;
	static u16 adcRawBuff[ADC_BUFF_SIZE];
	static bool fRequestReboot = false;

	PT_BEGIN(pt);
	_SYSLOG_("[System] Thread Start");
	ACD_Init((u32)(&adcRawBuff), ADC_BUFF_SIZE);
	PT_YIELD(pt);

	ADC_ConfigChannel(ADC, ADC_VCC);
	ADC_Enable(ADC, ENABLE);
	DMA_EnableChannel(DMA_CH1, ENABLE);

	while (1) {
		if (DMA_GetFlagStatus(DMA_FLAG_TC1, DMA)) {
			adcSum = 0;
			for (u8 i = 0; i < ADC_BUFF_SIZE; i++) {
				adcSum += adcRawBuff[i];
			}
			adcSum = adcSum / ADC_BUFF_SIZE;
			switch (adcCurCh) {
			case 0: { // ADC_VCC
				SystemInfo.Vcc.MCU = ADC_ConverValueToVoltage(adcSum, ADC_VCC);
				adcCurCh++;
				ADC_Enable(ADC, DISABLE);
				ADC_ConfigChannel(ADC, ADC_BATT);
				DMA_EnableChannel(DMA_CH1, DISABLE);
				DMA_ClearFlag(DMA_FLAG_TC1, DMA);
				DMA_SetCurrDataCounter(DMA_CH1, ADC_BUFF_SIZE);
				DMA_EnableChannel(DMA_CH1, ENABLE);
				ADC_Enable(ADC, ENABLE);
			} break;
			case 1: { // ADC_BATT
				SystemInfo.Vcc.Batt =
				    ADC_BATT_K * ADC_ConverValueToVoltage(adcSum, ADC_BATT);
				if (SystemInfo.Vcc.Batt < BAT_LOWER_VOLT_LIM) {
					SystemInfo.status.vBatt = V_BATT_STATUS_LOWER;
				} else if (SystemInfo.Vcc.Batt <
				           BAT_LOW_VOLT_LIM) { // Напряжение меньше 2.2 вольт,
					                            // батарея почти разряжена
					SystemInfo.status.vBatt = V_BATT_STATUS_LOW;
				} else if (SystemInfo.Vcc.Batt < BAT_MID_VOLT_LIM) {
					SystemInfo.status.vBatt = V_BATT_STATUS_MID;
				} else {
					SystemInfo.status.vBatt = V_BATT_STATUS_FULL;
				}
				adcCurCh++;
				ADC_Enable(ADC, DISABLE);
				ADC_ConfigChannel(ADC, ADC_USB);
				DMA_EnableChannel(DMA_CH1, DISABLE);
				DMA_ClearFlag(DMA_FLAG_TC1, DMA);
				DMA_SetCurrDataCounter(DMA_CH1, ADC_BUFF_SIZE);
				DMA_EnableChannel(DMA_CH1, ENABLE);
				ADC_Enable(ADC, ENABLE);
			} break;
			case 2: { // ADC_USB
				SystemInfo.Vcc.USB =
				    ADC_USB_K * ADC_ConverValueToVoltage(adcSum, ADC_USB);
				adcCurCh++;
				ADC_Enable(ADC, DISABLE);
				ADC_ConfigChannel(ADC, ADC_TS);
				DMA_EnableChannel(DMA_CH1, DISABLE);
				DMA_ClearFlag(DMA_FLAG_TC1, DMA);
				DMA_SetCurrDataCounter(DMA_CH1, ADC_BUFF_SIZE);
				DMA_EnableChannel(DMA_CH1, ENABLE);
				ADC_Enable(ADC, ENABLE);
			} break;
			case 3: { // ADC_TS
				SystemInfo.temperature = ADC_ConverValueToTemperature(adcSum) * 100;
				adcCurCh = 0;
				ADC_Enable(ADC, DISABLE);
				ADC_ConfigChannel(ADC, ADC_VCC);
				DMA_EnableChannel(DMA_CH1, DISABLE);
				DMA_ClearFlag(DMA_FLAG_TC1, DMA);
				DMA_SetCurrDataCounter(DMA_CH1, ADC_BUFF_SIZE);
				DMA_EnableChannel(DMA_CH1, ENABLE);
			} break;
			}
		}
		if (!fRequestReboot) {
			if ((tim.get(TIMER_FORCED_REBOOT_DEVICE) >
			     SystemInfo.timeout.forcedReboot) &&
			    (SystemInfo.timeout.forcedReboot != 0)) {
				fRequestReboot = true;
				NVIC_SystemReset();
			}
		}
		if (tim.get(TIMER_ADC) > 1000) {
			tim.reset(TIMER_ADC);
			ADC_Enable(ADC, ENABLE);
		}
		PT_YIELD(pt);

		if (SystemInfo.image.fStartVerified) {
			if (SystemInfo.image.fStartVerified ==
			    sizeof(SystemInfo.image.transferredBlockStatus)) {
				MD5Init();
			}
			u8 tmpData[256];
			for (u8 i = 0; i < 8; i++) {
				memory.readBytes(
				    ((MEMORY_UPDATE_DEVICE_BUFF_START +
				      ((sizeof(SystemInfo.image.transferredBlockStatus) * 8) -
				       (SystemInfo.image.fStartVerified * 8) + i)) *
				     memory.getPageSize()),
				    tmpData, SystemInfo.image.blockSize);
				MD5Update((char *)tmpData, SystemInfo.image.blockSize);
			}
			SystemInfo.image.fStartVerified--;
			if (0 == SystemInfo.image.fStartVerified) {
				MD5Final(SystemInfo.image.calcMD5);
				SystemInfo.image.transferStatus =
				    IMAGE_TRANSFER_VERIFICATION_SUCCESSFUL;
			}
		}
	}

	PT_END(pt);
} //----------------------------------------------------------------------------//
