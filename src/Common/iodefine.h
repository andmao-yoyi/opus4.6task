#ifndef IODEFINE_H
#define IODEFINE_H

#define Tr(x) x
#define X(a)  (a - 1)
#define Y(a)  (a - 1)

// 轴号定义
typedef enum
{
    axis1_pull,
    axis2_front_x,
    axis3_front_y,
    axis4_rear_x,
    axis5_rear_y,
    axis_trun,
    axis7_cut,
    axis8_unload,
    // 伺服端子机相关轴，若无使用，可注释掉
    axis9_terminal_a,
    axis10_terminal_b,
} AxisNum;

// 逻辑轴按物理轴顺序的映射列表，未接入的轴请从列表中移除
#define AXIS_LOGIC_USED_LIST_DEFAULT \
    {                                \
        axis1_pull,                  \
        axis2_front_x,               \
        axis3_front_y,               \
        axis4_rear_x,                \
        axis5_rear_y,                \
        axis_trun,                   \
        axis7_cut,                   \
        axis8_unload,                \
        axis9_terminal_a,            \
        axis10_terminal_b,           \
    }

#define AXIS_LOGIC_USED_LIST_TYPEA \
    {                                \
        axis1_pull,                  \
        axis2_front_x,               \
        axis3_front_y,               \
        axis4_rear_x,                \
        axis5_rear_y,                \
        axis7_cut,                   \
        axis9_terminal_a,            \
        axis10_terminal_b,           \
    }

// 输出定义
#define Q_Y1 Y(1)      //
#define Q_Y2 Y(2)      //
#define Q_Y3 Y(3)      //
#define Q_Y4 Y(4)      //
#define Q_Y5 Y(5)      //
#define Q_Y6 Y(6)      //
#define Q_Y7 Y(7)      //
#define Q_Y8 Y(8)      //
#define Q_Y9 Y(9)      //
#define Q_Y10 Y(10)    //
#define Q_Y11 Y(11)    //
#define Q_Y12 Y(12)    //
#define Q_Y13 Y(13)    //
#define Q_Y14 Y(14)    //
#define Q_Y15 Y(15)    //
#define Q_Y16 Y(16)    //
#define Q_green Y(17)  // 运行灯
#define Q_yellow Y(18) // 停止灯
#define Q_red Y(19)    // 警报灯
#define Q_Y20 Y(20)    //
#define Q_Y21 Y(21)    //
#define Q_Y22 Y(22)    //
#define Q_Y23 Y(23)    //
#define Q_Y24 Y(24)    //

// 1号扩展卡输出
#define Q_Y25 Y(25) //
#define Q_Y26 Y(26) //
#define Q_Y27 Y(27) //
#define Q_Y28 Y(28) //
#define Q_Y29 Y(29) //
#define Q_Y30 Y(30) //
#define Q_Y31 Y(31) //
#define Q_Y32 Y(32) //
#define Q_Y33 Y(33) //
#define Q_Y34 Y(34) //
#define Q_Y35 Y(35) //
#define Q_Y36 Y(36) //
#define Q_Y37 Y(37) //
#define Q_Y38 Y(38) //
#define Q_Y39 Y(39) //
#define Q_Y40 Y(40) //
#define Q_Y41 Y(41) //
#define Q_Y42 Y(42) //
#define Q_Y43 Y(43) //
#define Q_Y44 Y(44) //
#define Q_Y45 Y(45) //
#define Q_Y46 Y(46) //
#define Q_Y47 Y(47) //
#define Q_Y48 Y(48) //
#define Q_Y49 Y(49) //
#define Q_Y50 Y(50) //
#define Q_Y51 Y(51) //
#define Q_Y52 Y(52) //
#define Q_Y53 Y(53) //
#define Q_Y54 Y(54) //
#define Q_Y55 Y(55) //
#define Q_Y56 Y(56) //

// 输入定义
#define I_start X(1) // 启动按钮
#define I_stop X(2)  // 停止按钮
#define I_scram X(3) // 急停按钮
#define I_reset X(4) // 复位按钮
#define I_X5 X(5)    //
#define I_X6 X(6)    //
#define I_X7 X(7)    //
#define I_X8 X(8)    //
#define I_X9 X(9)    //
#define I_X10 X(10)  //
#define I_X11 X(11)  //
#define I_X12 X(12)  //
#define I_X13 X(13)  //
#define I_X14 X(14)  //
#define I_X15 X(15)  //
#define I_X16 X(16)  //
#define I_X17 X(17)  //
#define I_X18 X(18)  //
#define I_X19 X(19)  //
#define I_X20 X(20)  //
#define I_X21 X(21)  //
#define I_X22 X(22)  //
#define I_X23 X(23)  //
#define I_X24 X(24)  //

// 扩展卡1输入
#define I_X25 X(25) //
#define I_X26 X(26) //
#define I_X27 X(27) //
#define I_X28 X(28) //
#define I_X29 X(29) //
#define I_X30 X(30) //
#define I_X31 X(31) //
#define I_X32 X(32) //
#define I_X33 X(33) //
#define I_X34 X(34) //
#define I_X35 X(35) //
#define I_X36 X(36) //
#define I_X37 X(37) //
#define I_X38 X(38) //
#define I_X39 X(39) //
#define I_X40 X(40) //
#define I_X41 X(41) //
#define I_X42 X(42) //
#define I_X43 X(43) //
#define I_X44 X(44) //
#define I_X45 X(45) //
#define I_X46 X(46) //
#define I_X47 X(47) //
#define I_X48 X(48) //
#define I_X49 X(49) //
#define I_X50 X(50) //
#define I_X51 X(51) //
#define I_X52 X(52) //
#define I_X53 X(53) //
#define I_X54 X(54) //
#define I_X55 X(55) //
#define I_X56 X(56) //

// 伺服端子机相关输入输出，若无使用，可注释掉
#define I_terminal_1 X(100)      // 端子机1原点
#define I_terminal_2 X(101)      // 端子机2原点
#define I_terminal_up_1 X(102)   // 端子机1手动上
#define I_terminal_down_1 X(103) // 端子机1手动下
#define I_terminal_up_2 X(104)   // 端子机2手动上
#define I_terminal_down_2 X(105) // 端子机2手动下

#endif
