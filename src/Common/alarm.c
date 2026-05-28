#include "alarm.h"
#include "data_define.h"
#include "hz_fsm.h"
#include "vendor_sdk.h"
#include "hpm_interlayer.h"
#include "logic.h"

/// @brief 报警检查
void alarm_check(void)
{
    // 是否有清除报警标记
    if (GSW.ClearAlarm)
    {
        GSW.ClearAlarm = 0;
        AlarmClearAll();
    }

    // 判断电机是否有错误
    for (int i = 0; i < pBsp->axis_num_pluse + pBsp->axis_num_ethercat; i++)
    {
        if (i < 32)
        {
            u32 logic_axis = 0;
            if (axis_map_to_logic(i, &logic_axis))
            {
                // 电机错误
                if (axis_get_error_status(i) == AXIS_ERR_ALM)
                {
                    AlarmSetBit(ALM4_AxisErr(logic_axis + 1));
                }
                // 使能错误
                else if (axis_get_error_status(i) == AXIS_ERR_DISABLE)
                {
                    // 轴未准备好,断使能
                    AlarmSetBit(ALM3_AxisDisable);
                }
                // 电机限位错误
                else if (axis_get_error_status(i) != AXIS_ERR_NONE)
                {
                    AlarmSetBit(ALM3_AxisErr(logic_axis + 1));
                }
            }
        }
    }

    // 判断应用程序的报警

    // 判断是否有标记报警
    if (GUR.HaveToReset)
    {
        AlarmSetBit(ALM3_ShouldRst);
    }
    if (GUR.HaveToRepower)
    {
        AlarmSetBit(ALM4_Reboot);
    }

    // 更新报警等级
    GSR.ErrorLevel = AlarmGetLevel();
}

/// @brief 报警处理
/// @param
void alarm_handle(void)
{
    // 报警检查
    alarm_check();

    // 报警处理
    if (GSR.ErrorLevel == 4)
    {
        // 系统需重启
        GUR.HaveToRepower = 1;
        // 写入急停
        cmd_set(CMD_SCRAM);
    }
    if (GSR.ErrorLevel == 3)
    {
        // 系统需复位
        GUR.HaveToReset = 1;
    }

    if (GUR.RunStatus == RUN && GSR.ErrorLevel > 1)
    {
        cmd_set(CMD_STOP);
    }

    // 急停状态处理
    if (GUR.RunStatus == SCRAM)
    {
        /** 急停时 最后把所有轴的指令清0 */
        for (int i = 0; i < pBsp->axis_num_pluse + pBsp->axis_num_ethercat; i++)
        {
            //mc_stop(i);
        }
    }
}
