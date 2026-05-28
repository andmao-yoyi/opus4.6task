#include "data_define.h"
#include "hz_fsm.h"
#include "vendor_sdk.h"
#include "hpm_interlayer.h"
#include "logic.h"
#include "stdio.h"
#include "string.h"
#include <stdint.h>

// 状态机
HZ_FSM_Handle fsm;

// 回调函数
extern void reset_callback(void);
extern void scram_callback(void);
extern void run_callback(void);
extern void stop_callback(void);
extern void paused_callback(void);

// 数据初始化
void data_init(void);

// 状态转换表
HZ_FSM_TransItem trans_table[] = {
    {INIT, CMD_RESET, RESET, reset_callback},  {INIT, CMD_SCRAM, SCRAM, scram_callback}, {RESET, CMD_STOP, STOP, stop_callback},
    {RESET, CMD_SCRAM, SCRAM, scram_callback}, {STOP, CMD_RUN, RUN, run_callback},       {STOP, CMD_SCRAM, SCRAM, scram_callback},
    {RUN, CMD_STOP, STOP, stop_callback},      {RUN, CMD_SCRAM, SCRAM, scram_callback},  {SCRAM, CMD_STOP, STOP, stop_callback},
    {STOP, CMD_RESET, RESET, reset_callback},
};

/// @brief 设置命令
/// @param cmd 命令
void cmd_set(uint32_t cmd)
{
    hz_fsm_cmd(&fsm, cmd);
}

void hz_auto_vision()
{
    char              s_month[5];
    int               month, day, year;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    sscanf(__DATE__, "%s %d %d", s_month, &day, &year);
    month = (strstr(month_names, s_month) - month_names) / 3;
    sprintf((char *)GSR.SoftWare_Ver, "Soft:%d.%d.%d-%s", year, month + 1, day, __TIME__); // 版本号
    // 硬件版本信息暂时手动填写
    sprintf((char *)GSR.HardWare_Ver, "HVer: HMC-E08");
    debug_printf("SoftWare_Ver: %s\n", GSR.SoftWare_Ver);
    debug_printf("HardWare_Ver: %s\n", GSR.HardWare_Ver);
}

// 输出口初始化
void init_IO(void)
{
    for (int i = 0; i < OUT_MAX_NUM; i++)
    {
        out_set(i, OFF);
    }
}

// 轴口初始化
void init_axis(void)
{
    for (int i = 0; i < AXIS_MAX_NUM; i++)
    {
        axis_reset(i);
    }
}

void app_init(void)
{
    hz_auto_vision();
    // 初始化输出状态
    out_status_init(GSS.opentime, GSS.clostime);
    // 初始化状态机
    hz_fsm_init(&fsm, trans_table, sizeof(trans_table) / sizeof(trans_table[0]), INIT);
    // 数据初始化
    data_init();

}

void data_init(void)
{
    GSW.Jog.JogSpeedRate = 0.5;
    GSW.Jog.JogType = 0;
    GUR.HaveToReset = 1;
    GUW.Button.AxisEn = 1;
    GUW.Button.DeviceMode = STEPMODE;
    mc_param_load(GSS.axis);
}
