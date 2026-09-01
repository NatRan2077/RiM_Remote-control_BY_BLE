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
 * @file hp_sec.h
 
 * @version v1.0.4
 *
  */

/** @addtogroup HP_SEC
 * @{
 */

#ifndef HP_SEC_H_
#define HP_SEC_H_

/* Includes ------------------------------------------------------------------*/
#include "rwip_config.h"

/* Enable LE Secure Connections pairing (ECDH P-256). RiM_BY61 requires SC;
 * without this the stack advertises GAPM_PAIRING_LEGACY only and a SEC_CON
 * bond request aborts locally with SMP_ERROR_LOC_AUTH_REQ (0x63). */
#ifndef BLE_APP_SEC_CON
#define BLE_APP_SEC_CON                     1
#endif

#if (BLE_APP_SEC)
#include "global_func.h"
/* Public define ------------------------------------------------------------*/
/* Enable LE Secure Connections pairing (ECDH P-256). RiM_BY61 requires SC;
 * without this the stack advertises GAPM_PAIRING_LEGACY only and a SEC_CON
 * bond request aborts locally with SMP_ERROR_LOC_AUTH_REQ (0x63). */
#ifndef BLE_APP_SEC_CON
#define BLE_APP_SEC_CON                     1
#endif
#ifndef MAX_BOND_PEER
#define MAX_BOND_PEER                       8
#endif
/* Public typedef -----------------------------------------------------------*/


enum sec_msg_id
{
    HP_SEC_NULL_MSG = 0,
    HP_SEC_BOND_STATE,
    HP_SEC_PAIRING_RSP,
    HP_SEC_PINCODE_DISPLAY,
    HP_SEC_PINCODE_ENTER,
    HP_SEC_PAIR_SUCCEED,
    HP_SEC_PAIR_FAILED,
    HP_SEC_LTK_FOUND,
    HP_SEC_LTK_MISSING,
    HP_SEC_ENC_SUCCEED,
    
};

struct sec_msg_t
{
    enum sec_msg_id msg_id;
    //income message
    union 
    {
        uint8_t const*  peer_num;
        uint32_t const* pincode;
        struct gapc_bond_cfm* cfm;
    }msg;
    
};


/// Application Security Bond Data environment structure
struct app_sec_bond_data_env_tag
{
    // LTK
    struct gap_sec_key ltk;
    
    // Random Number
    rand_nb_t rand_nb;
    
    // EDIV
    uint16_t ediv;
    
    // Remote IRK
    struct gapc_irk irk;

    // LTK key size
    uint8_t key_size;

    // Last paired peer address type
    uint8_t peer_addr_type;
    
    // Last paired peer address
    bd_addr_t peer_addr;

    // authentication level
    uint8_t auth;
};

/* bonding structure
 * --------------------------------------------------------------------------------
 * |         ADDR            |                 FIELD                   |   SIZE   |
 * --------------------------|-----------------------------------------|----------|
 * | BOND_DATA_BASE_ADDR+0   | Bond space valid flag                   |    2     |
 * --------------------------|-----------------------------------------|----------|
 * | BOND_DATA_BASE_ADDR+2   | Bonded peer device numbers              |    1     |
 * --------------------------------------------------------------------------------
 * | BOND_DATA_BASE_ADDR+4   | Bond data0(@ app_sec_bond_data_env_tag) |          |
 * --------------------------------------------------------------------------------
 * |                         | Bond data1(@ app_sec_bond_data_env_tag) |          |
 * --------------------------------------------------------------------------------
 * |                         |               ......                    |          |
 * --------------------------------------------------------------------------------
 */  
struct local_device_bond_data_tag
{
    uint16_t valid_flag;
    uint8_t num;
    struct app_sec_bond_data_env_tag single_peer_bond_data[MAX_BOND_PEER];
};



struct hp_sec_init_t
{
    //Pairing Features 
    struct gapc_pairing pairing_feat;
    
    //pin code for pairing
    uint32_t pin_code;
    bool     rand_pin_enable;
    
    //config bond
    uint8_t  bond_enable;
    uint8_t  bond_max_peer;
    uint16_t bond_sync_delay;
    uint32_t bond_db_addr;
    
    //sec msg hanlder
    void (*hp_sec_msg_handler)(struct sec_msg_t const*);    
    
};

struct app_sec_env_tag
{
    // Bond status
    uint8_t  bonded;
    uint16_t store_latency;
    //Pairing Features 
    struct hp_sec_init_t sec_init;
};

/* Public define ------------------------------------------------------------*/ 
/* Public constants ---------------------------------------------------------*/
/* Public variables ---------------------------------------------------------*/
extern const struct app_subtask_handlers app_sec_handlers;
/* Public function prototypes -----------------------------------------------*/

/**
 * @brief  Initialize the Application Security Module
 */
void hp_sec_init(struct hp_sec_init_t const* init);

/**
 * @brief Send a security request to the peer device. This function is used to require the
 * central to start the encryption with a LTK that would have shared during a previous
 * bond procedure.
 *
 * @param[in]   - conidx: Connection Index
 */
void hp_sec_send_security_req(uint8_t conidx);

/**
 * @brief  start bond by central 
 */
void hp_sec_send_bond_start(uint8_t conidx);

/**
 * @brief  start encrypt request by central
 */
void hp_sec_send_encrypt_req(uint8_t conidx);

/**
 * @brief  respond the pincode to remote when get GAPC_TK_EXCH message 
 *         and the tk_type is GAP_TK_KEY_ENTRY.
 */
void hp_sec_pincode_respond(uint8_t conidx, uint32_t pincode);
/**
 * @brief  get the bond status
 */
bool hp_sec_get_bond_status(void);
/**
 * @brief  erase all bond information
 */
void hp_sec_bond_db_erase_all(void);

/**
 * @brief  store bond information event hander
 */
int  hp_sec_bond_store_evt_handler(ke_msg_id_t const msgid, void *p_param, \
                                  ke_task_id_t const dest_id, ke_task_id_t const src_id);

uint8_t hp_sec_get_iocap(void);

void hp_bond_last_bonded_addr(struct gap_bdaddr *p_addr);
void hp_bond_last_bonded_peer_id(struct gap_bdaddr *p_addr);
void hp_bond_last_bonded_ral_info(struct gap_ral_dev_info *ral_list);
void hp_bond_resolv_addr_start(struct bd_addr* addr, uint8_t idx);
void hp_bond_search_addr_irk(struct bd_addr* addr,uint8_t addr_type);
#endif //(BLE_APP_SEC)

#endif // APP_SEC_H_

/// @} HP_SEC
