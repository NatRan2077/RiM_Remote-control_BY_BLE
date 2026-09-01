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
 * @file hp_aes.h
 
 * @version v1.0.0
 *
  */

/** @addtogroup 
 * @{
 */
#ifndef __HP_AES_H__
#define __HP_AES_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes -----------------------------------------------------------------*/
#include "global_func.h"
#include "aes.h"
#include "aes_int.h"
/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/  
#ifndef AES_DECRYPT_ENABLE
#define AES_DECRYPT_ENABLE 0
#endif
#define DECRYPT_SRC_MARK  0xffffffff
/* Public constants ---------------------------------------------------------*/
/* Public function prototypes -----------------------------------------------*/
void hp_ase_swap_array(uint8_t * p_out, uint8_t const * p_in, uint8_t size);
void hp_aes_xor_128_swap(uint8_t* result, const uint8_t* a, const uint8_t* b, uint8_t size);
void hp_aes_xor_128_lsb(uint8_t* result, const uint8_t* a, const uint8_t* b, uint8_t size, uint8_t offset);
void hp_aes_xor_128_msb(uint8_t* result, const uint8_t* a, const uint8_t* b, uint8_t size);

void hp_aes_start(struct aes_func_env* env, const uint8_t* key, const uint8_t *val);
#if AES_DECRYPT_ENABLE
void hp_aes_deciphering(uint8_t *key, uint8_t *val, aes_func_result_cb res_cb);
#endif
void hp_aes_ciphering(uint8_t *key, uint8_t *val, aes_func_result_cb res_cb);



/**
 * @brief Start the AES CBC crypto function. Allocate memory for the CBC and start processing it
 *        Execute result callback at end of function execution
 *
 * @param[in]  key               Pointer to the Key to be used
 * @param[in]  iv                16 Bytes iv to use for cipher/decipher
 * @param[in]  in_message        Input message for AES-CBC exectuion
 * @param[out] out_message       Output message that will contain cipher or decipher data
 * @param[in]  message_len       Length of Input/Output message without mic
 * @param[in]  cipher            True to encrypt message, False to decrypt it.
 * @param[in]  res_cb            Function that will handle the AES CCM result
 */
void hp_aes_cbc(const uint8_t* key, const uint8_t* iv, const uint8_t* in_message, uint8_t* out_message,
                bool cipher, uint16_t message_len, aes_func_result_cb res_cb);


/**
 * @brief Start the AES CCM crypto function. Allocate memory for the CCM and start processing it
 *        Execute result callback at end of function execution
 *
 * @param[in]  key               Pointer to the Key to be used (LSB mode!!!!)
 * @param[in]  nonce             13 Bytes Nonce to use for cipher/decipher (MSB)
 * @param[in]  in_message        Input message for AES-CCM exectuion (MSB)
 * @param[out] out_message       Output message that will contain cipher+mic or decipher data
 * @param[in]  message_len       Length of Input/Output message without mic
 * @param[in]  mic_len           Length of the mic to use (2, 4, 6, 8, 10, 12, 14, 16 valid)
 * @param[in]  cipher            True to encrypt message, False to decrypt it.
 * @param[in]  add_auth_data     Additional Authentication data used for computation of MIC (MSB)
 * @param[in]  add_auth_data_len Length of Additional Authentication data
 * @param[in]  res_cb            Function that will handle the AES CCM result
 * @param[in]  src_info          Information used retrieve requester
 */
void aes_ccm(const uint8_t* key, const uint8_t* nonce, const uint8_t* in_message,
             uint8_t* out_message, uint16_t message_len, uint8_t mic_len, bool cipher,
             const uint8_t* add_auth_data, uint8_t add_auth_data_len, aes_ccm_func_result_cb res_cb, uint32_t src_info);
#define hp_aes_ccm aes_ccm

#endif /* __HP_AES_H__ */
/**
 * @}
 */


