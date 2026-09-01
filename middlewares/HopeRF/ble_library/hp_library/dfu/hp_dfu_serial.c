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
 * @file hp_dfu_serial.c
 * @version v1.0.1
 *
  */

/** @addtogroup 
 * @{
 */

 /* Includes ------------------------------------------------------------------*/
#include "hp_dfu_serial.h"
#include "hp_scheduler.h"
#include "hp_dfu_boot.h"
#include "dfu_usart.h"
#include "cmt453x.h"
#include "hp_error.h"
#include "dfu_delay.h"
#include "dfu_crc.h"
/* Private typedef -----------------------------------------------------------*/
static struct
{
    uint8_t buffer[4096];
    uint32_t offset;
}m_pkt;

typedef struct 
{
    uint32_t crc;
    uint32_t app_start_address;
    uint32_t app_size;
    uint32_t app_crc;
    uint32_t app_version;
    uint32_t reserve[10];
}_init_pkt;
static _init_pkt m_init_pkt;
typedef struct 
{
    uint32_t offset;
    uint32_t size;
    uint32_t crc;
}_pkt_header;

#define OTP_BUF_MAX   240 //256-15 // head + cmd + ack + crc + address + size  = 15byte

/* Private define ------------------------------------------------------------*/
#define  DFU_SERIAL_HEADER                      0xAA
#define  DFU_SERIAL_CMD_Ping                    0x01
#define  DFU_SERIAL_CMD_InitPkt                 0x02
#define  DFU_SERIAL_CMD_Pkt_header              0x03
#define  DFU_SERIAL_CMD_Pkt                     0x04
#define  DFU_SERIAL_CMD_PostValidate            0x05
#define  DFU_SERIAL_CMD_ActivateReset           0x06
#define  DFU_SERIAL_CMD_JumpToMasterBoot        0x07

#define  DFU_SERIAL_CMD_OtpRead                 0x08
#define  DFU_SERIAL_CMD_OtpWrite                0x09
#define  DFU_SERIAL_CMD_OtpErase                0x0A
#define  DFU_SERIAL_CMD_OtpLock                 0x0B

#define SCHED_EVT_RX_DATA            1
/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static _pkt_header m_pkt_header;
static HP_Bootsetting_t m_hp_bootsetting;
/* Private function prototypes -----------------------------------------------*/
static void sched_evt(void * p_event_data, uint16_t event_size);
static uint32_t serial_send_data(uint8_t *p_data, uint32_t length);
static uint8_t m_buffer[256];
static void dfu_serial_cmd_ping(void);
static void dfu_serial_cmd_init_pkt(void);
static void dfu_serial_cmd_pkt_header(void);
static void dfu_serial_cmd_pkt(void);
static void dfu_serial_cmd_postvalidate(void);
static void dfu_serial_cmd_activate_reset(void);
static void dfu_serial_cmd_jump_to_master_boot(void);
static void dfu_serial_cmd_otp_read(void);
static void dfu_serial_cmd_otp_write(void);
static void dfu_serial_cmd_otp_erase(void);
static void dfu_serial_cmd_otp_lock(void);
/* Private functions ---------------------------------------------------------*/

/**
 * @brief Initialize serial dfu.
 * @param[in] none.
 * @return none
 */
void hp_dfu_serial_init(void)
{
    Qflash_Init();
    if(hp_bootsetting.crc == dfu_crc32((uint8_t *)&hp_bootsetting.crc + 4, sizeof(HP_Bootsetting_t) - 4))
    {
        if(hp_bootsetting.master_boot_force_update == HP_BOOTSETTING_MASTER_BOOT_FORCE_UPDATE_YES)
        {
            HP_Bootsetting_t hp_bootsetting_tmp;
            do{
                memcpy(&hp_bootsetting_tmp,&hp_bootsetting,sizeof(HP_Bootsetting_t));
                hp_bootsetting_tmp.master_boot_force_update = HP_BOOTSETTING_MASTER_BOOT_FORCE_UPDATE_NO;
                hp_bootsetting_tmp.crc = dfu_crc32((uint8_t *)&hp_bootsetting_tmp.crc + 4, sizeof(HP_Bootsetting_t) - 4);
                Qflash_Erase_Sector(HP_BOOTSETTING_START_ADDRESS);
                Qflash_Write(HP_BOOTSETTING_START_ADDRESS, (uint8_t *)&hp_bootsetting_tmp, sizeof(HP_Bootsetting_t));            
            }while(hp_bootsetting_tmp.crc != dfu_crc32((uint8_t *)((uint32_t *)(HP_BOOTSETTING_START_ADDRESS + 4)), sizeof(HP_Bootsetting_t) - 4));
        }    
    }
    
    
    dfu_usart1_interrupt_config();
    dfu_usart1_enable();
}



/**
 * @brief Process data received from serial port.
 * @param[in] p_event_data event type.
 * @param[in] event_size event size.
 * @return none
 */
static void sched_evt(void * p_event_data, uint16_t event_size)
{
    switch(*(uint8_t *)p_event_data)
    {
        case SCHED_EVT_RX_DATA:{
            if(m_buffer[0] == DFU_SERIAL_HEADER)
            {
                switch(m_buffer[1]){
                
                    case DFU_SERIAL_CMD_Ping:{
                        dfu_serial_cmd_ping();
                    }break;
                    case DFU_SERIAL_CMD_InitPkt:{
                        dfu_serial_cmd_init_pkt();
                    }break;
                    case DFU_SERIAL_CMD_Pkt_header:{
                        dfu_serial_cmd_pkt_header();
                    }break;
                    case DFU_SERIAL_CMD_Pkt:{
                        dfu_serial_cmd_pkt();
                    }break;
                    case DFU_SERIAL_CMD_PostValidate:{
                        dfu_serial_cmd_postvalidate();
                    }break;
                    case DFU_SERIAL_CMD_ActivateReset:{
                        dfu_serial_cmd_activate_reset();
                    }break;                    
                    case DFU_SERIAL_CMD_JumpToMasterBoot:{
                        dfu_serial_cmd_jump_to_master_boot();
                    }break;
                    case DFU_SERIAL_CMD_OtpRead:{
                        dfu_serial_cmd_otp_read();
                    }break;
                    case DFU_SERIAL_CMD_OtpWrite:{
                        dfu_serial_cmd_otp_write();
                    }break;
                    case DFU_SERIAL_CMD_OtpErase:{
                        dfu_serial_cmd_otp_erase();
                    }break;
                    case DFU_SERIAL_CMD_OtpLock:{
                        dfu_serial_cmd_otp_lock();
                    }break;
                    
                }
            }
        }break;
    }
}
/**
 * @brief Ping the board.
 * @param[in] none.
 * @return none
 */
static void dfu_serial_cmd_ping(void)
{
    uint8_t cmd[] = {DFU_SERIAL_HEADER,DFU_SERIAL_CMD_Ping};
    serial_send_data(cmd, sizeof(cmd));
}
/**
 * @brief Process init packet.
 * @param[in] none.
 * @return none
 */
static void dfu_serial_cmd_init_pkt(void)
{
    uint8_t error = 0;
    memcpy(&m_init_pkt,m_buffer+2,sizeof(_init_pkt));
    if(m_init_pkt.crc != dfu_crc32((uint8_t *)&m_init_pkt.crc + 4, sizeof(_init_pkt) - 4))
    {
        error = 1;
    }
    if(m_init_pkt.app_start_address < HP_BOOTSETTING_START_ADDRESS)
    {
        error = 1;
    }
    uint8_t cmd[] = {DFU_SERIAL_HEADER,DFU_SERIAL_CMD_InitPkt,error};
    serial_send_data(cmd, sizeof(cmd));
}
/**
 * @brief Process packet header.
 * @param[in] none.
 * @return none
 */
static void dfu_serial_cmd_pkt_header(void)
{
    m_pkt.offset = 0;
    uint8_t error = 0;
    memcpy(&m_pkt_header,m_buffer+2,sizeof(_pkt_header));
    uint8_t cmd[] = {DFU_SERIAL_HEADER,DFU_SERIAL_CMD_Pkt_header,error};
    serial_send_data(cmd, sizeof(cmd));    
}
/**
 * @brief Process packet data.
 * @param[in] none.
 * @return none
 */
static void dfu_serial_cmd_pkt(void)
{    
    uint8_t error = 0;
    memcpy(m_pkt.buffer+m_pkt.offset,m_buffer+3,m_buffer[2]);    
    m_pkt.offset += m_buffer[2];
    
    if(m_pkt.offset >= m_pkt_header.size)
    {
        if(m_pkt_header.crc == dfu_crc32(m_pkt.buffer, m_pkt_header.size))
        {
            Qflash_Erase_Sector(m_init_pkt.app_start_address + m_pkt_header.offset);
            Qflash_Write(m_init_pkt.app_start_address + m_pkt_header.offset, m_pkt.buffer, m_pkt_header.size);
            if(m_pkt_header.crc != dfu_crc32((uint8_t *)((uint32_t *)(m_init_pkt.app_start_address + m_pkt_header.offset)), m_pkt_header.size))
            {
                error = 1;
            }
        }
        else
        {
            error = 1;
        }
    }
    uint8_t cmd[] = {DFU_SERIAL_HEADER,DFU_SERIAL_CMD_Pkt,error};
    serial_send_data(cmd, sizeof(cmd));    
}
/**
 * @brief Validate receviced data.
 * @param[in] none.
 * @return none
 */
static void dfu_serial_cmd_postvalidate(void)
{
    uint8_t error = 0;
    
    if(m_init_pkt.app_crc != dfu_crc32((uint8_t *)((uint32_t *)m_init_pkt.app_start_address), m_init_pkt.app_size))
    {
        error = 1;
    }
    else{
        memcpy(&m_hp_bootsetting,&hp_bootsetting,sizeof(HP_Bootsetting_t));
        
        m_hp_bootsetting.app1.activation = HP_BOOTSETTING_ACTIVATION_NO;
        m_hp_bootsetting.app2.activation = HP_BOOTSETTING_ACTIVATION_NO;
        m_hp_bootsetting.ImageUpdate.activation = HP_BOOTSETTING_ACTIVATION_NO;
        m_hp_bootsetting.master_boot_force_update = HP_BOOTSETTING_MASTER_BOOT_FORCE_UPDATE_NO;
        if(m_init_pkt.app_start_address == HP_APP1_START_ADDRESS)
        {
            m_hp_bootsetting.app1.start_address = HP_APP1_START_ADDRESS;
            m_hp_bootsetting.app1.size = m_init_pkt.app_size;
            m_hp_bootsetting.app1.version = m_init_pkt.app_version;
            m_hp_bootsetting.app1.crc = m_init_pkt.app_crc;
            m_hp_bootsetting.app1.activation = HP_BOOTSETTING_ACTIVATION_YES;
        }
        else if(m_init_pkt.app_start_address == HP_APP2_START_ADDRESS)
        {
            m_hp_bootsetting.app2.start_address = HP_APP2_START_ADDRESS;
            m_hp_bootsetting.app2.size = m_init_pkt.app_size;
            m_hp_bootsetting.app2.version = m_init_pkt.app_version;
            m_hp_bootsetting.app2.crc = m_init_pkt.app_crc;
            m_hp_bootsetting.app2.activation = HP_BOOTSETTING_ACTIVATION_YES;        
        }
        else if(m_init_pkt.app_start_address == HP_IMAGE_UPDATE_START_ADDRESS)
        {
            m_hp_bootsetting.ImageUpdate.start_address = HP_IMAGE_UPDATE_START_ADDRESS;
            m_hp_bootsetting.ImageUpdate.size = m_init_pkt.app_size;
            m_hp_bootsetting.ImageUpdate.version = m_init_pkt.app_version;
            m_hp_bootsetting.ImageUpdate.crc = m_init_pkt.app_crc;
            m_hp_bootsetting.ImageUpdate.activation = HP_BOOTSETTING_ACTIVATION_YES;        
        }  
        else
        {
            error = 1;
        }
        
        if(error == 0){
            m_hp_bootsetting.crc = dfu_crc32((uint8_t *)&m_hp_bootsetting.crc + 4, sizeof(HP_Bootsetting_t) - 4);
            Qflash_Erase_Sector(HP_BOOTSETTING_START_ADDRESS);
            Qflash_Write(HP_BOOTSETTING_START_ADDRESS, (uint8_t *)&m_hp_bootsetting, sizeof(HP_Bootsetting_t));
            if(m_hp_bootsetting.crc != dfu_crc32((uint8_t *)((uint32_t *)(HP_BOOTSETTING_START_ADDRESS + 4)), sizeof(HP_Bootsetting_t) - 4))
            {
                error = 2;
            }        
        }
        
    }
    uint8_t cmd[] = {DFU_SERIAL_HEADER,DFU_SERIAL_CMD_PostValidate,error};
    serial_send_data(cmd, sizeof(cmd));        
}



/**
 * @brief Reset System.
 * @param[in] none.
 * @return none
 */
static void dfu_serial_cmd_activate_reset(void)
{
    uint8_t error = 0;

    uint8_t cmd[] = {DFU_SERIAL_HEADER,DFU_SERIAL_CMD_ActivateReset,error};
    serial_send_data(cmd, sizeof(cmd));        
    
    if(error == 0)
    {
        dfu_delay_ms(100);
        NVIC_SystemReset();
        
    }
    
    
}





static void dfu_serial_cmd_jump_to_master_boot(void)
{
    uint8_t cmd[] = {DFU_SERIAL_HEADER,DFU_SERIAL_CMD_JumpToMasterBoot};
    serial_send_data(cmd, sizeof(cmd));
}


static void dfu_serial_cmd_otp_read(void)
{
    //rec: head,cmd,offset,size,
    //rsp: head,cmd,error,offset,size,crc,data*size
    _pkt_header rec_pkt = {0};    
    uint8_t otp_rsp[256] = {DFU_SERIAL_HEADER};
    uint8_t error = 0; 
    memset(&otp_rsp[1],0,255);
    otp_rsp[1] = m_buffer[1];
    memcpy(&rec_pkt,m_buffer+2,sizeof(_pkt_header)); // copy  address+size+crc
    if(rec_pkt.size > OTP_BUF_MAX)
    {
        error = 1;
    }
    else{
        error = OTPTrim_Read(rec_pkt.offset, &otp_rsp[3+sizeof(_pkt_header)], rec_pkt.size);
    }
    
    if(error == 0)
    {
        rec_pkt.crc = dfu_crc32(&otp_rsp[3+sizeof(_pkt_header)],rec_pkt.size);
        memcpy(&otp_rsp[3],&rec_pkt,sizeof(_pkt_header));// copy  address+size+crc
    }
    otp_rsp[2] = error;
    serial_send_data(otp_rsp, sizeof(otp_rsp));
}


uint32_t otp_update(uint32_t address, uint8_t* p_data, uint32_t len)
{
    uint32_t error = 0;  
    uint32_t bank = OTP_ADDRESS_TO_SECTOR(address);
    uint32_t offset = address&0x00000FFF;
    uint8_t otp_buffer[OTP_SECTOR_SIZE];
    error = OTPTrim_Read(bank, otp_buffer, OTP_SECTOR_SIZE);
    if(error != FlashOperationSuccess) return error;
    error = OTPTrim_Erase(bank);
    if(error != FlashOperationSuccess) return error;
    memcpy(otp_buffer+offset, p_data, len);
    error = OTPTrim_Write(bank, otp_buffer, OTP_SECTOR_SIZE);
    return error;
}

static void dfu_serial_cmd_otp_write(void)
{
    //rec: head,cmd,offset,size,crc,data*size
    //rsp: head,cmd,error
    uint8_t error = 0;
    _pkt_header rec_pkt = {0};  
    memcpy(&rec_pkt,m_buffer+2,sizeof(_pkt_header)); // copy  address+size+crc    
    uint32_t check_crc = dfu_crc32(m_buffer+2+sizeof(_pkt_header), rec_pkt.size);    

    if(check_crc == rec_pkt.crc)
    {
        if(rec_pkt.size > OTP_BUF_MAX)
        {
            error = 1;
        }
        else{
            #if 1
            error = otp_update(rec_pkt.offset, &m_buffer[2+sizeof(_pkt_header)], rec_pkt.size);
            #else
            error = OTPTrim_Write(rec_pkt.offset, &m_buffer[2+sizeof(_pkt_header)], rec_pkt.size);
            #endif
        }
    }
    else{
        error = 2;
    }
    uint8_t cmd[3] = {DFU_SERIAL_HEADER,DFU_SERIAL_CMD_OtpWrite,error};
    serial_send_data(cmd, sizeof(cmd));
}


static void dfu_serial_cmd_otp_erase(void)
{
    //rec: head,cmd,offset
    //rsp: head,cmd,error
    uint8_t error = 0;    
    _pkt_header rec_pkt = {0};  
    memcpy(&rec_pkt,m_buffer+2,sizeof(_pkt_header)); // copy  address+size+crc
    error = OTPTrim_Erase(rec_pkt.offset);
    uint8_t cmd[3] = {DFU_SERIAL_HEADER,DFU_SERIAL_CMD_OtpErase,error};
    serial_send_data(cmd, sizeof(cmd));
}


static void dfu_serial_cmd_otp_lock(void)
{
    //rec: head,cmd,offset
    //rsp: head,cmd,error
    uint8_t error = 0; 
    _pkt_header rec_pkt = {0};  
    memcpy(&rec_pkt,m_buffer+2,sizeof(_pkt_header)); // copy  address+size+crc
    uint32_t bank = OTP_ADDRESS_TO_SECTOR(rec_pkt.offset);
    error = OTPTrim_Lock(bank);
    uint8_t cmd[3] = {DFU_SERIAL_HEADER,DFU_SERIAL_CMD_OtpLock,error};
    serial_send_data(cmd, sizeof(cmd));
}



void USART1_IRQHandler(void)
{
    static uint32_t index = 0;
    static uint8_t buffer[256];
    
    if(USART_GetFlagStatus(USART1, USART_FLAG_RXDNE) != RESET)
    {
        buffer[index] = USART_ReceiveData(USART1);
        
        if(buffer[0] == DFU_SERIAL_HEADER)
        {
            index++;
            if(index >= 256)
            {
                index = 0;    
                memset(m_buffer,0,sizeof(m_buffer));
                memcpy(m_buffer,buffer, 256);
                
                uint8_t event = SCHED_EVT_RX_DATA;
                uint32_t    err_code = app_sched_event_put(&event ,sizeof(uint8_t),sched_evt);
                ERROR_CHECK(err_code);
            }                            
        }
    }    
}



static uint32_t serial_send_data(uint8_t *p_data, uint32_t length)
{
    uint8_t cmd[256];
    memset(cmd,0,sizeof(cmd));
    memcpy(cmd,p_data,length);
    dfu_usart1_send(cmd,sizeof(cmd));
    return 0;
}






