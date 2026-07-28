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
 * @file hp_delay.h
 
 * @version v1.0.0
 *
  */

#ifndef __HP_DELAY_H__
#define __HP_DELAY_H__

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h> // standard boolean definitions
#include <stdint.h>  // standard integer functions
/* Public define ------------------------------------------------------------*/
/* Public typedef -----------------------------------------------------------*/
/* Public define ------------------------------------------------------------*/ 
/* Public constants ---------------------------------------------------------*/
/* Public function prototypes -----------------------------------------------*/

/**
 * @brief  delay in cycles which is running 4 asm instructions in (10/110) us 
 * @param  ui32Cycles delay time in (10/110) us 
 * @return 
 * @note   
 */
void delay_cycles(uint32_t ui32Cycles);

/**
 * @brief  delay in 10us
 * @param  val delay time in 10us 
 * @return 
 * @note   
 */
void delay_n_10us(uint32_t val);

/**
 * @brief  delay in ms
 * @param  val delay time in ms 
 * @return 
 * @note   
 */
void delay_n_ms(uint32_t val);

/**
 * @brief  delay in us
 * @param  val delay time in us 
 * @return 
 * @note   
 */
void delay_n_us(uint32_t val);


#endif //__HP_DELAY_H__
