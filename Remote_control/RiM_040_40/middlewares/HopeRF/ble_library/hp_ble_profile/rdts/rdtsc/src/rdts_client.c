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
 * @file rdts_client.c
 * @version v1.0.4
 *
  */

/**
 ****************************************************************************************
 * @addtogroup RDTS_CLIENT
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_RDTS_CLIENT)
#include "prf.h"
#include "rdts_client.h"
#include "rdts_client_task.h"
#include "prf_utils.h"
#include "prf_utils_128.h"
#include "ke_mem.h"
/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief     Profile Initialization
 * @param[in] env enviroment 
 * @param[in] start_hdl start handle 
 * @param[in] app_task application task
 * @param[in] sec_lvl security level
 * @param[in] params configuration parameters
 * @return    void
 ****************************************************************************************
 */
uint8_t rdtsc_init(struct prf_task_env *env, uint16_t *start_hdl, uint16_t app_task, uint8_t sec_lvl, struct rdtsc_db_cfg *params)
{
    //-------------------- allocate memory required for the profile  ---------------------

    struct rdtsc_env_tag* rdtsc_env =
            (struct rdtsc_env_tag* ) ke_malloc(sizeof(struct rdtsc_env_tag), KE_MEM_ATT_DB);

    // allocate RDTSC required environment variable
    env->env = (prf_env_t*) rdtsc_env;

    rdtsc_env->prf_env.app_task = app_task | PERM(PRF_MI, DISABLE);
    rdtsc_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);
    rdtsc_env->nb_svc = 0;

    // initialize environment variable
    env->id = TASK_ID_RDTSC;
    rdtsc_task_init(&(env->desc));
    
    ke_state_set(rdtsc_env->prf_env.prf_task, RDTSC_IDLE);
    
    // save db cfg 
    memcpy(&(rdtsc_env->rdtsc_cfg),params,sizeof(struct rdtsc_db_cfg));
   
    return GAP_ERR_NO_ERROR;
}

/**
 ****************************************************************************************
 * @brief     Profile enable confirmation
 * @param[in] env_tag enviroment 
 * @param[in] conidx Connection index
 * @param[in] status
 * @return    void
 ****************************************************************************************
 */
void rdtsc_enable_cfm_send(struct rdtsc_env_tag *rdtsc_env, uint8_t conidx, uint8_t status)
{
   
    struct rdtsc_enable_cfm * cfm = KE_MSG_ALLOC(RDTSC_ENABLE_CFM, 
                                                TASK_APP, 
                                                prf_src_task_get(&(rdtsc_env->prf_env), conidx), 
                                                rdtsc_enable_cfm); 
    
    cfm->status = status;
    
    if (status == ATT_ERR_NO_ERROR)
    {
        HP_LOG_INFO("[RDTSC] discovery OK - writing CCCD now\r\n");
        cfm->rdts  = rdtsc_env->rdts;
        
        prf_register_atthdl2gatt(&(rdtsc_env->prf_env), conidx, &rdtsc_env->rdts.svc); 
        
        // Set value 0x0001 to CFG
        prf_gatt_write_ntf_ind(&rdtsc_env->prf_env, conidx, rdtsc_env->rdts.descs[RDTSC_SRV_TX_DATA_CLI_CFG].desc_hdl, PRF_CLI_START_NTF);
        
        // Reset counter
        rdtsc_env->pending_wr_no_rsp_cmp = 0;
        rdtsc_env->pending_tx_ntf_cmp = false;

        //Place in connected state after discover state   
        ke_state_set(rdtsc_env->prf_env.prf_task, RDTSC_CONNECTED);
    }
    else
    {
        HP_LOG_INFO("[RDTSC] discovery FAILED status=0x%x - CCCD NOT written!\r\n", status);
    }
    
    ke_msg_send(cfm);
}

/**
 ****************************************************************************************
 * @brief Confirm that data has been sent
 * @param[in] rdtsc_env enviroment 
 * @param[in] status
 * @return    void
 ****************************************************************************************
 */
void rdtsc_confirm_data_tx(struct rdtsc_env_tag *rdtsc_env, uint8_t status)
{
    struct rdtsc_data_tx_cfm * cfm = KE_MSG_ALLOC(RDTSC_DATA_TX_CFM,
                                                        TASK_APP,
                                                        prf_src_task_get(&(rdtsc_env->prf_env), rdtsc_env->conidx),
                                                        rdtsc_data_tx_cfm);

    cfm->status = status;

    ke_msg_send(cfm);
}

/**
 ****************************************************************************************
 * @brief Receives data and forwards it to application
 * @param[in] rdtsc_env enviroment 
 * @param[in] message received
 * @param[in] length
 * @return    void
 ****************************************************************************************
 */

void rdtsc_data_receive(struct rdtsc_env_tag *rdtsc_env, void *msg )
{
    // Forward the message
    struct ke_msg * kmsg = ke_param2msg(msg);
    kmsg->dest_id = TASK_APP;
    kmsg->src_id  = prf_src_task_get(&(rdtsc_env->prf_env), rdtsc_env->conidx);
    kmsg->id      = RDTSC_DATA_RX_IND;
    
    struct rdtsc_data_rx_ind *req =  ( struct rdtsc_data_rx_ind *) msg; 
    
    ke_msg_send(req);
}

/**
 ****************************************************************************************
 * @brief  Find next empty characteristic description
 * @param[in] rdtsc_env   enviroment 
 * @param[in] desc_def  service characteristic description information table
 * @return    position of next characteristic description
 ****************************************************************************************
 */
uint8_t rdtsc_get_next_desc_char_code(struct rdtsc_env_tag *rdtsc_env,
                                     const struct prf_char_desc_def *desc_def)
{
    uint8_t i;
    uint8_t next_char_code;

    for (i=0; i<RDTSC_DESC_MAX; i++)
    {
        next_char_code = desc_def[i].char_code;

        if (next_char_code > rdtsc_env->last_char_code)
        {
            //If Char. has been found and number of attributes is upper than 2
            if ((rdtsc_env->rdts.chars[next_char_code].char_hdl != 0)
                    & (rdtsc_env->rdts.chars[next_char_code].char_ehdl_off > 2))
            {
                return next_char_code;
            }
        }
    }

    return RDTSC_CHAR_MAX;
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    env        environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void rdtsc_create(struct prf_task_env *env, uint8_t conidx)
{
     struct rdtsc_env_tag *rdts_env = PRF_ENV_GET(RDTSC, rdtsc);
    
     ke_state_set(env->task, RDTSC_CONNECTED);
}
/**
 ****************************************************************************************
 * @brief Handles Disconnection
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 * @param[in]        reason     Detach reason
 ****************************************************************************************
 */
static void rdtsc_cleanup(struct prf_task_env *env, uint8_t conidx, uint8_t reason)
{
    struct rdtsc_env_tag *rdts_env = PRF_ENV_GET(RDTSC, rdtsc);
    rdts_env->nb_svc = 0;
}
/**
 ****************************************************************************************
 * @brief Destruction of the rdts module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void rdtsc_destroy(struct prf_task_env *env)
{
    struct rdtsc_env_tag *rdtsc_env = PRF_ENV_GET(RDTSC, rdtsc);
    
    // free profile environment variables
    env->env = NULL;
    ke_free(rdtsc_env);
}
/**
 ****************************************************************************************
 * @brief Receives data and forwards it to application
 * @param[in] msg voi pointer to RDTSC_DATA_TX_REQ message
 * @param[in] prf_env rdtsc profile enviroment 
 * @param[in] conidx connection index
 * @param[in] handle RDTSC_SRV_RX_DATA_CHAR handle
 * @param[in] value data pointer to be written
 * @param[in] operation GATTC_WRITE_NO_RESPONSE
 * @return    void
 ****************************************************************************************
 */
void rdtsc_write_data_tx(void *msg, prf_env_t *prf_env, uint8_t conidx, uint16_t handle, uint8_t* value, uint16_t length, uint8_t operation)
{
    if(handle != ATT_INVALID_HANDLE)
    {
        struct ke_msg * kmsg = ke_param2msg(msg);
        
        kmsg->dest_id = KE_BUILD_ID(TASK_GATTC, conidx);
        kmsg->src_id  =  prf_src_task_get(prf_env, conidx);
        kmsg->id = GATTC_WRITE_CMD;
        
        struct gattc_write_cmd *wr_char =  ( struct gattc_write_cmd *) msg; 
            
        // Offset
        wr_char->offset         = 0x0000;
        // cursor always 0
        wr_char->cursor         = 0x0000;
        // Write Type
        wr_char->operation       = operation;
        // Characteristic Value attribute handle
        wr_char->handle         = handle;
        // Value Length
        wr_char->length         = length;
        // Auto Execute
        wr_char->auto_execute   = true;
        // Value
        
        // Send the message
        ke_msg_send(wr_char);
    }
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// RDTS Task interface required by profile manager
const struct prf_task_cbs spc_itf =
{
        (prf_init_fnct) rdtsc_init,
        rdtsc_destroy,
        rdtsc_create,
        rdtsc_cleanup,
};

 /*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Retrieve RDTS profile interface
 *
 * @return RDTS service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* rdtsc_prf_itf_get(void)
{
    return &spc_itf;
}

#endif //BLE_RDTS_CLIENT

/// @} RDTS_CLIENT

// ѕ”ЅЋ»„Ќјя: принудительна€ запись CCCD (subscribe) - как кнопка Notify в nRF Connect
void rdtsc_force_subscribe(uint8_t conidx)
{
    struct rdtsc_env_tag *rdtsc_env = PRF_ENV_GET(RDTSC, rdtsc);
    HP_LOG_INFO("[RDTSC] force_subscribe: env=%p handle=%x\r\n", rdtsc_env, rdtsc_env?rdtsc_env->rdts.descs[RDTSC_SRV_TX_DATA_CLI_CFG].desc_hdl:0);
    if(rdtsc_env != NULL){
        prf_gatt_write_ntf_ind(&rdtsc_env->prf_env, conidx,
                               rdtsc_env->rdts.descs[RDTSC_SRV_TX_DATA_CLI_CFG].desc_hdl,
                               PRF_CLI_START_NTF);
    }
}

