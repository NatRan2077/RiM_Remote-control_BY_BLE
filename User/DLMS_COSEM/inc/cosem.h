//----------------------------------------------------------------------------//
#ifndef INCCOSEM_H
#define INCCOSEM_H
//----------------------------------------------------------------------------//
#include <stdbool.h>
#include "hdlc.h"
//----------------------------------------------------------------------------//
#define APDU_SIZE_MAX 2030
//----------------------------------------------------------------------------//
#define LAST_BLOCK_FLAG 0x01
#define RESPONSE_NORMAL 0x01
#define GET_RESPONSE_WITH_DATABLOCK 0x02
//----------------------------------------------------------------------------//
typedef enum {
	TAG_NULLDATA = 0,
	TAG_ARRAY = 1,
	TAG_STRUCTURE = 2,
	TAG_BOOLEAN = 3,
	TAG_BITSTRING = 4,
	TAG_INT32 = 5,
	TAG_UINT32 = 6,
	TAG_FLOATING_POINT = 7,
	TAG_OCTET_STRING = 9,
	TAG_VISIBLE_STRING = 10,
	TAG_GENERALIZED_TIME = 11,
	TAG_UTF8_STRING = 12,
	TAG_BCD = 13,
	TAG_INT8 = 15,
	TAG_INT16 = 16,
	TAG_UINT8 = 17,
	TAG_UINT16 = 18,
	TAG_COMPACT_ARRAY = 19,
	TAG_INT64 = 20,
	TAG_UINT64 = 21,
	TAG_ENUM = 22,
	TAG_FLOAT32 = 23,
	TAG_FLOAT64 = 24,
	TAG_DATETIME = 25,
	TAG_DATE = 26,
	TAG_TIME = 27,
	TAG_DONTCARE = 255
} cosemTag_t;
//----------------------------------------------------------------------------//
typedef enum {
	ATTR_ACCESS_READ,
	ATTR_ACCESS_WRITE,
} attrAccess_t;
//----------------------------------------------------------------------------//
#endif
