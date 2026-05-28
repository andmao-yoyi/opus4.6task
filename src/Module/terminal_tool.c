#include "Logic.h"
#include "vendor_sdk.h"
#include "hpm_interlayer.h"

#define TerminalPulse 20000 // 端子机一圈脉冲数(加减速机)
#define TerminalMPR 360     // 端子机一圈行程
#define Slip_Travel 30.0f   // 端子机滑块最大行程，也可使用可设定参数

// 端子机角度计算
/***************************************************
//传入参数 RatoLength：端子机凸轮圆半径   TarPos：目标压接高度
//return 端子旋转角度
***************************************************/

float terminal_angle_calulate(float RatoLength, float TarPos)
{
    static float TargAngle = 0;
    float R, Height;
    if (TarPos == 0)
    {
        return 0;
    }
    if (TarPos == RatoLength)
    {
        return 90;
    }
    if (TarPos == (2 * RatoLength))
    {
        return 180;
    }

    if (TarPos > ((2 * RatoLength)))
    { // 传入高度大于最高压接高度按照最高压接高度算 单圈模式除外
        return 180;
    }
    else
    {
        if (TarPos > RatoLength)
        { // 下半圈
            Height = (float)(TarPos - RatoLength) / 1;
            R = (float)RatoLength / 1;
            TargAngle = (float)((acos(Height / R) / 3.14f) * 180.00f);
            TargAngle = 180.00f - TargAngle;
            return TargAngle;
        }
        else
        { // 上半圈
            Height = (float)(TarPos) / 1;
            R = (float)RatoLength / 1;
            TargAngle = (float)((acos((R - Height) / R) / 3.14f) * 180.00f);
            return TargAngle;
        }
    }
}

/***************************************************
//把旋转角度换算成高度
//传入参数 rad：旋转半径   angle：角度
//return 端子机滑块移动高度
***************************************************/
float angle_to_height(float rad, float angle)
{
    float height = rad - rad * (float)cos(angle / 180.00f * 3.1415926f);
    return height;
}

/***************************************************
//把压接高度转换角度
//传入参数 len：滑块下降距离   Slip_Travel：端子机滑块最大行程（标准行程：30mm或40mm）
//return 端子机凸轮旋转角度
***************************************************/
float height_to_angle(float len)
{
    return terminal_angle_calulate(Slip_Travel / 2.00f, len);
}

/********************************************************************************************/

/********************************************************************************************
//打端子动作示例：单圈模式、半圈模式（精度模式）
//伺服驱动设定电机一圈脉冲数1800，若减速机比是10比，那么板卡的单圈脉冲数要设置1800*10=18000，即端子机转一圈要18000脉冲
//该算法计算的起点是凸轮圆的最顶点，所以原点最好是滑块停在最顶点
********************************************************************************************/

// 端子机冲头当前位置转换
void terminal_cur_pos(logic_para_t *LG)
{
    servo_terminal_def *servoterminal = LG->para;
    float TerminalCurrPulse; // 当前角度
    float TerminalPos;
    GSS.axis[servoterminal->axis_num].Config.set.soft_pos_limit = 0; // 没有限位
    GSS.axis[servoterminal->axis_num].Config.set.soft_neg_limit = 0; // 没有限位
    // 端子机当前位置
    TerminalCurrPulse = fmod(fabs(GSR.AxisUnitPosition[servoterminal->axis_num]), TerminalMPR);
    TerminalPos = angle_to_height(Slip_Travel / 2.00f, TerminalCurrPulse);

    // 冲头当前位置
    *servoterminal->cur_pos = TerminalPos;
    if (TerminalPos > Slip_Travel) // 大于半圈位置
    {
        *servoterminal->cur_pos = Slip_Travel * 2 - TerminalPos;
    }

    // 手动限制
    GSS.axis[servoterminal->axis_num].Config.max_pos = Slip_Travel;
    GSS.axis[servoterminal->axis_num].Config.min_pos = 0.f;
    if ((*servoterminal->cur_pos >= GSS.axis[servoterminal->axis_num].Config.max_pos) &&
        (GSW.Jog.JogForward[0] >> servoterminal->axis_num & 1))
    {
        mc_stop(servoterminal->axis_num);
    }

    if ((*servoterminal->cur_pos <= GSS.axis[servoterminal->axis_num].Config.min_pos) &&
        (GSW.Jog.JogBackward[0] >> servoterminal->axis_num & 1))
    {
        mc_stop(servoterminal->axis_num);
    }

    // 输入限制
    if ((*servoterminal->terminal_pos >= GSS.axis[servoterminal->axis_num].Config.max_pos))
    {
        *servoterminal->terminal_pos = GSS.axis[servoterminal->axis_num].Config.max_pos;
    }
    if ((*servoterminal->terminal_pos <= GSS.axis[servoterminal->axis_num].Config.min_pos))
    {
        *servoterminal->terminal_pos = GSS.axis[servoterminal->axis_num].Config.min_pos;
    }
    if ((*servoterminal->ready_pos >= GSS.axis[servoterminal->axis_num].Config.max_pos) ||
        (*servoterminal->ready_pos <= GSS.axis[servoterminal->axis_num].Config.min_pos))
    {
        *servoterminal->ready_pos = 0.f;
    }

    // 端子机点动下
    static trig_def ter_tr[TERMINAL_USED][4];
    if (trig_one(&ter_tr[servoterminal->idx][0], in_get(servoterminal->input_down) == ON, 5) &&
        (GUR.RunStatus == STOP || GUR.RunStatus == INIT))
    {
        mc_move_abs_rate(servoterminal->axis_num, height_to_angle(GSS.axis[servoterminal->axis_num].Config.max_pos), spd_rate(3));
    }
    if (trig_one(&ter_tr[servoterminal->idx][1], in_get(servoterminal->input_down) == OFF, 5))
    {
        mc_stop(servoterminal->axis_num);
    }
    // 端子机点动上
    if (trig_one(&ter_tr[servoterminal->idx][2], in_get(servoterminal->input_up) == ON, 5) &&
        (GUR.RunStatus == STOP || GUR.RunStatus == INIT))
    {
        mc_move_abs_rate(servoterminal->axis_num, height_to_angle(GSS.axis[servoterminal->axis_num].Config.min_pos), spd_rate(3));
    }
    if (trig_one(&ter_tr[servoterminal->idx][3], in_get(servoterminal->input_up) == OFF, 5))
    {
        mc_stop(servoterminal->axis_num);
    }
}

// 端子机复位
void servo_terminal_reset(logic_para_t *LG)
{
    servo_terminal_def *servoterminal = LG->para;
    TASK_PROCESS()
    {
    case 1:
        if (in_get(servoterminal->input) == ON)
        {
            // 正常回原点 (反向)
            mc_home(servoterminal->axis_num);
            STEP++;
        }
        else
        {
            mc_move_abs_rate(servoterminal->axis_num, 200.0f, spd_rate(8)); // 正向回原点
            STEP = 0xa1;
        }
        break;
    case 0xa1:
        if (in_get(servoterminal->input) == ON)
        {
            mc_stop(servoterminal->axis_num);
            STEP = 1;
        }
        break;
    case 2:
        if (mc_ready(servoterminal->axis_num))
        {
            STEP = ENDCASE; // 结束
        }
        break;
    case 3:
        if (mc_ready(servoterminal->axis_num) && TCNT > 200)
        {
            mc_move_abs_rate(servoterminal->axis_num, -200.0f, spd_rate(3)); // 慢速往回跑
            STEP++;
        }
        break;
    case 4:
        if (in_get(servoterminal->input) == OFF) // 常开信号 碰到了感应器
        {
            STEP++;
        }
        break;
    case 5:
        if (in_get(servoterminal->input) == ON) // 离开了感应器
        {
            mc_stop(servoterminal->axis_num);
            STEP++;
        }
        break;
    case 6:
        if (mc_ready(servoterminal->axis_num)) // 确保轴停下
        {
            mc_move_rel_rate(servoterminal->axis_num, -(GSR.AxisUnitPosition[servoterminal->axis_num] * 0.5f),
                               spd_rate(30)); // 走到正中间
            STEP++;
        }
        break;
    case 7:
        if (mc_ready(servoterminal->axis_num))
        {
            mc_set_cur_unit_pos(servoterminal->axis_num, 0);
            STEP++;
        }
        break;
    case 8: // 去预备位
        if (mc_ready(servoterminal->axis_num) && TCNT > 200)
        {
            mc_move_rel_rate(servoterminal->axis_num, height_to_angle(*servoterminal->ready_pos), spd_rate(30));
            STEP++;
        }
        break;
    case 9:
        if (mc_ready(servoterminal->axis_num))
        {
            STEP = ENDCASE;
        }
        break;
    case ENDCASE:
        LT_End;
        break;
    }
}

// 半圈模式（精度模式）
void accurate(logic_para_t *LG)
{
    servo_terminal_def *servoterminal = LG->para;
    TASK_PROCESS()
    {
    case 1:
        if (mc_ready(servoterminal->axis_num))
        {
            mc_move_abs_rate(servoterminal->axis_num, height_to_angle(*servoterminal->terminal_pos),
                               spd_rate(*servoterminal->terminal_spd));
            STEP++;
        }
        break;
    case 2:
        if (mc_ready(servoterminal->axis_num))
        {
            STEP++;
        }
        break;
    case 3: // 保压时间，滑块返回0位
        if (TCNT > *servoterminal->workdone_delay)
        {
            mc_move_abs_rate(servoterminal->axis_num, height_to_angle(*servoterminal->ready_pos), spd_rate(*servoterminal->terminal_spd));
            STEP++;
        }
        break;
    case 4:
        if (mc_ready(servoterminal->axis_num))
        {
            STEP = ENDCASE;
        }
        break;
    case ENDCASE:
        LT_End;
        break;
    }
}

// 单圈模式
void Single(logic_para_t *LG)
{
    servo_terminal_def *servoterminal = LG->para;
    TASK_PROCESS()
    {
    case 1:
        if (mc_ready(servoterminal->axis_num))
        {
            mc_move_rel_rate(servoterminal->axis_num, 360.0f, spd_rate(*servoterminal->terminal_spd));
            STEP++;
        }
        break;
    case 2:
        if (mc_ready(servoterminal->axis_num))
        {
            mc_set_cur_unit_pos(servoterminal->axis_num, 0);
            STEP = ENDCASE;
        }
        break;
    case ENDCASE:
        LT_End;
        break;
    }
}

/// @brief 伺服端子机参数
servo_terminal_def Terminal_1 = {.idx = 0,
                                 .axis_num = axis9_terminal_a,
                                 .input = I_terminal_1,
                                 .input_up = I_terminal_up_1,
                                 .input_down = I_terminal_down_1,
                                 .workdone_delay = &GDLY.terminal_done_1,
                                 .terminal_spd = &GCFG.speed.terminal_1,
                                 .ready_pos = &GPOS.terminal_1.ready,
                                 .terminal_pos = &GPOS.terminal_1.cut_off,
                                 .cur_pos = &GPOS.terminal_1.cur_pos};
servo_terminal_def Terminal_2 = {.idx = 1,
                                 .axis_num = axis10_terminal_b,
                                 .input = I_terminal_2,
                                 .input_up = I_terminal_up_2,
                                 .input_down = I_terminal_down_2,
                                 .workdone_delay = &GDLY.terminal_done_2,
                                 .terminal_spd = &GCFG.speed.terminal_2,
                                 .ready_pos = &GPOS.terminal_2.ready,
                                 .terminal_pos = &GPOS.terminal_2.cut_off,
                                 .cur_pos = &GPOS.terminal_2.cur_pos};

/// @brief 伺服端子机相关的函数
void servo_terminal(void)
{
    logic_task.ter_cur_oos_task[0].para = &Terminal_1;
    logic_task.ter_cur_oos_task[1].para = &Terminal_2;

    logic_task.servo_terminal_reset_task[0].para = &Terminal_1;
    logic_task.servo_terminal_reset_task[1].para = &Terminal_2;

    logic_task.accurate_task[0].para = &Terminal_1;
    logic_task.accurate_task[1].para = &Terminal_2;

    logic_task.single_task[0].para = &Terminal_1;
    logic_task.single_task[1].para = &Terminal_2;

    for (int i = 0; i < TERMINAL_USED; i++)
    {
        if (GUR.RunStatus == RUN)
        {
            accurate(&logic_task.accurate_task[i]); // 半圈打端一次
        }
        terminal_cur_pos(&logic_task.ter_cur_oos_task[i]);              // 端子机冲头当前位置转换
        servo_terminal_reset(&logic_task.servo_terminal_reset_task[i]); // 复位
        Single(&logic_task.single_task[i]);                             // 单圈打端一次
    }
}

// 伺服点动函数
int servo_terminal_jog_go(u32 axisnum, float pos, float spd)
{
    static float terminal_tar;
    static float terminal_cur;

    if (axisnum == Terminal_1.axis_num)
    {
        terminal_cur = GPOS.terminal_1.cur_pos;
    }
    else if (axisnum == Terminal_2.axis_num)
    {
        terminal_cur = GPOS.terminal_2.cur_pos;
    }
    else
    {
        return -1;
    }
    if (GSW.Jog.JogType) // 单步点动
    {
        if (GSW.Jog.JogForward[0] & (1 << axisnum))
        {
            if ((terminal_cur + GSW.Jog.JogLen >= GSS.axis[axisnum].Config.max_pos))
                terminal_tar = GSS.axis[axisnum].Config.max_pos;
            else
                terminal_tar = terminal_cur + GSW.Jog.JogLen;
        }
        else if (GSW.Jog.JogBackward[0] & (1 << axisnum))
        {
            if ((terminal_cur - GSW.Jog.JogLen <= GSS.axis[axisnum].Config.min_pos))
                terminal_tar = GSS.axis[axisnum].Config.min_pos;
            else
                terminal_tar = terminal_cur - GSW.Jog.JogLen;
        }
        mc_move_abs_rate(axisnum, height_to_angle(terminal_tar), spd_rate(100));
        return 0;
    }
    if (pos >= 0)
    {
        mc_move_abs_rate(axisnum, height_to_angle(GSS.axis[axisnum].Config.max_pos), spd_rate(spd));
    }
    else
    {
        mc_move_abs_rate(axisnum, height_to_angle(GSS.axis[axisnum].Config.min_pos), spd_rate(spd));
    }
    return 0;
}

