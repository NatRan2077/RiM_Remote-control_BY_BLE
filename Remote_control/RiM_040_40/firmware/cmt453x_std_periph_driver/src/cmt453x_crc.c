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
 * @file cmt453x_crc.c
 * @version v1.0.0
 *
  */
#include "cmt453x_crc.h"

/** @addtogroup CMT453X_StdPeriph_Driver
 * @{
 */

/** @addtogroup CRC
 * @brief CRC driver modules
 * @{
 */

/** @addtogroup CRC_Private_TypesDefinitions
 * @{
 */

/**
 * @}
 */

/** @addtogroup CRC_Private_Defines
 * @{
 */

/**
 * @}
 */

/** @addtogroup CRC_Private_Macros
 * @{
 */

/**
 * @}
 */

/** @addtogroup CRC_Private_Variables
 * @{
 */

/**
 * @}
 */

/** @addtogroup CRC_Private_FunctionPrototypes
 * @{
 */

/**
 * @}
 */

/** @addtogroup CRC_Private_Functions
 * @{
 */

/**
 * @brief  Resets the CRC Data register (DAT).
 */
void CRC32_ResetCrc(void)
{
    /* Reset CRC generator */
    CRC->CRC32CTRL = CRC32_CTRL_RESET;
}

/**
 * @brief  Computes the 32-bit CRC of a given data word(32-bit).
 * @param Data data word(32-bit) to compute its CRC
 * @return 32-bit CRC
 */
uint32_t CRC32_CalcCrc(uint32_t Data)
{
    CRC->CRC32DAT = Data;

    return (CRC->CRC32DAT);
}

/**
 * @brief  Computes the 32-bit CRC of a given buffer of data word(32-bit).
 * @param pBuffer pointer to the buffer containing the data to be computed
 * @param BufferLength length of the buffer to be computed
 * @return 32-bit CRC
 */
uint32_t CRC32_CalcBufCrc(uint32_t pBuffer[], uint32_t BufferLength)
{
    uint32_t index = 0;

    for (index = 0; index < BufferLength; index++)
    {
        CRC->CRC32DAT = pBuffer[index];
    }
    return (CRC->CRC32DAT);
}

/**
 * @brief  Returns the current CRC value.
 * @return 32-bit CRC
 */
uint32_t CRC32_GetCrc(void)
{
    return (CRC->CRC32DAT);
}

/**
 * @brief  Stores a 8-bit data in the Independent Data(ID) register.
 * @param IDValue 8-bit value to be stored in the ID register
 */
void CRC32_SetIDat(uint8_t IDValue)
{
    CRC->CRC32IDAT = IDValue;
}

/**
 * @brief  Returns the 8-bit data stored in the Independent Data(ID) register
 * @return 8-bit value of the ID register
 */
uint8_t CRC32_GetIDat(void)
{
    return (CRC->CRC32IDAT);
}

// CRC16 add
void __CRC16_SetLittleEndianFmt(void)
{
    CRC->CRC16CTRL = CRC16_CTRL_LITTLE | CRC->CRC16CTRL;
}
void __CRC16_SetBigEndianFmt(void)
{
    CRC->CRC16CTRL = CRC16_CTRL_BIG & CRC->CRC16CTRL;
}
void __CRC16_SetCleanEnable(void)
{
    CRC->CRC16CTRL = CRC16_CTRL_RESET | CRC->CRC16CTRL;
}
void __CRC16_SetCleanDisable(void)
{
    CRC->CRC16CTRL = CRC16_CTRL_NO_RESET & CRC->CRC16CTRL;
}

uint16_t __CRC16_CalcCrc(uint8_t Data)
{
    CRC->CRC16DAT = Data;
    return (CRC->CRC16D);
}

void __CRC16_SetCrc(uint8_t Data)
{
    CRC->CRC16DAT = Data;
}

uint16_t __CRC16_GetCrc(void)
{
    return (CRC->CRC16D);
}

void __CRC16_SetLRC(uint8_t Data)
{
    CRC->LRC = Data;
}

uint8_t __CRC16_GetLRC(void)
{
    return (CRC->LRC);
}

uint16_t CRC16_CalcBufCrc(uint8_t pBuffer[], uint32_t BufferLength)
{
    uint32_t index = 0;

    CRC->CRC16D = 0x00;
    for (index = 0; index < BufferLength; index++)
    {
        CRC->CRC16DAT = pBuffer[index];
    }
    return (CRC->CRC16D);
}

uint16_t CRC16_CalcCRC(uint8_t Data)
{
    CRC->CRC16DAT = Data;

    return (CRC->CRC16D);
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
