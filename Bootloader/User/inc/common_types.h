//----------------------------------------------------------------------------//
#ifndef INCCOMMONTYPES_H
#define INCCOMMONTYPES_H
//----------------------------------------------------------------------------//
#include <stdint.h>
//----------------------------------------------------------------------------//
typedef	uint64_t	u64;
typedef	uint32_t	u32;
typedef	uint16_t	u16;
typedef	uint8_t		u8;

typedef	int64_t		s64;
typedef	int32_t		s32;
typedef	int16_t		s16;
typedef	int8_t 		s8;
//----------------------------------------------------------------------------//
#define BYTE_0(n)	((uint8_t)((n) & (uint8_t)0xFF))
#define BYTE_1(n)	((uint8_t)(BYTE_0((n) >> (uint8_t)8)))
#define BYTE_2(n)	((uint8_t)(BYTE_0((n) >> (uint8_t)16)))
#define BYTE_3(n)	((uint8_t)(BYTE_0((n) >> (uint8_t)24)))
#define BYTE_4(n)	((uint8_t)(BYTE_0((n) >> (uint8_t)32)))
#define BYTE_5(n)	((uint8_t)(BYTE_0((n) >> (uint8_t)40)))
#define BYTE_6(n)	((uint8_t)(BYTE_0((n) >> (uint8_t)48)))
#define BYTE_7(n)	((uint8_t)(BYTE_0((n) >> (uint8_t)56)))
//----------------------------------------------------------------------------//
#define QWORD(a, b, c, d, e, f, g, h)	(((u64)a<<56) | ((u64)b<<48) | ((u64)c<<40) | ((u64)d<<32) | ((u64)e<<24) | ((u64)f<<16) | ((u64)g<<8) | ((u64)h<<0))
#define DWORD(a, b, c, d)				(((u32)a<<24) | ((u32)b<<16) | ((u32)c<<8) | ((u32)d<<0))
#define WORD(a, b)						(((u16)a<<8) | ((u16)b<<0))
//----------------------------------------------------------------------------//
#define BCD_TO_BIN(bcd) ((((bcd) >> 4) & 0x0F) * 10 + ((bcd) & 0x0F))
//----------------------------------------------------------------------------//
typedef enum{
	RESULT_SUCCESS				= 0,
	RESULT_ERROR 				= 1,
	RESULT_ERROR_OUT_OF_RANGE	= 2,
	RESULT_ERROR_UNMATCHED		= 3,
	RESULT_ERROR_MEM_WRITE		= 4,
	RESULT_ERROR_MEM_READ		= 5,
	RESULT_TIMEOUT				= 6,
}result_t;
//----------------------------------------------------------------------------//
#endif
