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
 * @file cmt453x_lpuart.h
 
 * @version v1.0.4
 *
  */
#ifndef __CMT453X_LPUART_H__
#define __CMT453X_LPUART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmt453x.h"

/** @addtogroup CMT453X_StdPeriph_Driver
 * @{
 */

/** @addtogroup LPUART
 * @{
 */

/** @addtogroup LPUART_Exported_Types
 * @{
 */

/**
 * @brief  LPUART Init Structure definition
 */

typedef struct
{
    uint32_t BaudRate; /*!< This member configures the LPUART communication baud rate.
                                  The baud rate is computed using the following formula:
                                   - IntegerDivider = ((CLK) / (LPUART_InitStruct->BaudRate)))
                                   - FractionalDivider */

    uint16_t Parity; /*!< Specifies the parity mode.
                                This parameter can be a value of @ref Parity
                                @note When parity is enabled, the computed parity is inserted
                                      at the MSB position of the transmitted data (only support
                                      8 data bits). */ 

    uint16_t Mode; /*!< Specifies wether the Receive or Transmit mode is enabled or disabled.
                              This parameter can be a value of @ref Mode */

    uint16_t RtsThreshold; /* Specifies RTS Threshold.
                                    This parameter can be a value of @ref RtsThreshold */                          

    uint16_t HardwareFlowControl; /*!< Specifies wether the hardware flow control mode is enabled
                                             or disabled.
                                             This parameter can be a value of @ref LPUART_Hardware_Flow_Control */
} LPUART_InitType;

/**
 * @}
 */

/** @addtogroup LPUART_Exported_Constants
 * @{
 */

#define IS_LPUART_ALL_PERIPH(PERIPH) ((PERIPH) == LPUART1)

/** @addtogroup Parity
 * @{
 */

#define LPUART_PE_NO             ((uint16_t)0x0008)
#define LPUART_PE_EVEN           ((uint16_t)0x0000)
#define LPUART_PE_ODD            ((uint16_t)0x0001)
#define IS_LPUART_PARITY(PARITY) (((PARITY) == LPUART_PE_NO) || ((PARITY) == LPUART_PE_EVEN) || ((PARITY) == LPUART_PE_ODD))
/**
 * @}
 */

/** @addtogroup Mode
 * @{
 */

#define LPUART_MODE_RX       ((uint16_t)0x0000)
#define LPUART_MODE_TX       ((uint16_t)0x0002)
#define IS_LPUART_MODE(MODE) (((MODE) == LPUART_MODE_RX) || ((MODE) == LPUART_MODE_TX))
/**
 * @}
 */

/** @addtogroup RtsThreshold
 * @{
 */

#define LPUART_RTSTH_FIFOHF       ((uint16_t)0x0000)
#define LPUART_RTSTH_FIFO3QF      ((uint16_t)0x0100)
#define LPUART_RTSTH_FIFOFU       ((uint16_t)0x0200)
#define IS_LPUART_RTSTHRESHOLD(RTSTHRESHOLD)                                                                                \
    (((RTSTHRESHOLD) == LPUART_RTSTH_FIFOHF) || ((RTSTHRESHOLD) == LPUART_RTSTH_FIFO3QF) || ((RTSTHRESHOLD) == LPUART_RTSTH_FIFOFU))
/**
 * @}
 */

/** @addtogroup Hardware_Flow_Control
 * @{
 */
#define LPUART_HFCTRL_NONE    ((uint16_t)0x0000)
#define LPUART_HFCTRL_CTS     ((uint16_t)0x0400)
#define LPUART_HFCTRL_RTS     ((uint16_t)0x0800)
#define LPUART_HFCTRL_RTS_CTS ((uint16_t)0x0C00)
#define IS_LPUART_HARDWARE_FLOW_CONTROL(CONTROL)                                                                        \
    (((CONTROL) == LPUART_HFCTRL_NONE) || ((CONTROL) == LPUART_HFCTRL_RTS) || ((CONTROL) == LPUART_HFCTRL_CTS)          \
     || ((CONTROL) == LPUART_HFCTRL_RTS_CTS))
/**
 * @}
 */

/** @addtogroup LPUART_Interrupt_definition
 * @{
 */
#define LPUART_INT_PE      ((uint16_t)0x0001)
#define LPUART_INT_TXC     ((uint16_t)0x0102)
#define LPUART_INT_FIFO_OV ((uint16_t)0x0204)
#define LPUART_INT_FIFO_FU ((uint16_t)0x0308)
#define LPUART_INT_FIFO_HF ((uint16_t)0x0410)
#define LPUART_INT_FIFO_NE ((uint16_t)0x0520)
#define LPUART_INT_WUF     ((uint16_t)0x0680)

#define IS_LPUART_CFG_INT(IT)                                                                                           \
    (((IT) == LPUART_INT_PE) || ((IT) == LPUART_INT_TXC) || ((IT) == LPUART_INT_FIFO_OV) || ((IT) == LPUART_INT_FIFO_FU)       \
     || ((IT) == LPUART_INT_FIFO_HF) || ((IT) == LPUART_INT_FIFO_NE) || ((IT) == LPUART_INT_WUF))
#define IS_LPUART_GET_INT(IT)                                                                                           \
    (((IT) == LPUART_INT_PE) || ((IT) == LPUART_INT_TXC) || ((IT) == LPUART_INT_FIFO_OV) || ((IT) == LPUART_INT_FIFO_FU)       \
     || ((IT) == LPUART_INT_FIFO_HF) || ((IT) == LPUART_INT_FIFO_NE) || ((IT) == LPUART_INT_WUF))
#define IS_LPUART_CLR_INT(IT)                                                                                           \
    (((IT) == LPUART_INT_PE) || ((IT) == LPUART_INT_TXC) || ((IT) == LPUART_INT_FIFO_OV) || ((IT) == LPUART_INT_FIFO_FU)       \
     || ((IT) == LPUART_INT_FIFO_HF) || ((IT) == LPUART_INT_FIFO_NE) || ((IT) == LPUART_INT_WUF))   
/**
 * @}
 */

/** @addtogroup LPUART_DMA_Requests
 * @{
 */

#define LPUART_DMAREQ_TX         ((uint16_t)0x0020)
#define LPUART_DMAREQ_RX         ((uint16_t)0x0040)
#define IS_LPUART_DMAREQ(DMAREQ) ((((DMAREQ) & (uint16_t)0xFF9F) == (uint16_t)0x00) && ((DMAREQ) != (uint16_t)0x00))

/**
 * @}
 */

/** @addtogroup LPUART_WakeUp_methods
 * @{
 */

#define LPUART_WUSTP_STARTBIT    ((uint16_t)0x0000)
#define LPUART_WUSTP_RXNE        ((uint16_t)0x1000)
#define LPUART_WUSTP_BYTE        ((uint16_t)0x2000)
#define LPUART_WUSTP_FRAME       ((uint16_t)0x3000)
#define IS_LPUART_WAKEUP(WAKEUP)  \
    (((WAKEUP) == LPUART_WUSTP_STARTBIT) || ((WAKEUP) == LPUART_WUSTP_RXNE) || ((WAKEUP) == LPUART_WUSTP_BYTE) || ((WAKEUP) == LPUART_WUSTP_FRAME))
/**
 * @}
 */

/** @addtogroup LPUART_Sampling_methods
 * @{
 */

#define LPUART_SMPCNT_3B    ((uint16_t)0x0000)
#define LPUART_SMPCNT_1B    ((uint16_t)0x4000)
#define IS_LPUART_SAMPLING(SAMPLING) (((SAMPLING) == LPUART_SMPCNT_1B) || ((SAMPLING) == LPUART_SMPCNT_3B))
/**
 * @}
 */

/** @addtogroup LPUART_Flags
 * @{
 */

#define LPUART_FLAG_PEF      ((uint16_t)0x0001)
#define LPUART_FLAG_TXC      ((uint16_t)0x0002)
#define LPUART_FLAG_FIFO_OV  ((uint16_t)0x0004)
#define LPUART_FLAG_FIFO_FU  ((uint16_t)0x0008)
#define LPUART_FLAG_FIFO_HF  ((uint16_t)0x0010)
#define LPUART_FLAG_FIFO_NE  ((uint16_t)0x0020)
#define LPUART_FLAG_CTS      ((uint16_t)0x0040)
#define LPUART_FLAG_WUF      ((uint16_t)0x0080)
#define LPUART_FLAG_NF       ((uint16_t)0x0100)
#define IS_LPUART_FLAG(FLAG)                                                                                                                                            \
    (((FLAG) == LPUART_FLAG_PEF) || ((FLAG) == LPUART_FLAG_TXC) || ((FLAG) == LPUART_FLAG_FIFO_OV)                        \
     || ((FLAG) == LPUART_FLAG_FIFO_FU) || ((FLAG) == LPUART_FLAG_FIFO_HF) || ((FLAG) == LPUART_FLAG_FIFO_NE)    \
     || ((FLAG) == LPUART_FLAG_CTS) || ((FLAG) == LPUART_FLAG_WUF) || ((FLAG) == LPUART_FLAG_NF))

#define IS_LPUART_CLEAR_FLAG(FLAG) ((((FLAG) & (uint16_t)0xFE40) == 0x00) && ((FLAG) != (uint16_t)0x00))

#define IS_LPUART_BAUDRATE(BAUDRATE) (((BAUDRATE) > 0) && ((BAUDRATE) < 2000001))
#define IS_LPUART_DATA(DATA) ((DATA < 0x0100))

/**
 * @}
 */

/**
 * @}
 */

/** @addtogroup LPUART_Exported_Macros
 * @{
 */

/**
 * @}
 */

/** @addtogroup LPUART_Exported_Functions
 * @{
 */

void LPUART_DeInit(LPUART_Module* LPUARTx);
void LPUART_Init(LPUART_Module* LPUARTx, LPUART_InitType* LPUART_InitStruct);
void LPUART_StructInit(LPUART_InitType* LPUART_InitStruct);
void LPUART_FlushRxFifo(LPUART_Module* LPUARTx);
void LPUART_ConfigInt(LPUART_Module* LPUARTx, uint16_t LPUART_INT, FunctionalState Cmd);
void LPUART_EnableDMA(LPUART_Module* LPUARTx, uint16_t LPUART_DMAReq, FunctionalState Cmd);
void LPUART_ConfigWakeUpMethod(LPUART_Module* LPUARTx, uint16_t LPUART_WakeUpMethod);
void LPUART_EnableWakeUpStop(LPUART_Module* LPUARTx, FunctionalState Cmd);
void LPUART_ConfigSamplingMethod(LPUART_Module* LPUARTx, uint16_t LPUART_SamplingMethod);
void LPUART_EnableLoopBack(LPUART_Module* LPUARTx, FunctionalState Cmd);
void LPUART_SendData(LPUART_Module* LPUARTx, uint8_t Data);
uint8_t LPUART_ReceiveData(LPUART_Module* LPUARTx);
void LPUART_ConfigWakeUpData(LPUART_Module* LPUARTx, uint32_t LPUART_WakeUpData);
FlagStatus LPUART_GetFlagStatus(LPUART_Module* LPUARTx, uint16_t LPUART_FLAG);
void LPUART_ClrFlag(LPUART_Module* LPUARTx, uint16_t LPUART_FLAG);
INTStatus LPUART_GetIntStatus(LPUART_Module* LPUARTx, uint16_t LPUART_INT);
void LPUART_ClrIntPendingBit(LPUART_Module* LPUARTx, uint16_t LPART_INT);

#ifdef __cplusplus
}
#endif

#endif /* __CMT453X_LPUART_H__ */
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
