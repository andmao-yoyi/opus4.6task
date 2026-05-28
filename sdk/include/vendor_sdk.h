/**
 * @file vendor_sdk.h
 * @brief VENDOR SDK 公共接口库
 *
 * 该文件包含VENDOR平台的公共接口定义，包括系统配置、外设驱动、
 * 轴控制、IO操作等功能的接口声明。
 */

#ifndef VENDOR_SDK_H_
#define VENDOR_SDK_H_

#include <stdbool.h>
#include "stdint.h"
#include <stdint.h>

// ============================================================================
// 公共常量定义
// ============================================================================

/// @brief 每单位脉冲数的非0保护阈值（0.1微米精度场景下取较小值）
#define VENDOR_AXIS_PPU_EPS  (1.0e-6f)

/// @brief 轴组最大轴数
#define MAX_GROUP_AXIS_NUM 4

/// @brief 轴组最大数量
#define MAX_GROUP_ID       5

/* CAN 应用层公共常量（主从对象字典 / 组网约定） */

#ifndef CONFIG_CAN_MASTER_MAX_NODES
#define CONFIG_CAN_MASTER_MAX_NODES 4 /*!< 主站侧最大从站槽位或管理节点数（可被工程覆盖） */
#endif

#ifndef CAN_MAX_NODE_ID
#define CAN_MAX_NODE_ID CONFIG_CAN_MASTER_MAX_NODES /*!< 与 CONFIG_CAN_MASTER_MAX_NODES 对齐的节点规模上限 */
#endif

#ifndef CAN_MAX_AXIS_COUNT
#define CAN_MAX_AXIS_COUNT 16 /*!< 单块运动控制卡对象字典中单轴条目最大序号 +1（常用于 i 上界检查） */
#endif

#ifndef CAN_MAX_IO_BITS
#define CAN_MAX_IO_BITS 48 /*!< 单节点离散 IO / 脉冲通道在协议中的最大位数 */
#endif

#ifndef CAN_AXIS_MODE_RELATIVE
#define CAN_AXIS_MODE_RELATIVE 0U /*!< 轴模式：相对运动 */
#define CAN_AXIS_MODE_ABSOLUTE 1U /*!< 轴模式：绝对定位 */
#define CAN_AXIS_MODE_TORQUE   2U /*!< 轴模式：力矩 / 推力控制 */
#endif

/*===========================================================================
 * 对象字典地址映射
 * 主站通过 SDO 下发配置时使用这些地址；从站的 CanData.Data_8[] 按此布局。
 *===========================================================================*/
#define NODE_ID                  0 /*!< 从站节点号（NODE_ID）；SDO 索引 */
#define NODE_ID_LEN              1 /*!< NODE_ID 区长度（字节） */

#define BOARD_TYPE               1 /*!< 板类型 ID */
#define BOARD_TYPE_LEN           1

#define AXIS_NUM                 2 /*!< 本节点运动轴数量 */
#define AXIS_NUM_LEN             1

#define INPUTS_NUM               3 /*!< 离散输入点数 / 字数 */
#define INPUTS_NUM_LEN           1

#define OUTPUTS_NUM              4 /*!< 离散输出点数 / 字数 */
#define OUTPUTS_NUM_LEN          1

#define MANUFACTURER_ID          5 /*!< 制造商标识 */
#define MANUFACTURER_ID_LEN      2

#define PRODUCT_ID               7 /*!< 产品型号标识 */
#define PRODUCT_ID_LEN           4

#define FW_VERSION               11 /*!< 固件版本号（按协议编码） */
#define FW_VERSION_LEN           4

#define NODE_STATUS              63 /*!< 节点运行 / 在线状态字 */
#define NODE_STATUS_LEN          1

#define DFU_CONTROL              1294 /*!< 固件升级（DFU）控制寄存器 */
#define DFU_CONTROL_LEN          1

#define TRANSACTION_ID           1295 /*!< 事务 ID（命令/应答配对用） */
#define TRANSACTION_ID_LEN       4

/* --- 轴参数数组：每轴在 CanData.Data_8 中占位 *_LEN 字节，索引按 *_BASE + i * *_LEN 递增 (i = 0~15) --- */

/** 轴控制字（ControlWord），1 字节/轴 */
#define CONTROL_WORD_BASE        15
#define CONTROL_WORD_LEN         1
#define CONTROL_WORD(i)          (CONTROL_WORD_BASE + (i)*CONTROL_WORD_LEN)

/** 轴状态字（StatusWord），1 字节/轴 */
#define STATUS_WORD_BASE         31
#define STATUS_WORD_LEN          1
#define STATUS_WORD(i)           (STATUS_WORD_BASE + (i)*STATUS_WORD_LEN)

/** 轴运动模式（见 CAN_AXIS_MODE_*），1 字节/轴 */
#define AXIS_MODE_BASE           47
#define AXIS_MODE_LEN            1
#define AXIS_MODE(i)             (AXIS_MODE_BASE + (i)*AXIS_MODE_LEN)

/** 目标位置，4 字节/轴（编码方式与从站固件一致，多为 float/int32） */
#define TARGET_POSITION_BASE     64
#define TARGET_POSITION_LEN      4
#define TARGET_POSITION(i)       (TARGET_POSITION_BASE + (i)*TARGET_POSITION_LEN)

/** 目标力矩，2 字节/轴 */
#define TARGET_TORQUE_BASE       128
#define TARGET_TORQUE_LEN        2
#define TARGET_TORQUE(i)         (TARGET_TORQUE_BASE + (i)*TARGET_TORQUE_LEN)

/** 实际位置反馈，4 字节/轴 */
#define ACTUAL_POSITION_BASE     160
#define ACTUAL_POSITION_LEN      4
#define ACTUAL_POSITION(i)       (ACTUAL_POSITION_BASE + (i)*ACTUAL_POSITION_LEN)

/** 实际速度反馈，4 字节/轴 */
#define ACTUAL_VELOCITY_BASE     224
#define ACTUAL_VELOCITY_LEN      4
#define ACTUAL_VELOCITY(i)       (ACTUAL_VELOCITY_BASE + (i)*ACTUAL_VELOCITY_LEN)

/** 实际力矩反馈，2 字节/轴 */
#define ACTUAL_TORQUE_BASE       288
#define ACTUAL_TORQUE_LEN        2
#define ACTUAL_TORQUE(i)         (ACTUAL_TORQUE_BASE + (i)*ACTUAL_TORQUE_LEN)

/** 曲线最大速度 / 限速设定，4 字节/轴 */
#define MAX_VELOCITY_BASE        320
#define MAX_VELOCITY_LEN         4
#define MAX_VELOCITY(i)          (MAX_VELOCITY_BASE + (i)*MAX_VELOCITY_LEN)

/** 加速时间或加速段参数，4 字节/轴（单位与上位机/SDK 换算约定一致，常见为 ms） */
#define ACC_TIME_BASE            384
#define ACC_TIME_LEN             4
#define ACC_TIME(i)              (ACC_TIME_BASE + (i)*ACC_TIME_LEN)

/** 减速时间或减速段参数，4 字节/轴（与 ACC_TIME 成对使用，DEC_TIME(i) 为第 i 轴起始索引） */
#define DEC_TIME_BASE            448
#define DEC_TIME_LEN             4
#define DEC_TIME(i)              (DEC_TIME_BASE + (i)*DEC_TIME_LEN)

/** 硬件正限位使能，1 字节/轴 */
#define LIM_EN_UP_H_BASE         638
#define LIM_EN_UP_H_LEN          1
#define LIM_EN_UP_H(i)           (LIM_EN_UP_H_BASE + (i)*LIM_EN_UP_H_LEN)

/** 硬件负限位使能，1 字节/轴 */
#define LIM_EN_DW_H_BASE         654
#define LIM_EN_DW_H_LEN          1
#define LIM_EN_DW_H(i)           (LIM_EN_DW_H_BASE + (i)*LIM_EN_DW_H_LEN)

/** 软件正限位使能，1 字节/轴 */
#define LIM_EN_UP_S_BASE         670
#define LIM_EN_UP_S_LEN          1
#define LIM_EN_UP_S(i)           (LIM_EN_UP_S_BASE + (i)*LIM_EN_UP_S_LEN)

/** 软件负限位使能，1 字节/轴 */
#define LIM_EN_DW_S_BASE         686
#define LIM_EN_DW_S_LEN          1
#define LIM_EN_DW_S(i)           (LIM_EN_DW_S_BASE + (i)*LIM_EN_DW_S_LEN)

/** 正限位输入端口/信号序号，1 字节/轴 */
#define LIM_SIG_UP_BASE          702
#define LIM_SIG_UP_LEN           1
#define LIM_SIG_UP(i)            (LIM_SIG_UP_BASE + (i)*LIM_SIG_UP_LEN)

/** 负限位输入端口/信号序号，1 字节/轴 */
#define LIM_SIG_DW_BASE          718
#define LIM_SIG_DW_LEN           1
#define LIM_SIG_DW(i)            (LIM_SIG_DW_BASE + (i)*LIM_SIG_DW_LEN)

/** 正限位信号有效电平，1 字节/轴 */
#define LIM_SIG_UP_LEV_BASE      734
#define LIM_SIG_UP_LEV_LEN       1
#define LIM_SIG_UP_LEV(i)        (LIM_SIG_UP_LEV_BASE + (i)*LIM_SIG_UP_LEV_LEN)

/** 负限位信号有效电平，1 字节/轴 */
#define LIM_SIG_DW_LEV_BASE      750
#define LIM_SIG_DW_LEV_LEN       1
#define LIM_SIG_DW_LEV(i)        (LIM_SIG_DW_LEV_BASE + (i)*LIM_SIG_DW_LEV_LEN)

/** 软件正限位位置，4 字节/轴 */
#define LIM_POS_UP_BASE          766
#define LIM_POS_UP_LEN           4
#define LIM_POS_UP(i)            (LIM_POS_UP_BASE + (i)*LIM_POS_UP_LEN)

/** 软件负限位位置，4 字节/轴 */
#define LIM_POS_DW_BASE          830
#define LIM_POS_DW_LEN           4
#define LIM_POS_DW(i)            (LIM_POS_DW_BASE + (i)*LIM_POS_DW_LEN)

/** 电机每转脉冲数（PPR），4 字节/轴 */
#define PPR_BASE                 894
#define PPR_LEN                  4
#define PPR(i)                   (PPR_BASE + (i)*PPR_LEN)

/** 导程 / 脉冲当量相关（MPR），4 字节/轴 */
#define MPR_BASE                 958
#define MPR_LEN                  4
#define MPR(i)                   (MPR_BASE + (i)*MPR_LEN)

/** 扭矩/过载相关档位或缩放（TOR_S），1 字节/轴 */
#define TOR_S_BASE               1022
#define TOR_S_LEN                1
#define TOR_S(i)                 (TOR_S_BASE + (i)*TOR_S_LEN)

/** 伺服报警信号端口号（ALM_NUM），1 字节/轴 */
#define ALM_NUM_BASE             1038
#define ALM_NUM_LEN              1
#define ALM_NUM(i)               (ALM_NUM_BASE + (i)*ALM_NUM_LEN)

/** 伺服报警信号有效电平（ALM_LEV），1 字节/轴 */
#define ALM_LEV_BASE             1054
#define ALM_LEV_LEN              1
#define ALM_LEV(i)               (ALM_LEV_BASE + (i)*ALM_LEV_LEN)

/** 原点信号端口号（ORG_NUM），1 字节/轴 */
#define ORG_NUM_BASE             1070
#define ORG_NUM_LEN              1
#define ORG_NUM(i)               (ORG_NUM_BASE + (i)*ORG_NUM_LEN)

/** 原点信号有效电平（ORG_LEV），1 字节/轴 */
#define ORG_LEV_BASE             1086
#define ORG_LEV_LEN              1
#define ORG_LEV(i)               (ORG_LEV_BASE + (i)*ORG_LEV_LEN)

/** 回零快速段速度，4 字节/轴 */
#define HOME_SPEED_FAST_BASE     1102
#define HOME_SPEED_FAST_LEN      4
#define HOME_SPEED_FAST(i)       (HOME_SPEED_FAST_BASE + (i)*HOME_SPEED_FAST_LEN)

/** 回零低速 / 精找零速度，4 字节/轴 */
#define HOME_SPEED_SLOW_BASE     1166
#define HOME_SPEED_SLOW_LEN      4
#define HOME_SPEED_SLOW(i)       (HOME_SPEED_SLOW_BASE + (i)*HOME_SPEED_SLOW_LEN)

/** 回零完成后的原点偏移量，4 字节/轴 */
#define HOME_OFFSET_BASE         1230
#define HOME_OFFSET_LEN          4
#define HOME_OFFSET(i)           (HOME_OFFSET_BASE + (i)*HOME_OFFSET_LEN)

/** 轴错误码 / 错误日志字，4 字节/轴 */
#define ERROR_LOG_BASE           1299
#define ERROR_LOG_LEN            4
#define ERROR_LOG(i)             (ERROR_LOG_BASE + (i)*ERROR_LOG_LEN)

/* --- IO 映像区：按块 i 索引 (i = 0~2)，每块 2 字节 --- */
/** 离散输入当前值（位图打包），2 字节/块 */
#define INPUT_VALUE_BASE         512
#define INPUT_VALUE_LEN          2
#define INPUT_VALUE(i)           (INPUT_VALUE_BASE + (i)*INPUT_VALUE_LEN)

/** 输入有效位掩码，与 INPUT_VALUE 同块索引 */
#define INPUT_MASK_BASE          518
#define INPUT_MASK_LEN           2
#define INPUT_MASK(i)            (INPUT_MASK_BASE + (i)*INPUT_MASK_LEN)

/** 输出设定值，2 字节/块 */
#define OUTPUT_VALUE_BASE        524
#define OUTPUT_VALUE_LEN         2
#define OUTPUT_VALUE(i)          (OUTPUT_VALUE_BASE + (i)*OUTPUT_VALUE_LEN)

/** 输出有效位掩码 */
#define OUTPUT_MASK_BASE         530
#define OUTPUT_MASK_LEN          2
#define OUTPUT_MASK(i)           (OUTPUT_MASK_BASE + (i)*OUTPUT_MASK_LEN)

/** 输出有效电平配置（高/低有效），2 字节/块 */
#define OUTPUT_ACTIVE_LEVEL_BASE 536
#define OUTPUT_ACTIVE_LEVEL_LEN  2
#define OUTPUT_ACTIVE_LEVEL(i)   (OUTPUT_ACTIVE_LEVEL_BASE + (i)*OUTPUT_ACTIVE_LEVEL_LEN)

/* --- 输出脉宽表：每路 2 字节，i = 0~47（与 CAN_MAX_IO_BITS 等协议一致） --- */
/** 单路输出脉冲宽度设定 */
#define PULSE_WIDTH_BASE         542
#define PULSE_WIDTH_LEN          2
#define PULSE_WIDTH(i)           (PULSE_WIDTH_BASE + (i)*PULSE_WIDTH_LEN)

#ifndef VENDOR_CAN_PUBLIC_TYPES_DEFINED
#define VENDOR_CAN_PUBLIC_TYPES_DEFINED
#pragma pack(push, 1)
/// @brief CAN节点类型
typedef enum
{
    CAN_NODE_TYPE_UNKNOWN = 0,
    CAN_NODE_TYPE_MANAGER,
    CAN_NODE_TYPE_MOTION_CARD,
    CAN_NODE_TYPE_IO_CARD,
    CAN_NODE_TYPE_GATEWAY
} can_node_type_t;

/// @brief CAN轴控制字位定义
typedef struct
{
    uint8_t enable : 1;
    uint8_t start : 1;
    uint8_t stop : 1;
    uint8_t abort : 1;
    uint8_t home : 1;
    uint8_t jog_plus : 1;
    uint8_t jog_minus : 1;
    uint8_t reset : 1;
} control_word_bits_t;

/// @brief CAN轴控制字
typedef union
{
    uint8_t byte;
    control_word_bits_t bits;
} control_word_t;

/// @brief CAN轴状态字位定义
typedef struct
{
    uint8_t enabled : 1;
    uint8_t ready : 1;
    uint8_t error : 1;
    uint8_t moving : 1;
    uint8_t arrived : 1;
    uint8_t home_done : 1;
    uint8_t lim_plus : 1;
    uint8_t lim_minus : 1;
} status_word_bits_t;

/// @brief CAN轴状态字
typedef union
{
    uint8_t byte;
    status_word_bits_t bits;
} status_word_t;

/// @brief CAN节点信息
typedef struct
{
    uint8_t node_id;
    can_node_type_t type;
    uint8_t axis_num;
    uint8_t input_num;
    uint8_t output_num;
    bool online;
    uint16_t alarm_code;
} can_node_info_t;
#pragma pack(pop)
#endif

// ============================================================================
// 枚举类型定义
// ============================================================================

/// @brief 轴接口统一返回码
typedef enum
{
    AXIS_OK = 0,              /*!< 成功 */
    AXIS_ERR_PARAM = -1,      /*!< 参数错误（无效轴号/空指针/超范围） */
    AXIS_ERR_STATE = -2,      /*!< 状态错误（当前轴状态不允许该操作） */
    AXIS_ERR_BUSY = -3,       /*!< 忙（邮箱满或轴运行中） */
    AXIS_ERR_ALGO_CALL = -4,  /*!< 算法错误（初始化或计算失败） */
    AXIS_ERR_NOT_SUPPORTED = -5, /*!< 功能不支持 */
} axis_status_t;

/// @brief 轴状态枚举
typedef enum
{
    AXIS_STATE_INIT = -1,    /*!< 轴初始状态 未激活*/
    AXIS_STATE_IDLE = 0,     /*!< 静止状态 准备好了*/
    AXIS_STATE_HOMING = 1,   /*!< 轴Homing状态 正在回原点*/
    AXIS_STATE_SINGLE = 2,   /*!< 独立运动状态 单轴独立运动*/
    AXIS_STATE_SYNC = 3,     /*!< 轴同步状态 多轴同步运动*/
    AXIS_STATE_STOPING = 4,  /*!< 轴停止状态 正在停止运动*/
    AXIS_STATE_ERROR = 5,    /*!< 轴错误状态 */
    AXIS_STATE_STOP_DEC = 6, /*!< 轴减速停止状态 仅stop_dec触发 */
} axis_state_t;

/// @brief 轴错误码枚举
typedef enum
{
    AXIS_ERR_NONE = 0,              /*!< 无错误*/
    AXIS_ERR_ALM = 1,               /*!< 报警错误*/
    AXIS_ERR_LIMI_NEG_HARDWARE = 2, /*!< 负限位硬件错误*/
    AXIS_ERR_LIMI_POS_HARDWARE = 3, /*!< 正限位硬件错误*/
    AXIS_ERR_LIMI_NEG_SOFTWARE = 4, /*!< 负限位软件错误*/
    AXIS_ERR_LIMI_POS_SOFTWARE = 5, /*!< 正限位软件错误*/
    AXIS_ERR_DISABLE = 6,           /*!< 轴使能错误*/
    AXIS_ERR_HOME_FAILED = 7,       /*!< 回零失败*/
    AXIS_ERR_ALGO = 8,              /*!< 算法错误*/
} axis_err_t;

/// @brief 轴组运动类型
typedef enum
{
    AXIS_GROUP_MOTION_NONE = 0, /*!< 未设置*/
    AXIS_GROUP_MOTION_INTERP,   /*!< 插补/同步运动*/
    AXIS_GROUP_MOTION_CAM,      /*!< 凸轮运动*/
} axis_group_motion_type_t;

/// @brief 轴组错误码
typedef enum
{
    AXIS_GROUP_ERR_NONE = 0,      /*!< 无错误*/
    AXIS_GROUP_ERR_BUSY,          /*!< 轴组忙/轴忙*/
    AXIS_GROUP_ERR_PARAM,         /*!< 参数错误*/
    AXIS_GROUP_ERR_AXIS,          /*!< 轴报警/错误*/
    AXIS_GROUP_ERR_AXIS_CONFLICT, /*!< 轴被其他组占用*/
    AXIS_GROUP_ERR_ALGO,          /*!< 算法错误*/
} axis_group_err_t;

/// @brief 插补线段类型
typedef enum
{
    AXIS_INTERP_LINE = 0, /*!< 直线 */
    AXIS_INTERP_ARC,      /*!< 圆弧（三点定义） */
    AXIS_INTERP_CIRCLE,   /*!< 整圆 */
} axis_interp_line_type_t;

/// @brief 插补线段（与 motion_plan4 mp_line_t 兼容）
typedef struct
{
    int type;   /*!< 线段类型 @ref axis_interp_line_type_t */
    int p1[3];  /*!< 起始点坐标（脉冲） */
    int p2[3];  /*!< 终点/中间点坐标（脉冲） */
    int p3[3];  /*!< 圆弧第三点坐标（脉冲） */
    int r;      /*!< 半径（圆时使用） */
} axis_interp_line_t;

/// @brief 插补运动配置（通过 axis_group_set_motion 传入）
/// @note  所有指针引用的数据在轴组运动期间必须保持有效
typedef struct
{
    axis_interp_line_t *lines;      /*!< 线段数组指针 */
    int                *line_vmax;  /*!< 每段最大线速度数组（pulse/s） */
    int                *line_acc;   /*!< 每段加速度数组（pulse/s^2） */
    int                *turn_acc;   /*!< 转角加速度数组（pulse/s^2） */
    int16_t             line_num;   /*!< 线段数量 */
    int16_t             start_vel;  /*!< 起始速度（pulse/s） */
    int16_t             end_vel;    /*!< 结束速度（pulse/s） */
    int16_t             reserved;
} axis_interp_config_t;

// ============================================================================
// 结构体类型定义
// ============================================================================

/// @brief BSP用户配置结构体
/// @note 该结构体用于配置当前板子的各种参数，如modbus通信配置、fram配置等。
typedef struct
{
    /* modbus configuration */
    uint32_t com1_modbus_en; /*!< 是否使能串口1的modbus通信*/
    uint32_t com1_baudrate;  /*!< 串口1的波特率*/
    uint32_t com1_slave_idx; /*!< 串口1的从机地址*/

    uint32_t com2_modbus_en; /*!< 是否使能串口2的modbus通信*/
    uint32_t com2_baudrate;  /*!< 串口2的波特率*/
    uint32_t com2_slave_idx; /*!< 串口2的从机地址*/

    uint32_t tcp_modbus_en; /*!< 是否使能tcp的modbus通信*/
    uint32_t tcp_slave_idx; /*!< tcp的从机地址*/
    uint8_t  ip_addr[4];    /*!< tcp的ip地址*/
    uint32_t tcp_port;      /*!< tcp的端口号*/
    uint32_t dynlib_en;     /*!< 是否使能 dynlib 0x40 扩展协议，仅对 TCP 生效*/

    /* fram configuration */
    void    *fram_data;        /*!< fram数据指针*/
    uint32_t fram_size;        /*!< fram大小*/
    uint32_t fram_save_period; /*!< fram保存周期*/

    /* axis configuration */
    uint32_t axis_num_pluse;       /*!< 脉冲轴数量*/
    uint32_t axis_num_ethercat;    /*!< 总线轴数量*/
    uint32_t axis_num_can;         // can轴数量
    uint32_t encoder_num;          // 编码器数量
    uint32_t encoder_num_ethercat; // 总线编码器数量

    /* normal io configuration */
    uint32_t main_input_num;          /*!< 主输入数量*/
    uint32_t main_output_num;         /*!< 主输出数量*/
    uint32_t ethercat_io_num;         /*!< 总线io卡数量*/
    uint8_t  ethercat_input_num[16];  /*!< 总线io卡输入数量*/
    uint8_t  ethercat_output_num[16]; /*!< 总线io卡输出数量*/

    /* can configuration */
    uint32_t can_master_en;    /*!< 是否使能CAN主站*/
    uint32_t can_slave_en;     /*!< 是否使能CAN从站*/
    uint32_t can_slave_node_id; /*!< CAN从站节点号*/
    uint32_t can_bitrate;      /*!< CAN波特率*/

    /* ethercat configuration */
    uint32_t axis_cycle_time; /*!< 轴周期时间*/
} bsp_user_config_t;

/// @brief 板卡资源信息结构体
typedef struct
{
    uint32_t read_input_num;   /*!< 实际输入数量*/
    uint32_t read_output_num;  /*!< 实际输出数量*/
    uint32_t read_axis_num;    /*!< 实际轴数量*/
    uint32_t read_encoder_num; /*!< 编码器数量*/
} board_resource_t;

/// @brief 芯片ID结构体
typedef struct
{
    uint32_t id[3]; /*!< 芯片ID数组*/
} ChipID;

/// @brief 单轴运动配置结构体
typedef struct
{
    float   max_vel;        /*!< 最大速度(单位 unit/s)，算法层输入参数*/
    int32_t acc_time;       /*!< 加速度时间(单位 ms)，算法层输入参数*/
    int32_t dec_time;       /*!< 减速时间(单位 ms)，算法层输入参数*/
    float   pulse_per_unit; /*!< 历史兼容字段，当前 move 指令不直接使用，脉冲当量请通过 axis_set_pulse_per_unit() 配置*/
    int     acc_model;      /*!< 加减速模型(0:T型 1:S型)，算法层输入参数*/
} single_move_config_t;

/// @brief 回零配置结构体
typedef struct
{
    uint32_t home_type;  /*!< 回零方式*/
    uint32_t org_num;    /*!< 原点编号*/
    uint32_t org_lev;    /*!< 原点电平*/
    float    search_spd; /*!< 搜索速度(单位 unit/s)*/
    float    search_zero_spd; /*!< 找零速度(单位 unit/s)，0 表示使用默认慢速*/
    float    org_offset; /*!< 原点偏移(单位 unit)*/
    uint32_t timeout_ms; /*!< 回零超时(毫秒)，0 表示使用默认兜底值*/
} home_config_t;

/// @brief 限位配置结构体
typedef struct
{
    uint32_t alm_lvl;                /*!< 报警输入电平 0/1有效，大于1失效*/
    uint32_t alm_single_idx;         /*!< 报警输入*/
    int32_t  software_limit_max;     /*!< 软件正限位，单位：脉冲*/
    int32_t  software_limit_min;     /*!< 软件负限位，单位：脉冲*/
    uint32_t hardware_limit_neg_idx; /*!< 负限位输入*/
    int32_t  hardware_limit_pos_idx; /*!< 正限位输入*/
    uint8_t  software_pos_limit_en;  /*!< 软件正限位使能*/
    uint8_t  software_neg_limit_en;  /*!< 软件负限位使能*/
    uint8_t  hardware_pos_limit_en;  /*!< 硬件正限位使能*/
    uint8_t  hardware_neg_limit_en;  /*!< 硬件负限位使能*/
    uint8_t  hardware_pos_limit_lev; /*!< 硬件正限位有效电平 0/1*/
    uint8_t  hardware_neg_limit_lev; /*!< 硬件负限位有效电平 0/1*/
    uint8_t  reserved[2];            /*!< 保留，保持结构体大小稳定*/
} limit_config_t;

/// @brief CAN从站单位脉冲配置导出结构体
typedef struct
{
    uint32_t ppr;            /*!< 主站下发的 PPR */
    int      mpr;            /*!< 主站下发的 MPR */
    float    pulse_per_unit; /*!< 由 PPR/MPR 计算出的脉冲当量 */
} can_slave_axis_unit_params_t;

/// @brief CAN从站运动速度配置导出结构体
typedef struct
{
    uint32_t max_velocity; /*!< 最大速度 */
    uint32_t acc_time;     /*!< 加速时间，单位 ms */
    uint32_t dec_time;     /*!< 减速时间，单位 ms */
    uint8_t  acc_model;    /*!< 加减速模型 */
} can_slave_axis_profile_params_t;

/// @brief CAN从站限位和报警配置导出结构体
typedef struct
{
    uint8_t hardware_pos_enable; /*!< 硬件正限位使能 */
    uint8_t hardware_neg_enable; /*!< 硬件负限位使能 */
    uint8_t software_pos_enable; /*!< 软件正限位使能 */
    uint8_t software_neg_enable; /*!< 软件负限位使能 */
    uint8_t hardware_pos_signal; /*!< 硬件正限位输入编号 */
    uint8_t hardware_neg_signal; /*!< 硬件负限位输入编号 */
    uint8_t hardware_pos_level;  /*!< 硬件正限位有效电平 */
    uint8_t hardware_neg_level;  /*!< 硬件负限位有效电平 */
    int32_t software_pos_limit;  /*!< 软件正限位位置 */
    int32_t software_neg_limit;  /*!< 软件负限位位置 */
    uint8_t alarm_signal;        /*!< 报警输入编号 */
    uint8_t alarm_level;         /*!< 报警有效电平，大于 1 表示忽略报警 */
} can_slave_axis_limit_params_t;

/// @brief CAN从站回零配置导出结构体
typedef struct
{
    uint32_t home_type;         /*!< 回零方式；当前 CAN 数据区没有该字段，默认导出 0 */
    uint8_t  origin_signal;     /*!< 原点输入编号 */
    uint8_t  origin_level;      /*!< 原点有效电平 */
    uint32_t search_speed;      /*!< 回零快速搜索速度 */
    uint32_t search_zero_speed; /*!< 回零慢速搜索速度 */
    int32_t  origin_offset;     /*!< 原点偏移 */
} can_slave_axis_home_params_t;

#ifndef VENDOR_CAN_AXIS_FB_T_DEFINED
#define VENDOR_CAN_AXIS_FB_T_DEFINED
/// @brief CAN主站缓存的单轴反馈信息
typedef struct
{
    bool valid;                /*!< 反馈是否有效 */
    status_word_t status_word; /*!< 从站上报的状态字 */
    int32_t actual_value;      /*!< 从站上报的实际位置 */
    uint16_t velocity;         /*!< 从站上报的速度值 */
    uint32_t timestamp_ms;     /*!< 最近一次反馈更新时间 */
} can_axis_fb_t;
#endif

/// @brief RTC日期时间结构体
typedef struct _rtc_datetime
{
    uint16_t year;   /*!< 年份，范围从1900开始*/
    uint16_t month;  /*!< 月份，范围从1到12*/
    uint16_t day;    /*!< 日期，范围从1到31（取决于月份）*/
    uint16_t hour;   /*!< 小时，范围从0到23*/
    uint16_t minute; /*!< 分钟，范围从0到59*/
    uint16_t second; /*!< 秒，范围从0到59*/
} rtc_datetime_t;

/// @brief EtherCAT RM伺服参数结构体
typedef struct _EC_RM_PARAMS_
{
    unsigned short slave; /*!< 从站号*/

    /* RPDO映射相关 */
    float    *pp_position;     /*!< 目标位置*/
    float    *pp_velocity;     /*!< 目标速度*/
    float    *pp_acceleration; /*!< 目标加速度*/
    float    *pp_torque;       /*!< 目标扭矩*/
    uint32_t *commond_stop;    /*!< 停止命令*/
    uint32_t *reset_error;     /*!< 清除错误*/

    /* TPDO映射相关 */
    float         *pp_position_act;             /*!< 实际位置*/
    float         *pp_velocity_act;             /*!< 实际速度*/
    float         *pp_torque_act;               /*!< 实际扭矩*/
    uint32_t      *error_alarm;                 /*!< 错误报警*/
    uint32_t      *position_deviation_oveiflow; /*!< 位置偏差过限*/
    uint32_t      *velocity_deviation_oveiflow; /*!< 速度偏差过限*/
    uint32_t      *motor_stuck;                 /*!< 电机堵转*/
    unsigned short status;                      /*!< 状态*/
} EC_RM_PARAMS;

// ============================================================================
// 外部全局变量声明
// ============================================================================

/// @brief 当前板子的配置
extern bsp_user_config_t *pBsp;

/// @brief 当前板子的资源
extern const board_resource_t *pBoardResource;

// ============================================================================
// 系统级函数
// ============================================================================

/// @brief 创建BSP任务
/// @param cfg 当前板子的配置
void bsp_task_create(bsp_user_config_t *cfg);

/// @brief 创建Modbus任务
/// @param cfg 当前板子的配置
void modbus_task_create(bsp_user_config_t *cfg);

/// @brief 创建FPGA任务
/// @param cfg 当前板子的配置
/// @return 板卡资源信息
const board_resource_t *fpga_task_create(bsp_user_config_t *cfg);

/// @brief 创建CAN任务
/// @param cfg 当前板子的配置
void can_task_create(bsp_user_config_t *cfg);

/// @brief 获取指定CAN节点信息，节点无效或未在线时返回NULL
const can_node_info_t *can_node_manager_get_info(uint8_t node_id);

/// @brief 获取当前CAN主站在线节点列表
int can_node_manager_get_online_list(can_node_info_t *out, uint8_t *count);

/// @brief 读取CAN主站缓存的单轴反馈
const can_axis_fb_t *can_axis_get_fb(uint8_t node_id, uint8_t axis);

/// @brief 发送预装轴命令，从站等待下一次SYNC帧后执行
int can_axis_send_preload(uint8_t node_id, uint8_t axis, int32_t value, control_word_t control_word);

/// @brief 发送立即执行轴命令，从站收到后直接执行
int can_axis_send_execute(uint8_t node_id, uint8_t axis, int32_t value, control_word_t control_word);

/// @brief 写CAN从站IO输出，data为物理输出电平
int can_io_write(uint8_t node_id, uint8_t block_index, uint16_t bitmask, uint16_t data);

/// @brief 读取CAN主站缓存的IO输入
int64_t can_io_read(uint8_t node_id);

/// @brief 发送CAN从站IO脉冲命令，脉冲有效电平由从站配置决定
int can_io_write_pulse(uint8_t node_id, uint8_t block_index, uint16_t bitmask);

/// @brief 获取CAN主站缓存的输入位图
int can_io_get_inputs(uint8_t node_id, uint32_t *out_inputs);

/// @brief 获取CAN主站缓存的输出位图
int can_io_get_outputs(uint8_t node_id, uint32_t *out_outputs);

/// @brief 阻塞读取CAN从站SDO对象
int can_sdo_read_blocking(uint8_t node_id, uint16_t addr, uint8_t *data, uint16_t len);

/// @brief 阻塞写入CAN从站SDO对象
int can_sdo_write_blocking(uint8_t node_id, uint16_t addr, uint8_t *data, uint16_t len);

/// @brief 检查轴号是否在当前 CAN 从站轴数量范围内
/// @param axis CAN从站本地轴号
/// @return 1:有效 0:无效
int can_slave_axis_config_valid(uint8_t axis);

/// @brief 读取主站通过 SDO 写入的单位脉冲配置
/// @param axis CAN从站本地轴号
/// @param out 输出配置，应用层决定如何写入 GSS 或算法层
/// @return 0:成功 -1:失败
int can_slave_axis_get_unit_config(uint8_t axis, can_slave_axis_unit_params_t *out);

/// @brief 读取主站通过 SDO 写入的运动速度配置
/// @param axis CAN从站本地轴号
/// @param out 输出配置，应用层决定如何写入 GSS 或算法层
/// @return 0:成功 -1:失败
int can_slave_axis_get_profile_config(uint8_t axis, can_slave_axis_profile_params_t *out);

/// @brief 读取主站通过 SDO 写入的限位和报警配置
/// @param axis CAN从站本地轴号
/// @param out 输出配置，应用层决定如何写入 GSS 或算法层
/// @return 0:成功 -1:失败
int can_slave_axis_get_limit_config(uint8_t axis, can_slave_axis_limit_params_t *out);

/// @brief 读取主站通过 SDO 写入的回零配置
/// @param axis CAN从站本地轴号
/// @param out 输出配置，应用层决定如何写入 GSS 或算法层
/// @return 0:成功 -1:失败
int can_slave_axis_get_home_config(uint8_t axis, can_slave_axis_home_params_t *out);

/// @brief 检查传入的结构体大小是否与 bsp_user_config_t 匹配，若不匹配则进入断言，模板工程中使用此函数
/// @param struct_size 调用方传入的结构体大小，通常为 sizeof(user_struct)
void bsp_user_config_size_check(uint32_t struct_size);

/// @brief Modbus从站寄存器映射配置
/// @param read_tab 读寄存器表
/// @param read_start_addr 读寄存器起始地址
/// @param read_end_addr 读寄存器结束地址
/// @param write_tab 写寄存器表
/// @param write_start_addr 写寄存器起始地址
/// @param write_end_addr 写寄存器结束地址
/// @param save_tab 保存寄存器表
/// @param save_start_addr 保存寄存器起始地址
/// @param save_end_addr 保存寄存器结束地址
/// @return 0:成功 -1:失败
int modbus_slave_map_config(uint16_t *read_tab, int read_start_addr, int read_end_addr, uint16_t *write_tab, int write_start_addr,
                            int write_end_addr, uint16_t *save_tab, int save_start_addr, int save_end_addr);

/// @brief 获取芯片ID
/// @return 芯片ID信息
ChipID Get_ChipID(void);

// ============================================================================
// 外设功能接口 - FRAM
// ============================================================================

/// @brief 保存FRAM数据
/// @param addr 保存地址
/// @param data 数据指针
/// @param len 数据长度
void fram_write_block(uint32_t addr, uint8_t *data, uint32_t len);

/// @brief 读取FRAM数据
/// @param addr 读取地址
/// @param data 数据缓冲区指针
/// @param len 数据长度
/// @return 0:成功 -1:失败
int fram_read_block(uint32_t addr, uint8_t *data, uint32_t len);

// ============================================================================
// 外设功能接口 - USB
// ============================================================================

/// @brief USB文件写入函数
/// @param file_name 文件名
/// @param data 数据指针
/// @param len 数据长度
/// @return 0:成功 -1:失败
int usb_file_write(const char *file_name, const char *data, uint32_t len);

/// @brief USB文件读取函数
/// @param file_name 文件名
/// @param data 数据缓冲区指针
/// @param len 数据长度
/// @return 0:成功 -1:失败
int usb_file_read(const char *file_name, char *data, uint32_t len);

/// @brief USB连接状态查询
/// @return true:已连接 false:未连接
int usb_msc_status(void);

// ============================================================================
// 外设功能接口 - RTC
// ============================================================================

/// @brief 获取RTC时间
/// @param dt 时间结构体指针
void rtc_get_datetime(rtc_datetime_t *dt);

/// @brief 设置RTC时间
/// @param dt 时间结构体指针
void rtc_set_datetime(rtc_datetime_t *dt);

// ============================================================================
// 安全功能接口
// ============================================================================

/// @brief MD5锁机注册
/// @param Cid 锁机ID
/// @param registerInfo 注册信息字符串
/// @return 0:成功 -1:失败
int Md5Regist(int Cid, char registerInfo[]);

// ============================================================================
// IO功能接口
// ============================================================================

/// @brief 读取数字量输入
/// @param num 数字量输入编号
///            0x00-0xFFF:     普通输入（本体输入 + EtherCAT IO 输入）
///            0x1000-0x10FF: 轴附属输入（0x1000-0x100F: LIM, 0x1010-0x101F: WARN）
/// @return 0/1
int read_digital_input(uint32_t num);

/// @brief 判断数字量输入编号当前是否有效
/// @param num 数字量输入编号
/// @return true 编号有效
/// @return false 编号无效
bool digital_input_is_valid(uint32_t num);

/// @brief 写入数字量输出
/// @param num 数字量输出编号
///            0x00-0xFFF:     普通输出（本体输出 + EtherCAT IO 输出）
///            0x1000-0x10FF: 轴附属输出（0x1000-0x100F: EN）
/// @param value 输出值 0/1
/// @return 0/1
int write_digital_output(uint32_t num, uint8_t value);

/// @brief 判断数字量输出编号当前是否有效
/// @param num 数字量输出编号
/// @return true 编号有效
/// @return false 编号无效
bool digital_output_is_valid(uint32_t num);

/// @brief 读取数字量输出
/// @param num 数字量输出编号
///            0x00-0xFFF:     普通输出（本体输出 + EtherCAT IO 输出）
///            0x1000-0x10FF: 轴附属输出（0x1000-0x100F: EN）
/// @return 0/1
int read_digital_output(uint32_t num);

/// @brief 翻转数字量输出
/// @param num 数字量输出编号
///            0x00-0xFFF:     普通输出（本体输出 + EtherCAT IO 输出）
///            0x1000-0x10FF: 轴附属输出（0x1000-0x100F: EN）
/// @return 0/1
int trigger_digital_output(uint32_t num);

// ============================================================================
// 编码器接口
// ============================================================================

/// @brief 获取编码器计数值
/// @param encoder_num 编码器编号
/// @return 计数值
int64_t get_encoder_count(uint8_t encoder_num);

/// @brief 重置编码器计数值
/// @param encoder_num 编码器编号
void reset_encoder_count(uint8_t encoder_num);

// ============================================================================
// 单轴控制接口（axis_*）- 轴使能与状态控制
// ============================================================================

/// @brief 轴使能控制
/// @param idx 轴号
/// @param en 使能 0:关闭 1:打开
/// @return 0:成功 -1:失败
int axis_set_en(uint32_t idx, int en);

/// @brief 轴停止
/// @param ax 轴号
/// @return 0:成功 -1:失败
int axis_stop(uint32_t ax);

/// @brief 轴减速停止
/// @param ax 轴号
/// @return 0:成功 -1:失败
int axis_stop_dec(uint32_t ax);

/// @brief 轴复位
/// @param ax 轴号
/// @return 0:成功 -1:失败
int axis_reset(uint32_t ax);

/// @brief 获取轴状态
/// @param ax 轴号
/// @return 轴状态
int axis_get_status(uint32_t ax);

/// @brief 轴是否处于空闲状态（空闲且邮箱无待处理命令）
/// @param ax 轴号
/// @return 1:空闲 0:非空闲
int axis_ready(uint32_t ax);

/// @brief 获取轴错误状态
/// @param idx 轴号
/// @return 错误状态
axis_err_t axis_get_error_status(uint32_t idx);

// ============================================================================
// 单轴控制接口（axis_*）- 轴参数配置
// ============================================================================

/// @brief 设置轴方向
/// @param idx 轴号
/// @param dir 方向 0:正向 1:反向
/// @return axis_status_t
int axis_set_dir(uint32_t idx, int dir);

/// @brief 获取轴方向
/// @param idx 轴号
/// @return 方向 0:正向 1:反向
int axis_get_dir(uint32_t idx);

/// @brief 设置轴单圈脉冲数
/// @param idx 轴号
/// @param pulse 单圈脉冲数
/// @return axis_status_t
int axis_set_ppr(uint32_t idx, uint32_t pulse);

/// @brief 获取轴单圈脉冲数
/// @param idx 轴号
/// @return 单圈脉冲数
int axis_get_ppr(uint32_t idx);

/// @brief 设置轴当前位置（脉冲单位）
/// @param idx 轴号
/// @param pos 当前位置（单位：脉冲数）
int axis_set_cur_pos(uint32_t idx, int32_t pos);

/// @brief 设置轴当前位置（用户单位）
/// @param idx 轴号
/// @param pos 当前位置（单位：用户单位）
/// @return axis_status_t
int axis_set_cur_unit_pos(uint32_t idx, float pos);

/// @brief 设置脉冲当量（脉冲/单位）
/// @param idx 轴号
/// @param ppu 脉冲当量
/// @return axis_status_t
int axis_set_pulse_per_unit(uint32_t idx, float ppu);

/// @brief 加载轴回零配置
/// @param ax 轴号
/// @param home_config 回零配置指针
/// @return axis_status_t
int axis_load_home_config(uint32_t ax, home_config_t *home_config);

/// @brief 加载轴限位配置
/// @param ax 轴号
/// @param limit_config 限位配置指针
/// @return axis_status_t
int axis_load_limit_config(uint32_t ax, limit_config_t *limit_config);

// ============================================================================
// 单轴控制接口（axis_*）- 轴运动控制
// ============================================================================

/// @brief 轴回零（带速度配置）
/// @param ax 轴号
/// @param home_cfg 回零配置（包含回零方式、原点信号、搜索速度等）
/// @return axis_status_t
int axis_home(uint32_t ax, const home_config_t *home_cfg);

/// @brief 轴绝对运动（带速度参数）
/// @param ax 轴号
/// @param pos 目标位置（单位：用户单位）
/// @param profile 速度配置（最大速度、加减速时间、加减速模型）
/// @return axis_status_t
int axis_move_abs(uint32_t ax, float pos, const single_move_config_t *profile);

/// @brief 轴相对运动（带速度参数）
/// @param ax 轴号
/// @param pos 相对位移（单位：用户单位）
/// @param profile 速度配置（最大速度、加减速时间、加减速模型）
/// @return axis_status_t
int axis_move_rel(uint32_t ax, float pos, const single_move_config_t *profile);

/// @brief 临时重载单轴运行速度
/// @param ax 轴号
/// @param new_speed 新速度（单位：用户单位/s）
/// @return axis_status_t
int axis_reload_speed(uint32_t ax, float new_speed);

// ============================================================================
// 单轴控制接口（axis_*）- 轴状态查询
// ============================================================================

/// @brief 获取轴当前位置（脉冲单位）
/// @param idx 轴号
/// @return 当前位置（单位：脉冲数）
int axis_get_cur_pos(uint32_t idx);

/// @brief 获取轴当前位置（用户单位）
/// @param idx 轴号
/// @return 当前位置（单位：用户单位）
float axis_get_cur_unit_pos(uint32_t idx);

/// @brief 获取驱动器当前位置（用户单位）
/// @param idx 轴号
/// @return 当前位置（单位：用户单位）
float axis_get_cur_unit_pos_ex(uint32_t idx);

/// @brief 获取轴当前速度（脉冲单位）
/// @param idx 轴号
/// @return 当前速度（单位：脉冲数/秒）
int32_t axis_get_cur_spd(uint32_t idx);

/// @brief 获取轴每单位对应的脉冲数
/// @param idx 轴号
/// @return 每单位对应的脉冲数
uint32_t axis_get_pulse_per_unit(uint32_t idx);

/// @brief 获取轴当前速度配置
/// @param idx 轴号
/// @param profile 速度配置结构体指针
/// @return 0:成功 -1:失败
int32_t axis_get_cur_spd_config(uint32_t idx, single_move_config_t *profile);

// ============================================================================
// 单轴控制接口（axis_*）- 扭矩与特殊功能
// ============================================================================

/// @brief 获取轴当前扭矩
/// @param idx 轴号
/// @return 当前扭矩值
int axis_get_cur_torque(uint32_t idx);

/// @brief 设置轴的扭矩限制
/// @param idx 轴号
/// @param forward_torque 正向扭矩限位
/// @param reverse_torque 反向扭矩限位
/// @return -1:参数错误 0:成功
int axis_set_torque_limit(uint32_t idx, uint16_t forward_torque, uint16_t reverse_torque);

/// @brief 设置轴视觉飞拍位置
/// @param idx 轴号
/// @param pos 飞拍位置：单位用户单位
/// @param direction 飞拍方向：1:正向 2:反向 3:两边飞拍
/// @param counts 飞拍次数：最大32个飞拍点
/// @param puls_time 脉冲时间：单位ms
/// @return 0:成功 -1:失败
int axis_set_visual_pos(uint32_t idx, float pos[], int direction[], int counts, uint32_t puls_time);

// ============================================================================
// 轴组控制接口
// ============================================================================

/// @brief 轴组XY配置
/// @param group_id 轴组ID
/// @brief 配置轴组成员
/// @param group_id 轴组ID
/// @param axis_list 轴号数组
/// @param axis_num 轴数量（1..MAX_GROUP_AXIS_NUM）
/// @return 0:成功 -1:失败
int axis_group_config(uint32_t group_id, const uint32_t *axis_list, uint8_t axis_num);

/// @brief 清除轴组配置
/// @param group_id 轴组ID
/// @return 0:成功 -1:失败
int axis_group_clear_config(uint32_t group_id);

/// @brief 轴组复位
/// @param group_id 轴组ID
/// @return 0:成功 -1:失败
int axis_group_reset(uint32_t group_id);

/// @brief 设置轴组运动类型与参数
/// @param group_id 轴组ID
/// @param motion_type 运动类型
/// @param motion_data 运动参数指针（由上层管理生命周期）
/// @param motion_len 运动参数长度
/// @return 0:成功 -1:失败
int axis_group_set_motion(uint32_t group_id, uint32_t motion_type, const void *motion_data, uint32_t motion_len);

/// @brief 启动轴组运动
/// @param group_id 轴组ID
/// @return 0:成功 -1:失败
int axis_group_start(uint32_t group_id);

/// @brief 停止轴组运动
/// @param group_id 轴组ID
/// @return 0:成功 -1:失败
int axis_group_stop(uint32_t group_id);

/// @brief 轴组运行状态查询
/// @param group_id 轴组ID
/// @return 0:空闲 1:运行中 -1:失败
int axis_group_state(uint32_t group_id);

/// @brief 获取轴组最近一次错误码
/// @param group_id 轴组ID
/// @return 0:无错误 其他:错误码
int axis_group_last_error(uint32_t group_id);

// ============================================================================
// EtherCAT总线接口
// ============================================================================

typedef struct{
    int compare_value;
    int direction; // 1: 正向穿过 2: 反向穿过 3: 两边穿过
}compare_config_t;

/// @brief 获取RM伺服参数
/// @param id RM从站号
/// @return RM伺服参数指针
EC_RM_PARAMS *ec_get_rm_params(uint8_t id);

/// @brief EtherCAT读取字典数据
/// @param slave 从站号
/// @param index 索引
/// @param subindex 子索引
/// @param size 数据大小
/// @return 读取的数据值
uint32_t ec_read_dic(int slave, uint16_t index, uint8_t subindex, uint16_t size);

/// @brief 设置伺服位置比较
/// @param slave 从站号
/// @param compare_config 比较配置
/// @param compare_count 比较配置数量
/// @param out_pulse_time 输出脉冲时间
/// @return -1:参数错误 0:成功
int ec_servo_set_position_compare(int slave,compare_config_t compare_config[],int compare_count, int out_pulse_time);

// ============================================================================
// CFM模块接口
// ============================================================================
/// @brief 读取CFM状态
/// @return CFM状态值: 0:无结果 1:OK 2:NG
uint16_t ch1_get_result(void);

/// @brief 读取CFM结果
/// @return CFM结果值: 0:无结果 1:OK 2:NG
uint16_t ch2_get_result(void);

//结果复位
void ch1_result_reset(void);
void ch2_result_reset(void);
//结果复位
void result_reset(void);
// ============================================================================
// 温控模块接口
// ============================================================================

/// @brief 读取温控结果
/// @param channel 通道号
/// @return 温控结果值
float tcm_read_result(uint8_t channel);

/// @brief 设置输出值
/// @param channel 通道号
/// @param value 输出值
/// @return 0:成功 -1:失败
int tcm_set_output(uint8_t channel, uint8_t value);

/// @brief 获取输出值
/// @param channel 通道号
/// @return 输出值
int tcm_get_output(uint8_t channel);

void fpga_ecat_reconfig(void);

#endif /* VENDOR_SDK_H_ */
