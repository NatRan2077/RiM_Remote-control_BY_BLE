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
 * @file cmt453x_exti.h
 
 * @version v1.0.0
 *
  */
#ifndef __CMT453X_EXTI_H__
#define __CMT453X_EXTI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmt453x.h"

/** @addtogroup CMT453X_StdPeriph_Driver
 * @{
 */

/** @addtogroup EXTI
 * @{
 */

/** @addtogroup EXTI_Exported_Types
 * @{
 */

/**
 * @brief  EXTI mode enumeration
 */

typedef enum
{
    EXTI_Mode_Interrupt = 0x00,
    EXTI_Mode_Event     = 0x04
} EXTI_ModeType;

#define IS_EXTI_MODE(MODE) (((MODE) == EXTI_Mode_Interrupt) || ((MODE) == EXTI_Mode_Event))

/**
 * @brief  EXTI Trigger enumeration
 */

typedef enum
{
    EXTI_Trigger_Rising         = 0x08,
    EXTI_Trigger_Falling        = 0x0C,
    EXTI_Trigger_Rising_Falling = 0x10
} EXTI_TriggerType;

#define IS_EXTI_TRIGGER(TRIGGER)                                                                                       \
    (((TRIGGER) == EXTI_Trigger_Rising) || ((TRIGGER) == EXTI_Trigger_Falling)                                         \
     || ((TRIGGER) == EXTI_Trigger_Rising_Falling))
/**
 * @brief  EXTI Init Structure definition
 */

typedef struct
{
    uint32_t EXTI_Line; /*!< Specifies the EXTI lines to be enabled or disabled.
                             This parameter can be any combination of @ref EXTI_Lines */

    EXTI_ModeType EXTI_Mode; /*!< Specifies the mode for the EXTI lines.
                                     This parameter can be a value of @ref EXTI_ModeType */

    EXTI_TriggerType EXTI_Trigger; /*!< Specifies the trigger signal active edge for the EXTI lines.
                                           This parameter can be a value of @ref EXTI_ModeType */

    FunctionalState EXTI_LineCmd; /*!< Specifies the new state of the selected EXTI lines.
                                       This parameter can be set either to ENABLE or DISABLE */
} EXTI_InitType;

/**
 * @}
 */

/** @addtogroup EXTI_Exported_Constants
 * @{
 */

/** @addtogroup EXTI_Lines
 * @{
 */

#define EXTI_LINE0  ((uint32_t)0x00001) /*!< External interrupt line 0 Connected to the PA0 PA1 PB0 */
#define EXTI_LINE1  ((uint32_t)0x00002) /*!< External interrupt line 1 Connected to the PA2 PA3 PB1 */
#define EXTI_LINE2  ((uint32_t)0x00004) /*!< External interrupt line 2 Connected to the PA6 PB2 */
#define EXTI_LINE3  ((uint32_t)0x00008) /*!< External interrupt line 3 Connected to the PA5 PB3 */
#define EXTI_LINE4  ((uint32_t)0x00010) /*!< External interrupt line 4 Connected to the PB9 PB4 */
#define EXTI_LINE5  ((uint32_t)0x00020) /*!< External interrupt line 5 Connected to the PB10 PB11 PB5 */
#define EXTI_LINE6  ((uint32_t)0x00040) /*!< External interrupt line 6 Connected to the PB12 PB6 PA4 */
#define EXTI_LINE7  ((uint32_t)0x00080) /*!< External interrupt line 7 Connected to the PB13 PB7 PB8 */
#define EXTI_LINE8  ((uint32_t)0x00100) /*!< External interrupt line 8 Connected to the RTC Alarm event */
#define EXTI_LINE9  ((uint32_t)0x00200) /*!< External interrupt line 9 Connected to the RTC Wakeup event */
#define EXTI_LINE10 ((uint32_t)0x00400) /*!< External interrupt line 10 Connected to the LPUART Wakeup event */
#define EXTI_LINE11 ((uint32_t)0x00800) /*!< External interrupt line 11 Connected to the LPTIM Wakeup event */
#define EXTI_LINE12 ((uint32_t)0x01000) /*!< External interrupt line 12 Connected to the NRST Wakeup event */
#define EXTI_LINE13 ((uint32_t)0x02000) /*!< External interrupt line 13 Connected to the KEYSCAN Wakeup event */

#define IS_EXTI_LINE(LINE) ((((LINE) & (uint32_t)0xFF000000) == 0x00) && ((LINE) != (uint16_t)0x00))
#define IS_GET_EXTI_LINE(LINE)                                                                                         \
    (((LINE) == EXTI_LINE0) || ((LINE) == EXTI_LINE1) || ((LINE) == EXTI_LINE2) || ((LINE) == EXTI_LINE3)              \
     || ((LINE) == EXTI_LINE4) || ((LINE) == EXTI_LINE5) || ((LINE) == EXTI_LINE6) || ((LINE) == EXTI_LINE7)           \
     || ((LINE) == EXTI_LINE8) || ((LINE) == EXTI_LINE9) || ((LINE) == EXTI_LINE10) || ((LINE) == EXTI_LINE11)         \
     || ((LINE) == EXTI_LINE12) || ((LINE) == EXTI_LINE13) )       

/**
 * @}
 */

/** @addtogroup EXTI_TSSEL_Line
 * @{
 */
#define EXTI_TSSEL_LINE_MASK ((uint32_t)0x00000)
#define EXTI_TSSEL_LINE0     ((uint32_t)0x00000) /*!< External interrupt line 0 */
#define EXTI_TSSEL_LINE1     ((uint32_t)0x00001) /*!< External interrupt line 1 */
#define EXTI_TSSEL_LINE2     ((uint32_t)0x00002) /*!< External interrupt line 2 */
#define EXTI_TSSEL_LINE3     ((uint32_t)0x00003) /*!< External interrupt line 3 */
#define EXTI_TSSEL_LINE4     ((uint32_t)0x00004) /*!< External interrupt line 4 */
#define EXTI_TSSEL_LINE5     ((uint32_t)0x00005) /*!< External interrupt line 5 */
#define EXTI_TSSEL_LINE6     ((uint32_t)0x00006) /*!< External interrupt line 6 */
#define EXTI_TSSEL_LINE7     ((uint32_t)0x00007) /*!< External interrupt line 7 */
#define EXTI_TSSEL_LINE8     ((uint32_t)0x00008) /*!< External interrupt line 8 */
#define EXTI_TSSEL_LINE9     ((uint32_t)0x00009) /*!< External interrupt line 9 */
#define EXTI_TSSEL_LINE10    ((uint32_t)0x0000A) /*!< External interrupt line 10 */
#define EXTI_TSSEL_LINE11    ((uint32_t)0x0000B) /*!< External interrupt line 11 */
#define EXTI_TSSEL_LINE12    ((uint32_t)0x0000C) /*!< External interrupt line 12 */
#define EXTI_TSSEL_LINE13    ((uint32_t)0x0000D) /*!< External interrupt line 13 */
#define EXTI_TSSEL_LINE14    ((uint32_t)0x0000E) /*!< External interrupt line 14 */
#define EXTI_TSSEL_LINE15    ((uint32_t)0x0000F) /*!< External interrupt line 15 */

#define IS_EXTI_TSSEL_LINE(LINE)                                                                                       \
    (((LINE) == EXTI_TSSEL_LINE0) || ((LINE) == EXTI_TSSEL_LINE1) || ((LINE) == EXTI_TSSEL_LINE2)                      \
     || ((LINE) == EXTI_TSSEL_LINE3) || ((LINE) == EXTI_TSSEL_LINE4) || ((LINE) == EXTI_TSSEL_LINE5)                   \
     || ((LINE) == EXTI_TSSEL_LINE6) || ((LINE) == EXTI_TSSEL_LINE7) || ((LINE) == EXTI_TSSEL_LINE8)                   \
     || ((LINE) == EXTI_TSSEL_LINE9) || ((LINE) == EXTI_TSSEL_LINE10) || ((LINE) == EXTI_TSSEL_LINE11)                 \
     || ((LINE) == EXTI_TSSEL_LINE12) || ((LINE) == EXTI_TSSEL_LINE13) || ((LINE) == EXTI_TSSEL_LINE14)                \
     || ((LINE) == EXTI_TSSEL_LINE15))
/**
 * @}
 */

/**
 * @}
 */

/** @addtogroup EXTI_Exported_Macros
 * @{
 */

/**
 * @}
 */

/** @addtogroup EXTI_Exported_Functions
 * @{
 */

void EXTI_DeInit(void);
void EXTI_InitPeripheral(EXTI_InitType* EXTI_InitStruct);
void EXTI_InitStruct(EXTI_InitType* EXTI_InitStruct);
void EXTI_TriggerSWInt(uint32_t EXTI_Line);
FlagStatus EXTI_GetStatusFlag(uint32_t EXTI_Line);
void EXTI_ClrStatusFlag(uint32_t EXTI_Line);
INTStatus EXTI_GetITStatus(uint32_t EXTI_Line);
void EXTI_ClrITPendBit(uint32_t EXTI_Line);

#ifdef __cplusplus
}
#endif

#endif /* __CMT453X_EXTI_H__ */
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
