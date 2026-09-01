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
 * @file hp_timer.h
 
 * @version v1.0.1
 *
  */

/** @addtogroup 
 * @{
 */

#ifndef _HP_TIMER_H_
#define _HP_TIMER_H_

/* Includes ------------------------------------------------------------------*/
#include "ke_msg.h"
/* Public typedef -----------------------------------------------------------*/
/// Timer handler type
typedef uint8_t timer_hnd_t;
/// Timer callback function type definition
typedef void (* timer_callback_t)(void);
/* Public define ------------------------------------------------------------*/    
/// Max timer delay 41943sec (41943000ms)
#define KE_TIMER_DELAY_MAX          (41943000)
/// Value indicating an invalide timer operation
#define HP_TIMER_INVALID_HANDLER    (0xFF)
/* Public constants ---------------------------------------------------------*/
/* Public function prototypes -----------------------------------------------*/


/**
 * @brief     Process handler for the Timer messages.
 * @param msgid   Id of the message received
 * @param param   Pointer to the parameters of the message
 * @param dest_id ID of the receiving task instance (probably unused)
 * @param src_id  ID of the sending task instance
 * @param msg_ret Result of the message handler
 * @return    Returns if the message is handled by the process handler
 * @note   
 */

enum process_event_response hp_timer_api_process_handler(ke_msg_id_t const msgid,
                                                          void const *param,
                                                          ke_task_id_t const dest_id,
                                                          ke_task_id_t const src_id,
                                                          enum ke_msg_status_tag *msg_ret);


/**
 * @brief  Create a new timer.
 * @param  Param input
 * @brief Create a new timer.
 * @param delay The amount of timer value to wait (time resolution is 1ms)
 * @param fn    The callback to be called when the timer expires
 * @return The handler of the timer for future reference. If there are not timers available
 *         HP_TIMER_INVALID_HANDLER will be returned
 */
timer_hnd_t hp_timer_create(const uint32_t delay, timer_callback_t fn);

/**
 * @brief Cancel an active timer.
 * @param timer_id The timer handler to cancel
 */
void hp_timer_cancel(const timer_hnd_t timer_id);

/**
 * @brief Modify the delay of an existing timer.
 * @param timer_id The timer handler to modify
 * @param delay    The new delay value (time resolution is 1ms)
 * @return The timer handler if everything is ok
 */
timer_hnd_t hp_timer_modify(const timer_hnd_t timer_id, const uint32_t delay);

/**
 * @brief Cancel all the active timers.
 */
void hp_timer_cancel_all(void);

#endif // _HP_TIMER_H_

/**
 * @}
 */
