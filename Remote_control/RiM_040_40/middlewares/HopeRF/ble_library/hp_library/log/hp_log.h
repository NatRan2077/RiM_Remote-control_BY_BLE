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
 * @file hp_log.h
 
 * @version v1.0.0
 *
  */

#ifndef __HP_LOG_H__
#define __HP_LOG_H__


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_user_config.h"

#if   (HP_LOG_LPUART_ENABLE)
#include "hp_log_lpuart.h" 
#elif (HP_LOG_USART_ENABLE)
#include "hp_log_usart.h"
#elif (HP_LOG_RTT_ENABLE)
#include "hp_log_rtt.h"       
#endif

/* Private define ------------------------------------------------------------*/
#ifndef HP_LOG_ERROR_ENABLE
#define HP_LOG_ERROR_ENABLE      0
#endif
#ifndef HP_LOG_WARNING_ENABLE
#define HP_LOG_WARNING_ENABLE    0
#endif
#ifndef HP_LOG_INFO_ENABLE
#define HP_LOG_INFO_ENABLE       0
#endif
#ifndef HP_LOG_DEBUG_ENABLE
#define HP_LOG_DEBUG_ENABLE      0
#endif
#ifndef HP_LOG_LPUART_ENABLE
#define HP_LOG_LPUART_ENABLE     0
#endif
#ifndef HP_LOG_USART_ENABLE
#define HP_LOG_USART_ENABLE      0
#endif
#ifndef HP_LOG_RTT_ENABLE
#define HP_LOG_RTT_ENABLE        0
#endif

#ifndef PRINTF_COLOR_ENABLE
#define PRINTF_COLOR_ENABLE      0
#endif
#define LOG_COLOR_RED            "\033[0;31m"
#define LOG_COLOR_YELLOW         "\033[0;33m"
#define LOG_COLOR_CYAN           "\033[0;36m"
#define LOG_COLOR_GREEN          "\033[0;32m"


#if   (HP_LOG_LPUART_ENABLE)
#define HP_LOG_INTERNAL_OUTPUT(color, ...)  HP_LOG_LPUART_OUTPUT(color, __VA_ARGS__)
#define HP_LOG_INTERNAL_INIT()              HP_LOG_LPUART_INIT() 
#define HP_LOG_INTERNAL_DEINIT()           
#elif (HP_LOG_USART_ENABLE)
#define HP_LOG_INTERNAL_OUTPUT(color, ...)  HP_LOG_USART_OUTPUT(color, __VA_ARGS__)
#define HP_LOG_INTERNAL_INIT()              HP_LOG_USART_INIT()
#define HP_LOG_INTERNAL_DEINIT()            HP_LOG_USART_DEINIT()
#elif (HP_LOG_RTT_ENABLE)
#define HP_LOG_INTERNAL_OUTPUT(color, ...)  HP_LOG_RTT_OUTPUT(color, __VA_ARGS__)
#define HP_LOG_INTERNAL_INIT()              HP_LOG_RTT_INIT()
#define HP_LOG_INTERNAL_DEINIT()            HP_LOG_RTT_DEINIT()
#else
#define HP_LOG_INTERNAL_OUTPUT(color, ...)  
#define HP_LOG_INTERNAL_INIT() 
#define HP_LOG_INTERNAL_DEINIT()
#endif
/* Public typedef -----------------------------------------------------------*/
/* Public define ------------------------------------------------------------*/
#if  HP_LOG_ERROR_ENABLE
#define HP_LOG_ERROR(...)        HP_LOG_INTERNAL_OUTPUT(LOG_COLOR_RED, __VA_ARGS__)
#else
#define HP_LOG_ERROR( ...) 
#endif

#if HP_LOG_WARNING_ENABLE
#define HP_LOG_WARNING(...)      HP_LOG_INTERNAL_OUTPUT(LOG_COLOR_YELLOW, __VA_ARGS__)
#else
#define HP_LOG_WARNING( ...) 
#endif

#if  HP_LOG_INFO_ENABLE
#define HP_LOG_INFO(...)         HP_LOG_INTERNAL_OUTPUT(LOG_COLOR_CYAN, __VA_ARGS__)
#else
#define HP_LOG_INFO( ...) 
#endif

#if  HP_LOG_DEBUG_ENABLE
#define HP_LOG_DEBUG(...)        HP_LOG_INTERNAL_OUTPUT(LOG_COLOR_GREEN, __VA_ARGS__)
#else
#define HP_LOG_DEBUG( ...) 
#endif


/**
 * @brief   Initialization the log module
 * @param  
 * @return 
 * @note   
 */
#define HP_LOG_INIT()              HP_LOG_INTERNAL_INIT()

/**
 * @brief   Deinitialize the log module
 * @param  
 * @return 
 * @note   
 */
#define HP_LOG_DEINIT()            HP_LOG_INTERNAL_DEINIT()
/* Public constants ---------------------------------------------------------*/
/* Public function prototypes -----------------------------------------------*/



#ifdef __cplusplus
}
#endif

#endif /* __HP_LOG_H__ */
/**
 * @}
 */
