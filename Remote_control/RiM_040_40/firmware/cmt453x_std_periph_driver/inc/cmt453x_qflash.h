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
 * @file cmt453x_qflash.h
 
 * @version v1.0.2
 *
  */
#ifndef __CMT453X_QFLASH_H__
#define __CMT453X_QFLASH_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#include "cmt453x.h"
/** @addtogroup CMT453X_StdPeriph_Driver
 * @{
 */

/** @addtogroup QFLASH
 * @{
 */
    
/** @addtogroup QFLASH_Defines
 * @{
 */ 
#define Qflash_Erase_Sector_Raw     Qflash_Erase_Sector 
#define Qflash_Write_Raw            Qflash_Write
 
#define FLASH_PAGE_SIZE             0x100
#define FLASH_SECTOR_SIZE           0x1000


#define OTP_SECTOR_SIZE                0x200
#define OTP_ADDRESS_SECTOR1            0x1000 //(Locked)
#define OTP_ADDRESS_SECTOR2            0x2000 
#define OTP_ADDRESS_SECTOR3            0x3000 
#define OTP_ADDRESS_TO_SECTOR(addr)    (addr&0x7000)
#define OTP_ADDRESS_CHECK(address) (((address>=0x1000)&&(address<0x1200))||\
                                    ((address>=0x2000)&&(address<0x2200))||\
                                    ((address>=0x3000)&&(address<0x3200)))

/**
 * @}
 */
 
/** @addtogroup QFLASH_ReturnMsg
 * @{
 */
typedef enum{
    FlashOperationSuccess,
    FlashWriteRegFailed,
    FlashTimeOut,
    FlashIsBusy,
    FlashQuadNotEnable,
    FlashAddressInvalid,
}ReturnMsg; 
/**
 * @}
 */

/** @addtogroup RCC_Exported_Functions
 * @{
 */
void Qflash_Init(void);
uint32_t Qflash_Erase_Sector(uint32_t address);
uint32_t Qflash_Write(uint32_t address, uint8_t* p_data, uint32_t len);
uint32_t Qflash_Read(uint32_t address, uint8_t* p_data, uint32_t len);

uint32_t OTPTrim_Read(uint32_t address, uint8_t* p_data, uint32_t byte_length);
uint32_t OTPTrim_Write(uint32_t address, uint8_t* p_data, uint32_t byte_length);
uint32_t OTPTrim_Erase(uint32_t address);
uint32_t OTPTrim_Lock(uint32_t address);
    
#ifdef __cplusplus
}
#endif

#endif /* __CMT453X_QFLASH_H__ */
/**
 * @}
 */    

/**
 * @}
 */

/**
 * @}
 */
