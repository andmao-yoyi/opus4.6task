#include "logic.h"
#include "hz_fsm.h"
#include "vendor_sdk.h"
#include "teach.h"

/// @brief 逻辑任务
task_t     logic_task;
sys_data_t data;

void scram_callback(void)
{
    GUR.HaveToReset = 1;
    GUW.Button.AxisEn = 0;
    for (int i = 0; i < AX_USED; i++)
    {
        mc_set_en(i, 0);
    }
    PARAINIT(logic_task);
    debug_printf("scram_callback\n");
}

void run_callback(void)
{
    LT.run_main_task.execute = 1;
    debug_printf("run_callback\n");
}

void stop_callback(void)
{
    debug_printf("stop_callback\n");
}

void paused_callback(void)
{
    debug_printf("paused_callback\n");
}

/// @brief 应用程序中的数据进行安全性检查
/// @param
void data_check(void)
{
}

/// @brief 更新系统数据
void system_data_update(void)
{
    // 状态切换
    hz_fsm_transit(&fsm);
    GUR.RunStatus = hz_fsm_get_state(&fsm);
    // 更新时间
    rtc_get_datetime((rtc_datetime_t *)&GSR.CurrTime);
    // 更新设置数据标志
    if (GUR.HaveToSetData == 0)
    {
        GUR.HaveToSetData = 1;
        BSP_Notify();
    }
    if (GUR.HaveToSetData == -1)
    {
        GUR.HaveToRepower = 1;
        AlarmSetBit(ALM5_DataErr);
    }
    // 打印本次执行的一些调试信息
    debug();
    // 更新轴坐标 确保数据不溢出
    for (int i = 0; i < AX_USED; i++)
    {
        u32 logic_axis = 0;
        if (axis_map_to_logic(i, &logic_axis) == 0)
        {
            continue;
        }
        GSR.AxisUnitPosition[logic_axis] = axis_get_cur_unit_pos(i);
        GSR.AxisState[logic_axis] = axis_get_status(i);
    }
}

/// @brief 触摸屏通讯的cmd设置
void runcommand_check(void)
{
    switch (GUW.Button.RunCommand)
    {
    case SCRAM:
        cmd_set(CMD_SCRAM);
        break;
    case STOP:
        cmd_set(CMD_STOP);
        break;
    case RUN:
        cmd_set(CMD_RUN);
        break;
    case RESET:
        cmd_set(CMD_RESET);
        break;
    case PAUSED:
        cmd_set(CMD_PAUSE);
        break;
    default:
        break;
    }
    GUW.Button.RunCommand = 0;
}

/**
 * @author Rod
 * @brief  按键逻辑
 * @param  无
 * @return 无
 */
void BT_logic(void)
{
    static tm_typ tm1;
    // 单步处理
    if (GUW.Button.DeviceMode == STEPMODE)
    {
        if (tim_on(&tm1, GUR.RunStatus == RUN, 10))
        {
            GUW.Button.RunCommand = STOP;
        }
    }

    static trig_def Tr1;
    // 启动
    if (trig_one(&Tr1, in_get(I_start) == ON, 10))
    {
        GUW.Button.DeviceMode = STEPMODE;
        if (GSR.ErrorLevel < 2 && GUR.HaveToReset == 0 && GUR.HaveToRepower == 0)
        {
            GUW.Button.RunCommand = RUN;
        }
    }

    static trig_def Tr2;
    // 停止
    if (trig_one(&Tr2, in_get(I_stop) == ON, 5))
    {
        if (GUR.RunStatus == RUN)
        {
            GUW.Button.StopCache = 1;
        }
    }

    static trig_def Tr3;
    // 复位
    if (trig_one(&Tr3, in_get(I_reset) == ON, 5))
    {
        for (int i = 0; i < AX_USED; i++)
        {
            mc_reset(i);
        }
    }

    // 急停按下
    if (in_get(I_scram) == ON)
    {
        GUW.Button.AxisEn = 0;
        GUW.Button.RunCommand = SCRAM;
    }

    static trig_def Tr4;
    // 急停松开
    if (trig_one(&Tr4, in_get(I_scram) == OFF, 5))
    {
        for (int i = 0; i < AX_USED; i++)
        {
            mc_reset(i);
        }
        GUW.Button.AxisEn = 1;
        if (GSR.ErrorLevel < 4)
        {
            GUW.Button.RunCommand = STOP;
        }
        data.axis_need_reset = 1;
    }

    static tm_typ Resettm2;
    // 设置轴使能1S后复位轴
    if (tim_on(&Resettm2, data.axis_need_reset, 1000))
    {
        data.axis_need_reset = 0;
        for (int i = 0; i < AX_USED; i++)
        {
            mc_reset(i);
        }
        GSW.ClearAlarm = 1;
    }

    static trig_def axis_en_set1;
    if (trig_one(&axis_en_set1, GUW.Button.AxisEn == 1, 5))
    {
        for (int i = 0; i < AX_USED; i++)
        {
            mc_set_en(i, 1);
        }
    }
    static trig_def axis_en_set0;
    if (trig_one(&axis_en_set0, GUW.Button.AxisEn == 0, 5))
    {
        for (int i = 0; i < AX_USED; i++)
        {
            mc_set_en(i, 0);
        }
    }
}

// 三色灯  运行：绿灯    停止：黄灯   报警：红灯
void led_ctr(void)
{
    switch (GUR.RunStatus)
    {
    case RUN: // 运行态
        if (GSR.ErrorLevel == 0)
        {
            GREEN;
        }
        else
        {
            YELLOW;
        }
        break;

    case RESET: // 复位态
        YELLOW;
        break;

    case SCRAM: // 急停态
        RED;
        break;

    case STOP: // 停止态
        if (GSR.ErrorLevel == 0)
        {
            YELLOW;
        }
        else
        {
            RED;
        }
        break;

    case INIT: // 初始态
        RED;
        break;

    default:
        break;
    }
}

/// @brief 逻辑任务
void logic(void)
{
    // 数据检查
    data_check();
    // 按键
    BT_logic();
    // 三色灯
    led_ctr();
    // 加载轴参数
    mc_param_load(GSS.axis);
    // 受状态机管控的函数
    switch (GUR.RunStatus)
    {
    case SCRAM:
        // 急停时函数
        dev_reset();
        break;
    case STOP:
        // 停止时函数
        jog(&GSW.Jog);
        teach();
        break;
    case RUN:
        // 运行时函数
        run_main(&logic_task.run_main_task);
        break;
    case RESET:
        // 复位时函数
        dev_reset();
        break;
    case INIT:
        teach();
        jog(&GSW.Jog);
        break;
    }
    // 不受状态机控制的函数
    runcommand_check();
    // 报警
    alarm_handle();
    // 伺服端子机相关函数
    servo_terminal();
    // 锁机
    // LockCheckModul();
}
