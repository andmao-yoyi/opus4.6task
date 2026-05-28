/**
 * @file reset_axis.c
 * @author Application Team
 * @brief 各个轴回原点 然后走到自己的预备位
 * @version 0.1
 * @date 2023-03-12
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "reset_axis.h"
#include "Alarm.h"
#include "logic.h"
#include "hpm_interlayer.h"

void axis_to_ready_pos(logic_para_t *LG, reset_axis *pAxis)
{
    if (GCFG.speed.reset_spd == 0)
    {
        GCFG.speed.reset_spd = 5;
    }
    TASK_PROCESS()
    {
    case 1:
        if (mc_ready(pAxis->axis_num))
        {
            mc_home(pAxis->axis_num);
            STEP = 2;
            LG->tm_Exe = sys_get_tick();
        }
        break;
    case 2:
        if (mc_ready(pAxis->axis_num))
        {
            if (pAxis->ready_pos != NULL)
            {
                mc_move_abs_rate(pAxis->axis_num, *pAxis->ready_pos, spd_rate(GCFG.speed.reset_spd));
            }
            STEP = 3;
        }
        if ((sys_get_tick() - LG->tm_Exe) > 350000) // 35秒内没完成
        {
            GUW.Button.RunCommand = STOP;
            LT_End;
            AlarmSetBit(ALM3_RESET_E(pAxis->axis_num + 1));
        }
        break;
    case 3:
        if (mc_ready(pAxis->axis_num))
        {
            STEP = ENDCASE;
        }
        break;
    case ENDCASE:
        LT_End;
        break;
    }
}
