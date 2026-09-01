/***************************************************************************/ /**
                                                                               * # License
                                                                               * Copyright 2023 Shenzhen HOPE Microelectronics Co., Ltd.
                                                                               * All rights reserved.
                                                                               *
                                                                               * IMPORTANT: All rights of this software belong to Shenzhen HOPE Microelectronics
                                                                               * CO., Ltd. ("HOPERF"). Your use of this Software is limited to those
                                                                               * specific rights granted under the terms of the business contract, the
                                                                               * confidential agreement, the non-disclosure agreement and any other forms
                                                                               * of agreements as a customer or a partner of HOPERF. You may not use this
                                                                               * Software unless you agree to abide by the terms of these agreements.
                                                                               * You acknowledge that the Software may not be modified, copied,
                                                                               * distributed or disclosed unless embedded on a HOPERF Bluetooth Low Energy
                                                                               * (BLE) integrated circuit, either as a product or is integrated into your
                                                                               * products.  Other than for the aforementioned purposes, you may not use,
                                                                               * reproduce, copy, prepare derivative works of, modify, distribute, perform,
                                                                               * display or sell this Software and/or its documentation for any purposes.
                                                                               *
                                                                               * YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
                                                                               * PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
                                                                               * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
                                                                               * NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
                                                                               * CMOSTEK OR ITS SUBSIDIARIES BE LIABLE OR OBLIGATED UNDER CONTRACT,
                                                                               * NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
                                                                               * LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING
                                                                               * BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
                                                                               * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
                                                                               * SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
                                                                               * (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
                                                                               *******************************************************************************/

/**
 * @file hp_ble.h

 * @version v1.0.0
 *
  */
#ifndef __APP_BLE_H__
#define __APP_BLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hp_ble.h"
#include "hp_ble_task.h"

/// User Task messages
enum user_msg_id {
	APP_USER_DEMO_EVE = APP_FREE_EVE_FOR_USER,
	APP_UART_TX_EVT,
	APP_START_PAIRING_EVT, // ƒобавить новое событие дл€ спаривани€

};

/* ble application initialization */
void app_ble_init(void);
void app_ble_gap_params_init(void);
void app_ble_adv_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_BLE_H__ */
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
