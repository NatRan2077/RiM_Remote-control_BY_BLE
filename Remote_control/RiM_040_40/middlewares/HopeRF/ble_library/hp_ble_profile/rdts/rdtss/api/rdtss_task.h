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
 * @file rdtss_task.h
 
 * @version v1.0.2
 *
  */
#ifndef __RDTSS_TASK_PRF_H
#define __RDTSS_TASK_PRF_H

 /* Includes ------------------------------------------------------------------*/
#if (BLE_RDTSS_SERVER)

#include <stdint.h>
#include "ke_task.h"
#include "prf_types.h"
#include "compiler.h"        // compiler definition
#include "att.h"
#include "attm.h" 

#include "rwip_task.h"
/* Public typedef -----------------------------------------------------------*/

#define RDTSS_NOTIFY_ALL_CONN  BLE_CONNECTION_MAX

/// Possible states of the rdtss task
enum rdtss_state
{
    /// Idle state
    RDTSS_IDLE,
    /// Busy state
    RDTSS_BUSY,
    /// Number of defined states.
    RDTSS_STATE_MAX,
};

/// Messages for RDTS
enum
{
    /// Add a rdtss instance into the database
    RDTSS_CREATE_DB_REQ = TASK_FIRST_MSG(TASK_ID_RDTSS),
    /// Inform APP of database creation status
    RDTSS_CREATE_DB_CFM,

    /// Start the Custom Service Task - at connection
    RDTSS_ENABLE_REQ,
    /// Set/update characteristic value
    RDTSS_VAL_SET_REQ,
    /// Peer device request to get a non-database value (RI enabled)
    RDTSS_VALUE_REQ_IND,
    /// Response to non-database value request
    RDTSS_VALUE_REQ_RSP,
    /// Set/update characteristic value and trigger a notification
    RDTSS_VAL_NTF_REQ,
    /// Response after receiving a RDTSS_VAL_NTF_REQ message and a notification is triggered
    RDTSS_VAL_NTF_CFM,
    /// Set/update characteristic value and trigger an indication
    RDTSS_VAL_IND_REQ,
    ///Response after receiving a RDTSS_VAL_IND_REQ message and an indication is triggered
    RDTSS_VAL_IND_CFM,
    /// Indicate that the characteristic value has been written
    RDTSS_VAL_WRITE_IND,
    /// Inform the application that the profile service role task has been disabled after a disconnection
    RDTSS_DISABLE_IND,
    /// Profile error report
    RDTSS_ERROR_IND,
    /// Inform the application that there is an attribute info request that shall be processed
    RDTSS_ATT_INFO_REQ,
    /// Inform back that the attribute info request has been processed
    RDTSS_ATT_INFO_RSP,
};





/// Parameters of the @ref RDTSS_CREATE_DB_CFM message
struct rdtss_create_db_cfm
{
    ///Status
    uint8_t status;
};

/// Parameters of the @ref RDTSS_ENABLE_REQ message
struct rdtss_enable_req
{
    /// Connection index
    uint8_t conidx;
    /// security level: b0= nothing, b1=unauthenticated, b2=authenticated, b3=authorized; b1 or b2 and b3 can go together
    uint8_t sec_lvl;
    /// Type of connection
    uint8_t con_type;
};

/// Parameters of the @ref RDTSS_DISABLE_IND message
struct rdtss_disable_ind
{
    /// Connection index
    uint8_t conidx;
};

/// Parameters of the @ref RDTSS_VAL_WRITE_IND message
struct rdtss_val_write_ind
{
    /// Connection index
    uint8_t  conidx;
    /// Handle of the attribute that has to be written
    uint16_t handle;
    /// Data length to be written
    uint16_t length;
    /// Data to be written in attribute database
    uint8_t  value[__ARRAY_EMPTY];
};

/// Parameters of the @ref RDTSS_VAL_NTF_CFM message
struct rdtss_val_ntf_cfm
{
    /// Connection index
//    uint8_t  conidx;
    /// Handle of the attribute that has been updated
    uint16_t handle;
    /// Confirmation status
    uint8_t status;
};

/// Parameters of the @ref RDTSS_VAL_IND_CFM message
struct rdtss_val_ind_cfm
{
    /// Connection index
    uint8_t  conidx;
    /// Handle of the attribute that has been updated
    uint16_t handle;
    /// Confirmation status
    uint8_t status;
};

/// Parameters of the @ref RDTSS_VAL_SET_REQ message
struct rdtss_val_set_req
{
    /// Connection index
    uint8_t  conidx;
    /// Handle of the attribute that has to be written
    uint16_t handle;
    /// Data length to be written
    uint16_t length;
    /// Data to be written in attribute database
    uint8_t  value[__ARRAY_EMPTY];
};

/// Parameters of the @ref RDTSS_VAL_REQ_IND message
struct rdtss_value_req_ind
{
    /// Connection index
    uint8_t  conidx;
    /// Index of the attribute for which value has been requested
    uint16_t att_idx;
};

/// Parameters of the @ref RDTSS_VAL_REQ_RSP message
struct rdtss_value_req_rsp
{
    /// Connection index
    uint8_t  conidx;
    /// Index of the attribute for which value has been requested
    uint16_t att_idx;
    /// Current length of that attribute
    uint16_t length;
    /// ATT error code
    uint8_t  status;
    /// Data value
    uint8_t  value[__ARRAY_EMPTY];
};

/// Parameters of the @ref RDTSS_VAL_NTF_REQ message
struct rdtss_val_ntf_ind_req
{
    /// Connection index
    uint8_t  conidx;
    /// Notificatin/indication
    bool     notification;
    /// Handle of the attribute that has to be written
    uint16_t handle;
    /// Data length to be written
    uint16_t length;
    /// Data to be written in attribute database
    uint8_t  value[__ARRAY_EMPTY];
};

/// Parameters of the @ref RDTSS_VAL_IND_REQ message
struct rdtss_val_ind_req
{
    /// Connection index
    uint8_t  conidx;
    /// Handle of the attribute that has to be written
    uint16_t handle;
    /// Data length to be written
    uint16_t length;
    /// Data to be written in attribute database
    uint8_t  value[__ARRAY_EMPTY];
};

/// Parameters of the @ref RDTSS_ATT_INFO_REQ message
struct rdtss_att_info_req
{
    /// Connection index
    uint8_t  conidx;
    /// Index of the attribute for which info has been requested
    uint16_t att_idx;
};

/// Parameters of the @ref RDTSS_ATT_INFO_RSP message
struct rdtss_att_info_rsp
{
    /// Connection index
    uint8_t  conidx;
    /// Index of the attribute for which info has been requested
    uint16_t att_idx;
    /// Current length of that attribute
    uint16_t length;
    /// ATT error code
    uint8_t  status;
};

/** 
 * @brief Initialize Client Characteristic Configuration fields.
 * @details Function initializes all CCC fields to default value.
 * @param[in] att_db         Id of the message received.
 * @param[in] max_nb_att     Pointer to the parameters of the message. 
 */
void rdtss_init_ccc_values(const struct attm_desc_128 *att_db, int max_nb_att);

/** 
 * @brief Set per connection CCC value for attribute
 * @details Function sets CCC for specified connection.
 * @param[in] conidx         Connection index.
 * @param[in] att_idx        Attribute index.
 * @param[in] ccc            Value of ccc. 
 */
void rdtss_set_ccc_value(uint8_t conidx, uint8_t att_idx, uint16_t ccc);


#endif // BLE_RDTSS_SERVER
#endif // __RDTSS_TASK_PRF_H
