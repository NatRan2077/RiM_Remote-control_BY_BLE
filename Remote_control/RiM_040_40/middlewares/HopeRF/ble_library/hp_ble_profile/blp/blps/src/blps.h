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
 * @file blps.h
 
 * @version v1.0.2
 *
  */


#ifndef _BLPS_H_
#define _BLPS_H_

/**
 ****************************************************************************************
 * @addtogroup BLPS Blood Pressure Profile Sensor
 * @ingroup BLP
 * @brief Blood Pressure Profile Sensor
 *
 * Blood pressure sensor (BPS) profile provides functionalities to upper layer module
 * application. The device using this profile takes the role of Blood pressure sensor.
 *
 * The interface of this role to the Application is:
 *  - Enable the profile role (from APP)
 *  - Disable the profile role (from APP)
 *  - Notify peer device during Blood Pressure measurement (from APP)
 *  - Indicate measurements performed to peer device (from APP)
 *
 * Profile didn't manages multiple users configuration and storage of offline measurements.
 * This must be handled by application.
 *
 * Blood Pressure Profile Sensor. (BLPS): A BLPS (e.g. PC, phone, etc)
 * is the term used by this profile to describe a device that can perform blood pressure
 * measurement and notify about on-going measurement and indicate final result to a peer
 * BLE device.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "blp_common.h"

#if (BLE_BP_SENSOR)

#include "prf_types.h"
#include "prf.h"
#include "blps_task.h"
#include "attm.h"

/*
 * DEFINES
 ****************************************************************************************
 */
///Maximum number of Blood Pressure task instances
#define BLPS_IDX_MAX     0x01

/// Maximum notification length
#define BLPS_BP_MEAS_MAX_LEN            (19)
#define BLPS_RACP_MAX_LEN               (20)

///BPS Configuration Flag Masks
#define BLPS_MANDATORY_MASK              (0x003F)
#define BLPS_INTM_CUFF_PRESS_MASK        (0x01C0)
#define BLPS_RACP_MASK                   (0X0E00)

/// indication/notification config mask
#define BLPS_NTFIND_MASK                 (0x0E)

/*
 * MACROS
 ****************************************************************************************
 */

/// Possible states of the BLPS task
enum
{
    /// Idle state
    BLPS_IDLE,
    /// Busy state
    BLPS_BUSY,

    /// Number of defined states.
    BLPS_STATE_MAX
};

///Attributes State Machine
enum
{
    BPS_IDX_SVC,

    BPS_IDX_BP_MEAS_CHAR,
    BPS_IDX_BP_MEAS_VAL,
    BPS_IDX_BP_MEAS_IND_CFG,

    BPS_IDX_BP_FEATURE_CHAR,
    BPS_IDX_BP_FEATURE_VAL,

    BPS_IDX_INTM_CUFF_PRESS_CHAR,
    BPS_IDX_INTM_CUFF_PRESS_VAL,
    BPS_IDX_INTM_CUFF_PRESS_NTF_CFG,
    
    BPS_IDX_RACP_CHAR,
    BPS_IDX_RACP_VAL,
    BPS_IDX_RACP_IND_CFG,

    BPS_IDX_NB,
};

///Characteristic Codes
enum
{
    BPS_BP_MEAS_CHAR,
    BPS_INTM_CUFF_MEAS_CHAR,
    BPS_BP_FEATURE_CHAR,
    BPS_RACP_CHAR,
};

/// Database Configuration Bit Field Flags
enum blps_db_config_bf
{
    /// support of Intermediate Cuff Pressure
    BLPS_INTM_CUFF_PRESS_SUP_POS = 0,
    BLPS_INTM_CUFF_PRESS_SUP_BIT = CO_BIT(BLPS_INTM_CUFF_PRESS_SUP_POS),
    
    /// support of Record Access Control Point
    BLPS_RACP_SUP_POS = 1,
    BLPS_RACP_SUP_BIT = CO_BIT(BLPS_RACP_SUP_POS),
};

/// Indication/notification configuration (put in feature flag to optimize memory usage)
enum blps_indntf_config_bf
{
    /// Bit used to know if blood pressure measurement indication is enabled
    BLPS_BP_MEAS_IND_CFG_POS = 1,
    BLPS_BP_MEAS_IND_CFG_BIT = CO_BIT(BLPS_BP_MEAS_IND_CFG_POS),

    /// Bit used to know if cuff pressure measurement notification is enabled
    BLPS_INTM_CUFF_PRESS_NTF_CFG_POS = 2,
    BLPS_INTM_CUFF_PRESS_NTF_CFG_BIT = CO_BIT(BLPS_INTM_CUFF_PRESS_NTF_CFG_POS),
    
    /// Bit used to know if record access cotrol point notification is enabled
    BLPS_RACP_IND_CFG_POS = 3,
    BLPS_RACP_IND_CFG_BIT = CO_BIT(BLPS_RACP_IND_CFG_POS),
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Blood Pressure Profile Sensor environment variable per connection
struct blps_cnx_env
{
    /// Profile Notify/Indication Flags
    uint8_t prfl_ntf_ind_cfg;
};

/// Blood Pressure Profile Sensor environment variable
struct blps_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Blood Pressure Service Start Handle
    uint16_t shdl;
    /// Feature Configuration Flags
    uint16_t features;
    /// Profile configuration flags
    uint8_t prfl_cfg;
    /// Event (notification/indication) config
    uint8_t evt_cfg;
    /// Environment variable pointer for each connections
    uint8_t prfl_ntf_ind_cfg[BLE_CONNECTION_MAX];
    /// State of different task instances
    ke_state_t state[BLPS_IDX_MAX];
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Retrieve BLP service profile interface
 *
 * @return BLP service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* blps_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Pack Blood Pressure measurement value
 *
 * @param[out] p_packed_bp Pointer of the packed data of Blood Pressure Measurement information
 * @param[in] p_meas_val Blood Pressure measurement value
 * @return size of packed value
 ****************************************************************************************
 */
uint8_t blps_pack_meas_value(uint8_t *p_packed_bp, const struct bps_bp_meas* p_meas_val);

/**
 ****************************************************************************************
 * @brief Send an BLPS_CMP_EVT message to a requester.
 *
 * @param[in] p_blps_env        BLP Sensor environment variable
 * @param[in] conidx            Connection index
 * @param[in] operation         Code of the completed operation
 * @param[in] operation_code    Code of the operation
 * @param[in] status            Status of the request
 ****************************************************************************************
 */
void blps_send_cmp_evt(struct blps_env_tag *p_blps_env, uint8_t conidx, uint8_t operation, uint8_t operation_code, uint8_t status);

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * Initialize task handler
 *
 * @param p_task_desc Task descriptor to fill
 ****************************************************************************************
 */
void blps_task_init(struct ke_task_desc *p_task_desc);


#endif /* #if (BLE_BP_SENSOR) */

/// @} BLPS

#endif /* _BLPS_H_ */
