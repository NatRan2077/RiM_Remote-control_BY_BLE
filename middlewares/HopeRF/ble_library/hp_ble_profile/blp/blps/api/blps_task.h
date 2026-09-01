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
 * @file blps_task.h
 
 * @version v1.0.2
 *
  */



#ifndef _BLPS_TASK_H_
#define _BLPS_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup BLPSTASK Task
 * @ingroup BLPS
 * @brief Blood Pressure Profile Task.
 *
 * The BLPSTASK is responsible for handling the messages coming in and out of the
 * @ref BLPS collector block of the BLE Host.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include "rwip_task.h" // Task definitions
#include "blp_common.h"

/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

enum
{
    /// measurement sent by profile
    BLPS_BP_MEAS_SEND,
    /// peer device confirm reception
    BLPS_CENTRAL_IND_CFM,
};

/// Messages for Blood Pressure Profile Sensor
/*@TRACE*/
enum blps_msg_id
{
    /// Start the Blood Pressure Profile Sensor - at connection
    BLPS_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_BLPS),
    /// Start the Blood Pressure Profile Sensor - at connection
    BLPS_ENABLE_RSP,

    /// Send blood pressure measurement value from APP
    BLPS_MEAS_SEND_CMD,

    /// Inform APP of new configuration value
    BLPS_CFG_INDNTF_IND,

    /// Inform APP of new RACP value
    BLPS_RACP_WRITE_IND,
    
    /// Send record access control point response value from APP
    BLPS_RACP_RESP_SEND_CMD,
    
    /// Complete Event Information
    BLPS_CMP_EVT,
};

/// Operation codes
enum blps_op_codes
{
    /// Database Creation Procedure
    BLPS_RESERVED_OP_CODE = 0,

    /// Indicate Measurement Operation Code
    BLPS_MEAS_SEND_CMD_OP_CODE = 1,
    
    /// Indicate Record Access Control Point Code
    BLPS_RACP_RSP_SEND_CMD_OP_CODE = 2,
};

/// Parameters of the @ref BLPS_ENABLE_REQ message
struct blps_enable_req
{
    ///Connection index
    uint8_t conidx;

    /// Blood Pressure indication configuration
    uint16_t bp_meas_ind_en;
    /// Intermediate Cuff Pressure Notification configuration
    uint16_t interm_cp_ntf_en;
    /// Record Access Control Point configuration
    uint16_t racp_ind_en;
};

/// Parameters of the @ref BLPS_ENABLE_RSP message
struct blps_enable_rsp
{
    ///Connection index
    uint8_t conidx;
    ///Status
    uint8_t status;
};

///Parameters of the @ref BLPS_CFG_INDNTF_IND message
struct blps_cfg_indntf_ind
{
    ///Connection index
    uint8_t conidx;
    ///Own code for differentiating between Blood Pressure Measurement, and Intermediate
    /// Cuff Pressure Measurement characteristics
    uint8_t char_code;
    ///Stop/notify/indicate value to configure into the peer characteristic
    uint16_t cfg_val;
};

/////Parameters of the @ref BLPS_MEAS_SEND_CMD message
struct blps_meas_send_cmd
{
    ///Connection index
    uint8_t conidx;
    /// Flag indicating if it is a intermediary cuff pressure measurement (0) or
    /// stable blood pressure measurement (1).
    uint8_t flag_interm_cp;
    ///Blood Pressure measurement
    struct bps_bp_meas meas_val;
};

/////Parameters of the @ref BLPS_RACP_WRITE_IND message
struct blps_racp_write_ind
{
    ///Connection index
    uint8_t conidx;
    /// RACP write value
    struct bps_racp write_val;
    /// RACP write value length
    uint16_t write_val_len;
};

/////Parameters of the @ref BLPS_RACP_RESP_SEND_CMD message
struct blps_racp_resp_send_cmd
{
    ///Connection index
    uint8_t conidx;
    /// RACP response value
    struct bps_racp_rsp write_val;
    /// RACP response value length
    uint16_t write_val_len;
};

///Parameters of the @ref BLPS_CMP_EVT message
struct blps_cmp_evt
{
    /// Operation
    uint8_t operation;
    /// Operation code      see enum blps_op_codes
    uint8_t operation_code;
    /// Status
    uint8_t status;
};

///Parameters of the @ref BLPS_CREATE_DB_REQ message
struct blps_db_cfg
{
    /// Supported features
    uint16_t features;
    /// Profile Configuration
    uint8_t prfl_cfg;
};

/// @} BLPSTASK

#endif /* _BLPS_TASK_H_ */
