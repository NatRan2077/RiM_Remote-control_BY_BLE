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
 * @file hp_dfu_boot.h
 
 * @version v1.0.0
 *
  */

 /** @addtogroup 
 * @{
 */
#ifndef __HP_DFU_BOOT_H__
#define __HP_DFU_BOOT_H__


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Public define ------------------------------------------------------------*/
/* Public typedef -----------------------------------------------------------*/
typedef struct
{
    uint32_t start_address;
    uint32_t size;
    uint32_t crc;
    uint32_t version;
    uint32_t activation;
    uint32_t reserve[5];
}HP_Bank_t;

typedef struct
{
    uint32_t crc;
    uint32_t master_boot_force_update;
    HP_Bank_t app1;
    HP_Bank_t app2;
    HP_Bank_t ImageUpdate;
    uint8_t public_key[64];
}HP_Bootsetting_t;
/* Public define ------------------------------------------------------------*/  
extern int Image$$ER_IROM1$$Base;
#define CURRENT_APP_START_ADDRESS                      (uint32_t)&Image$$ER_IROM1$$Base

#define HP_MASTERBOOT_START_ADDRESS                    (0x01000000)
#define HP_MASTERBOOT_SIZE                             (0x2000)                                                                 
#define HP_BOOTSETTING_START_ADDRESS                   (0x01002000)
#define HP_BOOTSETTING_SIZE                            (0x1000)
#define HP_APP_DATA_START_ADDRESS                      (0x01003000)
#define HP_APP_DATA_SIZE                               (0x1000)
#define HP_APP1_START_ADDRESS                          (0x01004000)
#define HP_APP1_DEFAULT_SIZE                           (0x1C000)
#define HP_APP2_START_ADDRESS                          (0x01020000)
#define HP_APP2_DEFAULT_SIZE                           (0x1C000)
#define HP_IMAGE_UPDATE_START_ADDRESS                  (0x0103C000)
#define HP_IMAGE_UPDATE_SIZE                           (0x4000)

#define HP_BOOTSETTING_ACTIVATION_YES                       (0x00000001)
#define HP_BOOTSETTING_ACTIVATION_NO                        (0xFFFFFFFF)
#define HP_BOOTSETTING_MASTER_BOOT_FORCE_UPDATE_YES         (0x00000001)
#define HP_BOOTSETTING_MASTER_BOOT_FORCE_UPDATE_NO          (0xFFFFFFFF)

/* Public variables ---------------------------------------------------------*/
extern HP_Bootsetting_t hp_bootsetting;
/* Public function prototypes -----------------------------------------------*/
void hp_dfu_boot_jump(uint32_t address);
bool hp_dfu_boot_force_usart_dfu(void);


#ifdef __cplusplus
}
#endif


#endif //__HP_DFU_BOOT_H__
