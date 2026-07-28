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
 * @file TypeDefine.c
 
 * @version v1.0.0
 *
  */

#ifndef __TYPES_H__
#define __TYPES_H__

//------------------------------------------------------------------
//                        Headers
//------------------------------------------------------------------
#include "core_cm0.h"
//------------------------------------------------------------------
//                        Definitions
//------------------------------------------------------------------
#define    FALSE       0
#define    TRUE        1

//#define NULL        0

#define REG8(addr)          (*(volatile UINT8 *) (addr))
#define REG16(addr)          (*(volatile UINT16 *)(addr))
#define REG32(addr)          (*(volatile UINT32 *)(addr))

//------------------------------------------------------------------
//                        TypeDefs
//------------------------------------------------------------------
typedef    unsigned char     UINT8;    ///<unsigned char
typedef    signed char       INT8;    ///< char

typedef    unsigned short    UINT16;    ///<unsigned char
typedef    signed short      INT16;    ///<short

typedef unsigned int      UINT32;    ///<unsigned int
typedef    signed int        INT32;    ///<int

typedef unsigned char     BOOL;    ///<BOOL

typedef unsigned int      U32;
typedef unsigned short    U16;
typedef unsigned char     U8;
typedef signed short      S16;
typedef signed int        S32;
typedef signed char       S8;
typedef unsigned long long    U64;

//typedef unsigned char     uint8_t;
//typedef unsigned short    uint16_t;
//typedef unsigned int      uint32_t;

//------------------------------------------------------------------
//                        Exported variables
//------------------------------------------------------------------

//------------------------------------------------------------------
//                        Exported functions
//------------------------------------------------------------------

#endif

