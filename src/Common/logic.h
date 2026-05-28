#ifndef __LOGIC_H__
#define __LOGIC_H__

#include "logic_type.h"
#include "data_define.h"
#include "hpm_interlayer.h"
#include "hz_fsm.h"
#include "jog.h"
#include "alarm.h"
#include "iodefine.h"
#include "Lock.h"
#include "queue_tool.h"
#include "terminal_tool.h"
#include "Utils.h"

typedef struct // 系统任务队列
{
    logic_para_t run_main_task;                            // 启动开始的主调度
    logic_para_t ter_cur_oos_task[TERMINAL_USED];          // 伺服端子机当前位置换算
    logic_para_t servo_terminal_reset_task[TERMINAL_USED]; // 伺服端子机复位
    logic_para_t accurate_task[TERMINAL_USED];             // 半圈打端
    logic_para_t single_task[TERMINAL_USED];               // 单圈打端
    logic_para_t ax_home_task[AXIS_MAX_NUM];                    // 回零
} task_t;
extern task_t logic_task;

typedef struct
{
    u32 flag_user;       // 用户标志
    u32 axis_need_reset; // 需要复位轴
} sys_data_t;
extern sys_data_t data;

#define RED                \
    out_set(Q_red, ON);    \
    out_set(Q_green, OFF); \
    out_set(Q_yellow, OFF)
#define GREEN             \
    out_set(Q_red, OFF);  \
    out_set(Q_green, ON); \
    out_set(Q_yellow, OFF)
#define YELLOW             \
    out_set(Q_red, OFF);   \
    out_set(Q_green, OFF); \
    out_set(Q_yellow, ON)

#define _SPD spd_rate(GCFG.speed.all_spd) // 当前项目使用的是全局速度

#define NORMALMODE 0 // 正常模式
#define STEPMODE 1   // 单步调试
#define CYCLEMODE 2  // 单次调试
#define AGINGMODE 3  // 老化模式

// 功能函数
/// @brief 设置命令
/// @param cmd 命令
void cmd_set(uint32_t cmd);

// 总调度
void logic(void);
void app_init(void);
void dev_reset(void);
void init_IO(void);
void init_axis(void);

// 其他调用函数
void run_main(logic_para_t *LG);

#endif
