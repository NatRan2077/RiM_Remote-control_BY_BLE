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
 * @file app_rdtsc.c
 * @version v1.0.0
 *
  */

/** 
 * @addtogroup APP
 * @{ 
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_RDTS_CLIENT)

/* Includes ------------------------------------------------------------------*/
#include "co_utils.h"
#include "app_rdtsc.h"
#include "rdts_client_task.h"
#include "gapm_task.h"
#include "hp_ble.h"
#include "prf.h"
#include "ke_timer.h"
#include "stdio.h"
#include "hp_ble_task.h"
#include "string.h"
#include "prf_utils_128.h"
#include "log.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define RDTS_SERVICE_UUID    {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xE0, 0xFF, 0x00, 0x00}
#define RDTS_SERVER_TX_UUID  {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xE4, 0xFF, 0x00, 0x00}  // NOTIFY FFE4
#define RDTS_SERVER_RX_UUID  {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xE9, 0xFF, 0x00, 0x00}  // WRITE FFE9
//#define RDTS_SERVICE_UUID    {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xF0, 0xAB, 0x00, 0x00}
//#define RDTS_SERVER_TX_UUID  {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xF2, 0xAB, 0x00, 0x00}  // NOTIFY FFE4
//#define RDTS_SERVER_RX_UUID  {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xF1, 0xAB, 0x00, 0x00}  // WRITE FFE9
/* Private constants ---------------------------------------------------------*/
const uint8_t rdtsc_uuid128[]= RDTS_SERVICE_UUID;
/// State machine used to retrieve RDTS characteristics information
const struct prf_char_def_128 rdtsc_char[RDTSC_CHAR_MAX] =
{
    [RDTSC_SRV_TX_DATA_CHAR]    = {RDTS_SERVER_TX_UUID, ATT_MANDATORY, ATT_CHAR_PROP_NTF},
    [RDTSC_SRV_RX_DATA_CHAR]    = {RDTS_SERVER_RX_UUID, ATT_MANDATORY, ATT_CHAR_PROP_WR},
};
/// State machine used to retrieve RDTS service characteristic description information
const struct prf_char_desc_def rdtsc_desc[RDTSC_DESC_MAX] =
{
    ///RDTS Client Config
    [RDTSC_SRV_TX_DATA_CLI_CFG]  = {ATT_DESC_CLIENT_CHAR_CFG, ATT_MANDATORY, RDTSC_SRV_TX_DATA_CHAR},
};

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void app_rdtsc_init(void)
{
    //register application subtask to app task
    struct prf_task_t prf;
    prf.prf_task_id = TASK_ID_RDTSC;
    prf.prf_task_handler = &app_rdtsc_handlers;
    hp_ble_prf_task_register(&prf);
    
    //register get itf function to prf.c
    struct prf_get_func_t get_func;
    get_func.task_id = TASK_ID_RDTSC;
    get_func.prf_itf_get_func = rdtsc_prf_itf_get;
    prf_get_itf_func_register(&get_func);
}

/** 
 * @brief Create raw data transfer server profile database.
 * @return void
 */
void app_rdtsc_add_rdts(void)
{
    HP_LOG_DEBUG("%s\r\n", __func__);
    struct rdtsc_db_cfg *db_cfg;

    struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                             TASK_GAPM,
                                                             TASK_APP,
                                                             gapm_profile_task_add_cmd,
                                                             sizeof(struct rdtsc_db_cfg));

    // Fill message
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = PERM(SVC_AUTH, NO_AUTH);
    req->prf_task_id = TASK_ID_RDTSC;
    req->app_task = TASK_APP;
    req->start_hdl = 0;

    // Set parameters
    db_cfg = (struct rdtsc_db_cfg *) req->param;
    
    // Attribute table. In case the handle offset needs to be saved
    db_cfg->p_char      = &rdtsc_char[0];
    db_cfg->p_desc      = &rdtsc_desc[0];
    db_cfg->p_uuid128   = rdtsc_uuid128;

    // Send the message
    ke_msg_send(req);
    
    app_rdtsc_init();
}


void app_rdtsc_enable_prf(uint8_t conidx)
{
    HP_LOG_DEBUG("%s\r\n", __func__);
    struct rdtsc_enable_req * req = KE_MSG_ALLOC(RDTSC_ENABLE_REQ,
                                                  prf_get_task_from_id(TASK_ID_RDTSC),
                                                  TASK_APP,
                                                  rdtsc_enable_req);
                                                        
    // Fill in the parameter structure
    req->conidx = conidx;
    req->con_type = PRF_CON_DISCOVERY; 
    
    // Send the message
    ke_msg_send(req);
}


void user_send_ble_data(const uint8_t *data, uint16_t length)
{
    HP_LOG_DEBUG("%s\r\n", __func__);
    struct rdtsc_data_tx_req * req = KE_MSG_ALLOC_DYN(RDTSC_DATA_TX_REQ,
                                                      prf_get_task_from_id(TASK_ID_RDTSC),
                                                      TASK_APP,
                                                      rdtsc_data_tx_req,
                                                      length);
    
    req->length = length;
    memcpy(&req->data[0], data, length);
    ke_msg_send(req);
}


/**
 ****************************************************************************************
 * @brief Handles client enable confirmation
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int rdtsc_enable_cfm_handler(ke_msg_id_t const msgid,
                             struct rdtsc_enable_cfm const *param,
                             ke_task_id_t const dest_id,
                             ke_task_id_t const src_id)
{
    HP_LOG_DEBUG("%s\r\n", __func__);
    HP_LOG_INFO("RDTSC profile was enabled\r\n");
    
    hp_ble_get_peer_info(GAPC_GET_PEER_FEATURES);
    
    return (KE_MSG_CONSUMED);
}

/*
 ****************************************************************************************
 * @brief Handles confirmation that data has been sent
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
int rdtsc_data_tx_cfm_handler(ke_msg_id_t const msgid,
                                      struct rdtsc_data_tx_cfm const *param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    HP_LOG_DEBUG("%s\r\n", __func__);
    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles confirmation that data has been sent
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
extern u8 bleCntTx;
extern u8 bleCntRx;
extern u8 bleBuffTx[60];
extern u8 bleBuffRx[160];
#include "messages.h"

int rdtsc_data_rx_ind_handler(ke_msg_id_t const msgid,
                              struct rdtsc_data_rx_ind const *param,
                              ke_task_id_t const dest_id,
                              ke_task_id_t const src_id)
{
    HP_LOG_DEBUG("%s\r\n", __func__);
    HP_LOG_INFO("BLE Data was received\r\n");
    HP_LOG_DEBUG("length = %x\r\n", param->length);
    HP_LOG_DEBUG("handle = %x\r\n", param->handle);
    HP_LOG_DEBUG("type = %x\r\n", param->type);
    HP_LOG_DEBUG("data ");
	_SYSLOG_DATA_("[Data BLE RX]", (u8*)param->data, param->length);
//    for(uint8_t i=0; i<param->length; i++)
//    {
//        HP_LOG_DEBUG(" %x", param->data[i]);
//    }
    HP_LOG_DEBUG("\r\n");
	bleCntRx = param->length;
	memcpy(bleBuffRx, param->data,bleCntRx);
    msg.set(MSG_RADIO_ANSWER_OK, NULL, 0);
    return (KE_MSG_CONSUMED);
}

/// Default State handlers definition
const struct ke_msg_handler app_rdtsc_msg_handler_list[] =
{
    {RDTSC_ENABLE_CFM,                  (ke_msg_func_t)rdtsc_enable_cfm_handler},
    {RDTSC_DATA_TX_CFM,                 (ke_msg_func_t)rdtsc_data_tx_cfm_handler},
    {RDTSC_DATA_RX_IND,                 (ke_msg_func_t)rdtsc_data_rx_ind_handler},
};

const struct app_subtask_handlers app_rdtsc_handlers = APP_HANDLERS(app_rdtsc);


#endif //BLE_RDTS_CLIENT
