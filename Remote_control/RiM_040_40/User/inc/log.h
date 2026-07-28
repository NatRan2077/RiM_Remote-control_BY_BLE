//----------------------------------------------------------------------------//
#ifndef INCLOG_H
#define INCLOG_H
//----------------------------------------------------------------------------//
#include "common_types.h"
#include "fifo.h"
#include <stdio.h>
#include <stdint.h>
//----------------------------------------------------------------------------//
void _SYSLOG_DATA_(const char *tag, u8 *buf, u8 len);
void sysLogPut(void);
//----------------------------------------------------------------------------//
#define SYS_LOG_BUFF_SIZE 2048
//----------------------------------------------------------------------------//
typedef struct {
	u8 buf[SYS_LOG_BUFF_SIZE];
	uint32_t tail;
	uint32_t head;
} logStruct_t;
extern logStruct_t sysLog;
//----------------------------------------------------------------------------//
extern char str_tmp[SYS_LOG_BUFF_SIZE];
//----------------------------------------------------------------------------//
#define _SYSLOG_(...)                                                          \
	do {                                                                        \
		snprintf(str_tmp, SYS_LOG_BUFF_SIZE, __VA_ARGS__);                       \
		sysLogPut();                                                             \
	} while (0)
//----------------------------------------------------------------------------//
#endif
