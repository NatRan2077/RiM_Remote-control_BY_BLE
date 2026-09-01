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
 * @file cmt453x_keyscan.c
 * @version v1.0.4
 *
  */
#include "cmt453x_keyscan.h"
#include "cmt453x_rcc.h"

/** @addtogroup CMT453X_StdPeriph_Driver
 * @{
 */

/**
 * @brief  Initializes the KEYSCAN peripheral according to the specified
 *         parameters in the KEYSCAN_InitType.
 * @param KEYSCAN_InitStruct pointer to a KEYSCAN_InitType structure that
 *         contains the configuration information for the KEYSCAN peripheral.
 */
 
void KEYSCAN_Init(KEYSCAN_InitType* KEYSCAN_InitStruct)
{
    KEYSCAN->KEYCR = 0;
        
    KEYSCAN->KEYCR =   (KEYSCAN_InitStruct->Mask<<KEY_MASK_POS)
                    | (KEYSCAN_InitStruct->Mode<<KEY_MODE_POS)
                    | (KEYSCAN_InitStruct->Dts<<KEY_DTS_POS)
                    | (KEYSCAN_InitStruct->Wts<<KEY_WTS_POS)
                    | (KEYSCAN_InitStruct->Int_en<<KEY_INT_EN_POS);
    //configer retention voltag
    *(uint32_t*)0x40007014 = 0x00000818; 
}


/**
 * @brief Enables or disables the KEYSCAN.
 * @param Cmd new state of KEYSCAN.This parameter can be: ENABLE or DISABLE.
 */
void KEYSCAN_Enable(FunctionalState Cmd)
{    
    /* Check the parameters */
    assert_param(IS_FUNCTIONAL_STATE(Cmd));

    if (Cmd != DISABLE)
    {
        /* Enable the selected USART by setting the UE bit in the CTRL1 register */
        KEYSCAN->KEYCR |= (1<<KEYSCAN_EN_POS);
    }
    else
    {
        /* Disable the selected USART by clearing the UE bit in the CTRL1 register */
        KEYSCAN->KEYCR &= ~(1<<KEYSCAN_EN_POS);
    }
}

/**
 * @brief Clear the KEYDATA information of KEYSCAN peripheral.
 */
void KEYSCAN_InfoClear(void)
{
    KEYSCAN->KEYCR |= (1<<KEY_INFO_CLR_POS);
}

/**
 * @brief Start Software mode scan with KEYSCAN peripheral.
 */
void KEYSCAN_SoftwareStartScan(void)
{
    KEYSCAN->KEYCR |= (1<<KEY_SW_START_POS);
}

/**
 * @brief Get the interrupt status of KEYSCAN peripheral.
 */
FlagStatus KEYSCAN_GetInterruptState(void)
{
    if(KEYSCAN->KEYCR & (1<<KEY_IRP_POS))
    {
        return SET;
    }
    return RESET;
}

/**
 * @brief Clear the interrupt pending status of KEYSCAN peripheral.
 */
void KEYSCAN_ClearInterrupt(void)
{
    KEYSCAN->KEYCR |= (1<<KEY_IRP_POS);
}

/**
 * @brief Read the key data.
 * @param key_data  An array with 5 word len to read out the key data.
 */
void KEYSCAN_ReadKeyData(uint32_t *key_data)
{
    key_data[0] = KEYSCAN->KEYDATA0;
    key_data[1] = KEYSCAN->KEYDATA1;
    key_data[2] = KEYSCAN->KEYDATA2;
    key_data[3] = KEYSCAN->KEYDATA3;
    key_data[4] = KEYSCAN->KEYDATA4;
}

/**
 * @}
 */

