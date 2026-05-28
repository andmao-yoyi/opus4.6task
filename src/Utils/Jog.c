/*
 * 轴的手动点动操作
 */
#include "vendor_sdk.h"
#include "hpm_interlayer.h"
#include "jog.h"
#include "Logic.h"

//=============================================================================
// 定义
//=============================================================================
#define JOG_FORWARD  0
#define JOG_BACKWARD 1

// 轴运动状态记录，用于检测状态变化
static u32 LastAxisState[2][3] = {0}; // [0]=Forward, [1]=Backward

//=============================================================================
// 函数声明
//=============================================================================
extern void jog_go(u32 axisIndex, float len, float speed);
extern void jog_stop(u32 axisIndex);
extern void jog_go_home(u32 axisIndex);
//=============================================================================
// 函数实现
//=============================================================================
/// @brief 点动操作
void jog(jog_ctr *jog)
{
    int i, j, axisIndex;

    // 首先检查所有轴状态，找出需要停止的轴
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 32; j++)
        {
            // 检查是否有这个轴的状态记录
            if (GETBIT(LastAxisState[JOG_FORWARD][i], j) || GETBIT(LastAxisState[JOG_BACKWARD][i], j))
            {
                // 检查当前是否有新的点动指令
                if (!GETBIT(jog->JogForward[i], j) && !GETBIT(jog->JogBackward[i], j))
                {
                    // 没有新的点动指令，需要停止这个轴
                    axisIndex = i * 32 + j;
                    if (jog->JogType == 0)
                    {
                        // 连续点动:松开按键后停止
                        jog_stop(axisIndex);
                    }
                    // 清除状态记录
                    LastAxisState[JOG_FORWARD][i] &= ~(1U << j);
                    LastAxisState[JOG_BACKWARD][i] &= ~(1U << j);

                    // 找到并处理了一个需要停止的轴后就返回
                    return;
                }
            }
        }
    }

    // 查找哪个组和位有新的点动指令
    for (i = 0; i < 3; i++)
    {
        // 检查当前组是否有任何点动指令
        if (jog->JogForward[i] || jog->JogBackward[i] || jog->JogGohome[i])
        {
            // 找到具体哪一位有点动指令
            for (j = 0; j < 32; j++)
            {
                if (GETBIT(jog->JogGohome[i], j))
                {
                    // 找到回原点点动指令
                    axisIndex = i * 32 + j;
                    jog_go_home(axisIndex);

                    // 回原点不需要记录状态
                    // 但需要清除该轴的其他状态记录
                    LastAxisState[JOG_FORWARD][i] &= ~(1U << j);
                    LastAxisState[JOG_BACKWARD][i] &= ~(1U << j);

                    // 执行完就返回
                    return;
                }
                else if (GETBIT(jog->JogForward[i], j))
                {
                    // 找到正向点动指令
                    axisIndex = i * 32 + j;

                    // 根据点动类型执行不同操作
                    if (jog->JogType == 0)
                    {
                        // 连续点动
                        jog_go(axisIndex, 999999.0f, jog->JogSpeedRate);
                        // 设置Forward状态位
                        LastAxisState[JOG_FORWARD][i] |= (1U << j);
                        // 清除Backward状态位
                        LastAxisState[JOG_BACKWARD][i] &= ~(1U << j);
                    }
                    else
                    {
                        // 单步点动要等待上一次的指令执行完成
                        if (GETBIT(LastAxisState[JOG_FORWARD][i], j) == 0 && GETBIT(LastAxisState[JOG_BACKWARD][i], j) == 0)
                        {
                            jog_go(axisIndex, jog->JogLen, jog->JogSpeedRate);
                            // 设置Forward状态位
                            LastAxisState[JOG_FORWARD][i] |= (1U << j);
                            // 清除Backward状态位
                            LastAxisState[JOG_BACKWARD][i] &= ~(1U << j);
                        }
                    }
                    // 执行完就返回
                    return;
                }
                else if (GETBIT(jog->JogBackward[i], j))
                {
                    // 找到反向点动指令
                    axisIndex = i * 32 + j;

                    // 根据点动类型执行不同操作
                    if (jog->JogType == 0)
                    {
                        // 连续点动
                        jog_go(axisIndex, -999999.0f, jog->JogSpeedRate);
                        // 设置Backward状态位
                        LastAxisState[JOG_BACKWARD][i] |= (1U << j);
                        // 清除Forward状态位
                        LastAxisState[JOG_FORWARD][i] &= ~(1U << j);
                    }
                    else
                    {
                        // 单步点动要等待上一次的指令执行完成
                        if (GETBIT(LastAxisState[JOG_FORWARD][i], j) == 0 && GETBIT(LastAxisState[JOG_BACKWARD][i], j) == 0)
                        {
                            jog_go(axisIndex, -jog->JogLen, jog->JogSpeedRate);
                            // 设置Backward状态位
                            LastAxisState[JOG_BACKWARD][i] |= (1U << j);
                            // 清除Forward状态位
                            LastAxisState[JOG_FORWARD][i] &= ~(1U << j);
                        }
                    }

                    // 执行完就返回
                    return;
                }
            }
        }
    }
}

void jog_go_home(u32 axisIndex)
{
    // 如果轴是静止则执行回零操作（使用预加载的回零参数）
    if (mc_ready(axisIndex))
    {
        mc_home(axisIndex);
    }
}

void jog_stop(u32 axisIndex)
{
    mc_stop(axisIndex);
}

void jog_go(u32 axisIndex, float len, float speed)
{
    // 使用预加载的速度参数，并应用速度比例
    if (mc_ready(axisIndex))
    {
        if (servo_terminal_jog_go(axisIndex, len, speed) == 0)
        {
            return;
        }
        mc_move_rel_rate(axisIndex, len, speed);
    }
}


