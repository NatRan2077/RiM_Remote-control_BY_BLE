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
 * @file hp_ble.c
 * @version v1.0.4
 *
  */

/** 
 * @addtogroup APP
 * @{ 
 */

/* Includes ------------------------------------------------------------------*/
#include "rwip_config.h"             // SW configuration

#if (BLE_APP_PRESENT)
#include "hp_sleep.h"
#include "hp_ble.h"
#include "hp_ble_task.h"
#if (BLE_APP_SEC)
#include "hp_sec.h"                 // Application security Definition
#endif // (BLE_APP_SEC)
#if (HP_TIMER_ENABLE)
#include "hp_timer.h"
#endif //HP_TIMER_ENABLE


/* Private define ------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static __align(4) unsigned char  rwip_fifo_isr_codeArray[] ={
0x03,0xb4,0x01,0x48,0x01,0x90,0x01,0xbd,0xed,0xc3,0x02,0x00,0x10,0xb5,0x07,0x4c,
0xe0,0x69,0x00,0x04,0x08,0xd5,0xff,0xf7,0xf3,0xff,0x05,0x48,0x00,0x78,0x00,0x28,
0x02,0xd1,0x01,0x20,0xc0,0x03,0x20,0x62,0x10,0xbd,0x00,0x00,0x00,0x80,0x02,0x40,
0xc6,0x00,0x00,0x20};

/// List of functions used to create the database  
struct hp_ble_add_prf_evn_t  add_prf_evn;
struct hp_ble_prf_evn_t      ble_prf_evn;

/// Application Environment Structure
struct app_env_tag      app_env;
struct hp_gap_params_t  gap_env;
struct hp_adv_params_t  adv_env;
struct hp_scan_params_t scan_env;
/* Private function prototypes -----------------------------------------------*/
/// Application Task Descriptor
extern const struct ke_task_desc TASK_DESC_APP_M;
/* Private functions ---------------------------------------------------------*/


IRQ_HANNDLE_FUN user_HardFault_Handler = NULL;
IRQ_HANNDLE_FUN user_EXTI4_12_IRQHandler = NULL;
#ifdef SLEEP_LP_TIMER_ENABLE
IRQ_HANNDLE_FUN lpt_wakeup_hdl = NULL;
IRQ_HANNDLE_FUN user_RTC_IRQHandler = NULL;
/** 
 * @brief  RTC IRQ Handler
 * @param  
 * @return void
 * @note   
 */
void hp_RTC_IRQHandler(void)
{
    if(((RTC->INITSTS & RTC_INITSTS_WTF) != RESET) ||
        ((EXTI->PEND & EXTI_LINE9) != RESET))  //EXTI_GetITStatus
    {
        RTC->INITSTS &=~RTC_INITSTS_WTF;
        EXTI->PEND = EXTI_LINE9; //EXTI_ClrITPendBit
        if(lpt_wakeup_hdl != NULL)
        {
            lpt_wakeup_hdl();
        }
    }
    if(user_RTC_IRQHandler != NULL)
    {
        user_RTC_IRQHandler();
    }
}

#endif   
void hp_HardFault_Handler(void)
{
    uint32_t *stack_top = (uint32_t *)__get_MSP();
//    HP_LOG_INIT();  
    HP_LOG_ERROR("**************HardFault ERROR:REG INFO**************.\r\n");                                                                                                                            \
    HP_LOG_ERROR("msp = 0x%08x.\r\n", (uint32_t)stack_top);                                                                                                                                                        \
    HP_LOG_ERROR("r0  = 0x%08x, \tr1   = 0x%08x.\r\n", stack_top[0], stack_top[1]);                                                                                                                    \
    HP_LOG_ERROR("r2  = 0x%08x, \tr3   = 0x%08x.\r\n", stack_top[2], stack_top[3]);                                                                                                                    \
    HP_LOG_ERROR("r12 = 0x%08x, \tlr   = 0x%08x.\r\n", stack_top[4], stack_top[5]);                                                                                                                    \
    HP_LOG_ERROR("pc  = 0x%08x, \txpsr = 0x%08x.\r\n", stack_top[6], stack_top[7]);  
   
    
    if(user_HardFault_Handler )
    {
        user_HardFault_Handler();//call user HardFault_Handler
    }
    
    HP_LOG_ERROR("%s\r\n",__func__);
    
    NVIC_SystemReset();    
}

void hp_EXTI4_12_IRQHandler(void)
{
    /*  EXTI line 11 for BLE Stack */
    if (RESET != EXTI_GetITStatus(EXTI_LINE11))
    {
        EXTI_ClrITPendBit(EXTI_LINE11);
        g_sleep_status_flag = 2; //BB_WAKEUP_OSEN
    }
    /* NRST Wakeup Interrupt through EXTI line 12 */
    if (RESET != EXTI_GetITStatus(EXTI_LINE12))
    {
        EXTI_ClrITPendBit(EXTI_LINE12);   
//        HP_LOG_INIT();
        HP_LOG_INFO("NRST->NVIC_SystemReset\r\n");
        NVIC_SystemReset();
    }
    
    if(user_EXTI4_12_IRQHandler)
    {
        user_EXTI4_12_IRQHandler();//call user EXTI4_12_IRQHandler
    }
}

static void hp_NRST_config(void)
{
    EXTI_InitType EXTI_InitStructure = {0};
    /* Enable the NRST Wakeup Interrupt through EXTI line 12 */
    EXTI_InitStructure.EXTI_Line    = EXTI_LINE12;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitPeripheral(&EXTI_InitStructure);
}
#if(HP_RF_CTRL_ENABLE)
/// Structure of a frame element
struct sch_prog_frm_elt
{
    /// Timestamp of the programmed frame (in BLE half slots, based on local clock)
    uint32_t timestamp;
    /// Callback for handling interrupts related to the frame
    frm_cbk_t frm_cbk;
    /// Dummy value (to be reported to the driver)
    uint32_t dummy;
    /// Indicate if the frame is valid (programmed and not skipped or finished)
    bool valid;
};

struct sch_prog_env_tag
{
    /// Frame elements pool
    struct sch_prog_frm_elt tab[REG_EM_ET_SIZE];

    /// Exchange table index of the oldest entry currently used by the HW
    uint8_t et_idx_current;

    /// Next exchange table index to program
    uint8_t et_idx_next_prog;

    /// Number of programmed frames
    uint8_t nb_prog;

    #if BT_EMB_PRESENT
    /// Pointer to the clock element
    struct sch_prog_fm_clk_elt clk_elt;
    #endif // BT_EMB_PRESENT
};
void sch_prog_rx_isr_ttt(uint8_t et_idx)
{
    struct sch_prog_env_tag * sch_prog_env_p = (void *)0x20000844;
    if(sch_prog_env_p->tab[et_idx].frm_cbk != NULL)
    {
        // Invoke frame callback
        sch_prog_env_p->tab[et_idx].frm_cbk(sch_prog_env_p->tab[et_idx].timestamp,
                sch_prog_env_p->tab[et_idx].dummy,
                SCH_FRAME_IRQ_RX);
    }
}

void sch_prog_tx_isr_ttt(uint8_t et_idx)
{
    struct sch_prog_env_tag * sch_prog_env_p = (void *)0x20000844;
    if(sch_prog_env_p->tab[et_idx].frm_cbk != NULL)
    {
        // Invoke frame callback
        sch_prog_env_p->tab[et_idx].frm_cbk(sch_prog_env_p->tab[et_idx].timestamp,
                sch_prog_env_p->tab[et_idx].dummy,
                SCH_FRAME_IRQ_TX);
    }

}
       
void sch_prog_fifo_isr_t(void)
{
    // Fetch interrupt FIFO
    uint32_t actfifostat = ip_actfifostat_get();
    uint8_t curr_et_idx = GETF(actfifostat, IP_CURRENT_ET_IDX);
    uint8_t skip_et_idx = GETF(actfifostat, IP_SKIP_ET_IDX);

//    printf("actfifostat =%x\r\n", actfifostat);
    // TX
    if (actfifostat & IP_TXINTSTAT_BIT)
    {
        // Call handler
        sch_prog_tx_isr_ttt(curr_et_idx);
    }
    // RX
    if (actfifostat & IP_RXINTSTAT_BIT)
    {
        // Call handler
        sch_prog_rx_isr_ttt(curr_et_idx);
    }


    // Skip event
    if (actfifostat & IP_SKIPACTINTSTAT_BIT)
    {
        // Handle skip interrupt
        sch_prog_skip_isr(skip_et_idx);
    }

    // End of event
    if (actfifostat & IP_ENDACTINTSTAT_BIT)
    {
        // Handle end of frame
        sch_prog_end_isr(curr_et_idx);
    }
}
extern uint8_t flag_rx_tx_mode;
void rwip_fifo_isr_ttt(void)
{
    if(flag_rx_tx_mode )
    {
         uint32_t irq_stat      = ip_intstat1_get();
        // FIFO
        if (irq_stat & IP_FIFOINTSTAT_BIT) // FIFO interrupt
        {
            sch_prog_fifo_isr_t();
            ip_intack1_fifointack_clearf(1);  
        }
    }    
    // Check interrupt status and call the appropriate handlers
    else
    {    
        typedef void(*function_ptr)(void);
        function_ptr my_function_ptr = (void *)&rwip_fifo_isr_codeArray[0x0d];
        my_function_ptr();
    }
}
void rwip_error_isr_1(void)
{
    printf("rwip_error_isr_1\r\n");
}    
#endif ///(HP_RF_CTRL_ENABLE)
static void hp_ble_stack_vtor_init(void)
{
    extern uint32_t __Vectors;
    /*note: SPI1_IRQn and IRC_IRQn Interrupt not availed. */
    //init vtor address
    volatile uint32_t* p_IrqFun  = (void*)0x200009c0;
    uint32_t* user_vtor = (&__Vectors);

    //system Interrupt
    REG32(0x200000ec) = (uint32_t)user_vtor[16+NonMaskableInt_IRQn];
    REG32(0x200000f0) = (uint32_t)hp_HardFault_Handler;
    REG32(0x200000e8) = (uint32_t)user_vtor[16+SVCall_IRQn];
    REG32(0x200000f4) = (uint32_t)user_vtor[16+SysTick_IRQn];
    REG32(0x200000f8) = (uint32_t)user_vtor[16+PendSV_IRQn];
    //user Interrupt
    memcpy((void*)p_IrqFun,&user_vtor[16],124);
    //clear ble Interrupt
    p_IrqFun[BLE_SW_IRQn]               = 0;
    p_IrqFun[BLE_HSLOT_IRQn]            = 0;
    p_IrqFun[BLE_FINETGT_IRQn]          = 0;
    p_IrqFun[BLE_FIFO_IRQn]             = (uint32_t)(&rwip_fifo_isr_codeArray[0x0d]);
    p_IrqFun[BLE_ERROR_IRQn]            = 0;
    p_IrqFun[BLE_CRYPT_IRQn]            = 0;
    p_IrqFun[BLE_TIMESTAMP_TGT1_IRQn]   = 0;
    p_IrqFun[BLE_TIMESTAMP_TGT2_IRQn]   = 0;
    p_IrqFun[BLE_SLP_IRQn]              = (uint32_t)rwip_slp_isr; 
    
    p_IrqFun[EXTI4_12_IRQn]             = (uint32_t)hp_EXTI4_12_IRQHandler;
    
    
    //Register updated ble fifo irq function
    g_sch_prog_fifo_iar_branch = false;
    rwip_prog_delay = 3;
    //enable Interrupt of remaped in flash
	#if(HP_RF_CTRL_ENABLE)
	extern void EXTI2_3_IRQHandler(void);
    p_IrqFun[EXTI2_3_IRQn]              = (uint32_t)EXTI2_3_IRQHandler;
	p_IrqFun[BLE_ERROR_IRQn]            = (uint32_t)rwip_error_isr_1;;
	p_IrqFun[BLE_FIFO_IRQn]             = (uint32_t)rwip_fifo_isr_ttt;///
    NVIC_EnableIRQ(BLE_ERROR_IRQn);
	NVIC_EnableIRQ(EXTI2_3_IRQn);
	#endif ///HP_RF_CTRL_ENABLE
    NVIC_EnableIRQ(BLE_FIFO_IRQn);
    NVIC_EnableIRQ(BLE_SLP_IRQn);
    NVIC_EnableIRQ(EXTI4_12_IRQn);

    //register HardFault Interrupt if user define it
    uint32_t temp_addr  =(*(uint32_t *)&user_vtor[3]) - 1;
    uint16_t temp_cmd   = *((uint16_t*)temp_addr);
    if( temp_cmd != 0xe7fe) // ASM cmd B 
    {
        user_HardFault_Handler = (IRQ_HANNDLE_FUN)(*(uint32_t *)&user_vtor[3]);;
    }
    //register EXTI4_12 Interrupt if user define it
    temp_addr  =(*(uint32_t *)&user_vtor[24]) - 1;
    temp_cmd   = *((uint16_t*)temp_addr);
    if( temp_cmd != 0xe7fe) // ASM cmd B 
    {
        user_EXTI4_12_IRQHandler = (IRQ_HANNDLE_FUN)(*(uint32_t *)&user_vtor[24]);
    }
    #ifdef SLEEP_LP_TIMER_ENABLE
    p_IrqFun[RTC_IRQn]                  = (uint32_t)hp_RTC_IRQHandler;
    //register RTC Interrupt if user define it
    temp_addr  =(*(uint32_t *)&user_vtor[18]) - 1; //16+RTC_IRQn
    temp_cmd   = *((uint16_t*)temp_addr);
    if( temp_cmd != 0xe7fe) // ASM cmd B 
    {
        user_RTC_IRQHandler = (IRQ_HANNDLE_FUN)(*(uint32_t *)&user_vtor[18]);
    }
    #endif
    // enable NRST pin Interrupt
    hp_NRST_config();

}




static void app_create_advertising(void)
{
    HP_LOG_DEBUG("%s\r\n", __func__);
    if (app_env.adv_state == APP_ADV_STATE_IDLE )
    {
        // Prepare the GAPM_ACTIVITY_CREATE_CMD message
        struct gapm_activity_create_adv_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_CREATE_CMD,
                                                                  TASK_GAPM, TASK_APP,
                                                                  gapm_activity_create_adv_cmd);

        // Set operation code
        p_cmd->operation = GAPM_CREATE_ADV_ACTIVITY;

        // Fill the allocated kernel message
        p_cmd->own_addr_type                = gap_env.mac_addr_type;
        p_cmd->adv_param.type               = GAPM_ADV_TYPE_LEGACY;
        p_cmd->adv_param.prop               = GAPM_ADV_PROP_UNDIR_CONN_MASK;
        p_cmd->adv_param.filter_pol         = ADV_ALLOW_SCAN_ANY_CON_ANY;
        p_cmd->adv_param.disc_mode          = GAPM_ADV_MODE_GEN_DISC;        
        p_cmd->adv_param.prim_cfg.chnl_map  = ADV_ALL_CHNLS_EN;
        p_cmd->adv_param.prim_cfg.phy       = adv_env.adv_phy;
        
        if(adv_env.beacon_enable)
        {
            p_cmd->adv_param.prop           = GAPM_ADV_PROP_NON_CONN_NON_SCAN_MASK;
            p_cmd->adv_param.disc_mode      = GAPM_ADV_MODE_BEACON;
        }else
        if(adv_env.ex_adv_enable)
        {
            if( adv_env.adv_phy == PHY_2MBPS_VALUE)
            {
                p_cmd->adv_param.prim_cfg.phy   = PHY_1MBPS_VALUE;
            }
            p_cmd->adv_param.type               = GAPM_ADV_TYPE_EXTENDED;
            p_cmd->adv_param.prop               = GAPM_EXT_ADV_PROP_UNDIR_CONN_MASK;
            p_cmd->adv_param.second_cfg.phy     = adv_env.adv_phy;
            p_cmd->adv_param.second_cfg.adv_sid = 1;
            p_cmd->adv_param.second_cfg.max_skip = 0;
        }

        switch (app_env.adv_mode)
        {
            case APP_ADV_MODE_DIRECTED:
                /*
                 * If the peripheral is already bonded with a central device, use the direct advertising
                 * procedure (BD Address of the peer device is stored in NVDS.
                 */
                #if(BLE_APP_SEC)
                if (hp_sec_get_bond_status())
                {
                    p_cmd->adv_param.prop                   = GAPM_ADV_PROP_DIR_CONN_MASK;
                    p_cmd->adv_param.disc_mode              = GAPM_ADV_MODE_NON_DISC;  
                    p_cmd->adv_param.prim_cfg.adv_intv_min  = adv_env.directed_adv.adv_intv;
                    p_cmd->adv_param.prim_cfg.adv_intv_max  = adv_env.directed_adv.adv_intv;
                    //set addr for dir
                    hp_bond_last_bonded_peer_id(&p_cmd->adv_param.peer_addr);
                }
                else
                #endif    
                {
                    app_env.adv_mode = APP_ADV_MODE_FAST;
                    p_cmd->adv_param.prim_cfg.adv_intv_min = adv_env.fast_adv.adv_intv;
                    p_cmd->adv_param.prim_cfg.adv_intv_max = adv_env.fast_adv.adv_intv;
                }

                break;
            case APP_ADV_MODE_FAST:
                p_cmd->adv_param.prim_cfg.adv_intv_min = adv_env.fast_adv.adv_intv;
                p_cmd->adv_param.prim_cfg.adv_intv_max = adv_env.fast_adv.adv_intv;
                break;
            case APP_ADV_MODE_SLOW:
                p_cmd->adv_param.prim_cfg.adv_intv_min = adv_env.slow_adv.adv_intv;
                p_cmd->adv_param.prim_cfg.adv_intv_max = adv_env.slow_adv.adv_intv;
               
                break;                    
            default:
                break;
        }
        // Keep the current operation
        app_env.adv_state = APP_ADV_STATE_CREATING;
        // And the next expected operation code for the command completed event
        app_env.current_op = CURRENT_OP_CREATE_ADV;
        // Send the message
        ke_msg_send(p_cmd);
    }
}

static void app_delete_activity(void)
{
    // Prepare the GAPM_ACTIVITY_CREATE_CMD message
    struct gapm_activity_delete_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_DELETE_CMD,
                                                              TASK_GAPM, TASK_APP,
                                                              gapm_activity_delete_cmd);
    HP_LOG_DEBUG("%s\r\n", __func__);
    // Set operation code
    p_cmd->operation = GAPM_DELETE_ACTIVITY;
    p_cmd->actv_idx  = app_env.adv_actv_idx;

    // Keep the current operation
    // And the next expected operation code for the command completed event
    app_env.current_op   = CURRENT_OP_DELETE_ADV;
   
    app_env.adv_state = APP_ADV_STATE_IDLE; //reset state
    //set next adv mode
    if(ke_state_get(TASK_APP) == APP_CONNECTED)
    {
        //connected, stop adv
        app_env.adv_mode = APP_ADV_MODE_STOP;
    }
    else
    {    //not connect yet, start next adv
        switch (app_env.adv_mode)
        {
            case APP_ADV_MODE_ENABLE:
                //restart adv
            case APP_ADV_MODE_DIRECTED:
                if(adv_env.fast_adv.enable)
                {
                    app_env.adv_mode = APP_ADV_MODE_FAST;
                    break;
                }
            case APP_ADV_MODE_FAST:
                if(adv_env.slow_adv.enable)
                {
                    app_env.adv_mode = APP_ADV_MODE_SLOW;
                    break;
                }
            case APP_ADV_MODE_SLOW:
                app_env.adv_mode = APP_ADV_MODE_STOP;
                break;
            default:
                app_env.adv_mode = APP_ADV_MODE_STOP;
                break;
        }
    }
    // Send the message
    ke_msg_send(p_cmd);
}

static void app_start_advertising(void)
{  
    HP_LOG_DEBUG("%s\r\n", __func__);
    // Prepare the GAPM_ACTIVITY_START_CMD message
    struct gapm_activity_start_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_START_CMD,
                                                         TASK_GAPM, TASK_APP,
                                                         gapm_activity_start_cmd);

    p_cmd->operation = GAPM_START_ACTIVITY;
    p_cmd->actv_idx = app_env.adv_actv_idx;

    switch (app_env.adv_mode)
    {
        case APP_ADV_MODE_DIRECTED:
            HP_LOG_DEBUG("APP_ADV_MODE_DIRECTED ");
            p_cmd->u_param.adv_add_param.duration = SECS_TO_UNIT(adv_env.directed_adv.duration , SECS_UNIT_10MS);    
            break;
        case APP_ADV_MODE_FAST:
            HP_LOG_DEBUG("APP_ADV_MODE_FAST ");
            p_cmd->u_param.adv_add_param.duration = SECS_TO_UNIT(adv_env.fast_adv.duration, SECS_UNIT_10MS);
            break;
        case APP_ADV_MODE_SLOW:
            HP_LOG_DEBUG("APP_ADV_MODE_SLOW ");
            p_cmd->u_param.adv_add_param.duration = SECS_TO_UNIT(adv_env.slow_adv.duration, SECS_UNIT_10MS);
            break;
        default:
            p_cmd->u_param.adv_add_param.duration = 0;
            break;
    }
    HP_LOG_DEBUG(", duration: %d x10 ms\r\n",p_cmd->u_param.adv_add_param.duration);

    p_cmd->u_param.adv_add_param.max_adv_evt = 0;
    
    llhwc_modem_setmode(adv_env.adv_phy);
    // Keep the current operation
    app_env.adv_state = APP_ADV_STATE_STARTING;
    // And the next expected operation code for the command completed event
    app_env.current_op = CURRENT_OP_START_ADV;
    
    // Send the message
    ke_msg_send(p_cmd);
}


static void app_stop_advertising(void)
{
    // Prepare the GAPM_ACTIVITY_STOP_CMD message
    struct gapm_activity_stop_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_STOP_CMD,
                                                      TASK_GAPM, TASK_APP,
                                                      gapm_activity_stop_cmd);
    HP_LOG_DEBUG("%s\r\n", __func__);
    // Fill the allocated kernel message
    p_cmd->operation = GAPM_STOP_ACTIVITY;
    p_cmd->actv_idx = app_env.adv_actv_idx;
    // Update advertising state
    app_env.adv_state = APP_ADV_STATE_STOPPING;
    // And the next expected operation code for the command completed event
    app_env.current_op   = CURRENT_OP_STOP_ADV;
    // Send the message
    ke_msg_send(p_cmd);
}



static void app_set_adv_data(void)
{
    uint8_t data_len;
    if(adv_env.ex_adv_enable)
    {
        data_len = adv_env.ex_adv_data_len;
    }
    else{
        data_len = adv_env.adv_data_len;
    }
    // Prepare the GAPM_SET_ADV_DATA_CMD message
    struct gapm_set_adv_data_cmd *p_cmd = KE_MSG_ALLOC_DYN(GAPM_SET_ADV_DATA_CMD,
                                                           TASK_GAPM, TASK_APP,
                                                           gapm_set_adv_data_cmd,
                                                           data_len);
    HP_LOG_DEBUG("%s\r\n", __func__);
    // Fill the allocated kernel message
    p_cmd->operation = GAPM_SET_ADV_DATA;
    p_cmd->actv_idx = app_env.adv_actv_idx;

    p_cmd->length = 0;

    // GAP will use 3 bytes for the AD Type
    if(adv_env.ex_adv_enable)
    {
        memcpy(&p_cmd->data[0],adv_env.ex_adv_p_data, adv_env.ex_adv_data_len);
    }
    else{
        memcpy(&p_cmd->data[0],adv_env.adv_data, adv_env.adv_data_len);
    }
    p_cmd->length += data_len;

    // Update advertising state
    app_env.adv_state = APP_ADV_STATE_SETTING_ADV_DATA;
    // And the next expected operation code for the command completed event
    app_env.current_op   = CURRENT_OP_SET_ADV_DATA;
    // Send the message
    ke_msg_send(p_cmd);
}

static void app_set_scan_rsp_data(void)
{
    uint8_t rem_len = ADV_DATA_LEN;
    
    // Prepare the GAPM_SET_ADV_DATA_CMD message
    struct gapm_set_adv_data_cmd *p_cmd = KE_MSG_ALLOC_DYN(GAPM_SET_ADV_DATA_CMD,
                                                           TASK_GAPM, TASK_APP,
                                                           gapm_set_adv_data_cmd,
                                                           ADV_DATA_LEN);
    HP_LOG_DEBUG("%s\r\n", __func__);
    // Fill the allocated kernel message
    p_cmd->operation = GAPM_SET_SCAN_RSP_DATA;
    p_cmd->actv_idx = app_env.adv_actv_idx;

    p_cmd->length = adv_env.scan_rsp_data_len;
    memcpy(&p_cmd->data[0], adv_env.scan_rsp_data, adv_env.scan_rsp_data_len);

    // Get remaining space in the Advertising Data - 2 bytes are used for name length/flag
    rem_len -= p_cmd->length;
    
    uint8_t *p_buf = &p_cmd->data[p_cmd->length];
    // Check if additional data can be added to the Advertising data - 2 bytes needed for type and length
    if (rem_len > 3 && adv_env.attach_appearance)
    {
        // appearance length
        *p_buf = 3 ;
        // appearance flag
        *(p_buf + 1) = 0x19;//ADV_TYPE_APPEARANCE
        // Copy device name
        *(p_buf + 3) = (gap_env.appearance>>8)&0xff; //hight 8bit
        *(p_buf + 2) = gap_env.appearance&0xff;      //low 8bit
        // Update advertising data length
        p_cmd->length += 4;
        p_buf += 4;
        //Update Remaining Length
        rem_len -= 4;
    }
    // Check if additional data can be added to the Advertising data - 2 bytes needed for type and length
    if (rem_len > 2 && adv_env.attach_name)
    {
        uint8_t dev_name_length = co_min(gap_env.dev_name_len, (rem_len - 2));

        // Device name length
        *p_buf = dev_name_length + 1;
        // Device name flag (check if device name is complete or not)
        *(p_buf + 1) = (dev_name_length == gap_env.dev_name_len) ? '\x09' : '\x08';
        // Copy device name
        memcpy(p_buf + 2, gap_env.dev_name, dev_name_length);

        // Update advertising data length
        p_cmd->length += (dev_name_length + 2);
    }

    // Update advertising state
    app_env.adv_state = APP_ADV_STATE_SETTING_SCAN_RSP_DATA;
    // And the next expected operation code for the command completed event
    app_env.current_op   = CURRENT_OP_SET_RSP_DATA;
    // Send the message
    ke_msg_send(p_cmd);
}



static void app_send_gapm_reset_cmd(void)
{
    // Reset the stack
    struct gapm_reset_cmd *p_cmd = KE_MSG_ALLOC(GAPM_RESET_CMD,
                                                TASK_GAPM, TASK_APP,
                                                gapm_reset_cmd);

    p_cmd->operation = GAPM_RESET;
    ke_msg_send(p_cmd);
}


/**
 * @brief Initialize the BLE application environment.
 **/
static void hp_ble_evn_init(void)
{
    HP_LOG_DEBUG("%s\r\n", __func__);
    // Reset the application manager environment
    memset(&app_env, 0, sizeof(app_env));
    memset(&gap_env, 0, sizeof(gap_env));
    memset(&adv_env, 0, sizeof(adv_env));
    memset(&scan_env, 0, sizeof(scan_env));
    app_env.adv_actv_idx = 0xFF;
    app_env.scan_actv_idx = 0xFF;
    app_env.init_actv_idx = 0xFF;

    for (uint8_t i =0;i<APP_CON_IDX_MAX;i++)
    {
        app_env.conn_env[i].conidx = GAP_INVALID_CONIDX;
    }
}
/**
 * @brief Initialize the BLE application task.
 **/
static void hp_ble_task_init(void)
{
     HP_LOG_DEBUG("%s\r\n", __func__);
    // Create APP task
    ke_task_create(TASK_APP, &TASK_DESC_APP_M);

    // Initialize Task state
    ke_state_set(TASK_APP, APP_INIT);

}

/**
 * @brief Add a required service in the database
 **/
bool hp_ble_add_svc(void)
{
    // Indicate if more services need to be added in the database
    bool more_svc = false;

    // Check if another should be added in the database
    if (app_env.next_svc < add_prf_evn.prf_num)
    {
        ASSERT_INFO(app_add_svc_func_list[app_env.next_svc] != NULL, app_env.next_svc, 1);

        // Call the function used to add the required service
        HP_LOG_DEBUG("%s\r\n", __func__);
        add_prf_evn.add_prf_func_list[app_env.next_svc]();

        // Select following service to add
        app_env.next_svc++;
        more_svc = true;
    }
    return more_svc;
}

bool hp_ble_add_prf_func_register(hp_ble_add_prf_func_t func)
{
    if(add_prf_evn.prf_num < BLE_NB_PROFILES)
    {
        //add profile to list 
        add_prf_evn.add_prf_func_list[add_prf_evn.prf_num] = func;
        add_prf_evn.prf_num++;
        return true;
    }
    return false;
}



bool hp_ble_prf_task_register(struct prf_task_t *prf)
{
    if(ble_prf_evn.prf_num < BLE_NB_PROFILES)
    {
        //add profile to list 
        memcpy(&ble_prf_evn.prf_task_list[ble_prf_evn.prf_num],
                prf,sizeof(struct prf_task_t));
        ble_prf_evn.prf_num++;
        return true;
    }
    return false;
}




/**
 * @brief Set slave latency without connection parameters update
 **/
void hp_ble_set_slave_latency(uint16_t latency_cfg)
{
    // Prepare the GAPC_SET_PREF_SLAVE_LATENCY message
    struct gapc_set_pref_slave_latency_cmd *p_cmd = KE_MSG_ALLOC(GAPC_SET_PREF_SLAVE_LATENCY_CMD,
                                                     KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                                     gapc_set_pref_slave_latency_cmd);

    p_cmd->operation  = GAPC_SET_PREF_SLAVE_LATENCY;
    p_cmd->latency    = latency_cfg; 

    // Send the message
    ke_msg_send(p_cmd);
}

/**
 * @brief Send to request to update the connection parameters
 **/
bool hp_ble_update_param(struct gapc_conn_param *p_conn_param)
{
    /********** connection parameters requirement *************
    Interval Max * (Slave Latency + 1) <= 4 s
    Interval Max >= 7.5 ms
    Interval Min <= Interval Max
    ConnSupervisionTimeout <= 10 s
    Interval Max * ( Slave Latency + 1) * 3 < ConnSupervisionTimeout
    ****************************************************************/
    HP_LOG_DEBUG("%s\r\n",__func__);
    if(( p_conn_param->intv_max*(p_conn_param->latency+1) > 3200/* 4s */)  ||
       ( p_conn_param->intv_max < 6 /* 7.5ms */) ||     
       ( p_conn_param->intv_max < p_conn_param->intv_min) || 
       ( p_conn_param->time_out > 1000 /* 10s */)||  
       ( p_conn_param->intv_max*(p_conn_param->latency+1)*3) > (p_conn_param->time_out*8 /* 10/1.25 */))
    {
        HP_LOG_WARNING("request parameters error\r\n");
        return false;
    }
    
    // Prepare the GAPC_PARAM_UPDATE_CMD message
    struct gapc_param_update_cmd *p_cmd = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD,
                                                     KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                                     gapc_param_update_cmd);

    p_cmd->operation  = GAPC_UPDATE_PARAMS;
    p_cmd->intv_min   = p_conn_param->intv_min;
    p_cmd->intv_max   = p_conn_param->intv_max;
    p_cmd->latency    = p_conn_param->latency;
    p_cmd->time_out   = p_conn_param->time_out;

    // not used by a slave device
    p_cmd->ce_len_min = 0x0;
    p_cmd->ce_len_max = 0xFFFF;

    // Send the message
    ke_msg_send(p_cmd);
    return true;
}

/**
 * @brief master device get peer info
 *  op:  GAPC_GET_PEER_FEATURES
 *       GAPC_GET_PEER_VERSION
 */
void hp_ble_get_peer_info(uint8_t op)
{
    HP_LOG_DEBUG("%s,app_env.conidx= %d\r\n", __func__,app_env.conidx);
    struct gapc_get_info_cmd *cmd = KE_MSG_ALLOC(GAPC_GET_INFO_CMD,
                                                     KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                                     gapc_get_info_cmd);
    cmd->operation = op;
    // Send the message
    ke_msg_send(cmd);
}

/**
 * @brief set ble dle
 **/
void hp_ble_dle_set(uint16_t tx_octets, uint16_t tx_time)
{
    struct gapc_set_le_pkt_size_cmd *cmd = KE_MSG_ALLOC(GAPC_SET_LE_PKT_SIZE_CMD,
                                                     KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                                     gapc_set_le_pkt_size_cmd);
    cmd->operation = GAPC_SET_LE_PKT_SIZE;
    cmd->tx_octets = tx_octets;
    cmd->tx_time   = tx_time;
    // Send the message
    ke_msg_send(cmd);
}


/**
 * @brief set ble mtu
 **/
void hp_ble_mtu_set(uint16_t mtu)
{
    
    struct gattc_exc_mtu_cmd *cmd = KE_MSG_ALLOC(GATTC_EXC_MTU_CMD,
                                                KE_BUILD_ID(TASK_GATTC, app_env.conidx), TASK_APP,
                                                gattc_exc_mtu_cmd);
    //ROM bug fix
    gapm_env.max_mtu = mtu;
        
    cmd->operation = GATTC_MTU_EXCH;
    cmd->seq_num = 0;
    ke_msg_send(cmd);
}



/**
 * @brief set ble phy 
 **/
void hp_ble_phy_set(enum ble_phy_val phy)
{
    struct gapc_set_phy_cmd *cmd = KE_MSG_ALLOC(GAPC_SET_PHY_CMD,
                                                KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                                gapc_set_phy_cmd);

    cmd->operation = GAPC_SET_PHY;
    cmd->rx_phy  = phy & (GAP_PHY_LE_1MBPS|GAP_PHY_LE_2MBPS|GAP_PHY_LE_CODED);
    cmd->tx_phy  = phy & (GAP_PHY_LE_1MBPS|GAP_PHY_LE_2MBPS|GAP_PHY_LE_CODED);
    cmd->phy_opt = (phy>>4)&(PHY_OPT_S8_LE_CODED_TX_PREF|PHY_OPT_S2_LE_CODED_TX_PREF);

    ke_msg_send(cmd);
}

/**
 * @brief avtive get ble RSSI with an interval in ms. 0ms mean active one time only
 **/
void hp_ble_active_rssi(uint32_t interval)
{
	_SYSLOG_("hp_ble_active_rssi: interval=%d, conidx=%d, state=%d\r\n", 
                 interval, app_env.conidx, ke_state_get(TASK_APP));
    struct gapc_get_info_cmd* info_cmd = KE_MSG_ALLOC(GAPC_GET_INFO_CMD,
                    KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                    gapc_get_info_cmd);
    
    app_env.rssi_intv = interval;
    // request RSSI
    info_cmd->operation = GAPC_GET_CON_RSSI;
    // send command
    ke_msg_send(info_cmd);   
}


void hp_ble_adv_fsm_next(void)
{
    HP_LOG_DEBUG("%s,adv_state:%d \r\n",__func__,app_env.adv_state);
    switch (app_env.adv_state)
    {
        case (APP_ADV_STATE_IDLE):
        {
            if(app_env.adv_mode != APP_ADV_MODE_STOP)
            {
                // Create advertising
                app_create_advertising();
            }
        } break;

        case (APP_ADV_STATE_CREATING):
        {
            if(app_env.adv_mode == APP_ADV_MODE_DIRECTED)
            {
                // Start advertising activity
                app_start_advertising();
            }
            else{
                // Set advertising data
                app_set_adv_data();
            }
        } break;

        case (APP_ADV_STATE_SETTING_ADV_DATA):
        {
            if(adv_env.beacon_enable || adv_env.ex_adv_enable)
            {
                app_start_advertising();
            }else{
                // Set scan response data
                app_set_scan_rsp_data();
            }
        } break;

        case (APP_ADV_STATE_CREATED):
        case (APP_ADV_STATE_SETTING_SCAN_RSP_DATA):
        {
            // Start advertising activity
            app_start_advertising();
        } break;

        case (APP_ADV_STATE_STARTING):
        {
            // Go to started state
            app_env.adv_state = APP_ADV_STATE_STARTED;
        } break;

        case (APP_ADV_STATE_STARTED):
        {
            // Stop advertising activity
            app_stop_advertising();
        } break;

        case (APP_ADV_STATE_STOPPING):
        {
            //delete activity when advertising stop
            app_delete_activity();

        } break;

        default:
        {
            ASSERT_ERR(0);
        } break;
    }
}

void hp_ble_lsc_config(ble_lsc_cfg_t lsc_set)
{
    //Config the LSC for stack
    if(lsc_set == BLE_LSC_LSE_32768HZ)
    {
        //enable LSE and select LSC as LSE
        *(uint32_t*)0x40011008 |= 0xF00;
        RCC->LSCTRL |= RCC_LSCTRL_LSEEN;         
        while(!(RCC->LSCTRL & RCC_LSCTRL_LSERD));
        RCC->LSCTRL |= RCC_LSCTRL_LSXSEL_LSE;       
        
        g_lsi_1_syscle_cal_value = 1000;
        //lse frequency 32768
        g_lsi_1_syscle_cnt_value = 976;  //32000000/32768 = 976.56 //must fixed
    }
    else
    {
        //LSC select LSI
        RCC->LSCTRL |= RCC_LSCTRL_LSIEN;         
        while(!(RCC->LSCTRL & RCC_LSCTRL_LSIRD));
        RCC->LSCTRL &= ~RCC_LSCTRL_LSXSEL_LSE;  
        /* set lsi frequency, (32m/lsc_freq) 32k:1000 , 28.8k 1111.11 , 32.768: 976.56 */
        /* g_lsi_1_syscle_cal_value = 32000000/lsc_freq; */
        switch (lsc_set)
        {
            case BLE_LSC_LSI_32768HZ:
                g_lsi_1_syscle_cal_value = 977;
                break;
            case BLE_LSC_LSI_28800HZ:
                g_lsi_1_syscle_cal_value = 1111;
                break;
            case BLE_LSC_LSI_32000HZ:
            default:
                g_lsi_1_syscle_cal_value = 1000;
                break;
        }
        g_lsi_1_syscle_cnt_value = calib_lsi_clk(); //take 7.5ms
    }
}

void hp_ble_stack_init(struct hp_stack_cfg_t const* p_init)
{
    hp_ble_stack_vtor_init();
    hp_ble_evn_init();
    app_env.ble_msg_handler = p_init->ble_msg_handler;
    app_env.user_msg_handler = p_init->user_msg_handler;
    app_env.iwdg_cycle = p_init->iwdg_cycle;
    if( p_init->lsc_cfg <= BLE_LSC_LSE_32768HZ )
    {
        app_env.lsc_cfg = p_init->lsc_cfg;
    } 
    else
    {
        //set LSI 32000HZ as default
        app_env.lsc_cfg = BLE_LSC_LSI_32000HZ;
    }

    HP_BLE_PATCH_INIT();
    HP_BLE_STACK_INIT();
    hp_ble_task_init();
    prf_init(RWIP_INIT);
    // Reset the stack
    app_send_gapm_reset_cmd();
}

void hp_ble_gap_init(struct hp_gap_params_t const* p_init)
{
    if(p_init != NULL)
    {
        memcpy(&gap_env,p_init,sizeof(struct hp_gap_params_t));
    }
    
    //generate local IRK
    uint8_t tmp_irk[KEY_LEN];
    memcpy(&tmp_irk[0], SEC_DEFAULT_IRK, KEY_LEN);
    for (uint8_t i=0;i<BD_ADDR_LEN;i++)
    {
        tmp_irk[i]|= gap_env.mac_addr.addr[i];
    }
    memcpy(&app_env.loc_irk[0], &tmp_irk[0], KEY_LEN);        

    //set ble address
    memcpy(g_co_default_bdaddr.addr, &gap_env.mac_addr, BD_ADDR_LEN);
    
}


void hp_ble_adv_init(struct hp_adv_params_t const* p_init)
{
    if(p_init != NULL)
    {
        memcpy(&adv_env,p_init, sizeof(struct hp_adv_params_t)); 
    }

    if((adv_env.adv_data_len > ADV_DATA_LEN-3) 
        && (adv_env.beacon_enable == false) )
    {
        adv_env.adv_data_len = ADV_DATA_LEN-3;
    }
    if(adv_env.scan_rsp_data_len > ADV_DATA_LEN)
    {
        adv_env.scan_rsp_data_len = ADV_DATA_LEN;
    }
    if(adv_env.ex_adv_data_len > EXT_ADV_DATA_MAX_LEN-3)
    {
        adv_env.ex_adv_data_len = EXT_ADV_DATA_MAX_LEN-3;
    }
}

bool hp_ble_adv_data_set(uint8_t* p_dat, uint16_t len)
{
    adv_env.adv_data_len = len;
    if((adv_env.adv_data_len > ADV_DATA_LEN-3) 
        && (adv_env.beacon_enable == false) )
    {
        adv_env.adv_data_len = ADV_DATA_LEN-3;
    }
    
    memcpy(adv_env.adv_data,p_dat,adv_env.adv_data_len);
    
    if(app_env.adv_mode == APP_ADV_MODE_FAST || 
       app_env.adv_mode == APP_ADV_MODE_SLOW)
    {
        app_set_adv_data();
        app_env.current_op = 0xff; //cancel next step
        return true;
    }
    return false;
}

bool hp_ble_scan_rsp_data_set(uint8_t* p_dat, uint16_t len)
{
    adv_env.scan_rsp_data_len = len;
    if(adv_env.scan_rsp_data_len > ADV_DATA_LEN)
    {
        adv_env.scan_rsp_data_len = ADV_DATA_LEN;
    }
    
    memcpy(adv_env.scan_rsp_data,p_dat,adv_env.scan_rsp_data_len);
    
    if(app_env.adv_mode == APP_ADV_MODE_FAST || 
       app_env.adv_mode == APP_ADV_MODE_SLOW)
    {
        app_set_scan_rsp_data();
        app_env.current_op = 0xff; //cancel next step
        return true;
    }
    return false;
}

bool hp_ble_ex_adv_data_set(uint8_t* p_dat, uint16_t len)
{
    adv_env.ex_adv_data_len = len;
    if(adv_env.ex_adv_data_len > EXT_ADV_DATA_MAX_LEN-3)
    {
        adv_env.ex_adv_data_len = EXT_ADV_DATA_MAX_LEN-3;
    }
    adv_env.ex_adv_p_data = p_dat;
    if(app_env.adv_mode == APP_ADV_MODE_FAST || 
       app_env.adv_mode == APP_ADV_MODE_SLOW)
    {
        app_set_adv_data();
        app_env.current_op = 0xff; //cancel next step
        return true;
    }
    return false;
}

void hp_ble_adv_start(void)
{
    if(app_env.adv_state == APP_ADV_STATE_STOPPING)
    {
        app_env.adv_mode = APP_ADV_MODE_ENABLE;
        return;
    }

    switch (app_env.adv_mode)
    {
        case APP_ADV_MODE_IDLE:
            //on idle mode then enable.            
            app_env.adv_mode = APP_ADV_MODE_ENABLE;
            break;

        case APP_ADV_MODE_ENABLE:
        case APP_ADV_MODE_STOP:
            //on ready mode then start
            if(adv_env.directed_adv.enable)
            {
                app_env.adv_mode = APP_ADV_MODE_DIRECTED;
            }
            else if(adv_env.fast_adv.enable)
            {
                app_env.adv_mode = APP_ADV_MODE_FAST;
            }
            else if(adv_env.slow_adv.enable)
            {
                app_env.adv_mode = APP_ADV_MODE_SLOW;
            }
            else
            {
                app_env.adv_mode = APP_ADV_MODE_STOP;
            }
            app_create_advertising();
            break;
        default:
            //on started mode do nothing 
            break;
    }
    
    
}

void hp_ble_adv_stop(void)
{
    switch (app_env.adv_mode)
    {
        case APP_ADV_MODE_IDLE:
        case APP_ADV_MODE_ENABLE:
            //on idle and enable mode then disable
            app_env.adv_mode = APP_ADV_MODE_STOP;
            break;
        case APP_ADV_MODE_STOP:
            //on stop mode then do nothing     
            break;        
        case APP_ADV_MODE_DIRECTED:
        case APP_ADV_MODE_FAST:
        case APP_ADV_MODE_SLOW:
        default:
            //on started mode then stop
            app_env.adv_mode = APP_ADV_MODE_STOP;
            app_stop_advertising();
            break;
    }
}

void hp_ble_disconnect(void)
{
    struct gapc_disconnect_cmd *p_cmd = KE_MSG_ALLOC(GAPC_DISCONNECT_CMD,
                                                   KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                                   gapc_disconnect_cmd);

    HP_LOG_DEBUG("%s\r\n", __func__);
    p_cmd->operation = GAPC_DISCONNECT;
    p_cmd->reason    = CO_ERROR_REMOTE_USER_TERM_CON;

    // Send the message
    ke_msg_send(p_cmd);
}




/* Master device API*/

void hp_ble_scan_init(struct hp_scan_params_t *p_init)
{
    if(p_init != NULL)
    {
        memcpy(&scan_env,p_init, sizeof(struct hp_scan_params_t)); 
    }
}



void hp_ble_create_scan(void)
{
    HP_LOG_DEBUG("%s\r\n", __func__);
    if(app_env.scan_actv_idx == 0xFF)
    {
        // Prepare the GAPM_ACTIVITY_CREATE_CMD message
        struct gapm_activity_create_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_CREATE_CMD,
                                                                  TASK_GAPM, TASK_APP,
                                                                  gapm_activity_create_cmd);
        // Set operation code
        p_cmd->operation = GAPM_CREATE_SCAN_ACTIVITY;
        // Fill the allocated kernel message
        p_cmd->own_addr_type = gap_env.mac_addr_type;//GAPM_STATIC_ADDR;

        // Send the message
        ke_msg_send(p_cmd);
    }
}

void hp_ble_start_scan(void)
{
    HP_LOG_DEBUG("%s,%x\r\n", __func__,app_env.scan_actv_idx);
    
    if(app_env.scan_actv_idx == 0xFF)
    {
        scan_env.scan_enable = true;
        //creat scan active if not created yet
        hp_ble_create_scan();
        return;
    }
    else if(!scan_env.scan_enable)
    {
        //scan already started
        return;
    }
    scan_env.scan_enable = false;
    // Prepare the GAPM_ACTIVITY_START_CMD message
    struct gapm_activity_start_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_START_CMD,
                                                         TASK_GAPM, TASK_APP,
                                                         gapm_activity_start_cmd);

    p_cmd->operation = GAPM_START_ACTIVITY;
    p_cmd->actv_idx = app_env.scan_actv_idx;

    p_cmd->u_param.scan_param.type                      = scan_env.type;
    p_cmd->u_param.scan_param.dup_filt_pol              = scan_env.dup_filt_pol;
    p_cmd->u_param.scan_param.scan_param_1m.scan_intv   = scan_env.scan_intv;
    p_cmd->u_param.scan_param.scan_param_1m.scan_wd     = scan_env.scan_wd;
    p_cmd->u_param.scan_param.duration                  = scan_env.duration;
    p_cmd->u_param.scan_param.period                    = scan_env.period;
    //scan 1M as default
    p_cmd->u_param.scan_param.prop                      = GAPM_SCAN_PROP_PHY_1M_BIT;
    llhwc_modem_setmode(GAP_PHY_1MBPS);
    
    if(scan_env.phy_coded_enable)
    {
        p_cmd->u_param.scan_param.prop                        |= GAPM_SCAN_PROP_PHY_CODED_BIT;
        p_cmd->u_param.scan_param.scan_param_coded.scan_intv   = scan_env.scan_intv;
        p_cmd->u_param.scan_param.scan_param_coded.scan_wd     = scan_env.scan_wd;
        llhwc_modem_setmode(GAP_PHY_CODED);
    }

    if(scan_env.prop_active_enable)
    {
        p_cmd->u_param.scan_param.prop |= GAPM_SCAN_PROP_ACTIVE_1M_BIT|GAPM_SCAN_PROP_ACTIVE_CODED_BIT;
    }    
    
    app_env.current_op      = CURRENT_OP_START_SCAN;
    app_env.target_found    = false;
    // Send the message
    ke_msg_send(p_cmd);
}

void hp_ble_delete_scan(void)
{
    HP_LOG_DEBUG("%s\r\n", __func__);
    if(app_env.scan_actv_idx != 0xff)
    {
        struct gapm_activity_delete_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_DELETE_CMD,
                                                              TASK_GAPM, TASK_APP,
                                                              gapm_activity_delete_cmd);
        // Set operation code
        p_cmd->operation = GAPM_DELETE_ACTIVITY;
        p_cmd->actv_idx = app_env.scan_actv_idx;
        app_env.current_op   = CURRENT_OP_NULL;        
        // Send the message
        ke_msg_send(p_cmd);   
    }
}

void hp_ble_stop_scan(void)
{
    HP_LOG_DEBUG("%s,%x\r\n", __func__,app_env.scan_actv_idx);

    if(app_env.scan_actv_idx != 0xff && app_env.current_op != CURRENT_OP_STOP_SCAN)
    {
        struct gapm_activity_stop_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_STOP_CMD,
                                                              TASK_GAPM, TASK_APP,
                                                              gapm_activity_stop_cmd);
        // Set operation code
        p_cmd->operation = GAPM_STOP_ACTIVITY;
        p_cmd->actv_idx = app_env.scan_actv_idx;
        app_env.current_op = CURRENT_OP_STOP_SCAN;
        // Send the message
        ke_msg_send(p_cmd);
    }
}

void hp_ble_create_init(void)
{
    HP_LOG_DEBUG("%s\r\n", __func__);
    if(app_env.init_actv_idx == 0xFF)
    {
        // Prepare the GAPM_ACTIVITY_CREATE_CMD message
        struct gapm_activity_create_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_CREATE_CMD,
                                                              TASK_GAPM, TASK_APP,
                                                              gapm_activity_create_cmd);
        // Set operation code
        p_cmd->operation = GAPM_CREATE_INIT_ACTIVITY;
        // Fill the allocated kernel message
        p_cmd->own_addr_type = gap_env.mac_addr_type;
        // Send the message
        ke_msg_send(p_cmd);
    }
}

void hp_ble_start_init(uint8_t *addr, uint8_t addr_type)
{
    HP_LOG_DEBUG("%s,%x\r\n", __func__,app_env.init_actv_idx);
    if(app_env.init_actv_idx == 0xFF)
    {
        //creat init active if not created yet        
        hp_ble_create_init();
        
        app_env.target_addr_type = addr_type;
        memcpy(app_env.target_addr.addr, addr, 6);       
        app_env.target_found    = true;
        scan_env.connect_enable = true;
        return;
    }
    //set init timeout timer
    if(scan_env.initiating_timeout)
    {
        ke_timer_set(APP_INIT_TIMEOUT_EVT,TASK_APP,scan_env.initiating_timeout);
    }
    
    // Prepare the GAPM_ACTIVITY_START_CMD message
    struct gapm_activity_start_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_START_CMD,
                                                         TASK_GAPM, TASK_APP,
                                                         gapm_activity_start_cmd);

    p_cmd->operation = GAPM_START_ACTIVITY;
    p_cmd->actv_idx = app_env.init_actv_idx;

    p_cmd->u_param.init_param.type = GAPM_INIT_TYPE_DIRECT_CONN_EST;
    p_cmd->u_param.init_param.prop = GAPM_INIT_PROP_1M_BIT;
    p_cmd->u_param.init_param.scan_param_1m.scan_intv       = 0x320;
    p_cmd->u_param.init_param.scan_param_1m.scan_wd         = 0x320;
    p_cmd->u_param.init_param.conn_param_1m.conn_intv_min   = gap_env.dev_conn_param.intv_min; 
    p_cmd->u_param.init_param.conn_param_1m.conn_intv_max   = gap_env.dev_conn_param.intv_max;
    p_cmd->u_param.init_param.conn_param_1m.conn_latency    = gap_env.dev_conn_param.latency; 
    p_cmd->u_param.init_param.conn_param_1m.supervision_to  = gap_env.dev_conn_param.time_out; 
    p_cmd->u_param.init_param.conn_param_1m.ce_len_min      = 0;       
    p_cmd->u_param.init_param.conn_param_1m.ce_len_max      = 0x140; 
    
    if(scan_env.phy_coded_enable)
    {
        p_cmd->u_param.init_param.prop |= GAPM_INIT_PROP_CODED_BIT;
        p_cmd->u_param.init_param.scan_param_coded.scan_intv       = 0x320;
        p_cmd->u_param.init_param.scan_param_coded.scan_wd         = 0x320;
        p_cmd->u_param.init_param.conn_param_coded.conn_intv_min   = gap_env.dev_conn_param.intv_min; 
        p_cmd->u_param.init_param.conn_param_coded.conn_intv_max   = gap_env.dev_conn_param.intv_max;
        p_cmd->u_param.init_param.conn_param_coded.conn_latency    = gap_env.dev_conn_param.latency; 
        p_cmd->u_param.init_param.conn_param_coded.supervision_to  = gap_env.dev_conn_param.time_out; 
        p_cmd->u_param.init_param.conn_param_coded.ce_len_min      = 0;       
        p_cmd->u_param.init_param.conn_param_coded.ce_len_max      = 0x140; 
    }

    p_cmd->u_param.init_param.peer_addr.addr_type = addr_type;

    memcpy(p_cmd->u_param.init_param.peer_addr.addr.addr, addr, 6);
    
    if(p_cmd->u_param.init_param.prop & GAPM_INIT_PROP_2M_BIT)
    {
        llhwc_modem_setmode(GAP_PHY_2MBPS);
    }
    else if(p_cmd->u_param.init_param.prop & GAPM_INIT_PROP_CODED_BIT)
    {
        llhwc_modem_setmode(GAP_PHY_CODED);
    }
    else{
        llhwc_modem_setmode(GAP_PHY_1MBPS);
    }
    
    app_env.current_op = CURRENT_OP_START_INIT;    
    // Send the message
    ke_msg_send(p_cmd);
}

void hp_ble_stop_init(void)
{
    HP_LOG_DEBUG("%s,init_actv_idx = 0x%x\r\n", __func__,app_env.init_actv_idx);
    if (app_env.init_actv_idx != 0xFF)
    {
        struct gapm_activity_stop_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_STOP_CMD,
                                                              TASK_GAPM, TASK_APP,
                                                              gapm_activity_stop_cmd);
        // Set operation code
        p_cmd->operation = GAPM_STOP_ACTIVITY;
        p_cmd->actv_idx  = app_env.init_actv_idx;
        app_env.current_op = CURRENT_OP_STOP_INIT;
        // Send the message
        ke_msg_send(p_cmd);
    }
}

void hp_ble_delete_init(void)
{
    HP_LOG_DEBUG("%s\r\n", __func__);
    if (app_env.init_actv_idx != 0xFF)
    {
        struct gapm_activity_delete_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_DELETE_CMD,
                                                              TASK_GAPM, TASK_APP,
                                                              gapm_activity_delete_cmd);
        // Set operation code
        p_cmd->operation = GAPM_DELETE_ACTIVITY;
        p_cmd->actv_idx  = app_env.init_actv_idx;
        
        app_env.current_op    = CURRENT_OP_NULL;
        app_env.init_actv_idx = 0xFF;    
        // Send the message
        ke_msg_send(p_cmd);
    }
}


bool hp_ble_scan_data_find(uint8_t types, const uint8_t *p_filter_data, uint8_t *p_data, uint8_t len )
{
    uint8_t pack_len;
    uint8_t pack_type;
    while(len)
    {
        // check if pack incomplete
        if(len < (*p_data)+1 || len < 3)
        {
            break;
        }
        pack_len = (*p_data)-1; // get pack len
        p_data++;
        pack_type = *p_data; // get pack type
        p_data++;
        if(pack_type == types) //
        {
            if(memcmp(p_data,p_filter_data,pack_len) == 0)
            {
                return true;
            }
        }
        p_data += pack_len; //next pack
        len    -= (pack_len+2);
    }

    return false;
}

/**
 * @brief Search device name by mask (last N digits)
 * @param mask_data - the mask pattern to search for
 * @param p_data - advertising data
 * @param len - length of advertising data
 * @return true if found, false otherwise
 */
bool hp_ble_scan_data_find_by_mask(const char *mask_data, uint8_t mask_len, 
                                   uint8_t *p_data, uint8_t len)
{
    uint8_t pack_len;
    uint8_t pack_type;
    
    while(len)
    {
        // check if pack incomplete
        if(len < (*p_data)+1 || len < 3)
        {
            break;
        }
        pack_len = (*p_data)-1; // get pack len
        p_data++;
        pack_type = *p_data; // get pack type
        p_data++;
        
        if(pack_type == GAP_AD_TYPE_COMPLETE_NAME || 
           pack_type == GAP_AD_TYPE_SHORTENED_NAME) 
        {
            // If device name is shorter than mask length, skip
            if(pack_len < mask_len)
                continue;
                
            // Extract the last 'mask_len' characters from device name
            uint8_t *device_name_end = p_data + pack_len;
            uint8_t *name_part_to_compare = device_name_end - mask_len;
            
            // Compare the last part of device name with mask
            if(memcmp(name_part_to_compare, mask_data, mask_len) == 0)
            {
                return true;
            }
        }
        p_data += pack_len; //next pack
        len    -= (pack_len+2);
    }

    return false;
}

/* Used before connection operation or set connection parameter*/
void hp_ble_set_active_connection (uint8_t  conidx)
{
    app_env.conidx = conidx;
}

uint8_t hp_ble_get_active_connection (void)
{
    return app_env.conidx;
}

/* check the connection status by indx: true is connected,false is disconnected */
bool hp_ble_get_connection_state(uint8_t conidx)
{
    bool status = false;
    if ((app_env.conn_env[conidx].conidx != GAP_INVALID_CONIDX ) && (conidx < APP_CON_IDX_MAX) )
    {
        status = true;
    }
    return status;
}

/* Get the connection number*/
uint8_t hp_ble_get_connection_num (void)
{
    uint8_t  conidx_cnt = 0;
    for (uint8_t i =0;i<APP_CON_IDX_MAX;i++)
    {
        if (app_env.conn_env[i].conidx != GAP_INVALID_CONIDX)
            conidx_cnt ++;
    }
    return conidx_cnt;
}

/* Get the slave connection number, use at one master one slave example only*/
uint8_t hp_ble_get_conidx_by_role (uint8_t role)
{
    uint8_t  conidx = GAP_INVALID_CONIDX;
    for (uint8_t i =0;i<APP_CON_IDX_MAX;i++)
    {
        if ((app_env.conn_env[i].conidx != GAP_INVALID_CONIDX) && (app_env.conn_env[i].role == role))
        {
            conidx = app_env.conn_env[i].conidx;
            break;
        }
    }
    return conidx;
}
/* Get the master connection number*/
uint8_t hp_ble_master_connection_num (void)
{
    uint8_t  conidx_cnt = 0;
    for (uint8_t i =0;i<APP_CON_IDX_MAX;i++)
    {
        if ((app_env.conn_env[i].conidx != GAP_INVALID_CONIDX) && (app_env.conn_env[i].role == ROLE_MASTER))
            conidx_cnt ++;
    }
    return conidx_cnt;
}

/* Get the slave connection number*/
uint8_t hp_ble_slave_connection_num (void)
{
    uint8_t  conidx_cnt = 0;
    for (uint8_t i =0;i<APP_CON_IDX_MAX;i++)
    {
        if ((app_env.conn_env[i].conidx != GAP_INVALID_CONIDX) && (app_env.conn_env[i].role == ROLE_SLAVE))
            conidx_cnt ++;
    }
    return conidx_cnt;
}


void hp_ble_resolv_addr(struct gap_sec_key *irk, uint8_t nb_key,struct bd_addr* addr)
{
    // Prepare the GAPM_RESOLV_ADDR_CMD message
    struct gapm_resolv_addr_cmd *p_cmd = KE_MSG_ALLOC_DYN(GAPM_RESOLV_ADDR_CMD,
                                                     TASK_GAPM, TASK_APP,
                                                     gapm_resolv_addr_cmd,KEY_LEN *nb_key);
    // Set operation code
    p_cmd->operation  = GAPM_RESOLV_ADDR;
    p_cmd->nb_key  = nb_key;

    memcpy(&(p_cmd->addr.addr[0]), addr, sizeof(bd_addr_t));
    memcpy(&(p_cmd->irk[0]), irk, KEY_LEN*nb_key);
    HP_LOG_DEBUG("%s\r\n", __func__);

    // Send the message
    ke_msg_send(p_cmd);
}

/* Set resolving list */
void hp_ble_list_set_ral(struct gap_ral_dev_info *ral_list,uint8_t ral_cnt)
{
    struct gapm_list_set_ral_cmd *p_cmd = KE_MSG_ALLOC_DYN(GAPM_LIST_SET_CMD,
                                                       TASK_GAPM, TASK_APP,
                                                       gapm_list_set_ral_cmd,
                                                       (ral_cnt * sizeof(struct gap_ral_dev_info)));

    HP_LOG_DEBUG("%s,ral_cnt:%d\r\n", __func__,ral_cnt);
    p_cmd->operation = GAPM_SET_RAL;
    p_cmd->size = ral_cnt;
    for (uint8_t cnt = 0; cnt < ral_cnt; cnt++)
    {
        ral_list[cnt].priv_mode = 0;
        memcpy(&p_cmd->ral_info[cnt], &ral_list[cnt], sizeof(struct gap_ral_dev_info));  
        HP_LOG_DEBUG("set_ral:%d,%d,%d,%d,%d\r\n",ral_list[cnt].priv_mode,\
                                                  ral_list[cnt].local_irk[0],\
                                                  ral_list[cnt].peer_irk[0],\
                                                  ral_list[cnt].addr.addr_type,\
                                                  ral_list[cnt].addr.addr.addr[0]);
    }
    ke_msg_send(p_cmd);
}

/* Set white list */
void hp_ble_list_set_wl(struct gap_bdaddr *wl_addr,uint8_t wl_cnt)
{
    struct gapm_list_set_wl_cmd *p_cmd = KE_MSG_ALLOC_DYN(GAPM_LIST_SET_CMD,
                                                       TASK_GAPM, TASK_APP,
                                                       gapm_list_set_wl_cmd,
                                                       wl_cnt * sizeof(struct gap_bdaddr));

    HP_LOG_DEBUG("%s,wl_cnt:%d\r\n", __func__,wl_cnt);
    p_cmd->operation = GAPM_SET_WL;
    p_cmd->size = wl_cnt;
    for (uint8_t cnt = 0; cnt < wl_cnt; cnt++)
    {
        memcpy(&p_cmd->wl_info[cnt], &wl_addr[cnt], sizeof(struct gap_bdaddr));  
        HP_LOG_DEBUG("set_wl:%x,%x,%x,%x,%x,%x;addr type: %d\r\n", wl_addr[cnt].addr.addr[0],\
                                                                   wl_addr[cnt].addr.addr[1],\
                                                                   wl_addr[cnt].addr.addr[2],\
                                                                   wl_addr[cnt].addr.addr[3],\
                                                                   wl_addr[cnt].addr.addr[4],\
                                                                   wl_addr[cnt].addr.addr[5],\
                                                                   wl_addr[cnt].addr_type);
    }
    ke_msg_send(p_cmd);
}

void hp_ble_set_channel_map( uint8_t* newchmap)
{
    HP_LOG_DEBUG("%s\r\n", __func__);
    struct gapm_set_channel_map_cmd *p_cmd = KE_MSG_ALLOC(GAPM_SET_CHANNEL_MAP_CMD,
                                                          TASK_GAPM, TASK_APP,
                                                          gapm_set_channel_map_cmd);
    // Set operation code
    p_cmd->operation = GAPM_SET_CHANNEL_MAP;
    memcpy(&(p_cmd->chmap.map), newchmap, sizeof(le_chnl_map_t));
    // Send the message
    ke_msg_send(p_cmd);   
}


/* enter rf test mode */
void hp_ble_prod_test_cmd_send(struct gapm_le_test_mode_ctrl_cmd *p_params, bool cw_mode)
{
    struct gapm_le_test_mode_ctrl_cmd *p_cmd = KE_MSG_ALLOC(GAPM_LE_TEST_MODE_CTRL_CMD,
                                                     TASK_GAPM, TASK_APP,
                                                     gapm_le_test_mode_ctrl_cmd);
    
    if(cw_mode)
    {
        REG32(MODEM_BASE + 0xc6 * 4)  = 0;
        REG32(MODEM_BASE + 0xc7 * 4)  = 0;
        REG32(MODEM_BASE + 0xc8 * 4)  = 0;
        ble_rftestcntl_infinitetx_setf(1);
        p_params->tx_pkt_payload = 0;
        p_params->operation = GAPM_LE_TEST_TX_START;
    }
    
    memcpy(p_cmd,p_params,sizeof(struct gapm_le_test_mode_ctrl_cmd));
    llhwc_modem_setmode(p_params->phy);
    // Send the message
    ke_msg_send(p_cmd);
}



/**
 * @brief  set the radio fact frequency to calibrate it.
 * @param  fact_freq: the fact frequency test in CW mode, uint is Hz.
 * @param  std_freq: the standard frequency shoud get in CW mode.
 * @return void
 * @note   for example: you can use RFTest example to test the CW in 0 channel, 
 *         it should be 2402000000 Hz, but we get 2401950000, it meam it offset 50k Hz.
 *         We can call the function: hp_ble_radio_calibration(2401950000, 2402000000);
 */
void hp_ble_radio_calibration(uint32_t fact_freq,uint32_t std_freq)
{
    float    Ndiv = (float)std_freq / 8000000;
    uint32_t mco_freq = fact_freq / Ndiv;
    
    extern void radio_hse_mco_set(uint32_t mco_freq);
    radio_hse_mco_set(mco_freq);
}

/**
 * @brief  set the radio tx power.
 * @param  tx_pwr: the power value to set, it can be set as below.
            TX_POWER_0_DBM 
            TX_POWER_Neg2_DBM
            TX_POWER_Neg4_DBM
            TX_POWER_Neg8_DBM
            TX_POWER_Neg15_DBM
            TX_POWER_Neg20_DBM
            TX_POWER_Pos2_DBM
            TX_POWER_Pos3_DBM
            TX_POWER_Pos4_DBM
            TX_POWER_Pos6_DBM 
 * @return 
 * @note   
 */
void hp_ble_radio_power_set(rf_tx_power_t tx_pwr)
{
    rf_tx_power_set(tx_pwr);
}


static rwip_time_t reload_time = {0};

/**
 * @brief  Config the peripheral.
 * @param  
 * @return 
 * @note   
 */
void hp_peripheral_config(void)
{
    while(!(RCC->CTRL&RCC_CTRL_HSERDF));//wait HSE ready
    hp_ble_lsc_config(app_env.lsc_cfg);

    /* Check if the system has resumed from IWDG reset */
    if (((RCC->CTRLSTS&(RCC_CTRLSTS_PORRSTF|RCC_CTRLSTS_IWDGRSTF)) == RCC_CTRLSTS_IWDGRSTF))
    {
        /* IWDGRST flag set*/
        HP_LOG_WARNING("IWDGRST is set, reset from IWDG!!!\r\n");
    }
    /* Clear reset flags */
    RCC->CTRLSTS |= RCC_CTRLSTS_RMRSTF;
    RCC->CTRLSTS &= ~RCC_CTRLSTS_RMRSTF;
    //iwdg
    hp_iwdg_config();
    rwip_time_t current_time = rwip_time_get();
    reload_time.hs = current_time.hs;
    
    #if HP_LSI_CALIB_EN
    //enalbe lsi measurement
    if(app_env.lsc_cfg != BLE_LSC_LSE_32768HZ)   
    {
        hp_sleep_lock_acquire();//pending sleep for calib
        RCC->OSCFCCR &= ~(0xFF<< 8);
        RCC->OSCFCCR |= (LSI_CLOCK_CNT_CYCLES <<8);
        RCC->OSCFCCR |= 1;
        //start lis calib timer
        ke_timer_set(APP_LSI_CALIB_EVT,TASK_APP,1);
    }
    #endif
    
    //ble irq
    NVIC_SetPriority(BLE_SW_IRQn, 3);
    NVIC_SetPriority(BLE_FIFO_IRQn, 4); 
}


/**
 * @brief  Config the iwdg if iwdg_cycle is valid.
 * @param  
 * @return 
 * @note   
 */
void hp_iwdg_config(void)
{
    //Do not add any printf log here, because it call after wakeup!!!
    if(app_env.iwdg_cycle > 10) // iwdg_cycle < 4000ms
    {
        RCC->APB1PCLKEN |= RCC_APB1_PERIPH_PWR;
        RCC->DBGMCU_CR |= DBG_IWDG_STOP;
        /* Enable write access to IWDG_PR and IWDG_RLR registers */
        IWDG->KEY = IWDG_WRITE_ENABLE;
        /* IWDG counter clock: LSI/32 */
        IWDG->PREDIV = IWDG_PRESCALER_DIV32; 
        /* Set counter reload value to obtain 300ms IWDG TimeOut.*/
        IWDG->RELV = app_env.iwdg_cycle;
        /* Reload IWDG counter */
        IWDG->KEY = 0xAAAA;
        /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
        IWDG->KEY = 0xCCCC; 
    }
}
/**
 * @brief  Reload the iwdg if ((current_time-reload_time)/3.2) > iwdg_cycle.  
 * @param  
 * @return 
 * @note   
 */
void hp_iwdg_reload(void)
{
    rwip_time_t current_time = rwip_time_get();
    int32_t pass_hs = CLK_DIFF(reload_time.hs,current_time.hs );
    if((pass_hs>>1) > (app_env.iwdg_cycle))
    {
         /* Reload IWDG counter */
        IWDG->KEY = 0xAAAA;
        
        HP_LOG_DEBUG("%s, C_time:%d, R_time%d\r\n",__func__,current_time.hs,reload_time.hs);
        reload_time.hs = current_time.hs;
    }
        
    
}
#endif //(BLE_APP_PRESENT)

/// @} APP
