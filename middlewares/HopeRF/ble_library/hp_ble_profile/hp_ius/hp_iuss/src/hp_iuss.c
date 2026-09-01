/**
 ****************************************************************************************
 *
 * @file hp_iuss.c
 *
 * @brief Custom Service profile source file.
 *
 *
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"     // SW configuration


#if (BLE_APP_HP_IUS)
#include "hp_iuss.h"
#include "hp_iuss_task.h"
#include "app_hp_ius.h"


#include "prf_utils.h"
#include "prf.h"
#include "attm_db.h"
#include "ke_mem.h"
#include "att.h"
#include "co_utils.h"
#include "attm.h"
#include "ke_task.h"
#include "gapc.h"
#include "gapc_task.h"
#include "gattc_task.h"
#include "attm_db.h"
#include "hp_log.h"



/**
 * @brief Initialization of the HP IUS module.
 * This function performs all the initializations of the Profile module.
 *  - Creation of database (if it's a service)
 *  - Allocation of profile required memory
 *  - Initialization of task descriptor to register application
 *      - Task State array
 *      - Number of tasks
 *      - Default task handler
 *
 * @param[out]    p_env         Collector or Service allocated environment data.
 * @param[in|out] p_start_hdl   Service start handle (0 - dynamically allocated), only applies for services.
 * @param[in]     app_task      Application task number.
 * @param[in]     sec_lvl       Security level (AUTH, EKS and MI field of @see enum attm_value_perm_mask)
 * @param[in]     p_params      Configuration parameters of profile collector or service (32 bits aligned)
 *
 * @return status code to know if profile initialization succeed or not.
 */
static uint8_t hp_ius_init(struct prf_task_env *env, uint16_t *start_hdl, uint16_t app_task, uint8_t sec_lvl, struct hp_ius_db_cfg *params)
{
	uint8_t status = ATT_ERR_NO_ERROR;
	uint8_t uuid_128[16] = SERVICE_HP_IUS;
	uint32_t cfg_flag = ((1<<HP_IUS_IDX_NB)-1);
		
	status = attm_svc_create_db_128(start_hdl, uuid_128, (uint8_t*)&cfg_flag,
            HP_IUS_IDX_NB, NULL, env->task, hp_ius_att_db,
            (sec_lvl & PERM_MASK_SVC_AUTH) | (sec_lvl & PERM_MASK_SVC_EKS) | PERM(SVC_SECONDARY, DISABLE) | PERM_VAL(SVC_UUID_LEN,0x2));	
	
	
	if (status == ATT_ERR_NO_ERROR)
	{
		struct hp_ius_env_tag *hp_ius_env = (struct hp_ius_env_tag *) ke_malloc(sizeof(struct hp_ius_env_tag), KE_MEM_ATT_DB);
		env->env = (prf_env_t *)hp_ius_env;
		hp_ius_env->shdl = *start_hdl;
		hp_ius_env->max_nb_att = HP_IUS_IDX_NB;
		hp_ius_env->prf_env.app_task = app_task | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
		hp_ius_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);	
		env->id = TASK_ID_HP_IUS;	
		hp_ius_task_init(&(env->desc));
		co_list_init(&(hp_ius_env->values));
		hp_ius_init_ccc_values(hp_ius_att_db, HP_IUS_IDX_NB);
		hp_ble_ius_task = env->task;
		ke_state_set(hp_ble_ius_task, HP_IUS_IDLE);
	}
  return status;
}

/**
 * @brief Destruction of the HP IUS module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 */
static void hp_ius_destroy(struct prf_task_env *env)
{
    struct hp_ius_env_tag *hp_ius_env = (struct hp_ius_env_tag *)env->env;
    while (!co_list_is_empty(&(hp_ius_env->values)))
    {
        struct co_list_hdr *hdr = co_list_pop_front(&(hp_ius_env->values));
        ke_free(hdr);
    }
    env->env = NULL;
    ke_free(hp_ius_env);
}
/**
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 * @param[in]        conidx       Connection index
 */
static void hp_ius_create(struct prf_task_env *env, uint8_t conidx)
{
    int att_idx;
    for (att_idx = 1; att_idx < HP_IUS_IDX_NB; att_idx++)
    {
			if( PERM_GET(hp_ius_att_db[att_idx].perm, UUID_LEN) == 0 && (hp_ius_att_db[att_idx].uuid[0] + (hp_ius_att_db[att_idx].uuid[1]<<8) ) == ATT_DESC_CLIENT_CHAR_CFG)
			{
				hp_ius_set_ccc_value(conidx, att_idx, 0);
			}
	}
}
/**
 * @brief Handles Disconnection
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 * @param[in]        reason     Detach reason
 */
static void hp_ius_cleanup(struct prf_task_env *env, uint8_t conidx, uint8_t reason)
{
  int att_idx;
	struct hp_ius_env_tag *hp_ius_env = (struct hp_ius_env_tag *)env->env;
	for (att_idx = 1; att_idx < HP_IUS_IDX_NB; att_idx++)
	{
	if( PERM_GET(hp_ius_att_db[att_idx].perm, UUID_LEN) == 0 && (hp_ius_att_db[att_idx].uuid[0] + (hp_ius_att_db[att_idx].uuid[1]<<8) ) == ATT_DESC_CLIENT_CHAR_CFG
	)
			hp_ius_set_ccc_value(conidx, att_idx, 0);
	}
	ke_state_set(prf_src_task_get(&(hp_ius_env->prf_env), conidx), HP_IUS_IDLE);
}

/// HP IUS Task interface required by profile manager
const struct prf_task_cbs hp_ius_itf =
{
       (prf_init_fnct) hp_ius_init,
        hp_ius_destroy,
        hp_ius_create,
        hp_ius_cleanup,
};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
const struct prf_task_cbs* hp_ius_prf_itf_get(void)
{
    return &hp_ius_itf;
}












#endif  //BLE_APP_HP_IUS





