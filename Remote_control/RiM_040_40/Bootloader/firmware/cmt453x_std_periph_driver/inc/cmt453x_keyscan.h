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
 * @file cmt453x_keyscan.h
 
 * @version v1.0.4
 *
  */
#ifndef __CMT453X__KEYSCAN_H__
#define __CMT453X__KEYSCAN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmt453x.h"

/** @addtogroup cmt453x_StdPeriph_Driver
 * @{
 */

/** @addtogroup KEYSCAN
 * @{
 */

/** @addtogroup KEYSCAN_Exported_Types
 * @{
 */

/**
 * @brief  KEYSCAN Init Structure definition
 */
typedef struct
{
    uint32_t Mask; 
    uint16_t Mode; 
    uint16_t Wts; 
    uint16_t Dts; 
    uint16_t Int_en; 
    
} KEYSCAN_InitType;


/**
 * @brief  KEYSCAN NUM enumeration
 */
typedef enum 
{
    KEY_104,
    KEY_44,
    KEY_65,
}KEY_NUM;

/**
 * @brief  KEYSCAN NUM enumeration
 */
typedef enum 
{
    MODE_FIXED_INTV,
    MODE_SW_TRIG,
    MODE_PRESS_TRIG,
}KEY_MODE;

/**
 * @brief  KEYSCAN debounce time enumeration
 */
typedef enum 
{
    DTS_10MS,
    DTS_20MS,
    DTS_40MS,
    DTS_80MS,
    DTS_160MS,
    DTS_320MS,
    DTS_640MS,
    DTS_640MS_2,
}KEY_DTS;

/**
 * @brief  KEYSCAN scan wait time enumeration
 */
typedef enum 
{
    WTS_0MS,
    WTS_32MS,
    WTS_64MS,
    WTS_96MS,
    WTS_128MS,
    WTS_160MS,
    WTS_192MS,
    WTS_224MS,
}KEY_WTS;

/**
 * @brief  KEYSCAN INT trigger enumeration
 */
typedef enum 
{
    INT_DIS,
    INT_EN,
}KEY_INT_EN;
    
/**
 * @}
 */

/** @addtogroup KEYSCAN_Exported_Constants
 * @{
 */

/** @addtogroup KEYSCAN_Prescaler
 * @{
 */

/**
 * @}
 */

/**
 * @}
 */

/** @addtogroup KEYSCAN_Exported_Macros
 * @{
 */

#define KEYSCAN_EN_POS            0
#define KEY_MODE_POS              2
#define KEY_DTS_POS               4
#define KEY_WTS_POS               7
#define KEY_MASK_POS              10
#define KEY_SW_START_POS          12
#define KEY_INFO_CLR_POS          13
#define KEY_INT_EN_POS            22
#define KEY_IRP_POS               23

/**
 * @}
 */

/** @addtogroup KEYSCAN_Exported_Functions
 * @{
 */
void KEYSCAN_Init(KEYSCAN_InitType* KEYSCAN_InitStruct);
void KEYSCAN_Enable(FunctionalState Cmd);
void KEYSCAN_InfoClear(void);
void KEYSCAN_SoftwareStartScan(void);
FlagStatus KEYSCAN_GetInterruptState(void);
void KEYSCAN_ClearInterrupt(void);
void KEYSCAN_ReadKeyData(uint32_t *key_data);
#ifdef __cplusplus
}
#endif

#endif /* __CMT453X__KEYSCAN_H */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
