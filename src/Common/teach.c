#include "teach.h"
#include "logic.h"

/// @brief 数据初始化
void data_reset(void)
{
    // 默认配置50个轴参数 默认关闭全部报警和限位 速度50k 加减速时间100ms
    for (int i = 0; i < 50; i++)
    {
        memset(&GSS.axis[i], 0, sizeof(axis_data));
        GSS.axis[i].speed.max_speed = 50000;
        GSS.axis[i].speed.acc_time = 100;
        GSS.axis[i].speed.dec_time = 100;
        GSS.axis[i].speed.home_vel = 10000;
        GSS.axis[i].Config.set.alm_en = 0;
        GSS.axis[i].Config.set.arv_en = 0;
        GSS.axis[i].Config.set.soft_pos_limit = 0;
        GSS.axis[i].Config.set.soft_neg_limit = 0;
        GSS.axis[i].Config.set.hardware_pos_limit = 0;
        GSS.axis[i].Config.set.hardware_neg_limit = 0;
        GSS.axis[i].Config.set.hardware_pos_limit_lev = ON;
        GSS.axis[i].Config.set.hardware_neg_limit_lev = ON;
        GSS.axis[i].Config.set.org_single_lev = ON;
        GSS.axis[i].Config.org_idx = i;
        GSS.axis[i].Config.home_type = 0;
        GSS.axis[i].Config.org_offset = 0;
        GSS.axis[i].Config.max_pos = 999999;
        GSS.axis[i].Config.min_pos = 0;
        GSS.axis[i].Config.MPR = 1.0f;
        GSS.axis[i].Config.PPR = 1;
    }
    // 数据已经正常 可以进行数据保存
    if (GUR.HaveToSetData != 1)
    {
        GUR.HaveToSetData = 0;
    }
}

// 教示
void teach(void)
{
    if (GUW.TechWord)
    {
        switch (GUW.TechWord)
        {
        case 1:
            // 数据初始化
            data_reset();
            break;
        case 2:
            break;
        }
        GUW.TechWord = 0;
    }
}
