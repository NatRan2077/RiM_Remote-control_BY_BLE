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
 * @file blps_common.h
 
 * @version v1.0.2
 *
  */



#ifndef _BLP_COMMON_H_
#define _BLP_COMMON_H_

/**
 ****************************************************************************************
 * @addtogroup BLP Blood Pressure Profile
 * @ingroup PROFILE
 * @brief Blood Pressure Profile
 *
 * The BLP module is the responsible block for implementing the Blood Pressure Profile
 * functionalities in the BLE Host.
 *
 * The Blood Pressure Profile defines the functionality required in a device that allows
 * the user (Collector device) to configure and recover blood pressure measurements from
 * a blood pressure device.
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "prf_types.h"
#include <stdint.h>
#include "prf_utils.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// BLPC codes for the 2 possible client configuration characteristic descriptors determination in BPS
enum blp_ccc_code
{
    ///Blood Pressure Measurement
    BPS_BP_MEAS_CODE = 0x01,
    ///Intermediate Cuff Pressure Measurement
    BPS_INTERM_CP_CODE,
    ///Record Access Control Point
    BPS_RACP_CODE,
};

/// Blood Pressure Measurement Flags bit field values
enum blp_meas_bf
{
    /// Blood Pressure Units Flag
    /// 0 : Blood pressure for Systolic, Diastolic and MAP in units of mmHg
    /// 1 : Blood pressure for Systolic, Diastolic and MAP in units of kPa
    BPS_MEAS_FLAG_BP_UNITS_POS = 0,
    BPS_MEAS_FLAG_BP_UNITS_BIT = CO_BIT(BPS_MEAS_FLAG_BP_UNITS_POS),

    /// Time Stamp Flag
    /// 0 : not present
    /// 1 : present
    BPS_MEAS_FLAG_TIME_STAMP_POS = 1,
    BPS_MEAS_FLAG_TIME_STAMP_BIT = CO_BIT(BPS_MEAS_FLAG_TIME_STAMP_POS),

    /// Pulse Rate Flag
    /// 0 : not present
    /// 1 : present
    BPS_MEAS_PULSE_RATE_POS = 2,
    BPS_MEAS_PULSE_RATE_BIT = CO_BIT(BPS_MEAS_PULSE_RATE_POS),

    /// User ID Flag
    /// 0 : not present
    /// 1 : present
    BPS_MEAS_USER_ID_POS = 3,
    BPS_MEAS_USER_ID_BIT = CO_BIT(BPS_MEAS_USER_ID_POS),

    /// Measurement Status Flag
    /// 0 : not present
    /// 1 : present
    BPS_MEAS_MEAS_STATUS_POS = 4,
    BPS_MEAS_MEAS_STATUS_BIT = CO_BIT(BPS_MEAS_MEAS_STATUS_POS),

    // Bit 5 - 7 RFU
};

/// Blood Pressure Measurement Status Flags field bit values
enum blp_meas_status_bf
{
    /// Body Movement Detection Flag
    /// 0 : No body movement
    /// 1 : Body movement during measurement
    BPS_STATUS_MVMT_DETECT_POS = 0,
    BPS_STATUS_MVMT_DETECT_BIT = CO_BIT(BPS_STATUS_MVMT_DETECT_POS),

    /// Cuff Fit Detection Flag
    /// 0 : Cuff fits properly
    /// 1 : Cuff too loose
    BPS_STATUS_CUFF_FIT_DETECT_POS = 1,
    BPS_STATUS_CUFF_FIT_DETECT_BIT = CO_BIT(BPS_STATUS_CUFF_FIT_DETECT_POS),

    /// Irregular Pulse Detection Flag
    /// 0 : No irregular pulse detected
    /// 1 : Irregular pulse detected
    BPS_STATUS_IRREGULAR_PULSE_DETECT_POS = 2,
    BPS_STATUS_IRREGULAR_PULSE_DETECT_BIT = CO_BIT(BPS_STATUS_IRREGULAR_PULSE_DETECT_POS),

    /// Pulse Rate Range Detection Flags
    /// value 0 : Pulse rate is within the range
    /// value 1 : Pulse rate exceeds upper limit
    /// value 2 : Pulse rate is less than lower limit
    BPS_STATUS_PR_RANGE_DETECT_LSB_POS = 3,
    BPS_STATUS_PR_RANGE_DETECT_LSB_BIT = CO_BIT(BPS_STATUS_PR_RANGE_DETECT_LSB_POS),

    BPS_STATUS_PR_RANGE_DETECT_MSB_POS = 4,
    BPS_STATUS_PR_RANGE_DETECT_MSB_BIT = CO_BIT(BPS_STATUS_PR_RANGE_DETECT_MSB_POS),

    /// Measurement Position Detection Flag
    /// 0 : Proper measurement position
    /// 1 : Improper measurement position
    BPS_STATUS_MEAS_POS_DETECT_POS = 5,
    BPS_STATUS_MEAS_POS_DETECT_BIT = CO_BIT(BPS_STATUS_MEAS_POS_DETECT_POS),

    // Bit 6 - 15 RFU
};

/// Blood Pressure Feature Flags field bit values
enum blp_feat_flags_bf
{
    ///Body Movement Detection Support bit
    BPS_F_BODY_MVMT_DETECT_SUP_POS = 0,
    BPS_F_BODY_MVMT_DETECT_SUP_BIT = CO_BIT(BPS_F_BODY_MVMT_DETECT_SUP_POS),

    /// Cuff Fit Detection Support bit
    BPS_F_CUFF_FIT_DETECT_SUP_POS = 1,
    BPS_F_CUFF_FIT_DETECT_SUP_BIT = CO_BIT(BPS_F_CUFF_FIT_DETECT_SUP_POS),

    /// Irregular Pulse Detection Support bit
    BPS_F_IRREGULAR_PULSE_DETECT_SUP_POS = 2,
    BPS_F_IRREGULAR_PULSE_DETECT_SUP_BIT = CO_BIT(BPS_F_IRREGULAR_PULSE_DETECT_SUP_POS),

    /// Pulse Rate Range Detection Support bit
    BPS_F_PULSE_RATE_RANGE_DETECT_SUP_POS = 3,
    BPS_F_PULSE_RATE_RANGE_DETECT_SUP_BIT = CO_BIT(BPS_F_PULSE_RATE_RANGE_DETECT_SUP_POS),

    /// Measurement Position Detection Support bit
    BPS_F_MEAS_POS_DETECT_SUP_POS = 4,
    BPS_F_MEAS_POS_DETECT_SUP_BIT = CO_BIT(BPS_F_MEAS_POS_DETECT_SUP_POS),

    /// Multiple Bond Support bit
    BPS_F_MULTIPLE_BONDS_SUP_POS = 5,
    BPS_F_MULTIPLE_BONDS_SUP_BIT = CO_BIT(BPS_F_MULTIPLE_BONDS_SUP_POS),

    // Bit 6 - 15 RFU
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Blood Pressure measurement structure
struct bps_bp_meas
{
    /// Flag
    uint8_t flags;
    /// User ID
    uint8_t user_id;
    /// Systolic (mmHg/kPa)
    prf_sfloat systolic;
    /// Diastolic (mmHg/kPa)
    prf_sfloat diastolic;
    /// Mean Arterial Pressure (mmHg/kPa)
    prf_sfloat mean_arterial_pressure;
    /// Pulse Rate
    prf_sfloat pulse_rate;
    /// Measurement Status
    uint16_t meas_status;
    /// Time stamp
    struct prf_date_time time_stamp;
};

/// Record Access Control Point structure
struct bps_racp
{
    /// Opcode
    uint8_t opcode;
    /// Operator
    uint8_t op_operator;
    /// Operand
    uint8_t operand;
    /// Data
    uint8_t data[17];           //BLPS_RACP_MAX_LEN-3
};

/// Record Access Control Point structure
struct bps_racp_rsp
{
    /// Opcode
    uint8_t opcode;
    /// Operator
    uint8_t op_operator;
    /// Operand
    uint8_t operand[18];        //BLPS_RACP_MAX_LEN-2
};

/// @} blp_common

#endif /* _BLP_COMMON_H_ */
