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
 * @file hp_ecc.c
 * @version v1.0.0
 *
  */

/** @addtogroup 
 * @{
 */

 /* Includes ------------------------------------------------------------------*/
#include "hp_ecc.h"
#include <stdint.h>
#include <stdio.h>
#include "uECC.h"
#include "sha256.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/




/**
 * @brief Calculate hash degest.
 * @param[in] p_data raw data to process.
 * @param[in] data_len raw data length.
 * @param[out] p_hash hash degest. 
 * @return error code
 */
uint32_t hp_lib_ecc_hash_sha256(uint8_t *p_data, uint32_t data_len, uint8_t *p_hash)
{
    sha256_context_t   hash_context;
    sha256_init(&hash_context);
    if(!sha256_update(&hash_context, p_data, data_len)){
        sha256_final(&hash_context, p_hash, 0);
        return 0;
    }
    return 1;
}


/**
 * @brief big to little endian swap function.
 * @param[out] p_out output little endian data.
 * @param[in] p_in big endian raw data.
 * @param[in] size data length. 
 * @return none
 */
static void swap_endian(uint8_t * p_out, uint8_t const * p_in, size_t size)
{
    uint8_t const * p_first = p_in;
    uint8_t * p_last = p_out + size - 1;
    while (p_last >= p_out)
    {
        *p_last = *p_first;
        p_first++;
        p_last--;
    }
}

/**
 * @brief big to little endian swap function with two sets of data.
 * @param[out] p_out output little endian data.
 * @param[in] p_in big endian raw data.
 * @param[in] size half data length. 
 * @return none
 */
static void double_swap_endian(uint8_t * p_out, uint8_t const * p_in, size_t part_size)
{
    swap_endian(p_out, p_in, part_size);
    swap_endian(&p_out[part_size], &p_in[part_size], part_size);
}



/**
 * @brief ECDSA SHA256 digital signature verification function.
 * @param[in] p_public_key public key data.
 * @param[in] p_hash sha256 hash degest data.
 * @param[in] hash_size hash degest length. 
 * @param[in] p_signature digital signature. 
 * @return error code
 */
uint32_t hp_lib_ecc_ecdsa_verify(uint8_t *p_public_key, uint8_t *p_hash, uint32_t hash_size, uint8_t *p_signature)
{
    uint32_t error = 0;    
    uint8_t hash_le     [32];
    uint8_t signature_le[64];
    uint8_t public_key_le[64];    
    swap_endian(hash_le, p_hash, sizeof(hash_le));
    double_swap_endian(signature_le,p_signature,32);    
    double_swap_endian(public_key_le,p_public_key,32);
    int result = uECC_verify(public_key_le, hash_le, hash_size, signature_le, &curve_secp256r1);
    if(result != 1){
        error = 1;
    }
    return error;
}

















