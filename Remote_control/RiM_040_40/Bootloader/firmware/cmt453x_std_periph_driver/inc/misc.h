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
 * @file misc.h
 
 * @version v1.0.0
 *
  */
#ifndef __MISC_H__
#define __MISC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmt453x.h"

/** @addtogroup CMT453X_StdPeriph_Driver
 * @{
 */

/** @addtogroup MISC
 * @{
 */

/** @addtogroup MISC_Exported_Types
 * @{
 */

/**
 * @brief  NVIC Init Structure definition
 */

typedef struct
{
    IRQn_Type NVIC_IRQChannel; /*!< Specifies the IRQ channel to be enabled or disabled.
                                  This parameter can be a value of @ref IRQn_Type
                                  (For the complete CMT453X Devices IRQ Channels list, please
                                   refer to cmt453x.h file) */

    uint8_t NVIC_IRQChannelPriority; /*!< Specifies the priority for the IRQ channel
                                          specified in NVIC_IRQChannel. This parameter can be a value
                                          between 0 and 3 */

    FunctionalState NVIC_IRQChannelCmd; /*!< Specifies whether the IRQ channel defined in NVIC_IRQChannel
                                             will be enabled or disabled.
                                             This parameter can be set either to ENABLE or DISABLE */
} NVIC_InitType;

/**
 * @}
 */

/** @addtogroup MISC_Exported_Constants
 * @{
 */

/**
 * @}
 */

/** @addtogroup Preemption_Priority_Group
 * @{
 */

#define IS_NVIC_PRIORITY(PRIORITY) ((PRIORITY) < 0x04)

#define IS_NVIC_OFFSET(OFFSET) ((OFFSET) < 0x000FFFFF)

/**
 * @}
 */

/** @addtogroup SysTick_clock_source
 * @{
 */

#define SysTick_CLKSource_HCLK_Div8 ((uint32_t)0xFFFFFFFB)
#define SysTick_CLKSource_HCLK      ((uint32_t)0x00000004)
#define IS_SYSTICK_CLK_SOURCE(SOURCE)                                                                                  \
    (((SOURCE) == SysTick_CLKSource_HCLK) || ((SOURCE) == SysTick_CLKSource_HCLK_Div8))
/**
 * @}
 */

/**
 * @}
 */

/** @addtogroup MISC_Exported_Macros
 * @{
 */

/**
 * @}
 */

/** @addtogroup MISC_Exported_Functions
 * @{
 */

void NVIC_Init(NVIC_InitType* NVIC_InitStruct);
void SysTick_CLKSourceConfig(uint32_t SysTick_CLKSource);

#ifdef __cplusplus
}
#endif

#endif /* __MISC_H__ */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
