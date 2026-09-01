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
 * @file cmt453x_pwr.h
 
 * @version v1.1.0
 *
  */
#ifndef __CMT453X_PWR_H__
#define __CMT453X_PWR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmt453x.h"

/** @addtogroup CMT453X_StdPeriph_Driver
 * @{
 */

/**
 * @}
 */

/** @defgroup SLEEP_mode_entry 
  * @{
  */
#define PWR_IDLEENTRY_WFI         ((uint8_t)0x01)
#define PWR_IDLEENTRY_WFE         ((uint8_t)0x02)

 
/**
  * @}
  */


/** @defgroup SLEEP_mode_entry 
  * @{
  */

#define PWR_SLEEPENTRY_WFI        ((uint8_t)0x01)
#define PWR_SLEEPENTRY_WFE        ((uint8_t)0x02)

/** @defgroup Powerdown_mode_entry 
  * @{
  */

#define PWR_PDENTRY_WFI         ((uint8_t)0x01)
#define PWR_PDENTRY_WFE         ((uint8_t)0x02)


/** @addtogroup PWR_Exported_Functions
 * @{
 */

void PWR_DeInit(void);
void PWR_EnterIDLEMode(uint8_t IDLEONEXIT, uint8_t PWR_IdleEntry);
void PWR_EnterSLEEPMode(uint8_t PWR_SleepEntry);
void PWR_EnterPDMode(uint8_t PWR_PDEntry);

#ifdef __cplusplus
}
#endif

#endif /* __CMT453X_PWR_H__ */
       /**
        * @}
        */

/**
 * @}
 */

/**
 * @}
 */
