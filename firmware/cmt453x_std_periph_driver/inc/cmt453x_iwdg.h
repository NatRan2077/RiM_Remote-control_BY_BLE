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
 * @file cmt453x_iwdg.h
 
 * @version v1.0.0
 *
  */
#ifndef __CMT453X_IWDG_H__
#define __CMT453X_IWDG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmt453x.h"

/** @addtogroup CMT453X_StdPeriph_Driver
 * @{
 */

/** @addtogroup IWDG
 * @{
 */

/** @addtogroup IWDG_Exported_Types
 * @{
 */

/**
 * @}
 */

/** @addtogroup IWDG_Exported_Constants
 * @{
 */

/** @addtogroup IWDG_WriteAccess
 * @{
 */

#define IWDG_WRITE_ENABLE     ((uint16_t)0x5555)
#define IWDG_WRITE_DISABLE    ((uint16_t)0x0000)
#define IS_IWDG_WRITE(ACCESS) (((ACCESS) == IWDG_WRITE_ENABLE) || ((ACCESS) == IWDG_WRITE_DISABLE))
/**
 * @}
 */

/** @addtogroup IWDG_prescaler
 * @{
 */

#define IWDG_PRESCALER_DIV4   ((uint8_t)0x00)
#define IWDG_PRESCALER_DIV8   ((uint8_t)0x01)
#define IWDG_PRESCALER_DIV16  ((uint8_t)0x02)
#define IWDG_PRESCALER_DIV32  ((uint8_t)0x03)
#define IWDG_PRESCALER_DIV64  ((uint8_t)0x04)
#define IWDG_PRESCALER_DIV128 ((uint8_t)0x05)
#define IWDG_PRESCALER_DIV256 ((uint8_t)0x06)
#define IS_IWDG_PRESCALER_DIV(PRESCALER)                                                                               \
    (((PRESCALER) == IWDG_PRESCALER_DIV4) || ((PRESCALER) == IWDG_PRESCALER_DIV8)                                      \
     || ((PRESCALER) == IWDG_PRESCALER_DIV16) || ((PRESCALER) == IWDG_PRESCALER_DIV32)                                 \
     || ((PRESCALER) == IWDG_PRESCALER_DIV64) || ((PRESCALER) == IWDG_PRESCALER_DIV128)                                \
     || ((PRESCALER) == IWDG_PRESCALER_DIV256))
/**
 * @}
 */

/** @addtogroup IWDG_Flag
 * @{
 */

#define IWDG_PVU_FLAG          ((uint16_t)0x0001)
#define IWDG_CRVU_FLAG         ((uint16_t)0x0002)
#define IS_IWDG_FLAG(FLAG)     (((FLAG) == IWDG_PVU_FLAG) || ((FLAG) == IWDG_CRVU_FLAG))
#define IS_IWDG_RELOAD(RELOAD) ((RELOAD) <= 0xFFF)
/**
 * @}
 */

/**
 * @}
 */

/** @addtogroup IWDG_Exported_Macros
 * @{
 */

/**
 * @}
 */

/** @addtogroup IWDG_Exported_Functions
 * @{
 */

void IWDG_WriteConfig(uint16_t IWDG_WriteAccess);
void IWDG_SetPrescalerDiv(uint8_t IWDG_Prescaler);
void IWDG_CntReload(uint16_t Reload);
void IWDG_ReloadKey(void);
void IWDG_Enable(void);
FlagStatus IWDG_GetStatus(uint16_t IWDG_FLAG);

#ifdef __cplusplus
}
#endif

#endif /* __CMT453X_IWDG_H__ */
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
