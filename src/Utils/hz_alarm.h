#ifndef __HZ_ALARM_H__
#define __HZ_ALARM_H__

#include "vendor_common.h"

//============== 报警操作 ==============
void AlarmSetBit(u8 level, u8 word, u8 bit);
void AlarmClearBit(u8 level, u8 word, u8 bit);
s32 AlarmGetBit(u8 level, u8 word, u8 bit);
u8 AlarmGetLevel(void);

// 清除所有报警
void AlarmClearAll(void);

#endif

