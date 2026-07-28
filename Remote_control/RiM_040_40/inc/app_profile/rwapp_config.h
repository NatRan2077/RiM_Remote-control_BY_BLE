/***************************************************************************/ /**
                                                                               * # License
                                                                               * Copyright 2023 Shenzhen HOPE Microelectronics Co., Ltd.
                                                                               * All rights reserved.
                                                                               *
                                                                               * IMPORTANT: All rights of this software belong to Shenzhen HOPE
                                                                               * Microelectronics Co., Ltd. ("HOPERF"). Your use of this Software is limited
                                                                               * to those specific rights granted under the terms of the business contract,
                                                                               * the confidential agreement, the non-disclosure agreement and any other forms
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
                                                                               * HOPERF OR ITS SUBSIDIARIES BE LIABLE OR OBLIGATED UNDER CONTRACT,
                                                                               * NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
                                                                               * LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING
                                                                               * BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
                                                                               * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
                                                                               * SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
                                                                               * (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
                                                                               *******************************************************************************/

/**
 * @file rwapp_config.h

 * @version v1.0.0
 *
  */

#ifndef _RWAPP_CONFIG_H_
#define _RWAPP_CONFIG_H_

/**
 * @addtogroup app
 * @brief Application configuration definition
 *
 * @{
 **/

#include "app_user_config.h"
#include "rwip_config.h"
/* Includes ------------------------------------------------------------------*/

/* Define ------------------------------------------------------------*/

/// Application Profile
#if defined(CFG_APP_PRF)
#define BLE_APP_PRF 1
#else // defined(CFG_APP_PRF)
#define BLE_APP_PRF 0
#endif // defined(CFG_APP_PRF)

/// Health Thermometer Application
#if defined(CFG_APP_HT)
#define BLE_APP_HT 1
#else // defined(CFG_APP_HT)
#define BLE_APP_HT 0
#endif // defined(CFG_APP_HT)

/// HID Application
#if defined(CFG_APP_HID)
#define BLE_APP_HID 1
#else // defined(CFG_APP_HID)
#define BLE_APP_HID 0
#endif // defined(CFG_APP_HID)

/// DIS Application
#if defined(CFG_APP_DIS)
#define BLE_APP_DIS 1
#else // defined(CFG_APP_DIS)
#define BLE_APP_DIS 0
#endif // defined(CFG_APP_DIS)

/// Battery Service Application
#if (CFG_APP_BATT)
#define BLE_APP_BATT 1
#else
#define BLE_APP_BATT 0
#endif //(CFG_APP_BATT)

/// Security Application
#if (defined(CFG_APP_SEC) || BLE_APP_HID)
#define BLE_APP_SEC 1
#else //(defined(CFG_APP_SEC) || BLE_APP_HID)
#define BLE_APP_SEC 0
#endif //(defined(CFG_APP_SEC) || BLE_APP_HID)

#define AM0_APP_OPTIONAL_CHARACTERISTICS 0

#if defined(CFG_APP_HP_IUS)
#define BLE_APP_HP_IUS 1
#else // defined(CFG_APP_HP_IUS)
#define BLE_APP_HP_IUS 0
#endif // defined(CFG_APP_HP_IUS)

/// @} rwapp_config

#endif /* _RWAPP_CONFIG_H_ */
