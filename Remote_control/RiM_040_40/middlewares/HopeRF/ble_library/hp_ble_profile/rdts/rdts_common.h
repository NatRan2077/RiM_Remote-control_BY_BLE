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
 * @file rdts_common.h
 
 * @version v1.0.0
 *
  */

#ifndef __RDTS_COMMON_H__
#define __RDTS_COMMON_H__

#include "gattc_task.h"

/** 
 * @brief Validate the value of the Client Characteristic CFG.
 * @param[in] is_notification indicates whether the CFG is Notification (true) or Indication (false)
 * @param[in] param Pointer to the parameters of the message.
 * @return status. 
 */
int check_client_char_cfg(bool is_notification, const struct gattc_write_req_ind *param);

/** 
 * @brief Find the handle of the Characteristic Value having as input the Client Characteristic CFG handle
 * @param[in] cfg_handle the Client Characteristic CFG handle
 * @return the corresponding value handle 
 */
uint16_t get_value_handle(uint16_t cfg_handle);

/** 
 * @brief Find the handle of Client Characteristic CFG having as input the Characteristic value handle
 * @param[in] value_handle the Characteristic value handle
 * @return the corresponding Client Characteristic CFG handle 
 */
uint16_t get_cfg_handle(uint16_t value_handle);

#if (BLE_RDTSS_SERVER)
/** 
 * @brief Compute the handle of a given attribute based on its index
 * @details Specific to raw data transfer server in 128bit uuid
 * @param[in] att_idx attribute index
 * @return the corresponding handle 
 */
uint16_t rdtss_get_att_handle(uint8_t att_idx);

/** 
 * @brief Compute the handle of a given attribute based on its index
 * @details Specific to raw data transfer server in 128bit uuid
 * @param[in] handle attribute handle
 * @param[out] att_idx attribute index
 * @return high layer error code 
 */
uint8_t rdtss_get_att_idx(uint16_t handle, uint8_t *att_idx);
#endif // (BLE_RDTSS_SERVER)


#if (BLE_RDTSS_16BIT_SERVER)
/** 
 * @brief Compute the handle of a given attribute based on its index
 * @details Specific to raw data transfer server in 16bit uuid
 * @param[in] att_idx attribute index
 * @return the corresponding handle 
 */
uint16_t rdtss_16bit_get_att_handle(uint8_t att_idx);

/** 
 * @brief Compute the handle of a given attribute based on its index
 * @details Specific to raw data transfer server in 16bit uuid
 * @param[in] handle attribute handle
 * @param[out] att_idx attribute index
 * @return high layer error code 
 */
uint8_t rdtss_16bit_get_att_idx(uint16_t handle, uint8_t *att_idx);
#endif // (BLE_RDTSS_16BIT_SERVER)


#endif // __RDTS_COMMON_H__
