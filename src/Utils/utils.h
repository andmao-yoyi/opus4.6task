#ifndef __UTILS_H__
#define __UTILS_H__

#include "vendor_common.h"

//定时触发器
typedef struct
{
    u32 start;
    u32 starttick;
} tm_typ;

typedef struct
{
    u32 trig_flag;
    u32 trig_time_stamp;
} trig_def;

int32_t conver_to_pulse(uint32_t num, float MM);
float convert_to_user_unit(uint32_t num, int32_t pulse);

//定时器
extern u32 tim_on(tm_typ *tm, int ex, u32 timeout);
//触发器
extern s32 trig_one(trig_def *this, s32 Condition, u32 Time);
extern void trig_rst(trig_def *this);
// 调试信息显示
void debug(void);

#endif
