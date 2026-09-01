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
 * @file rdtss.h
 
 * @version v1.0.1
 *
  */
#ifndef _RDTSS_H_
#define _RDTSS_H_


#include "rwip_config.h"              // SW configuration

#if (BLE_RDTSS_SERVER)

 /* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "prf_types.h"
#include "prf.h"
#include "attm.h" 

#include "rdts_common.h"

#define RDTSS_IDX_MAX        (1)

/* Public typedef -----------------------------------------------------------*/
/// Parameters for the database creation
struct rdtss_db_cfg
{
    ///max number of casts1 service characteristics
    uint8_t max_nb_att;
    const struct attm_desc_128 *att_tbl;
    const uint8_t *svc_uuid;

};

/// Value element
struct rdtss_val_elmt
{
    /// list element header
    struct co_list_hdr hdr;
    /// value identifier
    uint8_t att_idx;
    /// value length
    uint8_t length;
    /// value data
    uint8_t data[__ARRAY_EMPTY];
};

/// rdtss environment variable
struct rdtss_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Service Start Handle
    uint16_t shdl;
    /// To store the DB max number of attributes
    uint8_t max_nb_att;
    /// On-going operation
    struct ke_msg *operation;

    /// Cursor on connection used to notify peer devices
    uint8_t cursor;
    /// CCC handle index, used during notification/indication busy state
    uint8_t ccc_idx;

    /// List of values set by application
    struct co_list values;
    /// RDTSS task state
    ke_state_t state[RDTSS_IDX_MAX];
    /// RDTSS data base configer
    struct rdtss_db_cfg db_cfg;
};

//extern struct rdtss_db_cfg db_cfg;
/* Private functions ---------------------------------------------------------*/

/** 
 * @brief Disable actions grouped in getting back to IDLE and sending configuration to requester task
 * @param[in] conhdl    Connection Handle 
 */
void rdtss_disable(uint16_t conhdl);

const struct prf_task_cbs *rdtss_prf_itf_get(void);
    
/** 
 * Initialize task handler
 *
 * @param p_task_desc Task descriptor to fill 
 */
void rdtss_task_init(struct ke_task_desc *p_task_desc);

#endif // (BLE_RDTSS_SERVER)

#endif // _RDTSS_H_
