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
 * @file cmt453x_irgen.h
 
 * @version v1.0.0
 *
  */
#ifndef __CMT453x_IRGEN_H__
#define __CMT453x_IRGEN_H__

#include <stdbool.h>
#include <stdint.h>
#include <cmt453x.h>

/**
 * \brief IR logic bit format
 *
 */
typedef enum {
    HW_IRGEN_LOGIC_FORMAT_MARK = 0, /**< logic bit starts with mark followed by space */
    HW_IRGEN_LOGIC_FORMAT_SPACE = 1 /**< logic bit starts with space followed by mark */
} HW_IRGEN_LOGIC_FORMAT;

/**
 * \brief IR FIFO
 *
 */
typedef enum {
    HW_IRGEN_FIFO_CODE = 0,         /**< code FIFO */
    HW_IRGEN_FIFO_REPEAT = 1        /**< repeat FIFO */
} HW_IRGEN_FIFO;

/**
 * \brief Paint symbol
 *
 */
typedef enum {
    HW_IRGEN_PAINT_SPACE = 0,       /**< space */
    HW_IRGEN_PAINT_MARK = 1         /**< mark */
} HW_IRGEN_PAINT;

/**
 * \brief IR generator output mode
 *
 */
typedef enum {
    HW_IRGEN_OUTPUT_NORMAL = 0,     /**< output is not inverted */
    HW_IRGEN_OUTPUT_INVERTED = 1    /**< output is inverted */
} HW_IRGEN_OUTPUT;

/**
 * \brief Set carrier frequency
 *
 * \param [in] hi_time   carrier signal high duration in IR clock cycles (must be >0)
 * \param [in] lo_time   carrier signal low duration in IR clock cycles (must be >0)
 *
 */
static __INLINE void hw_irgen_set_carrier_freq(uint16_t hi_time, uint16_t lo_time)
{
    IRC->FREQ_CARRIER_ON = hi_time;
    IRC->FREQ_CARRIER_OFF = lo_time;
}

/**
 * \brief Set "logic 0" parameters
 *
 * \param [in] format       "logic 0" format
 * \param [in] mark_time    mark duration in carrier clock cycles (must be >0)
 * \param [in] space_time   space duration in carrier clock cycles (must be >0)
 *
 * \sa hw_ir_insert_digital_message
 */
static __INLINE void hw_irgen_set_logic0_param(HW_IRGEN_LOGIC_FORMAT format, uint8_t mark_time,
                                                                                uint8_t space_time)
{
    IRC->LOGIC_ZERO_TIME = (uint16_t)(mark_time << IR_LOGIC_ZERO_TIME_LOGIC_ZERO_MARK_Pos | space_time);
    IRC->CTRL = ((IRC->CTRL & ~(IR_CTRL_LOGIC_ZERO_FORMAT_Msk)) | \
        ((IR_CTRL_LOGIC_ZERO_FORMAT_Msk) & ((format) << (IR_CTRL_LOGIC_ZERO_FORMAT_Pos))));
}

/**
 * \brief Set "logic 1" parameters
 *
 * \param [in] format       "logic 1" format
 * \param [in] mark_time    mark duration in carrier clock cycles (must be >0)
 * \param [in] space_time   space duration in carrier clock cycles (must be >0)
 *
 * \sa hw_ir_insert_digital_message
 */
static __INLINE void hw_irgen_set_logic1_param(HW_IRGEN_LOGIC_FORMAT format, uint8_t mark_time,
                                                                                uint8_t space_time)
{
    IRC->LOGIC_ONE_TIME = (uint16_t)(mark_time << IR_LOGIC_ONE_TIME_LOGIC_ONE_MARK_Pos | space_time);
    IRC->CTRL = ((IRC->CTRL & ~(IR_CTRL_LOGIC_ONE_FORMAT_Msk)) | \
        ((IR_CTRL_LOGIC_ONE_FORMAT_Msk) & ((format) << (IR_CTRL_LOGIC_ONE_FORMAT_Pos))));
}

/**
 * \brief Set command repeat time
 *
 * \param [in] time repeat time in carrier clock cycles
 *
 * \sa hw_irgen_set_repeat_type
 *
 */
static __INLINE void hw_irgen_set_repeat_time(uint16_t time)
{
    IRC->REPEAT_TIME = time;
}

/**
 * \brief Insert Digital message into FIFO
 *
 * Digital message is represented by \p payload as sequence of "logic 0" and "logic 1".
 *
 * \param [in] fifo      FIFO to be used
 * \param [in] length    number of valid bits in \p message (must be >0)
 * \param [in] payload   message content
 *
 * \sa hw_irgen_set_logic0_format
 * \sa hw_irgen_set_logic1_format
 *
 */
static __INLINE void hw_irgen_insert_digital_message(HW_IRGEN_FIFO fifo, uint8_t length, uint16_t payload)
{
    /* reg[20] = digital message (1), reg[19:16] = length-1, reg[15:0] = payload */
    uint32_t value = 0x100000 | (((length - 1) & 0xF) << 16) | (payload & 0xFFFF);
    if (fifo == HW_IRGEN_FIFO_REPEAT) {
        IRC->REPEAT_FIFO = value;
    } else {
        IRC->CODE_FIFO = value;
    }
}

/**
 * \brief Insert Paint message into FIFO
 *
 * Paint message is represented by either mark or space and its duration. This allows to represent
 * any custom "painted" waveform.
 *
 * \param [in] fifo       FIFO to be used
 * \param [in] symbol     symbol type
 * \param [in] duration   symbol duration in carrier clock cycles
 *
 */
static __INLINE void hw_irgen_insert_paint_message(HW_IRGEN_FIFO fifo, HW_IRGEN_PAINT symbol, uint16_t duration)
{
    /* reg[20] = paint message (0), reg[14] = symbol, reg[13:0] = duration */
    uint16_t value = (!!symbol << 14) | (duration & 0x3FFF);
    if (fifo == HW_IRGEN_FIFO_REPEAT) {
        IRC->REPEAT_FIFO = value;
    } else {
        IRC->CODE_FIFO = value;
    }
}

/**
 * \brief Flush FIFO
 *
 * Paint message is represented by either mark or space and its duration. This allows to represent
 * any custom "painted" waveform.
 *
 * \param [in] fifo   FIFO to be flushed
 *
 */
static __INLINE void hw_irgen_flush_fifo(HW_IRGEN_FIFO fifo)
{
    if (fifo == HW_IRGEN_FIFO_REPEAT) { 
        IRC->CTRL = ((IRC->CTRL & ~(IR_CTRL_REP_FIFO_RESET_Msk)) | \
            ((IR_CTRL_REP_FIFO_RESET_Msk) & ((1) << (IR_CTRL_REP_FIFO_RESET_Pos))));
    } else {
        IRC->CTRL = ((IRC->CTRL & ~(IR_CTRL_CODE_FIFO_RESET_Msk)) | \
            ((IR_CTRL_CODE_FIFO_RESET_Msk) & ((1) << (IR_CTRL_CODE_FIFO_RESET_Pos))));
    }
}

/**
 * \brief Set output type (normal or inverted)
 *
 * \param [in] output   output type
 *
 */
static __INLINE void hw_irgen_set_output_type(HW_IRGEN_OUTPUT output)
{
	IRC->CTRL = ((IRC->CTRL & ~(IR_CTRL_INVERT_OUTPUT_Msk)) | \
                ((IR_CTRL_INVERT_OUTPUT_Msk) & ((output) << (IR_CTRL_INVERT_OUTPUT_Pos))));
}

/**
 * \brief Start IR data transmission
 *
 */
static __INLINE void hw_irgen_start(void)
{
	IRC->CTRL = ((IRC->CTRL & ~(IR_CTRL_TX_START_Msk)) | \
                ((IR_CTRL_TX_START_Msk) & ((1) << (IR_CTRL_TX_START_Pos))));
	
}

/**
 * \brief Stop IR data transmission
 *
 * When called after transmission is started, this will also flush code FIFO.
 *
 */
static __INLINE void hw_irgen_stop(void)
{
	IRC->CTRL = ((IRC->CTRL & ~(IR_CTRL_TX_START_Msk)) | \
                ((IR_CTRL_TX_START_Msk) & ((0) << (IR_CTRL_TX_START_Pos))));
}

/**
 * \brief Check if IR generator is busy
 *
 * \return true if IR generator is busy, false otherwise
 *
 */
static __INLINE bool hw_irgen_is_busy(void)
{
	return ((IRC->STATUS) & (IR_STATUS_BUSY));
}

/**
 * \brief Get number of messages in FIFO
 *
 * \param [in] fifo   FIFO
 *
 * \return number of messages
 *
 */
static __INLINE uint16_t hw_irgen_get_fifo_level(HW_IRGEN_FIFO fifo)
{
    if (fifo == HW_IRGEN_FIFO_REPEAT) {
        return (((IRC->STATUS) & (IR_STATUS_REP_FIFO_WORDS_Msk)) >> (IR_STATUS_REP_FIFO_WORDS_Pos));
    } else {
        return (((IRC->STATUS) & (IR_STATUS_CODE_FIFO_WORDS_Msk)) >> (IR_STATUS_CODE_FIFO_WORDS_Pos));
    }
}

/**
 * \brief Reverse bit order
 *
 * Helper function to reverse order of bits in message.
 *
 * \param [in] val    message value
 * \param [in] bits   number of valid bits in \p val
 *
 * \return \p val with bit order reversed
 *
 */
static __INLINE uint16_t hw_irgen_reverse_bit_order(uint16_t val, uint8_t bits)
{
    uint16_t ret = 0;

    while (bits--) {
        ret = (ret << 1) | (val & 1);
        val >>= 1;
    }

    return ret;
}


#endif /* __CMT453x_IRGEN_H__ */
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

