#include "Utils.h"
#include "data_define.h"
#include "vendor_sdk.h"
#include "hpm_interlayer.h"


//// 用轴号的转换 适配老代码 ////
int32_t conver_to_pulse(uint32_t num, float MM)
{
    return (int32_t)(MM * mc_get_pulse_per_unit(num));
}
float convert_to_user_unit(uint32_t num, int32_t pulse)
{
    return (float)pulse / mc_get_pulse_per_unit(num);
}


/// @brief 调试周期
#define DEBUG_PERIOD 3000

/// @brief 调试
void debug(void)
{
    static uint32_t tick;
    if (sys_get_tick() - tick >= DEBUG_PERIOD)
    {
        tick = sys_get_tick();
        // 打印信息 暂时空着
        //print_freertos_task_status();
    }
}

/**
 * @brief 软件定时器
 */
u32 tim_on(tm_typ *tm, int ex, u32 timeout)
{
    if (ex) // 条件满足
    {
        if (tm->start == 0)
        {
            tm->start = 1; // 开始计时
            tm->starttick = sys_get_tick();
        }
    }
    else
    {
        memset(tm, 0, sizeof(tm_typ));
        return 0;
    }
    if (tm->start == 1 && (sys_get_tick() - tm->starttick) >= timeout)
    {
        return 1;
    }
    return 0;
}

// 触发器
s32 trig_one(trig_def *this, s32 Condition, u32 Time)
{
    if (Condition == 0) // 不成立
    {
        this->trig_flag = 1;
        this->trig_time_stamp = sys_get_tick();
    }
    if (Condition != 0 && this->trig_flag == 1 && (sys_get_tick() - this->trig_time_stamp) > Time) // 成立 并且是跳变
    {
        this->trig_flag = 0;
        return 1;
    }
    return 0;
}

// 触发器复位
void trig_rst(trig_def *this)
{
    this->trig_flag = 0;
    this->trig_time_stamp = sys_get_tick();
}


