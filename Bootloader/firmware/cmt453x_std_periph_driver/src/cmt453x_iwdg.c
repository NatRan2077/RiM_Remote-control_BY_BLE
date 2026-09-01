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
 * @file cmt453x_iwdg.c
 * @version v1.0.0
 *
  */
#include "cmt453x_iwdg.h"

/** @addtogroup CMT453X_StdPeriph_Driver
 * @{
 */

/** @addtogroup IWDG
 * @brief IWDG driver modules
 * @{
 */

/** @addtogroup IWDG_Private_TypesDefinitions
 * @{
 */

/**
 * @}
 */

/** @addtogroup IWDG_Private_Defines
 * @{
 */

/* ---------------------- IWDG registers bit mask ----------------------------*/

/* KEY register bit mask */
#define KEY_ReloadKey ((uint16_t)0xAAAA)
#define KEY_EnableKey ((uint16_t)0xCCCC)

/**
 * @}
 */

/** @addtogroup IWDG_Private_Macros
 * @{
 */

/**
 * @}
 */

/** @addtogroup IWDG_Private_Variables
 * @{
 */

/**
 * @}
 */

/** @addtogroup IWDG_Private_FunctionPrototypes
 * @{
 */

/**
 * @}
 */

/** @addtogroup IWDG_Private_Functions
 * @{
 */

/**
 * @brief  Enables or disables write access to IWDG_PR and IWDG_RLR registers.
 * @param IWDG_WriteAccess new state of write access to IWDG_PR and IWDG_RLR registers.
 *   This parameter can be one of the following values:
 *     @arg IWDG_WRITE_ENABLE Enable write access to IWDG_PR and IWDG_RLR registers
 *     @arg IWDG_WRITE_DISABLE Disable write access to IWDG_PR and IWDG_RLR registers
 */
void IWDG_WriteConfig(uint16_t IWDG_WriteAccess)
{
    /* Check the parameters */
    assert_param(IS_IWDG_WRITE(IWDG_WriteAccess));
    IWDG->KEY = IWDG_WriteAccess;
}

/**
 * @brief  Sets IWDG Prescaler value.
 * @param IWDG_Prescaler specifies the IWDG Prescaler value.
 *   This parameter can be one of the following values:
 *     @arg IWDG_PRESCALER_DIV4 IWDG prescaler set to 4
 *     @arg IWDG_PRESCALER_DIV8 IWDG prescaler set to 8
 *     @arg IWDG_PRESCALER_DIV16 IWDG prescaler set to 16
 *     @arg IWDG_PRESCALER_DIV32 IWDG prescaler set to 32
 *     @arg IWDG_PRESCALER_DIV64 IWDG prescaler set to 64
 *     @arg IWDG_PRESCALER_DIV128 IWDG prescaler set to 128
 *     @arg IWDG_PRESCALER_DIV256 IWDG prescaler set to 256
 */
void IWDG_SetPrescalerDiv(uint8_t IWDG_Prescaler)
{
    /* Check the parameters */
    assert_param(IS_IWDG_PRESCALER_DIV(IWDG_Prescaler));
    IWDG->PREDIV = IWDG_Prescaler;
}

/**
 * @brief  Sets IWDG Reload value.
 * @param Reload specifies the IWDG Reload value.
 *   This parameter must be a number between 0 and 0x0FFF.
 */
void IWDG_CntReload(uint16_t Reload)
{
    /* Check the parameters */
    assert_param(IS_IWDG_RELOAD(Reload));
    IWDG->RELV = Reload;
}

/**
 * @brief  Reloads IWDG counter with value defined in the reload register
 *   (write access to IWDG_PR and IWDG_RLR registers disabled).
 */
void IWDG_ReloadKey(void)
{
    IWDG->KEY = KEY_ReloadKey;
}

/**
 * @brief  Enables IWDG (write access to IWDG_PR and IWDG_RLR registers disabled).
 */
void IWDG_Enable(void)
{
    IWDG->KEY = KEY_EnableKey;
}

/**
 * @brief  Checks whether the specified IWDG flag is set or not.
 * @param IWDG_FLAG specifies the flag to check.
 *   This parameter can be one of the following values:
 *     @arg IWDG_PVU_FLAG Prescaler Value Update on going
 *     @arg IWDG_CRVU_FLAG Reload Value Update on going
 * @return The new state of IWDG_FLAG (SET or RESET).
 */
FlagStatus IWDG_GetStatus(uint16_t IWDG_FLAG)
{
    FlagStatus bitstatus = RESET;
    /* Check the parameters */
    assert_param(IS_IWDG_FLAG(IWDG_FLAG));
    if ((IWDG->STS & IWDG_FLAG) != (uint32_t)RESET)
    {
        bitstatus = SET;
    }
    else
    {
        bitstatus = RESET;
    }
    /* Return the flag status */
    return bitstatus;
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
