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
 * @file app_user_config.h

 * @version v1.0.0
 *
  */

#ifndef _APP_USER_CONFIG_H_
#define _APP_USER_CONFIG_H_

/* Device name */
#define CUSTOM_DEVICE_NAME "RiM 040.40" /**<The device name of this device*/

/* connection config  */
#define MIN_CONN_INTERVAL 15  /**< Minimum connection interval (15 ms) */
#define MAX_CONN_INTERVAL 30  /**< Maximum connection interval (30 ms). */
#define SLAVE_LATENCY 0       /**< Slave latency. */
#define CONN_SUP_TIMEOUT 5000 /**< Connection supervisory timeout (5000ms). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY                                         \
	(5000) /**<  Time of initiating event to update connection params (5        \
	          seconds). */

/* sec config */
#define SEC_PARAM_IO_CAPABILITIES                                              \
	GAP_IO_CAP_KB_ONLY         /**< Keyboard only: pult enters passkey 123456   \
	                              shown/held by RiM_BY61 */
#define SEC_PARAM_OOB 0       /**< Out Of Band data not available. */
#define SEC_PARAM_KEY_SIZE 16 /**< Minimum encryption key size. 7 to 16 */
#define SEC_PARAM_BOND 1      /**< Perform bonding. */
#define SEC_PARAM_MITM 1      /**< MITM on -> Passkey Entry (PIN 123456) */
#define SEC_PARAM_LESC                                                         \
	1 /**< LE Secure Connections enabled (RiM_BY61 requires SC; matches nRF).   \
	   */
#define SEC_PARAM_KEYPRESS 0 /**< Keypress notifications not enabled. */
#define SEC_PARAM_IKEY                                                         \
	GAP_KDIST_ENCKEY |                                                          \
	    GAP_KDIST_IDKEY /**< Initiator Key Distribution. (@enum gap_kdist) */
#define SEC_PARAM_RKEY                                                         \
	GAP_KDIST_ENCKEY |                                                          \
	    GAP_KDIST_IDKEY /**< Responder Key Distribution. (@enum gap_kdist) */
#define SEC_PARAM_SEC_MODE_LEVEL                                               \
	GAP_SEC1_AUTH_PAIR_ENC /**< Device security requirements (minimum security  \
	                          level). (@enum see gap_sec_req) */

/* bond conifg */
#define MAX_BOND_PEER 5
#define BOND_STORE_ENABLE 0
#define BOND_DATA_BASE_ADDR 0x0103B000

/* profiles config  */
#define CFG_PRF_RDTSC 1

/* scan config */
// ============================================================
// ÂÛÁÎÐ ÐÅÆÈÌÀ ÔÈËÜÒÐÀ ÑÊÀÍÈÐÎÂÀÍÈß
// Ðàñêîììåíòèðóéòå äëÿ ôèëüòðà ïî MAC-àäðåñó BE:74:FC:18:35:F2
// Ïî óìîë÷àíèþ - ôèëüòð ïî èìåíè "RiM_BY61_0000000"
// #define BLE_FILTER_BY_MAC
// ============================================================

#define SCAN_PARAM_TYPE                                                        \
	GAPM_SCAN_TYPE_CONN_DISC /**< Central role - can initiate connections */
#define SCAN_PARAM_PROP_ACTIVE                                                 \
	1 /**< if requre scan respond data. 1:active, 0:passive*/
#define SCAN_PARAM_DUP_FILT_POL                                                \
	GAPM_DUP_FILT_DIS          /**< (@enum gapm_dup_filter_pol) */
#define SCAN_PARAM_INTV 160   /**< scan interval, unit:0.625ms */
#define SCAN_PARAM_WD 150     /**< scan window, unit:0.625ms */
#define SCAN_PARAM_DURATION 0 /**< scan duration, unit:10ms, 0 mean forever */
#define SCAN_PARAM_CONNECT_EN true

/* User config  */
#define HP_LOG_ERROR_ENABLE 1
#define HP_LOG_WARNING_ENABLE 1
#define HP_LOG_INFO_ENABLE 1 // 1
#define HP_LOG_DEBUG_ENABLE 1

#define HP_LOG_LPUART_ENABLE 1
#define HP_TIMER_ENABLE 1

#define FIRMWARE_VERSION "1.0.0"
#define HARDWARE_VERSION "1.0.0"

#endif // _APP_USER_CONFIG_H_
