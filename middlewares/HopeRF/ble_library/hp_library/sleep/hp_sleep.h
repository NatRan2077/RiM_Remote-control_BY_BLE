/***************************************************************************//**
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
 * @file hp_sleep.h
 
 * @version v1.0.3
 *
  */

/** @addtogroup 
 * @{
 */
#ifndef __HP_SLEEP_H__
#define __HP_SLEEP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes -----------------------------------------------------------------*/
#include "global_func.h"
/* Public typedef -----------------------------------------------------------*/
/* Public define ------------------------------------------------------------*/    
/* Public constants ---------------------------------------------------------*/
/* Public function prototypes -----------------------------------------------*/

/**
 * @brief  sleep task function, usually run after rwip_schedule function in main loop.
 * @param  
 * @return 
 * @note   
 */
void hp_sleep(void);

/**
 * @brief  Acquire a sleep lock, it will prevent the os enter sleep mode. 
 *         We should call hp_sleep_lock_release function when this lock can be release. 
 * @param  
 * @return 
 * @note   
 */
uint8_t hp_sleep_lock_acquire(void);

/**
 * @brief  Release a sleep lock, if all the lock has been released, os will enter sleep mode 
 *         when run out of task.
 * @param  
 * @return 
 * @note   
 */
uint8_t hp_sleep_lock_release(void);
/**
 * @brief  User code beofre enter sleep mode
 * @param  
 * @return 
 * @note   
 */
void app_sleep_prepare_proc(void);

/**
 * @brief  User code after out of sleep mode. This function run after interrupt
 *         handler function if any interrupt pending when sleep.
 * @param  
 * @return 
 * @note   
 */
void app_sleep_resume_proc(void);

#ifdef SLEEP_LP_TIMER_ENABLE
/**
 * @brief  low power timer initialization.
 * @param  period_ms: 1 - 0x7fff 
 * @param  func: low power timer callback, not suggest run over 2ms
 * @return void
 * @note   
 */
void hp_sleep_lp_timer_config(uint16_t period_ms, IRQ_HANNDLE_FUN func);

/**
 * @brief  active rwip in wakeup callback.
 * @param  
 * @return void
 * @note   
 */
void hp_sleep_lp_timer_wake_ble(void);
#endif

#endif /* __HP_SLEEP_H__ */
/**
 * @}
 */


