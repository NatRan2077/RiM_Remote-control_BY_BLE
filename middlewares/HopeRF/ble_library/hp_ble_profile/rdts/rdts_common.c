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
 * @file rdts_common.c
 * @version v1.0.0
 *
  */

 /* Includes ------------------------------------------------------------------*/
#include "rwip_config.h"              // SW configuration

#if (BLE_RDTS_ENABLE)
#include "rdts_common.h"
#if (BLE_RDTSS_SERVER)
#include "rdtss.h"
#endif // (BLE_RDTSS_SERVER)

#if (BLE_RDTSS_16BIT_SERVER)
#include "rdtss_16bit.h"
#endif // (BLE_RDTSS_16BIT_SERVER)

#include "gattc_task.h"
#include "att.h"
#include "attm_db.h"
#include "prf_types.h"
#include "prf_utils.h"
/* Private define ------------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

int check_client_char_cfg(bool is_notification, const struct gattc_write_req_ind *param)
{
    uint8_t status = GAP_ERR_NO_ERROR;
    uint16_t ntf_cfg = 0;

    if (param->length != sizeof(uint16_t))
    {
        status =  ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
    }
    else
    {
        ntf_cfg = *((uint16_t*)param->value);

        if (is_notification)
        {
            if ( (ntf_cfg != PRF_CLI_STOP_NTFIND) && (ntf_cfg != PRF_CLI_START_NTF) )
            {
                status =  PRF_ERR_INVALID_PARAM;
            }
        }
        else
        {
            if ( (ntf_cfg != PRF_CLI_STOP_NTFIND) && (ntf_cfg != PRF_CLI_START_IND) )
            {
                status =  PRF_ERR_INVALID_PARAM;
            }
        }
    }

    return status;
}

uint16_t get_value_handle(uint16_t cfg_handle)
{
    uint8_t uuid[ATT_UUID_128_LEN];
    uint8_t uuid_len;
    uint16_t handle = cfg_handle;
    struct attm_svc *srv;

    srv = attmdb_get_service(handle);

    while ((handle >= srv->svc.start_hdl) && (handle <= srv->svc.end_hdl))
    {
        struct attm_elmt elmt;

        // Retrieve UUID
        attmdb_get_attribute(handle, &elmt);
        attmdb_get_uuid(&elmt, &uuid_len, uuid, false, false);

        // check for Characteristic declaration
        if (*(uint16_t *)&uuid[0] == ATT_DECL_CHARACTERISTIC)
            return handle + 1;

        handle--;
    }

    return 0;  //Should not reach this point. something is wrong with the database
}

uint16_t get_cfg_handle(uint16_t value_handle)
{
    uint8_t uuid[ATT_UUID_128_LEN];
    uint8_t uuid_len;
    uint16_t handle = value_handle;
    struct attm_svc *srv;

    srv = attmdb_get_service(handle);

    /* Iterate the database to find the client characteristic configuration.
    */
    while ((handle >= srv->svc.start_hdl) && (handle <= srv->svc.end_hdl))
    {
        struct attm_elmt elmt;

        // Retrieve UUID
        attmdb_get_attribute(handle, &elmt);
        attmdb_get_uuid(&elmt, &uuid_len, uuid, false, false);

        // check for Client Characteristic Configuration
        if (*(uint16_t *)&uuid[0] == ATT_DESC_CLIENT_CHAR_CFG && uuid_len == sizeof(uint16_t))
            return handle;
        else if (*(uint16_t *)&uuid[0] == ATT_DECL_CHARACTERISTIC && uuid_len == sizeof(uint16_t))
            break; // found the next Characteristic declaration without findig a CC CFG,

        handle++;
    }

    return 0;  //Should not reach this point. something is wrong with the database
}

#if (BLE_RDTSS_SERVER)
uint16_t rdtss_get_att_handle(uint8_t att_idx)
{
    struct rdtss_env_tag *rdtss_env = PRF_ENV_GET(RDTSS, rdtss);
    uint16_t handle = ATT_INVALID_HDL;

    if (att_idx < rdtss_env->max_nb_att)
    {
        handle = rdtss_env->shdl + att_idx;
    }

    return handle;
}

uint8_t rdtss_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
    struct rdtss_env_tag *rdtss_env = PRF_ENV_GET(RDTSS, rdtss);
    uint8_t status = PRF_APP_ERROR;

    if ((handle >= rdtss_env->shdl) && (handle < rdtss_env->shdl + rdtss_env->max_nb_att))
    {
        *att_idx = handle - rdtss_env->shdl;
        status = ATT_ERR_NO_ERROR;
    }

    return status;
}
#endif // (BLE_RDTSS_SERVER)


#if (BLE_RDTSS_16BIT_SERVER)
uint16_t rdtss_16bit_get_att_handle(uint8_t att_idx)
{
    struct rdtss_16bit_env_tag *rdtss_16bit_env = PRF_ENV_GET(RDTSS_16BIT, rdtss_16bit);
    uint16_t handle = ATT_INVALID_HDL;

    if (att_idx < rdtss_16bit_env->max_nb_att)
    {
        handle = rdtss_16bit_env->shdl + att_idx;
    }

    return handle;
}

uint8_t rdtss_16bit_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
    struct rdtss_16bit_env_tag *rdtss_16bit_env = PRF_ENV_GET(RDTSS_16BIT, rdtss_16bit);
    uint8_t status = PRF_APP_ERROR;

    if ((handle >= rdtss_16bit_env->shdl) && (handle < rdtss_16bit_env->shdl + rdtss_16bit_env->max_nb_att))
    {
        *att_idx = handle - rdtss_16bit_env->shdl;
        status = ATT_ERR_NO_ERROR;
    }

    return status;
}
#endif // (BLE_RDTSS_16BIT_SERVER)


#endif // (BLE_RDTS_ENABLE)
