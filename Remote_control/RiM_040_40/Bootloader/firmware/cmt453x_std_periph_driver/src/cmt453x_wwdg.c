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
 * @file cmt453x_wwdg.c
 * @version v1.0.0
 *
  */
#include "cmt453x_wwdg.h"
#include "cmt453x_rcc.h"

/** @addtogroup CMT453X_StdPeriph_Driver
 * @{
 */

/** @addtogroup WWDG
 * @brief WWDG driver modules
 * @{
 */

/** @addtogroup WWDG_Private_TypesDefinitions
 * @{
 */

/**
 * @}
 */

/** @addtogroup WWDG_Private_Defines
 * @{
 */



/* --------------------- WWDG registers bit mask ------------------------ */

/* CTRL register bit mask */
#define CTRL_ACTB_SET ((uint32_t)0x00000080)

/* CFG register bit mask */
#define CFG_TIMERB_MASK ((uint32_t)0xFFFFFE7F)
#define CFG_W_MASK      ((uint32_t)0xFFFFFF80)
#define BIT_MASK        ((uint8_t)0x7F)

/**
 * @}
 */

/** @addtogroup WWDG_Private_Macros
 * @{
 */

/**
 * @}
 */

/** @addtogroup WWDG_Private_Variables
 * @{
 */

/**
 * @}
 */

/** @addtogroup WWDG_Private_FunctionPrototypes
 * @{
 */

/**
 * @}
 */

/** @addtogroup WWDG_Private_Functions
 * @{
 */

/**
 * @brief  Deinitializes the WWDG peripheral registers to their default reset values.
 */
void WWDG_DeInit(void)
{
    RCC_EnableAPB1PeriphReset(RCC_APB1_PERIPH_WWDG, ENABLE);
    RCC_EnableAPB1PeriphReset(RCC_APB1_PERIPH_WWDG, DISABLE);
}

/**
 * @brief  Sets the WWDG Prescaler.
 * @param WWDG_Prescaler specifies the WWDG Prescaler.
 *   This parameter can be one of the following values:
 *     @arg WWDG_PRESCALER_DIV1 WWDG counter clock = (PCLK1/4096)/1
 *     @arg WWDG_PRESCALER_DIV2 WWDG counter clock = (PCLK1/4096)/2
 *     @arg WWDG_PRESCALER_DIV4 WWDG counter clock = (PCLK1/4096)/4
 *     @arg WWDG_PRESCALER_DIV8 WWDG counter clock = (PCLK1/4096)/8
 */
void WWDG_SetPrescalerDiv(uint32_t WWDG_Prescaler)
{
    uint32_t tmpregister = 0;
    /* Check the parameters */
    assert_param(IS_WWDG_PRESCALER_DIV(WWDG_Prescaler));
    /* Clear WDGTB[1:0] bits */
    tmpregister = WWDG->CFG & CFG_TIMERB_MASK;
    /* Set WDGTB[1:0] bits according to WWDG_Prescaler value */
    tmpregister |= WWDG_Prescaler;
    /* Store the new value */
    WWDG->CFG = tmpregister;
}

/**
 * @brief  Sets the WWDG window value.
 * @param WindowValue specifies the window value to be compared to the downcounter.
 *   This parameter value must be lower than 0x80.
 */
void WWDG_SetWValue(uint8_t WindowValue)
{
    __IO uint32_t tmpregister = 0;

    /* Check the parameters */
    assert_param(IS_WWDG_WVALUE(WindowValue));
    /* Clear W[6:0] bits */

    tmpregister = WWDG->CFG & CFG_W_MASK;

    /* Set W[6:0] bits according to WindowValue value */
    tmpregister |= WindowValue & (uint32_t)BIT_MASK;

    /* Store the new value */
    WWDG->CFG = tmpregister;
}

/**
 * @brief  Enables the WWDG Early Wakeup interrupt(EWI).
 */
void WWDG_EnableInt(void)
{
   
    WWDG->CFG |= (uint32_t)( ENABLE << 9);
}

/**
 * @brief  Sets the WWDG counter value.
 * @param Counter specifies the watchdog counter value.
 *   This parameter must be a number between 0x40 and 0x7F.
 */
void WWDG_SetCnt(uint8_t Counter)
{
    /* Check the parameters */
    assert_param(IS_WWDG_CNT(Counter));
    /* Write to T[6:0] bits to configure the counter value, no need to do
       a read-modify-write; writing a 0 to WDGA bit does nothing */
    WWDG->CTRL = Counter & BIT_MASK;
}

/**
 * @brief  Enables WWDG and load the counter value.
 * @param Counter specifies the watchdog counter value.
 *   This parameter must be a number between 0x40 and 0x7F.
 */
void WWDG_Enable(uint8_t Counter)
{
    /* Check the parameters */
    assert_param(IS_WWDG_CNT(Counter));
    WWDG->CTRL = CTRL_ACTB_SET | Counter;
}

/**
 * @brief  Checks whether the Early Wakeup interrupt flag is set or not.
 * @return The new state of the Early Wakeup interrupt flag (SET or RESET)
 */
FlagStatus WWDG_GetEWINTF(void)
{
    return (FlagStatus)(WWDG->STS);
}

/**
 * @brief  Clears Early Wakeup interrupt flag.
 */
void WWDG_ClrEWINTF(void)
{
    WWDG->STS = (uint32_t)RESET;
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
