#include "logic.h"
#include "reset_axis.h"

static logic_para_t reset_task;

void reset_callback(void)
{
    reset_task.execute = 1;
    debug_printf("reset_callback\n");
}

// 需要把所有定义的逻辑轴加上
reset_axis ax_rst[] = {
    {.axis_num = axis1_pull, NULL},
};

#define AX_RST_COUNT (sizeof(ax_rst) / sizeof(ax_rst[0]))

void dev_reset(void)
{
    logic_para_t *LG = &reset_task;
    // 轮询函数
    for (int i = 0; i < AX_RST_COUNT; i++)
    {
        axis_to_ready_pos(&logic_task.ax_home_task[i], &ax_rst[i]);
    }
    if (GUR.RunStatus == RESET)
    {
        TASK_PROCESS()
        {
        case 1:
            // 清除任务调度
            PARAINIT(logic_task);
            // 清除报警
            GSW.ClearAlarm = 1;
            // 轴口初始化
            init_axis();
            // 打开使能
            GUW.Button.AxisEn = 1;
            STEP++;
            break;
        case 2: // 初始化 输出口
            init_IO();
            STEP++;
            break;
        case 3:
            if (TCNT >= 500)
            {
                STEP = ENDCASE;
            }
            break;
        case ENDCASE:
            // 清除所有标记
            PARAINIT(data);
            GUR.HaveToReset = 0;
            GUW.Button.RunCommand = STOP;
            GUW.Button.DeviceMode = STEPMODE;
            GSW.ClearAlarm = 1;
            GUW.Button.StopCache = 0;
            PARAINIT(reset_task);
            break;
        }
    }
    else
    {
        PARAINIT(reset_task);
    }
}
