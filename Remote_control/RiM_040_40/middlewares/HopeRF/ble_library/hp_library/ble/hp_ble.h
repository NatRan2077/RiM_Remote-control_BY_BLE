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
 * @file hp_ble.h
 
 * @version v1.0.4
 *
  */


#ifndef _HP_BLE_H_
#define _HP_BLE_H_

/**
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief Application entry point.
 *
 * @{
 **/

/* Includes ------------------------------------------------------------------*/

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_PRESENT)
#include "global_func.h"
/* Define ------------------------------------------------------------*/
#define HP_IWDG_CYCLE_MAX              (0xfff)
#define APP_ADV_DURATION_MAX           (655)

#define SECS_UNIT_1_25_MS              (800)
#define SECS_UNIT_10MS                 (100)
#define SECS_TO_UNIT(sec,uint)         ((sec)*(uint))

#define MSECS_UNIT_1_25_MS              (1250)
#define MSECS_UNIT_10_MS                (10000)
#define MSECS_TO_UNIT(msec,uint)        ((msec*1000)/(uint))

/// Maximal length of the Device Name value
#define APP_DEVICE_NAME_MAX_LEN      (18)
#define APP_MESH_DEMO_TYPE_LEN        (1)
/// Device IRK used for Resolvable Private Address generation (LSB first)
#define SEC_DEFAULT_IRK  "\x50\x19\x21\x90\x1f\x04\xd5\x62\x4f\xa4\x89\xab\x90\xd0\xcf\x23"

/*
 * MACROS
 **/

#define APP_HANDLERS(subtask)    {&subtask##_msg_handler_list[0], ARRAY_LEN(subtask##_msg_handler_list)}


/* Typedef -----------------------------------------------------------*/
typedef void (* ble_hw_check_t)(uint8_t,uint32_t);
typedef void (*hp_ble_add_prf_func_t)(void);

enum current_op_t
{
    CURRENT_OP_NULL, 
    //adv
    CURRENT_OP_CREATE_ADV,
    CURRENT_OP_SET_ADV_DATA,
    CURRENT_OP_SET_RSP_DATA,
    CURRENT_OP_START_ADV,
    CURRENT_OP_STOP_ADV,
    CURRENT_OP_DELETE_ADV,
    //scan
    CURRENT_OP_CREATE_SCAN,
    CURRENT_OP_START_SCAN,
    CURRENT_OP_STOP_SCAN,
    CURRENT_OP_DELETE_SCAN,
    
    //init (master connect)
    CURRENT_OP_CREATE_INIT,
    CURRENT_OP_START_INIT,
    CURRENT_OP_STOP_INIT,
    CURRENT_OP_DELETE_INIT,
};

/// Advertising state machine
enum app_adv_state
{
    /// Advertising activity does not exists
    APP_ADV_STATE_IDLE = 0,
    #if BLE_APP_PRF
    /// Creating advertising activity
    APP_ADV_STATE_CREATING,
    /// Setting advertising data
    APP_ADV_STATE_SETTING_ADV_DATA,
    /// Setting scan response data
    APP_ADV_STATE_SETTING_SCAN_RSP_DATA,

    /// Advertising activity created
    APP_ADV_STATE_CREATED,
    /// Starting advertising activity
    APP_ADV_STATE_STARTING,
    /// Advertising activity started
    APP_ADV_STATE_STARTED,
    /// Stopping advertising activity
    APP_ADV_STATE_STOPPING,
    #endif //(BLE_APP_PRF)
};

enum app_adv_mode
{
    APP_ADV_MODE_IDLE = 0,
    APP_ADV_MODE_ENABLE,
    APP_ADV_MODE_DIRECTED,
    APP_ADV_MODE_FAST,
    APP_ADV_MODE_SLOW,
    APP_ADV_MODE_STOP,
    
};

enum app_ble_msg
{
    APP_BLE_NULL_MSG= 0,
    APP_BLE_OS_READY,
    APP_BLE_GAP_CONNECTED,
    APP_BLE_GAP_DISCONNECTED,
    APP_BLE_GAP_PARAMS_REQUEST,
    APP_BLE_GAP_CMP_EVT,
    APP_BLE_GAP_RSSI_IND,
    APP_BLE_GATTC_MTU_IND,
    APP_BLE_GAP_PARAMS_IND,
    APP_BLE_GATTC_CMP_EVT,
};

struct ble_msg_t
{
    enum app_ble_msg msg_id;
    
    //income message
    union 
    {
        struct gapm_cmp_evt const*              p_gapm_cmp;
        struct gapc_connection_req_ind const*   p_connection_ind;
        struct gapc_disconnect_ind const*       p_disconnect_ind;
        struct gapc_param_update_req_ind const* p_param_req;
        struct gapc_cmp_evt const*              p_gapc_cmp;
        struct gapc_con_rssi_ind const*         p_gapc_rssi;    
        struct gattc_mtu_changed_ind const*     p_gattc_mtu;
        struct gapc_param_updated_ind const*    p_param_updated;
        struct gattc_cmp_evt const*             p_gattc_cmp;
    }msg;
    // output command
    union 
    {
        struct gapm_set_dev_config_cmd*         p_dev_config;
        struct gapc_connection_cfm*             p_connection_cfm;
        struct gapc_param_update_cfm*           p_param_cfm;
    }cmd;
};


typedef enum
{
    BLE_LSC_LSI_32000HZ = 0,
    BLE_LSC_LSI_32768HZ,
    BLE_LSC_LSI_28800HZ,
    BLE_LSC_LSE_32768HZ
}ble_lsc_cfg_t;

/* Public define ------------------------------------------------------------*/

/// Structure containing information about the handlers for an application subtask
struct app_subtask_handlers
{
    /// Pointer to the message handler table
    const struct ke_msg_handler *p_msg_handler_tab;
    /// Number of messages handled
    uint16_t msg_cnt;
};

//APP_CON_IDX_MAX less than BLE_CONNECTION_MAX-1
/// Application connection environment structure
enum app_connection_num
{
    APP_CON_IDX_0 = 0,
    APP_CON_IDX_1,
    APP_CON_IDX_2,
    APP_CON_IDX_MAX,
};

struct app_con_env_tag
{
       /// Connection handle
    uint16_t conhdl;
    /// Connection Index
    uint8_t  conidx;
    /// Last paired peer address type
    uint8_t peer_addr_type;

    /// Last paired peer address
    struct bd_addr peer_addr;
    /// Maximum device MTU size
    uint16_t max_mtu;
    ///DLE, the maximum number of payload octets in TX
    uint16_t tx_pkt_size;    
    /// Role of device in connection (0 = Master / 1 = Slave)
    uint8_t role;        
    
};

/// Application environment structure
struct app_env_tag
{
    /// Connection handle
    uint16_t conhdl;
    /// Connection Index
    uint8_t  conidx;

    /// Advertising activity index
    uint8_t adv_actv_idx;
    /// Current advertising state (@see enum app_adv_state)
    uint8_t adv_state;
    /// Next expected operation completed event
    uint8_t current_op;  
    /// Advertising mode
    enum app_adv_mode adv_mode;

    /// Last initialized profile
    uint8_t next_svc;
    
    /// Last paired peer address type
    uint8_t peer_addr_type;

    /// Last paired peer address
    struct bd_addr peer_addr;

    /// Local device IRK
    uint8_t loc_irk[KEY_LEN];

    struct gap_sec_key peer_irk;

    /// Counter used to generate IRK
    uint8_t rand_cnt;

    /// Maximum device MTU size
    uint16_t max_mtu;
    ///DLE, the maximum number of payload octets in TX
    uint16_t tx_pkt_size;
       
    //master device 
    uint8_t scan_actv_idx;
    uint8_t init_actv_idx;

    // found the target near by
    uint8_t target_found;
    /// target device address type
    uint8_t target_addr_type;
    /// target device address going to connect
    struct bd_addr target_addr;

    
    //ble msg hanlder
    void (*ble_msg_handler)(struct ble_msg_t const *);
    //user msg handler
    void (*user_msg_handler)(ke_msg_id_t const, void const *);

    #if (BLE_APP_HP_IUS)    
    uint8_t manual_conn_param_update;
    uint8_t manual_mtu_update;
    #endif
    struct app_con_env_tag conn_env[APP_CON_IDX_MAX];

    uint32_t rssi_intv;
    ble_lsc_cfg_t lsc_cfg;
    uint16_t iwdg_cycle;
};


/// Application GAP device infomation structure
struct hp_gap_params_t
{
    /// Attribute database configuration (@see enum gapm_att_cfg_flag)
    uint16_t att_cfg;
    /// Device role (@see enum gap_role)
    uint8_t dev_role;
    /// Device MAC address type (enum gapm_own_addr)
    uint8_t mac_addr_type;
    /// Device MAC address
    struct bd_addr mac_addr;
    /// Device appearance
    uint16_t appearance;
    /// Device Name length
    uint8_t dev_name_len;
    /// Device Name
    uint8_t dev_name[APP_DEVICE_NAME_MAX_LEN];
    /// Device connection parameters
    struct gapc_conn_param dev_conn_param;
    /// Delay time of update connection parameters, 0 mean not active
    uint16_t conn_param_update_delay;
};

struct adv_time_t
{
    uint8_t enable;
    /// Advertising duration (in unit of 10ms). 0 means that advertising continues
    /// until the host disable it
    uint16_t duration;
    /// advertising interval (in unit of 625us). Must be greater than 20ms
    uint16_t adv_intv;
};

struct hp_adv_params_t
{    
    void (*ble_adv_msg_handler)(enum app_adv_mode);
    uint8_t adv_data[ADV_DATA_LEN];
    uint8_t adv_data_len;
    uint8_t scan_rsp_data[ADV_DATA_LEN];
    uint8_t scan_rsp_data_len;
    uint8_t attach_appearance;
    uint8_t attach_name;
    uint8_t adv_phy;
    uint8_t  ex_adv_enable;
    uint8_t* ex_adv_p_data;
    uint8_t  ex_adv_data_len;
    
    // beacon mode without presence of AD_TYPE_FLAG in advertising data
    uint8_t  beacon_enable;   
    
    struct adv_time_t directed_adv;
    struct adv_time_t fast_adv;
    struct adv_time_t slow_adv;
};

enum scan_state_t
{
    SCAN_STATE_IDLE = 0,
    SCAN_STATE_SCANING,
    SCAN_STATE_FOUND,
    SCAN_STATE_CONNECTING,
    SCAN_STATE_CONN_TIMEOUT,
};

enum scan_filter_type
{
    SCAN_FILTER_DISABLE = 0,
    SCAN_FILTER_BY_ADDRESS,
    SCAN_FILTER_BY_NAME,
    SCAN_FILTER_BY_UUID128,
    SCAN_FILTER_BY_UUID16,
    SCAN_FILTER_BY_APPEARANCE,
    SCAN_FILTER_BY_NAME_MASK,
    SCAN_FILTER_ALL,
};
struct hp_scan_params_t
{  
    /// Initiating type (@see enum gapm_init_type)
    uint8_t type;
    // if enable scan
    uint8_t scan_enable         :1;   
    // if connect the device if match
    uint8_t connect_enable      :1;    
    /// if use coded phy mode  
    uint8_t phy_coded_enable    :1;    
    /// Properties for the scan active mode, enable will require scan respond data 
    uint8_t prop_active_enable  :1;
    /// Duplicate packet filtering policy
    uint8_t dup_filt_pol        :2;
    /// Scan interval
    uint16_t scan_intv;
    /// Scan window
    uint16_t scan_wd;
    /// Scan duration (in unit of 10ms). 0 means that the controller will scan continuously until
    /// reception of a stop command from the application
    uint16_t duration;
    /// Scan period (in unit of 1.28s). Time interval betweem two consequent starts of a scan duration
    /// by the controller. 0 means that the scan procedure is not periodic
    uint16_t period;
    
    uint32_t initiating_timeout;
    
    void (*ble_scan_state_handler)(enum scan_state_t);
    void (*ble_scan_data_handler)(struct gapm_ext_adv_report_ind const*);

    enum scan_filter_type filter_type;
    const uint8_t         *filter_data;
	
	uint8_t               filter_data_len;  // Длина данных фильтра
    const char            *name_mask;       // Маски имени
    uint8_t               mask_offset;      // Смещение в имени для сравнения
    uint8_t               mask_length;      // Длина маски для сравнения
};


struct hp_stack_cfg_t
{
    //ble msg hanlder
    void (*ble_msg_handler)(struct ble_msg_t const *);
    //user msg handler
    void (*user_msg_handler)(ke_msg_id_t const, void const *);
    //system lsc selected
    ble_lsc_cfg_t lsc_cfg;
    uint16_t iwdg_cycle;
};


struct prf_task_t
{
    uint16_t prf_task_id;
    struct app_subtask_handlers const* prf_task_handler;
    
};

struct hp_ble_prf_evn_t
{
    uint8_t prf_num;
    struct prf_task_t prf_task_list[BLE_NB_PROFILES];
};



struct hp_ble_add_prf_evn_t
{
    uint8_t prf_num;
    hp_ble_add_prf_func_t add_prf_func_list[BLE_NB_PROFILES]; 
};


/// Enumeration of TX/RX PHY values
enum ble_phy_val
{
    /// No preferred PHY
    BLE_PHY_ANY          = GAP_PHY_ANY,
    /// LE 1M PHY (TX or RX)
    BLE_PHY_1MBPS        = GAP_PHY_LE_1MBPS,
    /// LE 2M PHY (TX or RX)
    BLE_PHY_2MBPS        = GAP_PHY_LE_2MBPS,
    /// LE Coded PHY (RX Only)
    BLE_PHY_CODED        = GAP_PHY_LE_CODED,
    /// LE Coded PHY with S=8 data coding (TX Only)
    BLE_PHY_500KBPS      = GAP_PHY_LE_CODED|(GAPC_PHY_OPT_LE_CODED_500K_RATE << 4),
    /// LE Coded PHY with S=2 data coding (TX Only)
    BLE_PHY_125KBPS      = GAP_PHY_LE_CODED|(GAPC_PHY_OPT_LE_CODED_125K_RATE << 4),
};

/* Public variables ---------------------------------------------------------*/
/// Application environment
extern struct app_env_tag           app_env;
extern struct hp_gap_params_t       gap_env;
extern struct hp_adv_params_t       adv_env;
extern struct hp_scan_params_t      scan_env;
extern struct hp_ble_prf_evn_t      ble_prf_evn;
/* Public function prototypes -----------------------------------------------*/
void llhwc_modem_setmode(uint8_t phy);
bool hp_ble_add_svc(void);
void hp_ble_adv_fsm_next(void);

//General function for master and slave
void hp_ble_resolv_addr(struct gap_sec_key *irk, uint8_t nb_key,struct bd_addr* addr);
void hp_ble_list_set_ral(struct gap_ral_dev_info *ral_list,uint8_t ral_cnt);
void hp_ble_list_set_wl(struct gap_bdaddr *wl_addr,uint8_t wl_cnt);
void hp_ble_set_channel_map( uint8_t* newchmap);
void hp_ble_set_slave_latency(uint16_t latency_cfg);
bool hp_ble_update_param(struct gapc_conn_param *conn_param);
void hp_ble_mtu_set(uint16_t mtu);
void hp_ble_phy_set(enum ble_phy_val phy);
void hp_ble_get_peer_info(uint8_t op);
void hp_ble_dle_set(uint16_t tx_octets, uint16_t tx_time);
void hp_ble_active_rssi(uint32_t interval);
void hp_ble_disconnect(void);
//stack and profile init function for master and slave
void hp_ble_lsc_config(ble_lsc_cfg_t lsc_set);
void hp_ble_stack_init(struct hp_stack_cfg_t const* p_handler);
void hp_ble_gap_init(struct hp_gap_params_t const* p_dev_info);
bool hp_ble_add_prf_func_register(hp_ble_add_prf_func_t func);
bool hp_ble_prf_task_register(struct prf_task_t *prf);

//function for slave role 
void hp_ble_adv_init(struct hp_adv_params_t const* p_adv_init);
void hp_ble_adv_start(void);
void hp_ble_adv_stop(void);
bool hp_ble_adv_data_set(uint8_t* p_dat, uint16_t len);
bool hp_ble_scan_rsp_data_set(uint8_t* p_dat, uint16_t len);
bool hp_ble_ex_adv_data_set(uint8_t* p_dat, uint16_t len);

//function for master role 
void hp_ble_scan_init(struct hp_scan_params_t *p_init);
void hp_ble_start_scan(void);
void hp_ble_delete_scan(void);
void hp_ble_stop_scan(void);
void hp_ble_start_init(uint8_t *addr, uint8_t addr_type);
void hp_ble_stop_init(void);
void hp_ble_delete_init(void);
bool hp_ble_scan_data_find(uint8_t types, const uint8_t *p_filter_data, uint8_t *p_data, uint8_t len);
bool hp_ble_scan_data_find_by_mask(const char *mask_data, uint8_t mask_len, uint8_t *p_data, uint8_t len);

void    hp_ble_set_active_connection(uint8_t  conidx);
uint8_t hp_ble_get_active_connection(void);
bool    hp_ble_get_connection_state(uint8_t conidx);
uint8_t hp_ble_get_connection_num(void);
uint8_t hp_ble_get_conidx_by_role(uint8_t role);
uint8_t hp_ble_master_connection_num(void);
uint8_t hp_ble_slave_connection_num(void);


void hp_ble_prod_test_cmd_send(struct gapm_le_test_mode_ctrl_cmd *p_params, bool cw_mode);

void hp_ble_radio_calibration(uint32_t fact_freq,uint32_t std_freq);
void hp_ble_radio_power_set(rf_tx_power_t tx_pwr);

void hp_peripheral_config(void);
void hp_iwdg_reload(void);
void hp_iwdg_config(void);

#define HP_WAKEUP_CONFIG()   do{hp_iwdg_config(); }while(0)

/// @} APP
///

#endif //(BLE_APP_PRESENT)

#endif // _HP_BLE_H_
