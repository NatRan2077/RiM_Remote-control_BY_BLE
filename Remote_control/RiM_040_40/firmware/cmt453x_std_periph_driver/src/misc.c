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
 * @file misc.c
 * @version v1.0.0
 *
  */
#include "misc.h"

/** @addtogroup CMT453X_StdPeriph_Driver
 * @{
 */

/** @addtogroup MISC
 * @brief MISC driver modules
 * @{
 */

/** @addtogroup MISC_Private_TypesDefinitions
 * @{
 */

/**
 * @}
 */

/** @addtogroup MISC_Private_Defines
 * @{
 */

#define AIRCR_VECTKEY_MASK ((uint32_t)0x05FA0000)
/**
 * @}
 */

/** @addtogroup MISC_Private_Macros
 * @{
 */

/**
 * @}
 */

/** @addtogroup MISC_Private_Variables
 * @{
 */

/**
 * @}
 */

/** @addtogroup MISC_Private_FunctionPrototypes
 * @{
 */

/**
 * @}
 */

/** @addtogroup MISC_Private_Functions
 * @{
 */

/**
 * @brief  Initializes the NVIC peripheral according to the specified
 *         parameters in the NVIC_InitStruct.
 * @param NVIC_InitStruct pointer to a NVIC_InitType structure that contains
 *         the configuration information for the specified NVIC peripheral.
 */
void NVIC_Init(NVIC_InitType* NVIC_InitStruct)
{
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(NVIC_InitStruct->NVIC_IRQChannelCmd));
    assert_param(IS_NVIC_PRIORITY(NVIC_InitStruct->NVIC_IRQChannelPriority));

    /* Set Interrupt Priority */
    NVIC_SetPriority(NVIC_InitStruct->NVIC_IRQChannel, NVIC_InitStruct->NVIC_IRQChannelPriority);

    if (NVIC_InitStruct->NVIC_IRQChannelCmd != DISABLE)
    {
        /* Enable the Selected IRQ Channels --------------------------------------*/
        NVIC_EnableIRQ(NVIC_InitStruct->NVIC_IRQChannel);
    }
    else
    {
        /* Disable the Selected IRQ Channels -------------------------------------*/
        NVIC_DisableIRQ(NVIC_InitStruct->NVIC_IRQChannel);
    }
}

/**
 * @brief  Configures the SysTick clock source.
 * @param SysTick_CLKSource specifies the SysTick clock source.
 *   This parameter can be one of the following values:
 *     @arg SysTick_CLKSource_HCLK_Div8 AHB clock divided by 8 selected as SysTick clock source.
 *     @arg SysTick_CLKSource_HCLK AHB clock selected as SysTick clock source.
 */
void SysTick_CLKSourceConfig(uint32_t SysTick_CLKSource)
{
    /* Check the parameters */
    assert_param(IS_SYSTICK_CLK_SOURCE(SysTick_CLKSource));
    if (SysTick_CLKSource == SysTick_CLKSource_HCLK)
    {
        SysTick->CTRL |= SysTick_CLKSource_HCLK;
    }
    else
    {
        SysTick->CTRL &= SysTick_CLKSource_HCLK_Div8;
    }
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
