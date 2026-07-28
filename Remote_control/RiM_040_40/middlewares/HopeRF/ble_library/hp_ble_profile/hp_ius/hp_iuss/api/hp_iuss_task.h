/**
 ****************************************************************************************
 *
 * @file hp_ius_task.h
 *
 * @brief HP IUS profile task header file
 *
 *
 *
 ****************************************************************************************
 */

#ifndef __HP_IUSS_TASK_H__
#define __HP_IUSS_TASK_H__


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_HP_IUS)


#include "prf_types.h"
#include "prf.h"
#include "attm.h"		











struct hp_ius_db_cfg
{
    uint8_t max_nb_att;
    uint8_t *att_tbl;
    uint8_t *cfg_flag;
    uint16_t features;
};


enum
{
    HP_IUS_CREATE_DB_REQ = TASK_FIRST_MSG(TASK_ID_HP_IUS),  
    HP_IUS_VALUE_REQ_IND,
    HP_IUS_VAL_WRITE_IND,
    HP_IUS_ATT_INFO_REQ,
	  HP_IUS_DISCONNECT,
};

enum hp_ius_state
{
    HP_IUS_IDLE,
    HP_IUS_BUSY,
    HP_IUS_STATE_MAX,
};

struct hp_ius_val_write_ind
{
    uint8_t  conidx;
    uint16_t handle;
    uint16_t length;
    uint8_t  value[__ARRAY_EMPTY];
};
struct hp_ius_value_req_ind
{
    uint8_t  conidx;
    uint16_t att_idx;
};

struct hp_ius_att_info_req
{
    uint8_t  conidx;
    uint16_t att_idx;
};


struct hp_ius_disconnect
{
    uint8_t  conidx;
};


extern ke_task_id_t  hp_ble_ius_task;


/** 
 * @brief Set value of CCC for given attribute and connection index.
 * @param[in] conidx   Connection index.
 * @param[in] att_idx  CCC attribute index.
 * @param[in] cc       Value to store. 
 */
void hp_ius_set_ccc_value(uint8_t conidx, uint8_t att_idx, uint16_t ccc);
/** 
 * @brief Initialize Client Characteristic Configuration fields.
 * @details Function initializes all CCC fields to default value.
 * @param[in] att_db         Id of the message received.
 * @param[in] max_nb_att     Pointer to the parameters of the message. 
 */
void hp_ius_init_ccc_values(const struct attm_desc_128 *att_db, int max_nb_att);



#endif  //BLE_APP_HP_IUS







#endif //__HP_IUSS_TASK_H__
