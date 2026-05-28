#ifndef _LOGICTYPE_H_
#define _LOGICTYPE_H_

#include "vendor_common.h"

#define TASK_PROCESS()                                                                                                           \
    if (LG->execute != 0 && LG->step == 0)                                                                                       \
    {                                                                                                                            \
        LG->step = 1;                                                                                                            \
        LG->done = 0;                                                                                                            \
    }                                                                                                                            \
    if (LG->step != LG->stepBuff)                                                                                                \
    {                                                                                                                            \
        LG->stepBuff = LG->step;                                                                                                 \
        TRST;                                                                                                                    \
    }                                                                                                                            \
    switch (STEP)

#define LT   logic_task
#define STEP LG->step
#define LT_End                                                                                                                   \
    {                                                                                                                            \
        LG->execute = 0;                                                                                                         \
        LG->step = 0;                                                                                                            \
        LG->count = 0;                                                                                                           \
    }

#define TRST (LG->tm = sys_get_tick())     // 逻辑定时重置
#define TCNT ((sys_get_tick() - LG->tm)) // 逻辑定时计数

typedef struct
{
    u8    execute;  // 启动任务
    u8    step;     // 任务状态
    u8    done;     // 完成标志
    u8    Taskflag; // 特殊标记  1半剥 0全剥
    u32   count;    // 用于记录圈数，必须放大单位
    u32   tm;       // 任务计时
    u32   err;      // 记录错误
    u32   tm_Exe;   // 任务执行时间计时器
    u32   Exetime;  // 记录任务执行时间
    u32   stepBuff; // 状态缓存
    void *para;     // 当前任务的参数
} logic_para_t;

#define PARAINIT(a) memset(&(a), 0, sizeof((a)));

#define ENDCASE     0xff // 最后一个case

#endif // _LOGICTYPE_H_
