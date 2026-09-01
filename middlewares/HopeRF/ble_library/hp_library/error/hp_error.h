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
 * @file hp_error.h
 
 * @version v1.0.0
 *
  */

 /** @addtogroup 
 * @{
 */
#ifndef __HP_LIB_ERROR_H__
#define __HP_LIB_ERROR_H__


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>  
#include <stdbool.h>     
#include <stddef.h>      
/* Public define ------------------------------------------------------------*/
#define ERROR_BASE_NUM                      (0x0)       
 
#define ERROR_SUCCESS                     (ERROR_BASE_NUM + 0)   
#define ERROR_INTERNAL                    (ERROR_BASE_NUM + 1)  
#define ERROR_NO_MEM                      (ERROR_BASE_NUM + 2)  
#define ERROR_NOT_FOUND                   (ERROR_BASE_NUM + 3)  
#define ERROR_NOT_SUPPORTED               (ERROR_BASE_NUM + 4)  
#define ERROR_INVALID_PARAM               (ERROR_BASE_NUM + 5)  
#define ERROR_INVALID_STATE               (ERROR_BASE_NUM + 6)  
#define ERROR_INVALID_LENGTH              (ERROR_BASE_NUM + 7)  
#define ERROR_INVALID_FLAGS               (ERROR_BASE_NUM + 8) 
#define ERROR_INVALID_DATA                (ERROR_BASE_NUM + 9) 
#define ERROR_DATA_SIZE                   (ERROR_BASE_NUM + 10) 
#define ERROR_TIMEOUT                     (ERROR_BASE_NUM + 11) 
#define ERROR_NULL                        (ERROR_BASE_NUM + 12) 
#define ERROR_FORBIDDEN                   (ERROR_BASE_NUM + 13) 
#define ERROR_INVALID_ADDR                (ERROR_BASE_NUM + 14) 
#define ERROR_BUSY                        (ERROR_BASE_NUM + 15) 
#define ERROR_CRC                         (ERROR_BASE_NUM + 16) 
#define ERROR_HARD_FAULT                  (ERROR_BASE_NUM + 17)     
/* Public typedef -----------------------------------------------------------*/
typedef struct
{
    uint16_t        line_num;    
    uint8_t const * p_file_name; 
    uint32_t        err_code;    
} error_info_t;
/* Public constants ---------------------------------------------------------*/
/* Public function prototypes -----------------------------------------------*/
void error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name);


#define ERROR_HANDLER(ERR_CODE)                                    \
    do                                                                 \
    {                                                                  \
        error_handler((ERR_CODE), __LINE__, (uint8_t*) __FILE__);  \
    } while (0)



#define ERROR_CHECK(ERR_CODE)                           \
    do                                                      \
    {                                                       \
        const uint32_t LOCAL_ERR_CODE = (ERR_CODE);         \
        if (LOCAL_ERR_CODE != ERROR_SUCCESS)                  \
        {                                                   \
            ERROR_HANDLER(LOCAL_ERR_CODE);              \
        }                                                   \
    } while (0)
    







#ifdef __cplusplus
}
#endif








#endif //__HP_LIB_ERROR_H__
/**
 * @}
 */


