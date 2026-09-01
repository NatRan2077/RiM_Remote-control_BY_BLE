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
 * @file cmt453x_wwdg.h
 
 * @version v1.0.0
 *
  */
#ifndef __CMT453X_WWDG_H__
#define __CMT453X_WWDG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmt453x.h"

/** @addtogroup CMT453X_StdPeriph_Driver
 * @{
 */

/** @addtogroup WWDG
 * @{
 */

/** @addtogroup WWDG_Exported_Types
 * @{
 */

/**
 * @}
 */

/** @addtogroup WWDG_Exported_Constants
 * @{
 */

/** @addtogroup WWDG_Prescaler
 * @{
 */

#define WWDG_PRESCALER_DIV1 ((uint32_t)0x00000000)
#define WWDG_PRESCALER_DIV2 ((uint32_t)0x00000080)
#define WWDG_PRESCALER_DIV4 ((uint32_t)0x00000100)
#define WWDG_PRESCALER_DIV8 ((uint32_t)0x00000180)
#define IS_WWDG_PRESCALER_DIV(PRESCALER)                                                                               \
    (((PRESCALER) == WWDG_PRESCALER_DIV1) || ((PRESCALER) == WWDG_PRESCALER_DIV2)                                      \
     || ((PRESCALER) == WWDG_PRESCALER_DIV4) || ((PRESCALER) == WWDG_PRESCALER_DIV8))
#define IS_WWDG_WVALUE(VALUE) ((VALUE) <= 0x7F)
#define IS_WWDG_CNT(COUNTER)  (((COUNTER) >= 0x40) && ((COUNTER) <= 0x7F))

/**
 * @}
 */

/**
 * @}
 */

/** @addtogroup WWDG_Exported_Macros
 * @{
 */
/**
 * @}
 */

/** @addtogroup WWDG_Exported_Functions
 * @{
 */

void WWDG_DeInit(void);
void WWDG_SetPrescalerDiv(uint32_t WWDG_Prescaler);
void WWDG_SetWValue(uint8_t WindowValue);
void WWDG_EnableInt(void);
void WWDG_SetCnt(uint8_t Counter);
void WWDG_Enable(uint8_t Counter);
FlagStatus WWDG_GetEWINTF(void);
void WWDG_ClrEWINTF(void);

#ifdef __cplusplus
}
#endif

#endif /* __CMT453X__WWDG_H */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
