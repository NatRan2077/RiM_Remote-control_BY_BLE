//----------------------------------------------------------------------------//
#include "log.h"
#include <string.h>
//----------------------------------------------------------------------------//
logStruct_t sysLog;
char str_tmp[SYS_LOG_BUFF_SIZE];
//----------------------------------------------------------------------------//
void sysLogPut(void) {
	u16 cnt = strlen(str_tmp);
	for (u16 i = 0; i < cnt; i++) {
		if ('\r' == str_tmp[i])
			; // Возврат каретки пропускаем
		else if ('\n' ==
		         str_tmp[i]) { // Вместо новой строки вводим символ табуляции
			FIFO_PUT(sysLog, 0x09);
		} else {
			FIFO_PUT(sysLog, str_tmp[i]);
		}
	}
	FIFO_PUT(sysLog, '\r');
	FIFO_PUT(sysLog, '\n');
};
//----------------------------------------------------------------------------//
void _SYSLOG_DATA_(const char *tag, u8 *buf, u8 len) {
	char tmp[SYS_LOG_BUFF_SIZE];
	u16 tagLen = sprintf(tmp, "%s ", tag);
	for (u8 i = 0; i < len; i++) {
		sprintf(((tmp + tagLen) + i * 3), "%02X ", buf[i]);
	}
	snprintf(str_tmp, SYS_LOG_BUFF_SIZE, tmp);
	sysLogPut();
}
//----------------------------------------------------------------------------//
