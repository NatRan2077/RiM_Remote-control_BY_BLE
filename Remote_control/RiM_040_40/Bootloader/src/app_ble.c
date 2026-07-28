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
 * @file app_ble.c
 * @version v1.0.4
 *
  */
#include <string.h>
#include "cmt453x.h"
#include "gapm_task.h"               // GAP Manager Task API
#include "app_ble.h"
#include "hp_sec.h"
#include "hp_sleep.h"
#include "app_rdtsc.h"
#include "app_user_config.h"
#include "messages.h"
#include "HAL.h"
#include "globals.h"
/** @addtogroup 
 * @{
 */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static bool g_waiting_for_passkey = false;
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void app_ble_connected(void);
void app_ble_disconnected(void);

void app_ble_start_pairing(void)
{
    if (app_env.conidx != GAP_INVALID_CONIDX) {
        HP_LOG_INFO("Starting pairing process\r\n");
        hp_sec_send_bond_start(app_env.conidx);
    } else {
        HP_LOG_ERROR("No active connection for pairing\r\n");
    }
}

/**
 * @brief  user message handler
 * @param  
 * @return 
 * @note   
 */
void app_user_msg_handler(ke_msg_id_t const msgid, void const *p_param)
{
    switch (msgid)
    {
        case APP_UART_TX_EVT:
            break;
            
        case APP_START_PAIRING_EVT:
            app_ble_start_pairing();
            break;
            
        default:
            break;
    }
}



int app_start_pairing_evt_handler(ke_msg_id_t const msgid, void *p_param,
                                  ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    HP_LOG_DEBUG("Starting pairing process after connection\r\n");
    app_ble_start_pairing();
    return (KE_MSG_CONSUMED);
}

int app_user_msg_dflt_handler(ke_msg_id_t const msgid, void *p_param,
                              ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    switch (msgid)
    {
        case APP_UART_TX_EVT:

            break;
            
        case APP_START_PAIRING_EVT:
            app_start_pairing_evt_handler(msgid, p_param, dest_id, src_id);
            break;
            
        default:
            break;
    }
    
    return (KE_MSG_CONSUMED);
}

void app_ble_cancel_pairing(void)
{
    HP_LOG_INFO("Pairing cancelled by user\r\n");
    g_waiting_for_passkey = false;
    // Здесь можно добавить дополнительную логику отмены
}




void app_ble_input_passkey(uint32_t passkey)
{
    // Проверить валидность пин-кода (6 цифр)
    if (passkey > 999999) {
        HP_LOG_ERROR("Invalid passkey: must be 6 digits\r\n");
        return;
    }
    
    if (g_waiting_for_passkey && app_env.conidx != GAP_INVALID_CONIDX) {
        HP_LOG_INFO("Sending passkey %06ld to security manager\r\n", passkey);
        hp_sec_pincode_respond(app_env.conidx, passkey);
        g_waiting_for_passkey = false;
    } else {
        HP_LOG_WARNING("No active connection waiting for passkey\r\n");
    }
}

void app_ble_security_handler(struct sec_msg_t const *p_sec_msg)
{
    switch (p_sec_msg->msg_id)
    {
        case HP_SEC_BOND_STATE:
            HP_LOG_INFO("Bond state: %d bonded devices\r\n", *p_sec_msg->msg.peer_num);
            break;
            
        case HP_SEC_PAIRING_RSP:
            HP_LOG_INFO("Pairing response required\r\n");
            break;
            
        case HP_SEC_PINCODE_DISPLAY:
            HP_LOG_INFO("Display passkey: %06ld\r\n", *p_sec_msg->msg.pincode);
            // Здесь можно отобразить пин-код на дисплее, если есть
            break;
            
        case HP_SEC_PINCODE_ENTER:
            HP_LOG_INFO("Passkey entry requested - waiting for user input\r\n");
            g_waiting_for_passkey = true;
            app_ble_input_passkey(SystemInfo.pinCode);
            break;
            
        case HP_SEC_PAIR_SUCCEED:
			msg.set(MSG_BLE_CONNECT_SUCCESS, NULL, 0);
            HP_LOG_INFO("Pairing successful!\r\n");
            g_waiting_for_passkey = false;
            break;
            
        case HP_SEC_PAIR_FAILED:
            HP_LOG_ERROR("Pairing failed!\r\n");
            g_waiting_for_passkey = false;
            // Перезапустить сканирование после неудачного спаривания
            hp_ble_start_scan();
            break;
            
        case HP_SEC_LTK_FOUND:
            HP_LOG_INFO("LTK found - connection encrypted\r\n");
            break;
            
        case HP_SEC_LTK_MISSING:
            HP_LOG_ERROR("LTK missing - encryption failed\r\n");
            break;
            
        case HP_SEC_ENC_SUCCEED:
            HP_LOG_INFO("Encryption successful\r\n");
            break;
            
        default:
            HP_LOG_DEBUG("Unhandled security event: %d\r\n", p_sec_msg->msg_id);
            break;
    }
}

// Функция для запуска периодического чтения RSSI
void app_ble_start_rssi_monitoring(uint32_t interval_ms){
	if(app_env.conidx != GAP_INVALID_CONIDX) {
		hp_ble_active_rssi(interval_ms);
		
		 // Сохраняем интервал для повторных запросов
        app_env.rssi_intv = interval_ms;
        
        // Отправляем первый запрос на чтение RSSI
//        ke_msg_send(create_rssi_request_cmd(app_env.conidx));
        
        HP_LOG_INFO("RSSI monitoring started with interval %d ms\r\n", interval_ms);
		
	}
}


/**
 * @brief  ble message handler
 * @param  
 * @return 
 * @note   
 */
void app_ble_msg_handler(struct ble_msg_t const *p_ble_msg)
{
    switch (p_ble_msg->msg_id)
    {
        case APP_BLE_OS_READY:
            HP_LOG_INFO("APP_BLE_OS_READY\r\n");
            break;
            
        case APP_BLE_GAP_CONNECTED:
            HP_LOG_INFO("BLE Connected successfully! conidx: %d\r\n", app_env.conidx);
            app_rdtsc_enable_prf(app_env.conidx);
            app_ble_connected();
            
			// ЗАПУСК МОНИТОРИНГА RSSI ПОСЛЕ ПОДКЛЮЧЕНИЯ
            app_ble_start_rssi_monitoring(1000); // Каждую секунду
            // Автоматически запустить процесс спаривания после подключения
            // Можно добавить задержку если нужно
            ke_timer_set(APP_START_PAIRING_EVT, TASK_APP, 1000);
            break;
            
        case APP_BLE_GAP_DISCONNECTED:
            HP_LOG_INFO("BLE Disconnected\r\n");
            app_ble_disconnected();
            break;
            
        case APP_BLE_GAP_PARAMS_REQUEST:
            HP_LOG_INFO("Connection parameters update requested\r\n");
            // Подтвердить обновление параметров
            if (p_ble_msg->cmd.p_param_cfm != NULL) {
                p_ble_msg->cmd.p_param_cfm->accept = 1;
            }
            break;

        default:
            break;
    }
}

/**
 * @brief  scan data handler
 * @param  
 * @return 
 * @note   
 */
void app_ble_scan_data_handler(struct gapm_ext_adv_report_ind const* p_param)
{
    HP_LOG_INFO("Found:%02x%02x%02x%02x%02x%02x, rssi:%d \r\n",
                              p_param->trans_addr.addr.addr[0],
                              p_param->trans_addr.addr.addr[1],
                              p_param->trans_addr.addr.addr[2],
                              p_param->trans_addr.addr.addr[3],
                              p_param->trans_addr.addr.addr[4],
                              p_param->trans_addr.addr.addr[5],
                              p_param->rssi);

}

/**
 * @brief  scan state handler
 * @param  
 * @return 
 * @note   
 */
void app_ble_scan_state_handler(enum scan_state_t state)
{
    switch (state)
    {
        case SCAN_STATE_SCANING:
            HP_LOG_INFO("Scaning.\r\n");
            break;
        case SCAN_STATE_FOUND:
            HP_LOG_INFO("Found target.\r\n");
            break;
        case SCAN_STATE_CONN_TIMEOUT:
            HP_LOG_INFO("Connect timeout.\r\n");
            break;
        default:
            break;
    }
}
    

/**
 * @brief  ble GAP initialization
 * @param  
 * @return 
 * @note   
 */
void app_ble_gap_params_init(void)
{
    struct hp_gap_params_t dev_info = {0};
    uint8_t *p_mac = SystemGetMacAddr();
    //get UUID from trim stored
    if(p_mac != NULL)
    {
        //set the uuid as mac address
        memcpy(dev_info.mac_addr.addr, p_mac , BD_ADDR_LEN); 
    }
    else{
        memcpy(dev_info.mac_addr.addr, "\x01\x02\x03\x04\x05\x06" , BD_ADDR_LEN);
    }
    

    /* init params*/
    dev_info.mac_addr_type = GAPM_STATIC_ADDR;
    dev_info.appearance = 0;
    dev_info.dev_role = GAP_ROLE_ALL;
    
    dev_info.dev_name_len = sizeof(CUSTOM_DEVICE_NAME)-1;
    memcpy(dev_info.dev_name, CUSTOM_DEVICE_NAME, dev_info.dev_name_len); 
   
    dev_info.dev_conn_param.intv_min = MSECS_TO_UNIT(MIN_CONN_INTERVAL,MSECS_UNIT_1_25_MS);
    dev_info.dev_conn_param.intv_max = MSECS_TO_UNIT(MAX_CONN_INTERVAL,MSECS_UNIT_1_25_MS);
    dev_info.dev_conn_param.latency  = SLAVE_LATENCY;
    dev_info.dev_conn_param.time_out = MSECS_TO_UNIT(CONN_SUP_TIMEOUT,MSECS_UNIT_10_MS);
    dev_info.conn_param_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    
    hp_ble_gap_init(&dev_info);
    
}


/**
 * @brief  ble scan initialization
 * @param  
 * @return 
 * @note   
 */
void app_ble_scan_init(void)
{
    struct hp_scan_params_t init = {0};
	
    init.type               = SCAN_PARAM_TYPE;
    init.dup_filt_pol       = SCAN_PARAM_DUP_FILT_POL;
    init.connect_enable     = SCAN_PARAM_CONNECT_EN;
    init.prop_active_enable = SCAN_PARAM_PROP_ACTIVE;
    init.scan_intv          = SCAN_PARAM_INTV;
    init.scan_wd            = SCAN_PARAM_WD;
    init.duration           = SCAN_PARAM_DURATION;
    init.initiating_timeout = 3000;
    
    init.filter_type    = SCAN_FILTER_BY_NAME_MASK;
    init.filter_data    = (uint8_t*)&SystemInfo.numMeterStr;
    init.filter_data_len = 8;
    
    init.ble_scan_data_handler  = app_ble_scan_data_handler;
    init.ble_scan_state_handler = app_ble_scan_state_handler;
    
    hp_ble_scan_init(&init);
}

void app_ble_sec_init(void)
{
    struct hp_sec_init_t sec_init = {0};
    
    sec_init.rand_pin_enable = false;
    sec_init.pin_code = 123456;
    
    sec_init.pairing_feat.auth      = ( SEC_PARAM_BOND | (SEC_PARAM_MITM<<2) | (SEC_PARAM_LESC<<3) | (SEC_PARAM_KEYPRESS<<4) );
    sec_init.pairing_feat.iocap     = SEC_PARAM_IO_CAPABILITIES;
    sec_init.pairing_feat.key_size  = SEC_PARAM_KEY_SIZE;
    sec_init.pairing_feat.oob       = SEC_PARAM_OOB;
    sec_init.pairing_feat.ikey_dist = SEC_PARAM_IKEY;
    sec_init.pairing_feat.rkey_dist = SEC_PARAM_RKEY;
    sec_init.pairing_feat.sec_req   = SEC_PARAM_SEC_MODE_LEVEL;
    
    sec_init.bond_enable            = BOND_STORE_ENABLE;
    sec_init.bond_db_addr           = BOND_DATA_BASE_ADDR;
    sec_init.bond_max_peer          = MAX_BOND_PEER;
    sec_init.bond_sync_delay        = 2000;
    
    sec_init.hp_sec_msg_handler     = app_ble_security_handler;
    
    hp_sec_init(&sec_init);
}

void app_ble_prf_init(void)
{
    // Добавить raw data transmit server(rdts)
    hp_ble_add_prf_func_register(app_rdtsc_add_rdts);
}


/**
 * @brief  ble initialization
 * @param  
 * @return 
 * @note   
 */
void app_ble_init(void)
{

    struct hp_stack_cfg_t app_handler = {0};
    app_handler.ble_msg_handler  = app_ble_msg_handler;
    app_handler.user_msg_handler = app_user_msg_handler;
    app_handler.lsc_cfg          = BLE_LSC_LSI_32000HZ;
    //initialization ble stack
    hp_ble_stack_init(&app_handler);
    
    app_ble_gap_params_init();
    app_ble_sec_init();
    app_ble_scan_init();
    app_ble_prf_init();
    //start scan
    hp_ble_start_scan();
}

/**
 * @brief  ble connected
 * @param  
 * @return 
 * @note   
 */
void app_ble_connected(void)
{
    //enable usart receive
	hp_sleep_lock_acquire();

//    LedOn(LED2_PORT,LED2_PIN);   
}

/**
 * @brief  ble disconnected
 * @param  
 * @return 
 * @note   
 */
void app_ble_disconnected(void)
{
    // Restart scan
    hp_ble_start_scan();
	hp_sleep_lock_release();
    //disable usart receive
//    LedOff(LED2_PORT,LED2_PIN);
}
