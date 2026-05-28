#ifndef DATA_DEFINE_H
#define DATA_DEFINE_H

#include "vendor_common.h"
#include "bsp_config.h"

#define AXIS_MAX_NUM   100 // 轴最大数量(实际是64+8=72)这里是为了地址表预留使用
#define OUT_MAX_NUM    300 // 输出最大数量
#define IN_MAX_NUM     300 // 输入最大数量

// 特殊定义
#define AX_USED        60  // 使用轴数量(按逻辑轴最大定义的数量填写)
#define TERMINAL_USED  2   // 端子机使用数量

/* modbus register map */
#define READ_MAP_SIZE  1000 /* 读寄存器个数 */
#define WRITE_MAP_SIZE 1000 /* 写寄存器个数 */
#define SAVE_MAP_SIZE  8000 /* 保存寄存器个数 */
#define CFM_SAVE_SIZE  4000 /* CFM保存数据总字节(FRam 2400 + File 1600) */

typedef struct
{
    u16 year;  // 432
    u16 month; // 433
    u16 day;   // 434
    u16 hour;  // 435
    u16 min;   // 436
    u16 sec;   // 437
} TimeDef;

typedef struct
{
    u32 checkflag; // 数据是否被初始化
    u32 Lockflag;  // 锁机标志 0xbb 永久解锁 0xab 日期解锁 0xff锁定了
    u32 year;      //
    u32 month;     //
    u32 day;       //
    u32 hour;      //
    u32 min;       //
    u32 sec;       //
    u32 LOCK_ID;   // 板卡ID
    u32 Define_ID; // 手动输入ID
    u32 res;       // 预留
} SaveCheck;
typedef struct
{
    struct
    {
        float max_speed; // 最大速度 unit/s 2000
        float acc_time;  // 加速时间 ms 2002
        float dec_time;  // 减速时间 ms 2004
        float home_vel;  // 找原点速度 unit/s 2006
    } speed;
    struct
    {
        struct
        {
            u32 soft_pos_limit : 1;         // 软件正限位
            u32 soft_neg_limit : 1;         // 软件负限位
            u32 hardware_pos_limit : 1;     // 硬件正限位
            u32 hardware_neg_limit : 1;     // 硬件负限位
            u32 hardware_pos_limit_lev : 1; // 硬件正限位有效电平
            u32 hardware_neg_limit_lev : 1; // 硬件负限位有效电平
            u32 alm_en : 1;                 // 轴报警信号使能
            u32 alm_single_lev : 1;         // 报警电平
            u32 alm_use_ext : 1;            // 使用外部信号(0:使用内部信号,1:使用外部信号)
            u32 arv_en : 1;                 // 轴到位信号使能
            u32 arv_single_lev : 1;         // 到位电平
            u32 arv_use_ext : 1;            // 使用外部信号(0:使用内部信号,1:使用外部信号)
            u32 org_single_lev : 1;         // 原点电平
        } set;                              // 2008
        u32   pos_limit_idx;                // 正限位信号索引 2010
        u32   neg_limit_idx;                // 负限位信号索引 2012
        u32   alm_ext_idx;                  // 报警信号索引(在alm_use_ext为1时有效) 2014
        u32   arv_ext_idx;                  // 到位信号索引(在arv_use_ext为1时有效) 2016
        u32   org_idx;                      // 原点信号索引 2018
        u32   home_type;                    // 找原点类型 2020
        float org_offset;                   // 原点偏移 unit 2022
        float max_pos;                      // 最大位置(soft_pos_limit为1时有效) unit 2024
        float min_pos;                      // 最小位置(soft_neg_limit为1时有效) unit 2026
        u32   PPR;                          // 每转脉冲 脉冲数 2028
        float MPR;                          // 导程 mm 2030
    } Config;
} axis_data; // 64字节 32个地址

typedef struct _jog_ctr
{
    // 正向点动 每一位代表一个轴
    u32 JogForward[3]; // 1100-1102-1104
    // 反向点动 每一位代表一个轴
    u32 JogBackward[3]; // 1106-1108-1110
    // 回原点 每一位代表一个轴
    u32 JogGohome[3]; // 1112-1114-1116
    // 速度比例
    float JogSpeedRate; // 1118
    // 点动类型 0连续 1单步
    u32 JogType; // 1120
    // 每次移动行程 在位置点动时生效
    float JogLen;     // 1122
    u32   jog_res[3]; // 1124 1126 1128 预留
} jog_ctr;

typedef struct
{
    struct
    {
        union // 地址从0-499
        {
            u16 Sys_Read_Addr[READ_MAP_SIZE / 2];
            struct
            {
                u32     HardWare_Ver[10];     // 0-19	硬件版本
                u32     SoftWare_Ver[10];     // 20-39 软件版本
                float   AxisUnitPosition[70]; // 40 当前轴位置
                u32     AxisState[65];        // 180 轴状态目前地址长度不够了
                u32     InputStatus[40];      // 310-389 输入口状态
                u32     ErrorCode[20];        // 390 1级报警390 2级398 3级406 4级414 5级422
                u32     ErrorLevel;           // 430 错误等级
                TimeDef CurrTime;             // 432 rtc 时间
                u32 axis_used;                // 438 轴使用情况
            } Data;
        } Sys;
        union // 地址从500-999
        {
            u16 User_Read_Addr[READ_MAP_SIZE / 2];
            struct
            {
                u32     RunStatus;      // 500 运行状态显示
                u32     HaveToReset;    // 502 复位标志
                u32     HaveToRepower;  // 504 重启标志
                s32     HaveToSetData;  // 506 设置数据标志 -1数据错误 0数据正常 1数据开始保存
                u32     UPH;            // 508 设备产量
                s32     CurrStopTime;   // 510 把数到达倒计时
                u32     res[3];         // 512 读取地址预留
                u16     TaskStep[100];  // 518 - 617 任务执行步骤
                u16     TaskExe[100];   // 618 - 717 任务是否执行
                int     Remainder_days; // 718 剩余的天数
                TimeDef LockTime;       // 720 锁机到期时间
            } Data;
        } User;
    } Read;

    struct
    {
        union // 地址从1000-1499
        {
            u16 Sys_Write_Addr[WRITE_MAP_SIZE / 2];
            struct
            {
                u32     res[10];          // 1000-1019 保留
                u32     OutputStatus[40]; // 1020-1099 输出口状态
                jog_ctr Jog;              // 1100-1119 点动操作
                u32     ClearAlarm;       // 1130      清除错误
                u32     ManualWorld;      // 1132      手动操作字
                u32     Decode[4];        // 1134      用户输入密码解锁
            } Data;
        } Sys;
        union // 地址从1500-1999
        {
            u16 User_Write_Addr[WRITE_MAP_SIZE / 2];
            struct
            {
                struct
                {
                    u32 RunCommand;   // 1500
                    u32 DeviceMode;   // 1502
                    u32 StopCache;    // 1504 停机标识表示需要停止
                    u32 HMILOCK;      // 1506 触摸屏锁
                    u32 Unlock;       // 1508 注册
                    u32 CylinderTest; // 1510 气缸测试
                    u32 AxisTest;     // 1512 轴测试
                    u32 TestMode;     // 1514 测试模式
                    u32 HMICMD;       // 1516 触摸屏命令
                    u32 AxisEn;       // 1518 轴使能
                } Button;
                u32 TechWord; // 1520

            } Data;
        } User;
    } Write;

    struct
    {
        union // 地址从2000-5999
        {
            u16 Sys_Save_Addr[SAVE_MAP_SIZE / 2];
            struct
            {
                axis_data axis[AXIS_MAX_NUM];    // 2000-5199 32*100=3200
                u16       opentime[OUT_MAX_NUM]; // 5200-5499 300
                u16       clostime[IN_MAX_NUM];  // 5500-5799 300
                SaveCheck LockPara;              // 5800-5819
            } Data;
        } Sys;
        union // 地址从6000-9999
        {
            u16 User_Save_Addr[SAVE_MAP_SIZE / 2];
            struct
            {
                union // 配置参数
                {
                    u16 config_addr_16[400]; // 6000-6399
                    struct
                    {
                        struct
                        {
                            // 功能开关 使用位域结构体
                            struct
                            {
                                u32 res : 1; // 6000.00
                            } all_fun;       // 6000
                            struct
                            {
                                u32 res : 1; // 6002.00
                            } check;         // 6002
                            u32 other[8];
                        } function; // 6004

                        struct
                        {
                            // 速度信息
                            u32 all_spd;    // 6020 整机速度
                            u32 reset_spd;  // 6022 复位速度
                            u32 terminal_1; // 6024 端子机1打端速度
                            u32 terminal_2; // 6026 端子机2打端速度
                            u32 speed[46];
                        } speed;

                        struct
                        {
                            u32 set_num;    // 6120 产量设置
                            u32 cur_num;    // 6122 当前总产量
                            u32 set_bundle; // 6124 把数设置
                            u32 cur_bundle; // 6126 当前把数
                            // 其它产品信息
                        } product;
                    } Data;
                } config;

                union // 位置参数
                {
                    u16 pos_addr_16[1200]; // 6400-7599
                    struct
                    {
                        struct
                        {
                            float ready;   // 6400 准备位置
                            float cut_off; // 6402 铆压位置
                            float cur_pos; // 6404 当前位置
                            float pos[12];
                        } terminal_1;
                        struct
                        {
                            float ready;   // 6430 准备位置
                            float cut_off; // 6432 铆压位置
                            float cur_pos; // 6434 当前位置
                            float pos[12];
                        } terminal_2;
                    } Data;
                } pos;

                union // 延时参数
                {
                    u16 delay_addr_16[100]; // 7600-7699
                    struct
                    {
                        u32 terminal_done_1; // 7600 端子机1保压时间
                        u32 terminal_done_2; // 7602 端子机2保压时间
                    } Data;
                } delay;

#if (defined(CFG_CFM_EN) && CFG_CFM_EN == 1)
                u8 cfm_reserved[CFM_SAVE_SIZE]; // CFM数据铁电占位(FRam 2400 + File 1600)
#endif
            } Data;
        } User;
    } Save;
} GlobalDataDef;

extern GlobalDataDef GlobalData;

#define GSR GlobalData.Read.Sys.Data
#define GSW GlobalData.Write.Sys.Data
#define GSS GlobalData.Save.Sys.Data
#define GUR GlobalData.Read.User.Data
#define GUW GlobalData.Write.User.Data
#define GUS GlobalData.Save.User.Data

#define GCFG GUS.config.Data
#define GPOS GUS.pos.Data
#define GDLY GUS.delay.Data

#endif
