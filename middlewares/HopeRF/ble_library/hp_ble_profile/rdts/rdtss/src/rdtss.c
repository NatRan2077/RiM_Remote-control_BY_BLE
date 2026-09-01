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
 * @file rdtss.c
 * @version v1.0.4
 *
  */


 /* Includes ------------------------------------------------------------------*/
#include "rwip_config.h"              // SW configuration

#if (BLE_RDTSS_SERVER)
#include "rdtss.h"
#include "rdtss_task.h"
#include "attm_db.h"
#include "gapc.h"
#include "prf.h"
#include "ke_mem.h"


/* Private functions ---------------------------------------------------------*/

/** 
 * @brief Initialization of the RDTSS module.
 * This function performs all the initializations of the Profile module.
 *  - Creation of database (if it's a service)
 *  - Allocation of profile required memory
 *  - Initialization of task descriptor to register application
 *      - Task State array
 *      - Number of tasks
 *      - Default task handler
 *
 * @param[out]    env        Collector or Service allocated environment data.
 * @param[in|out] start_hdl  Service start handle (0 - dynamically allocated), only applies for services.
 * @param[in]     app_task   Application task number.
 * @param[in]     sec_lvl    Security level (AUTH, EKS and MI field of @see enum attm_value_perm_mask)
 * @param[in]     param      Configuration parameters of profile collector or service (32 bits aligned)
 *
 * @return status code to know if profile initialization succeed or not. 
 */
static uint8_t rdtss_init(struct prf_task_env *env, uint16_t *start_hdl, uint16_t app_task, 
                            uint8_t sec_lvl, struct rdtss_db_cfg *params)
{
    //------------------ create the attribute database for the profile -------------------

    // DB Creation Status
    uint8_t status = ATT_ERR_NO_ERROR;

    uint32_t cfg_flag = ((1<<params->max_nb_att)-1);
    
    status = attm_svc_create_db_128(start_hdl, params->svc_uuid , (uint8_t*)&cfg_flag,
            params->max_nb_att, NULL, env->task, params->att_tbl,
            (sec_lvl & PERM_MASK_SVC_AUTH) | (sec_lvl & PERM_MASK_SVC_EKS) | 
             PERM(SVC_SECONDARY, DISABLE) | PERM_VAL(SVC_UUID_LEN,2));    

    //-------------------- allocate memory required for the profile  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {
        struct rdtss_env_tag *rdtss_env =
                (struct rdtss_env_tag *) ke_malloc(sizeof(struct rdtss_env_tag), KE_MEM_ATT_DB);
        
        //load config from init 
        memcpy(&rdtss_env->db_cfg,params,sizeof(struct rdtss_db_cfg));

        // allocate RDTSS required environment variable
        env->env = (prf_env_t *)rdtss_env;
        rdtss_env->shdl = *start_hdl;
        rdtss_env->max_nb_att = rdtss_env->db_cfg.max_nb_att;
        rdtss_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        rdtss_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        env->id                     = TASK_ID_RDTSS;

        rdtss_task_init(&(env->desc));
        
        co_list_init(&(rdtss_env->values));
        rdtss_init_ccc_values(rdtss_env->db_cfg.att_tbl, rdtss_env->db_cfg.max_nb_att);

        // service is ready, go into an Idle state
        ke_state_set(env->task, RDTSS_IDLE);
    }

    return status;
}
/** 
 * @brief Destruction of the RDTSS module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    env        Collector or Service allocated environment data. 
 */
static void rdtss_destroy(struct prf_task_env *env)
{
    struct rdtss_env_tag *rdtss_env = (struct rdtss_env_tag *)env->env;

    // remove all values present in list
    while (!co_list_is_empty(&(rdtss_env->values)))
    {
        struct co_list_hdr *hdr = co_list_pop_front(&(rdtss_env->values));
        ke_free(hdr);
    }

    // free profile environment variables
    env->env = NULL;
    ke_free(rdtss_env);
}

/** 
 * @brief Handles Connection creation
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index 
 */
static void rdtss_create(struct prf_task_env *env, uint8_t conidx)
{
    int att_idx;
    struct rdtss_env_tag *rdtss_env = (struct rdtss_env_tag *)env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // Find all ccc fields and clean them
    for (att_idx = 1; att_idx < rdtss_env->db_cfg.max_nb_att; att_idx++)
    {
        // Find only CCC characteristics
        if( PERM_GET(rdtss_env->db_cfg.att_tbl[att_idx].perm, UUID_LEN) == 0 &&
            (rdtss_env->db_cfg.att_tbl[att_idx].uuid[0] + 
            (rdtss_env->db_cfg.att_tbl[att_idx].uuid[1]<<8) ) == ATT_DESC_CLIENT_CHAR_CFG)
        {
            // Clear CCC value
            rdtss_set_ccc_value(conidx, att_idx, 0);
        }
    }
}

/** 
 * @brief Handles Disconnection
 *
 * @param[in|out]    env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 * @param[in]        reason     Detach reason 
 */
static void rdtss_cleanup(struct prf_task_env *env, uint8_t conidx, uint8_t reason)
{
    int att_idx;
    
    struct rdtss_env_tag *rdtss_env = (struct rdtss_env_tag *)env->env;
    
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // Find all ccc fields and clean them
    for (att_idx = 1; att_idx < rdtss_env->db_cfg.max_nb_att; att_idx++)
    {
        // Find only CCC characteristics
        if( PERM_GET(rdtss_env->db_cfg.att_tbl[att_idx].perm, UUID_LEN) == 0 &&
            (rdtss_env->db_cfg.att_tbl[att_idx].uuid[0] + 
            (rdtss_env->db_cfg.att_tbl[att_idx].uuid[1]<<8) ) == ATT_DESC_CLIENT_CHAR_CFG)
        {
            // Clear CCC value
            rdtss_set_ccc_value(conidx, att_idx, 0);
        }
    }
}

/* Public variables ---------------------------------------------------------*/

/// RDTSS Task interface required by profile manager
const struct prf_task_cbs rdtss_itf =
{
        (prf_init_fnct) rdtss_init,
        rdtss_destroy,
        rdtss_create,
        rdtss_cleanup,
};

/* Private functions ---------------------------------------------------------*/

const struct prf_task_cbs* rdtss_prf_itf_get(void)
{
    return &rdtss_itf;
}

#endif // (BLE_RDTSS_SERVER)
