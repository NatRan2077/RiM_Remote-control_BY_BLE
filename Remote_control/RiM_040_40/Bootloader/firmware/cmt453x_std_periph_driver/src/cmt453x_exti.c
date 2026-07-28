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
 * @file cmt453x_exti.c
 * @version v1.0.1
 *
  */
#include "cmt453x_exti.h"

/** @addtogroup CMT453X_StdPeriph_Driver
 * @{
 */

/** @addtogroup EXTI
 * @brief EXTI driver modules
 * @{
 */

/** @addtogroup EXTI_Private_TypesDefinitions
 * @{
 */

/**
 * @}
 */

/** @addtogroup EXTI_Private_Defines
 * @{
 */

#define EXTI_LINENONE ((uint32_t)0x00000000) /* No interrupt selected */

/**
 * @}
 */

/** @addtogroup EXTI_Private_Macros
 * @{
 */

/**
 * @}
 */

/** @addtogroup EXTI_Private_Variables
 * @{
 */

/**
 * @}
 */

/** @addtogroup EXTI_Private_FunctionPrototypes
 * @{
 */

/**
 * @}
 */

/** @addtogroup EXTI_Private_Functions
 * @{
 */

/**
 * @brief  Deinitializes the EXTI peripheral registers to their default reset values.
 */
void EXTI_DeInit(void)
{
    EXTI->IMASK  = 0x00000000;
    EXTI->EMASK  = 0x00000000;
    EXTI->RT_CFG = 0x00000000;
    EXTI->FT_CFG = 0x00000000;
    EXTI->PEND   = 0x00FFFFFF;
}

/**
 * @brief  Initializes the EXTI peripheral according to the specified
 *         parameters in the EXTI_InitStruct.
 * @param EXTI_InitStruct pointer to a EXTI_InitType structure
 *         that contains the configuration information for the EXTI peripheral.
 */
void EXTI_InitPeripheral(EXTI_InitType* EXTI_InitStruct)
{
    uint32_t tmp = 0;

    /* Check the parameters */
    assert_param(IS_EXTI_MODE(EXTI_InitStruct->EXTI_Mode));
    assert_param(IS_EXTI_TRIGGER(EXTI_InitStruct->EXTI_Trigger));
    assert_param(IS_EXTI_LINE(EXTI_InitStruct->EXTI_Line));
    assert_param(IS_FUNCTIONAL_STATE(EXTI_InitStruct->EXTI_LineCmd));

    tmp = (uint32_t)EXTI_BASE;

    if (EXTI_InitStruct->EXTI_LineCmd != DISABLE)
    {
        /* Clear EXTI line configuration */
        EXTI->IMASK &= ~EXTI_InitStruct->EXTI_Line;
        EXTI->EMASK &= ~EXTI_InitStruct->EXTI_Line;

        tmp += EXTI_InitStruct->EXTI_Mode;

        *(__IO uint32_t*)tmp |= EXTI_InitStruct->EXTI_Line;

        /* Clear Rising Falling edge configuration */
        EXTI->RT_CFG &= ~EXTI_InitStruct->EXTI_Line;
        EXTI->FT_CFG &= ~EXTI_InitStruct->EXTI_Line;

        /* Select the trigger for the selected external interrupts */
        if (EXTI_InitStruct->EXTI_Trigger == EXTI_Trigger_Rising_Falling)
        {
            /* Rising Falling edge */
            EXTI->RT_CFG |= EXTI_InitStruct->EXTI_Line;
            EXTI->FT_CFG |= EXTI_InitStruct->EXTI_Line;
        }
        else
        {
            tmp = (uint32_t)EXTI_BASE;
            tmp += EXTI_InitStruct->EXTI_Trigger;

            *(__IO uint32_t*)tmp |= EXTI_InitStruct->EXTI_Line;
        }
    }
    else
    {
        tmp += EXTI_InitStruct->EXTI_Mode;

        /* Disable the selected external lines */
        *(__IO uint32_t*)tmp &= ~EXTI_InitStruct->EXTI_Line;
    }
}

/**
 * @brief  Fills each EXTI_InitStruct member with its reset value.
 * @param EXTI_InitStruct pointer to a EXTI_InitType structure which will
 *         be initialized.
 */
void EXTI_InitStruct(EXTI_InitType* EXTI_InitStruct)
{
    EXTI_InitStruct->EXTI_Line    = EXTI_LINENONE;
    EXTI_InitStruct->EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStruct->EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct->EXTI_LineCmd = DISABLE;
}

/**
 * @brief  Generates a Software interrupt.
 * @param EXTI_Line specifies the EXTI lines to be enabled or disabled.
 *   This parameter can be any combination of EXTI_Linex where x can be (0..13).
 */
void EXTI_TriggerSWInt(uint32_t EXTI_Line)
{
    /* Check the parameters */
    assert_param(IS_EXTI_LINE(EXTI_Line));

    EXTI->SWIE |= EXTI_Line;
}

/**
 * @brief  Checks whether the specified EXTI line flag is set or not.
 * @param EXTI_Line specifies the EXTI line flag to check.
 *   This parameter can be:
 *     @arg EXTI_Linex External interrupt line x where x(0..13)
 * @return The new state of EXTI_Line (SET or RESET).
 */
FlagStatus EXTI_GetStatusFlag(uint32_t EXTI_Line)
{
    FlagStatus bitstatus = RESET;
    /* Check the parameters */
    assert_param(IS_GET_EXTI_LINE(EXTI_Line));

    if ((EXTI->PEND & EXTI_Line) != (uint32_t)RESET)
    {
        bitstatus = SET;
    }
    else
    {
        bitstatus = RESET;
    }
    return bitstatus;
}

/**
 * @brief  Clears the EXTI's line pending flags.
 * @param EXTI_Line specifies the EXTI lines flags to clear.
 *   This parameter can be any combination of EXTI_Linex where x can be (0..13).
 */
void EXTI_ClrStatusFlag(uint32_t EXTI_Line)
{
    /* Check the parameters */
    assert_param(IS_EXTI_LINE(EXTI_Line));

    EXTI->PEND = EXTI_Line;
}

/**
 * @brief  Checks whether the specified EXTI line is asserted or not.
 * @param EXTI_Line specifies the EXTI line to check.
 *   This parameter can be:
 *     @arg EXTI_Linex External interrupt line x where x(0..13)
 * @return The new state of EXTI_Line (SET or RESET).
 */
INTStatus EXTI_GetITStatus(uint32_t EXTI_Line)
{
    INTStatus bitstatus   = RESET;
    uint32_t enablestatus = 0;
    /* Check the parameters */
    assert_param(IS_GET_EXTI_LINE(EXTI_Line));

    enablestatus = EXTI->IMASK & EXTI_Line;
    if (((EXTI->PEND & EXTI_Line) != (uint32_t)RESET) && (enablestatus != (uint32_t)RESET))
    {
        bitstatus = SET;
    }
    else
    {
        bitstatus = RESET;
    }
    return bitstatus;
}

/**
 * @brief  Clears the EXTI's line pending bits.
 * @param EXTI_Line specifies the EXTI lines to clear.
 *   This parameter can be any combination of EXTI_Linex where x can be (0..13).
 */
void EXTI_ClrITPendBit(uint32_t EXTI_Line)
{
    /* Check the parameters */
    assert_param(IS_EXTI_LINE(EXTI_Line));

    EXTI->PEND = EXTI_Line;
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
