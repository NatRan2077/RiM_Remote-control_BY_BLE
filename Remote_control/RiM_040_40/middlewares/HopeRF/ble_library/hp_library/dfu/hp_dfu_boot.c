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
 * @file hp_dfu_boot.c
 * @version v1.0.1
 *
  */

/** @addtogroup 
 * @{
 */

 /* Includes ------------------------------------------------------------------*/
#include "hp_dfu_boot.h"
#include "cmt453x.h"
#include "dfu_crc.h"
#include "hp_error.h"
/* Private typedef -----------------------------------------------------------*/
typedef void (*func_ptr_t)(void);
/* Private define ------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
HP_Bootsetting_t hp_bootsetting __attribute__((at(HP_BOOTSETTING_START_ADDRESS))) __attribute__((used));
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/**
 * @brief Inter program jump function.
 * @param[in] address program flash address.
 * @return none, function will not return
 */
void hp_dfu_boot_jump(uint32_t address)
{
    
    uint32_t JumpAddress;
    func_ptr_t JumpToApplication;
    JumpAddress = *(__IO uint32_t *)(address + 4);
    JumpToApplication = (func_ptr_t)JumpAddress;
    __set_MSP(*(__IO uint32_t *)address);
    JumpToApplication();

}


/**
 * @brief Write force usart image update variable in bootsetting.
 * @param[in] none.
 * @return none
 */
bool hp_dfu_boot_force_usart_dfu(void)
{
    HP_Bootsetting_t hp_bootsetting_tmp;
    memcpy(&hp_bootsetting_tmp,&hp_bootsetting,sizeof(HP_Bootsetting_t));
    hp_bootsetting_tmp.master_boot_force_update = HP_BOOTSETTING_MASTER_BOOT_FORCE_UPDATE_YES;
    hp_bootsetting_tmp.crc = dfu_crc32((uint8_t *)&hp_bootsetting_tmp.crc + 4, sizeof(HP_Bootsetting_t) - 4);
    Qflash_Erase_Sector(HP_BOOTSETTING_START_ADDRESS);
    Qflash_Write(HP_BOOTSETTING_START_ADDRESS, (uint8_t *)&hp_bootsetting_tmp, sizeof(HP_Bootsetting_t));            
    
    if(hp_bootsetting_tmp.crc == dfu_crc32((uint8_t *)((uint32_t *)(HP_BOOTSETTING_START_ADDRESS + 4)), sizeof(HP_Bootsetting_t) - 4))
    {
        NVIC_SystemReset();
        return ERROR_SUCCESS;
    }
    else
    {
        return ERROR_INTERNAL;
    }
}








