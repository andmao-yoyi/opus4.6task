#ifndef __TERMINALTOOL_H
#define __TERMINALTOOL_H

#include "logic.h"

/*伺服端子机用*/
float angle_to_height(float rad, float angle);
float terminal_angle_calulate(float RatoLength, float TarPos);
float height_to_angle(float len);

// 端子机冲头当前位置转换
void terminal_cur_pos(logic_para_t *LG);

// 伺服端子机相关函数
void servo_terminal(void);
// 伺服端子机屏幕点动函数
int servo_terminal_jog_go(u32 axisnum, float pos, float spd);

typedef struct
{
    u32 idx;             // 端子机号
    u32 axis_num;        // 轴号
    u32 input;           // 原点
    u32 input_up;        // 手动上行
    u32 input_down;      // 手动下行
    u32 *workdone_delay; // 保压时间
    u32 *terminal_spd;   // 打端速度
    float *ready_pos;    // 准备位置
    float *terminal_pos; // 打端位置
    float *cur_pos;      // 当前位置
} servo_terminal_def;    // 伺服端子机参数

#endif
