
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
 * @file global_func.h
 
 * @version v1.0.4
 *
  */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "cmt453x.h"
#include "global_var.h"
#include "Typedefine.h"

#include "rwip_config.h"     // SW configuration
#include "compiler.h"
#include "co_version.h"      // version information
#include "co_utils.h"
#include "co_bt.h"           // Common BT Definitions
#include "co_math.h"         // Common Maths Definition
#include "co_bt_defines.h"


#include "arch.h"            // Platform architecture definition
#include "em_map.h"
#include "rf.h"

#include "rwip.h"            // RW definitions
#include "rwip_int.h"        // RW internal definitions
#include "rwip_task.h"       // Task definitions

#include "sch_alarm.h"       // for the half slot target ISR
#include "sch_arb.h"         // for the half us target ISR
#include "sch_prog.h"        // for the fifo/clock ISRs
#include "reg_ipcore.h"
#include "reg_blecore.h"
#include "aes.h"             // AES result function
#include "rwble.h"           // for sleep and wake-up specific functions
#include "lld.h"             // for AES encryption handler

#include "ke.h"              // kernel definition
#include "ke_event.h"        // kernel event
#include "ke_timer.h"        // definitions for timer
#include "ke_mem.h"          // kernel memory manager
#include "ke_task.h"         // Kernel Task


#include "rwble_hl.h"        // BLE HL definitions
#include "l2cc.h"
#include "llm_int.h"
#include "ahi_task.h"
#include "co_hci.h"
#include "hci.h"
#include "attm.h"
#include "atts.h"
#include "prf.h"
#include "gap.h"
#include "gapm.h"
#include "gapm_int.h"
#include "gapm_task.h"      // GAP Manager Task API
#include "gapc.h"
#include "gapc_task.h"      // GAP Controller Task API Definition
#include "gattm_int.h"
#include "gattc.h" 
#include "gatt.h"

#include "ble_stack_common.h"
#include "hp_adv_data_def.h"

#ifndef _GLOBAL_FUNC_H_
#define _GLOBAL_FUNC_H_

/* Public typedef -----------------------------------------------------------*/
typedef void (*IRQ_HANNDLE_FUN) (void);

/* Public define ------------------------------------------------------------*/ 
#define GLOBAL_INT_DISABLE()        \
uint32_t ui32IntStatus = 0;         \
do{                                 \
    ui32IntStatus = __get_PRIMASK();\
    __set_PRIMASK(1);               \
}while(0)

#define GLOBAL_INT_RESTORE()     \
do{                              \
    __set_PRIMASK(ui32IntStatus);\
}while(0)


/* Public constants ---------------------------------------------------------*/
/* Public function prototypes -----------------------------------------------*/

extern uint32_t calib_lsi_clk(void);
extern void EXTI_PA11_Configuration(void);
extern uint32_t ModuleIrqRegister(IRQn_Type irqn,IRQ_HANNDLE_FUN fun);
extern uint32_t ModuleIrqRemoval(IRQn_Type irqn);

extern void rwip_slp_isr(void);
extern void rf_tx_power_set(rf_tx_power_t pwr);
#endif    //_GLOBAL_FUNC_H_
