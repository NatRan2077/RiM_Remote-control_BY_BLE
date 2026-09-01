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
 * @file cmt453x_pwr.c
  
 * @version v1.0.4
 *
  */
#include "cmt453x_pwr.h"

/** @addtogroup CMT453X_StdPeriph_Driver
 * @{
 */

/**
 * @}
 */

/** @addtogroup PWR_Private_Macros
 * @{
 */

/**
 * @}
 */

/** @addtogroup PWR_Private_Variables
 * @{
 */

/**
 * @}
 */

/** @addtogroup PWR_Private_FunctionPrototypes
 * @{
 */

/**
 * @}
 */

/** @addtogroup PWR_Private_Functions
 * @{
 */
 
 
/**
 * @brief  Deinitializes the PWR peripheral registers to their default reset values.
 */
void PWR_DeInit(void)
{
    RCC_EnableAPB1PeriphReset(RCC_APB1_PERIPH_PWR, ENABLE);
    RCC_EnableAPB1PeriphReset(RCC_APB1_PERIPH_PWR, DISABLE);
}

/**
  * @brief  Enters IDLE mode.
  * @param  SLEEPONEXIT: specifies the SLEEPONEXIT state in IDLE mode.
  *   This parameter can be one of the following values:
  *     @arg DISABLE: SLEEP mode with SLEEPONEXIT disable
  *     @arg ENABLE : SLEEP mode with SLEEPONEXIT enable
  * @param  PWR_STOPEntry: specifies if SLEEP mode in entered with WFI or WFE instruction.
  *   This parameter can be one of the following values:
  *     @arg PWR_IDLEENTRY_WFI: enter IDLE mode with WFI instruction
  *     @arg PWR_IDLEENTRY_WFE: enter IDLE mode with WFE instruction
  * @retval None
  */
void PWR_EnterIDLEMode(uint8_t IDLEONEXIT, uint8_t PWR_IDLEEntry)
{  
  
    /* CLEAR SLEEPDEEP bit of Cortex System Control Register */
    SCB->SCR &= (uint32_t)~((uint32_t)SCB_SCR_SLEEPDEEP); 

      /* Select SLEEPONEXIT mode entry --------------------------------------------------*/
    if(IDLEONEXIT == ENABLE)
    {   
        /* the MCU enters Sleep mode as soon as it exits the lowest priority ISR */
        SCB->SCR |= SCB_SCR_SLEEPONEXIT;
    }
    else if(IDLEONEXIT == DISABLE)
    {
        /* Sleep-now */
        SCB->SCR &= (uint32_t)~((uint32_t)SCB_SCR_SLEEPONEXIT);
    }       
      
    /* Select SLEEP mode entry --------------------------------------------------*/
    if(PWR_IDLEEntry == PWR_IDLEENTRY_WFI)
    {   
        /* Request Wait For Interrupt */
        __WFI();        
    }
    else
    {
        /* Request Wait For Event */
        __SEV();
        __WFE();
        __WFE();
    }    
}

/**
  * @brief  Enters SLEEP mode.
  * @param  PWR_SLEEPEntry: specifies if SLEEP mode in entered with WFI or WFE instruction.
  *   This parameter can be one of the following values:
  *     @arg PWR_SLEEPENTRY_WFI: enter SLEEP mode with WFI instruction
  *     @arg PWR_SLEEPENTRY_WFE: enter SLEEP mode with WFE instruction
  * @retval None
  */
void PWR_EnterSLEEPMode(uint8_t PWR_SleepEntry)
{
    uint32_t tmpreg = 0;
    /* Set BLE modem sleep */
    *(uint32_t*)0x40011004 |= 0x40; //for without HSE
    *(uint32_t *)(BLE_BASE + 0x30) = 0x07;
    while(PWR->CR1&PWR_CR1_OSC_EN);
    *(uint32_t*)0x40011004 &= ~0x40;//for without HSE
      
    /* Select the regulator state in SLEEP mode ---------------------------------*/
    tmpreg = PWR->CR1;
    /* Clear PDDS and FLPDS bits */
    tmpreg &= ~PWR_CR1_MODE_SEL;
    /* Set FLPDS bit according to PWR_Regulator value */    
    tmpreg |= (PWR_CR1_MODE_SLEEP |PWR_CR1_MODE_EN);
    /* Store the new value */
    PWR->CR1 = tmpreg;
    /* Set SLEEPDEEP bit of Cortex System Control Register */
    SCB->SCR |= SCB_SCR_SLEEPDEEP;

    /* Select SLEEP mode entry --------------------------------------------------*/
    if(PWR_SleepEntry == PWR_SLEEPENTRY_WFI)
    {   
        /* Request Wait For Interrupt */
        __WFI();
    }
    else
    {
        /* Request Wait For Event */
        __SEV();
        __WFE();
        __WFE();
    }

    /* Reset SLEEPDEEP bit of Cortex System Control Register */
    SCB->SCR &= (uint32_t)~((uint32_t)SCB_SCR_SLEEPDEEP);  
}

 /**
  * @brief  Enters PD mode.
  * @param  PWR_PDEntry: specifies if PD mode in entered with WFI or WFE instruction.
  *   This parameter can be one of the following values:
  *     @arg PWR_PDENTRY_WFI: enter PD mode with WFI instruction
  *     @arg PWR_PDENTRY_WFE: enter PD mode with WFE instruction
  * @retval None
  */
void PWR_EnterPDMode(uint8_t PWR_PDEntry)
{
    uint32_t tmpreg = 0; 
    /* Select the regulator state in SHUTDOWN mode ---------------------------------*/
    tmpreg = PWR->CR1;
    /* Clear PDDS  bits */
    tmpreg &= ~PWR_CR1_MODE_SEL;
    /* Set FLPDS bit according to PWR_Regulator value */    
    tmpreg |= PWR_CR1_MODE_PD|PWR_CR1_MODE_EN;
    /* Store the new value */
    PWR->CR1 = tmpreg;    

    /* Set SLEEPDEEP bit of Cortex System Control Register */
    SCB->SCR |= SCB_SCR_SLEEPDEEP;
    /* This option is used to ensure that store operations are completed */
    #if defined ( __CC_ARM   )
    __force_stores();
    #endif
    /* Select SHUTDOWN mode entry --------------------------------------------------*/
    if(PWR_PDEntry == PWR_PDENTRY_WFI)
    {   
        /* Request Wait For Interrupt */
        __WFI();
    }
    else
    {
        /* Request Wait For Event */
        __SEV();
        __WFE();
        __WFE();
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
