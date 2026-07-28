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
 * @file rdts_client_task.h
 
 * @version v1.0.1
 *
  */

#ifndef RDTS_CLIENT_TASK_H_
#define RDTS_CLIENT_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup RDTSCTASK Raw Data Transfer Service client Task
 * @ingroup RDTSCTASK
 * @brief Raw Data Transfer Service client Task
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "ke_task.h"
#include "rdts_client.h"

#if (BLE_RDTS_CLIENT)
/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the RTDS_CLIENT task
enum
{
    /// IDLE state
    RDTSC_IDLE,
    
    /// Connected state
    RDTSC_CONNECTED,
    
    /// Discovering
    RDTSC_DISCOVERING,
    
    /// Number of defined states.
    RDTSC_STATE_MAX
};

///RDTSC Host API messages
enum
{
    /// RDTSC client role enable request from application.
    RDTSC_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_RDTSC),
    
    /// RDTSC Host role enable confirmation to application.
    RDTSC_ENABLE_CFM,
 
    /// Request to transmit data
    RDTSC_DATA_TX_REQ,
    
    /// Confirm that data has been sent
    RDTSC_DATA_TX_CFM,
    
    /// Send data to app
    RDTSC_DATA_RX_IND,

    /// Indicate flow control state
    RDTSC_TX_FLOW_CTRL_IND,
};

/*
 * API Messages Structures
 ****************************************************************************************
 */

///Parameters of the @ref RDTSC_ENABLE_REQ message
struct rdtsc_enable_req
{
    /// Connection handle
    uint8_t conidx;
    
    /// Connection type
    uint8_t con_type;
    
};

///Parameters of the @ref RDTSC_ENABLE_CFM message
struct rdtsc_enable_cfm
{
   
    /// Status
    uint8_t status;
    
    /// RDTS Device details to keep in APP
    struct rdtsc_rdts_content rdts;
};

///Parameters of the @ref RDTSC_DATA_TX_REQ message
///WARNING, DO NOT ALTER THIS STRUCT, IT SHOULD BE COMPATIBLE WITH gattc_write_cmd struct
struct rdtsc_data_tx_req
{
    uint8_t operation;
    /// Perform automatic execution
    /// (if false, an ATT Prepare Write will be used this shall be use for reliable write)
    bool auto_execute;
    /// operation sequence number
    uint16_t seq_num;
    /// Attribute handle
    uint16_t handle;
    /// Write offset
    uint16_t offset;
    /// Write length
    uint16_t length;
    /// Internal write cursor shall be initialized to 0
    uint16_t cursor;
    /// Value to write
    uint8_t data[__ARRAY_EMPTY];
};
///Parameters of the @ref RDTSC_DATA_TX_CFM message
struct rdtsc_data_tx_cfm
{
    ///Status
    uint8_t status;
};

///Parameters of the @ref RDTSC_DATA_RX_IND message
///WARNING, DO NOT ALTER THIS STRUCT, IT SHOULD BE COMPATIBLE WITH gattc_event_ind struct
struct rdtsc_data_rx_ind
{
    /// Event Type
    uint8_t type;
    /// Data length
    uint16_t length;
    /// Attribute handle
    uint16_t handle;
    /// Event Value
    uint8_t data[__ARRAY_EMPTY];
};

///Parameters of the @ref RDTSC_RX_FLOW_CTRL_REQ message
struct rdtsc_rx_flow_ctrl_req
{
    // flow control state
    uint8_t flow_control_state;
};

///Parameters of the @ref RDTSC_TX_FLOW_CTRL_IND message
struct rdtsc_tx_flow_ctrl_ind
{
    // flow control state
    uint8_t flow_control_state;
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

extern const struct ke_state_handler rdtsc_default_handler;

/*
 * Functions
 ****************************************************************************************
 */

#endif //BLE_RDTS_CLIENT

/// @} RDTSCTASK

#endif // RDTS_CLIENT_TASK_H_
