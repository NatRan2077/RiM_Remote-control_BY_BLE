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
 * @file hp_dfu_ble.c
 * @version v1.0.2
 *
  */

/** @addtogroup 
 * @{
 */

 /* Includes ------------------------------------------------------------------*/
#include "hp_dfu_ble.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ke_timer.h"
#include "hp_ble.h"
#include "hp_ble_task.h"
#include "hp_dfu_boot.h"
#include "hp_error.h"
#include "hp_log.h"
#include "dfu_crc.h"
#include "dfu_delay.h"
/* Private typedef -----------------------------------------------------------*/
static struct{
    uint32_t total_size;
    uint32_t address;
    uint32_t offset;
    uint32_t size;
    uint32_t crc;
}m_ota_image;
/* Private define ------------------------------------------------------------*/
#define OTA_ECC_ECDSA_SHA256_ENABLE                   1

#if OTA_ECC_ECDSA_SHA256_ENABLE
#include "hp_ecc.h"
#endif

#define OTA_CMD_CONN_PARAM_UPDATE                     1
#define OTA_CMD_MTU_UPDATE                            2
#define OTA_CMD_VERSION                               3
#define OTA_CMD_CREATE_OTA_SETTING                    4
#define OTA_CMD_CREATE_OTA_IMAGE                      5
#define OTA_CMD_VALIDATE_OTA_IMAGE                    6
#define OTA_CMD_ACTIVATE_OTA_IMAGE                    7
#define OTA_CMD_JUMP_IMAGE_UPDATE                     8

#define OTA_RC_STATE_NONE                             0
#define OTA_RC_STATE_DFU_SETTING                      1
#define OTA_RC_STATE_DFU_IMAGE                        2
/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t m_rc_state = OTA_RC_STATE_NONE;
static Dfu_setting_t m_dfu_setting;
static uint8_t m_buffer[2048];
static uint32_t rc_mtu_offset = 0;
static uint8_t ota_selection = 0;
static uint32_t m_ota_setting_size = 0;
/* Private function prototypes -----------------------------------------------*/
extern void hp_ble_ius_app_cc_send(uint8_t *p_data, uint16_t length);
/* Private functions ---------------------------------------------------------*/


/**
 * @brief Processing and handling ius service cc characteristic data.
 * @param[in] input data to process.
 * @param[in] input_len data length.
 * @param[out] output response data. 
 * @param[out] output_len response data length.
 * @return none
 */
void hp_dfu_ble_handler_cc(uint8_t const *input, uint8_t input_len, uint8_t *output, uint8_t *output_len)
{

    switch(input[0]){
    
        case OTA_CMD_CONN_PARAM_UPDATE:{
            struct gapc_conn_param conn_param;
            conn_param.intv_min = input[1]<<8 | input[2];
            conn_param.intv_max = input[3]<<8 | input[4];
            conn_param.latency = input[5]<<8 | input[6];
            conn_param.time_out = input[7]<<8 | input[8];    
            app_env.manual_conn_param_update = 1;
            hp_ble_update_param(&conn_param);            
            *output_len = 0;
        }break;
        case OTA_CMD_MTU_UPDATE:{
            app_env.manual_mtu_update = 1;
            hp_ble_mtu_set(input[1]<<8 | input[2]);
            *output_len = 0;
        }break;
    
        case OTA_CMD_VERSION:{
            rc_mtu_offset = 0;
            memset(&m_ota_image,0,sizeof(m_ota_image));
            uint32_t new_app1_size = input[1]<<24 | input[2]<<16 | input[3]<<8 | input[4];
            uint32_t new_app2_size = input[5]<<24 | input[6]<<16 | input[7]<<8 | input[8];
            uint32_t new_image_update_size = input[9]<<24 | input[10]<<16 | input[11]<<8 | input[12];
            uint32_t new_image_update_version = input[13]<<24 | input[14]<<16 | input[15]<<8 | input[16];

            
            #ifdef APPLICATION
            
                ota_selection = 0;
            
                if(CURRENT_APP_START_ADDRESS == HP_APP1_START_ADDRESS){
                    if(hp_bootsetting.app1.size > HP_APP1_DEFAULT_SIZE || new_app1_size > HP_APP1_DEFAULT_SIZE || new_app2_size == 0){
                        if(hp_bootsetting.ImageUpdate.crc == dfu_crc32((uint8_t *)((uint32_t *)hp_bootsetting.ImageUpdate.start_address), hp_bootsetting.ImageUpdate.size)){
                            if(new_image_update_version > hp_bootsetting.ImageUpdate.version){
                                ota_selection = 3;
                                m_ota_image.total_size = new_image_update_size;
                                m_ota_image.address = HP_IMAGE_UPDATE_START_ADDRESS;                                    
                            }else{
                                ota_selection = 4;
                            }
                        }else
                        {
                            ota_selection = 3;
                            m_ota_image.total_size = new_image_update_size;
                            m_ota_image.address = HP_IMAGE_UPDATE_START_ADDRESS;                            
                        }
                    }else{
                        ota_selection = 2;
                        m_ota_image.total_size = new_app2_size;
                        m_ota_image.address = HP_APP2_START_ADDRESS;                        
                    }
                }else if(CURRENT_APP_START_ADDRESS == HP_APP2_START_ADDRESS){
                    if(new_app1_size > HP_APP1_DEFAULT_SIZE){
                        if(hp_bootsetting.ImageUpdate.crc == dfu_crc32((uint8_t *)((uint32_t *)hp_bootsetting.ImageUpdate.start_address), hp_bootsetting.ImageUpdate.size)){
                            if(new_image_update_version > hp_bootsetting.ImageUpdate.version){
                                ota_selection = 3;
                                m_ota_image.total_size = new_image_update_size;
                                m_ota_image.address = HP_IMAGE_UPDATE_START_ADDRESS;                                                                    
                            }else{
                                ota_selection = 4;
                            }
                        }else{
                            ota_selection = 3;
                            m_ota_image.total_size = new_image_update_size;
                            m_ota_image.address = HP_IMAGE_UPDATE_START_ADDRESS;                                
                        }                    
                    }else{
                        ota_selection = 1;
                        m_ota_image.total_size = new_app1_size;
                        m_ota_image.address = HP_APP1_START_ADDRESS;                        
                    }
                }
            #else    
            (void)new_app2_size;
            (void)new_image_update_size;
            (void)new_image_update_version;
            #endif
            
            
            #ifdef IMAGE_UPDATE
                m_ota_image.total_size = new_app1_size;
                m_ota_image.address = HP_APP1_START_ADDRESS;
                ota_selection = 1;
            #endif            
            

            output[0] = OTA_CMD_VERSION;
            output[1] = hp_bootsetting.app1.version>>24;
            output[2] = hp_bootsetting.app1.version>>16;
            output[3] = hp_bootsetting.app1.version>>8;
            output[4] = hp_bootsetting.app1.version;
            output[5] = hp_bootsetting.app2.version>>24;
            output[6] = hp_bootsetting.app2.version>>16;
            output[7] = hp_bootsetting.app2.version>>8;
            output[8] = hp_bootsetting.app2.version;            
            output[9] = hp_bootsetting.ImageUpdate.version>>24;
            output[10] = hp_bootsetting.ImageUpdate.version>>16;
            output[11] = hp_bootsetting.ImageUpdate.version>>8;
            output[12] = hp_bootsetting.ImageUpdate.version;    
            output[13] = ota_selection;
            *output_len = 14;
        }break;        
        
        case OTA_CMD_JUMP_IMAGE_UPDATE:{
            uint8_t error = 0;
            HP_Bootsetting_t m_hp_bootsetting;
            memcpy(&m_hp_bootsetting,&hp_bootsetting,sizeof(HP_Bootsetting_t));
            m_hp_bootsetting.app1.activation = HP_BOOTSETTING_ACTIVATION_NO;
            m_hp_bootsetting.app2.activation = HP_BOOTSETTING_ACTIVATION_NO;
            m_hp_bootsetting.ImageUpdate.activation = HP_BOOTSETTING_ACTIVATION_YES;            
            m_hp_bootsetting.crc = dfu_crc32((uint8_t *)&m_hp_bootsetting.crc + 4, sizeof(HP_Bootsetting_t) - 4);
            Qflash_Erase_Sector(HP_BOOTSETTING_START_ADDRESS);
            Qflash_Write(HP_BOOTSETTING_START_ADDRESS, (uint8_t *)&m_hp_bootsetting, sizeof(HP_Bootsetting_t));
            if(m_hp_bootsetting.crc != dfu_crc32((uint8_t *)((uint32_t *)(HP_BOOTSETTING_START_ADDRESS + 4)), sizeof(HP_Bootsetting_t) - 4))
            {
                error = 2;
            }            
            
            output[0] = OTA_CMD_JUMP_IMAGE_UPDATE;
            output[1] = error;
            *output_len = 2;
            
            if(error == 0)ke_timer_set(APP_DFU_BLE_RESET_TIMER, TASK_APP, 1000);            
        
        }break;
        
        
        case OTA_CMD_CREATE_OTA_SETTING:{
            m_ota_setting_size = input[1]<<24 | input[2]<<16 | input[3]<<8 | input[4];
            m_rc_state = OTA_RC_STATE_DFU_SETTING;
            rc_mtu_offset = 0;
            *output_len = 0;
        }break;        
        
        case OTA_CMD_CREATE_OTA_IMAGE:{
            m_ota_image.offset = input[1]<<24 | input[2]<<16 | input[3]<<8 | input[4];
            m_ota_image.size = input[5]<<24 | input[6]<<16 | input[7]<<8 | input[8];
            m_ota_image.crc = input[9]<<24 | input[10]<<16 | input[11]<<8 | input[12];
            m_rc_state = OTA_RC_STATE_DFU_IMAGE;
            rc_mtu_offset = 0;
            *output_len = 0;
        }break;                
        
        
        case OTA_CMD_VALIDATE_OTA_IMAGE:{
            output[0] = OTA_CMD_VALIDATE_OTA_IMAGE;
            output[1] = 0;
            if(ota_selection == 1){
                if(m_dfu_setting.app1.crc != dfu_crc32((uint8_t *)((uint32_t *)m_dfu_setting.app1.start_address), m_dfu_setting.app1.size))
                {
                    output[1] = 2;
                }            
            }else if(ota_selection == 2){
                if(m_dfu_setting.app2.crc != dfu_crc32((uint8_t *)((uint32_t *)m_dfu_setting.app2.start_address), m_dfu_setting.app2.size))
                {
                    output[1] = 2;
                }                        
            }else if(ota_selection == 3){
                if(m_dfu_setting.image_update.crc != dfu_crc32((uint8_t *)((uint32_t *)m_dfu_setting.image_update.start_address), m_dfu_setting.image_update.size))
                {
                    output[1] = 2;
                }                        
            }
            *output_len = 2;
        }break;            
        
        case OTA_CMD_ACTIVATE_OTA_IMAGE:{
            
            uint8_t error = 0;
            HP_Bootsetting_t m_hp_bootsetting;
            memcpy(&m_hp_bootsetting,&hp_bootsetting,sizeof(HP_Bootsetting_t));
            m_hp_bootsetting.app1.activation = HP_BOOTSETTING_ACTIVATION_NO;
            m_hp_bootsetting.app2.activation = HP_BOOTSETTING_ACTIVATION_NO;
            m_hp_bootsetting.ImageUpdate.activation = HP_BOOTSETTING_ACTIVATION_NO;
            m_hp_bootsetting.master_boot_force_update = HP_BOOTSETTING_MASTER_BOOT_FORCE_UPDATE_NO;        
                
            if(ota_selection == 1){
                m_hp_bootsetting.app1.start_address = HP_APP1_START_ADDRESS;
                m_hp_bootsetting.app1.size = m_dfu_setting.app1.size;
                m_hp_bootsetting.app1.version = m_dfu_setting.app1.version;
                m_hp_bootsetting.app1.crc = m_dfu_setting.app1.crc;
                m_hp_bootsetting.app1.activation = HP_BOOTSETTING_ACTIVATION_YES;                            
            }else if(ota_selection == 2){
                m_hp_bootsetting.app2.start_address = HP_APP2_START_ADDRESS;
                m_hp_bootsetting.app2.size = m_dfu_setting.app2.size;
                m_hp_bootsetting.app2.version = m_dfu_setting.app2.version;
                m_hp_bootsetting.app2.crc = m_dfu_setting.app2.crc;
                m_hp_bootsetting.app2.activation = HP_BOOTSETTING_ACTIVATION_YES;                                                
            }else if(ota_selection == 3){
                m_hp_bootsetting.ImageUpdate.start_address = HP_IMAGE_UPDATE_START_ADDRESS;
                m_hp_bootsetting.ImageUpdate.size = m_dfu_setting.image_update.size;
                m_hp_bootsetting.ImageUpdate.version = m_dfu_setting.image_update.version;
                m_hp_bootsetting.ImageUpdate.crc = m_dfu_setting.image_update.crc;
                m_hp_bootsetting.ImageUpdate.activation = HP_BOOTSETTING_ACTIVATION_YES;                                            
            }            
            m_hp_bootsetting.crc = dfu_crc32((uint8_t *)&m_hp_bootsetting.crc + 4, sizeof(HP_Bootsetting_t) - 4);
            Qflash_Erase_Sector(HP_BOOTSETTING_START_ADDRESS);
            Qflash_Write(HP_BOOTSETTING_START_ADDRESS, (uint8_t *)&m_hp_bootsetting, sizeof(HP_Bootsetting_t));
            if(m_hp_bootsetting.crc != dfu_crc32((uint8_t *)((uint32_t *)(HP_BOOTSETTING_START_ADDRESS + 4)), sizeof(HP_Bootsetting_t) - 4))
            {
                error = 2;
            }            
            
            output[0] = OTA_CMD_ACTIVATE_OTA_IMAGE;
            output[1] = error;
            *output_len = 2;
            
            
            if(error == 0)ke_timer_set(APP_DFU_BLE_RESET_TIMER, TASK_APP, 1000);        
            
        }break;                    
        
    }
    
    


}

/**
 * @brief Reset System.
 * @param[in] msgid.
 * @param[in] p_param.
 * @param[in] dest_id.
 * @param[in] src_id.
 * @return msg
 */
int app_dfu_ble_reset_handler(ke_msg_id_t const msgid, void const *p_param, ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    NVIC_SystemReset();
    return KE_MSG_CONSUMED;
}
/**
 * @brief Connection update result response.
 * @param[in] status connection update succeed or fail.
 * @return none
 */
void hp_dfu_ble_handler_conn_param_update(uint8_t status)
{
    uint8_t response[2] = {OTA_CMD_CONN_PARAM_UPDATE};
  response[1] = status;
    hp_ble_ius_app_cc_send(response,sizeof(response));        
}
/**
 * @brief mtu update result response.
 * @param[in] status mtu update succeed or fail.
 * @return none
 */
void hp_dfu_ble_handler_mtu_update(uint8_t status)
{
    uint8_t response[2] = {OTA_CMD_MTU_UPDATE};
  response[1] = status;
    hp_ble_ius_app_cc_send(response,sizeof(response));        
}





/**
 * @brief Processing and handling ius service rc characteristic data.
 * @param[in] input data to process.
 * @param[in] input_len data length.
 * @return none
 */
void hp_dfu_ble_handler_rc(uint8_t const *input, uint32_t input_len)
{
    switch(m_rc_state){
    
        case OTA_RC_STATE_DFU_SETTING:{
            memcpy(m_buffer + rc_mtu_offset, input , input_len);
            rc_mtu_offset += input_len;            
            if(rc_mtu_offset >= m_ota_setting_size){
                rc_mtu_offset = 0;
                m_rc_state = OTA_RC_STATE_NONE;
                memcpy(&m_dfu_setting, m_buffer, sizeof(Dfu_setting_t));
                uint32_t crc = dfu_crc32((uint8_t *)&m_dfu_setting.crc + 4, sizeof(Dfu_setting_t) - 4);
                if(crc == m_dfu_setting.crc){
                    uint8_t error = 0;
                    
                    #if OTA_ECC_ECDSA_SHA256_ENABLE
                        uint8_t raw_data[sizeof(Dfu_setting_bank_t)*3];
                        memcpy(raw_data,&m_dfu_setting.app1,sizeof(Dfu_setting_bank_t));
                        memcpy(raw_data+sizeof(Dfu_setting_bank_t),&m_dfu_setting.app2,sizeof(Dfu_setting_bank_t));
                        memcpy(raw_data+sizeof(Dfu_setting_bank_t)*2,&m_dfu_setting.image_update,sizeof(Dfu_setting_bank_t));
                        uint8_t hash_digest[32];
                        if(ERROR_SUCCESS == hp_lib_ecc_hash_sha256(raw_data, sizeof(Dfu_setting_bank_t)*3, hash_digest)){
                            if(ERROR_SUCCESS != hp_lib_ecc_ecdsa_verify(hp_bootsetting.public_key, hash_digest, 32, m_dfu_setting.signature)){
                                error = 3;
                            }
                        }else{
                            error = 3;
                        }
                    #endif
                    
                    
                    uint8_t response[2] = {OTA_CMD_CREATE_OTA_SETTING};
                    response[1] = error;
                    hp_ble_ius_app_cc_send(response,sizeof(response));                
                }else{
                    uint8_t response[2] = {OTA_CMD_CREATE_OTA_SETTING};
                    response[1] = 2;
                    hp_ble_ius_app_cc_send(response,sizeof(response));            
                }            
            
            }
            
        }break;
    
        
        
        case OTA_RC_STATE_DFU_IMAGE:{
            memcpy(m_buffer + rc_mtu_offset, input , input_len);
            rc_mtu_offset += input_len;
            if(rc_mtu_offset >= m_ota_image.size){
                m_rc_state = OTA_RC_STATE_NONE;
                rc_mtu_offset = 0;
                uint32_t crc = dfu_crc32(m_buffer, m_ota_image.size);
                uint8_t error = 0;
                if(crc == m_ota_image.crc)
                {
                    if((m_ota_image.address + m_ota_image.offset) % FLASH_SECTOR_SIZE == 0){
                        #ifdef APPLICATION
                        HP_LOG_INFO("bsp_flash_erase_sector --> 0x%08X\r\n",m_ota_image.address + m_ota_image.offset);
                        #endif                            
                        Qflash_Erase_Sector(m_ota_image.address + m_ota_image.offset);
                    }
                    #ifdef APPLICATION
                    HP_LOG_INFO("bsp_flash_write --> 0x%08X\r\n",m_ota_image.address + m_ota_image.offset);
                    #endif                        
                    Qflash_Write(m_ota_image.address + m_ota_image.offset, m_buffer, m_ota_image.size);
                    if(m_ota_image.crc != dfu_crc32((uint8_t *)((uint32_t *)(m_ota_image.address + m_ota_image.offset)), m_ota_image.size))
                    {
                        error = 3;
                    }
                    uint8_t response[2] = {OTA_CMD_CREATE_OTA_IMAGE};
                    response[1] = error;
                    hp_ble_ius_app_cc_send(response,sizeof(response));                        
                
                }
                else
                {
                
                    uint8_t response[2] = {OTA_CMD_CREATE_OTA_IMAGE};
                    response[1] = 2;
                    hp_ble_ius_app_cc_send(response,sizeof(response));                
                }
            }
            
        }break;
    }
    
}




