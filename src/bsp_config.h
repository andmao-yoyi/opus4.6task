/**
 * @file bsp_config.h
 * @brief BSP 用户配置头文件
 *
 * 该文件用于配置 BSP 参数，可通过以下方式修改：
 * 1. 直接编辑本文件
 * 2. 使用 Python 图形化配置工具: python config_gui.py
 *
 * 注意：修改后需要重新编译工程
 */

#ifndef BSP_CONFIG_H
#define BSP_CONFIG_H

/* ========== Modbus Communication ========== */
#define CFG_COM1_MODBUS_EN      1
#define CFG_COM1_BAUDRATE       115200
#define CFG_COM1_SLAVE_IDX      1

#define CFG_COM2_MODBUS_EN      1
#define CFG_COM2_BAUDRATE       115200
#define CFG_COM2_SLAVE_IDX      1

#define CFG_TCP_MODBUS_EN       1
#define CFG_TCP_SLAVE_IDX       1
#define CFG_TCP_IP_ADDR0        192
#define CFG_TCP_IP_ADDR1        168
#define CFG_TCP_IP_ADDR2        1
#define CFG_TCP_IP_ADDR3        30
#define CFG_TCP_PORT            502
#define CFG_DYNLIB_EN           0          /* 是否使能 dynlib 0x40 扩展协议，仅对 TCP 生效 */

/* ========== Axis Configuration ========== */
#define CFG_AXIS_CYCLE_TIME     1000000    /* 轴周期时间(us) */
#define CFG_AXIS_NUM_PLUSE      0          /* 脉冲轴数量 */
#define CFG_AXIS_NUM_ETHERCAT   5          /* EtherCAT轴数量 */
#define CFG_ENCODER_NUM         0          /* 本地编码器数量 */
#define CFG_ENCODER_NUM_ETHERCAT 0         /* EtherCAT编码器数量 */

/* ========== IO Configuration ========== */
#define CFG_MAIN_INPUT_NUM      0          /* 本地输入IO数量 */
#define CFG_MAIN_OUTPUT_NUM     0          /* 本地输出IO数量 */
#define CFG_ETHERCAT_IO_NUM     0          /* EtherCAT IO卡数量 */

/* EtherCAT IO卡输入输出数量 (最大16个卡) */
#define CFG_ETHERCAT_INPUT_NUM  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#define CFG_ETHERCAT_OUTPUT_NUM {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

/* ========== CAN Marster Configuration ========== */
#define CFG_CAN_MARSTER_EN      0          /* CAN主站使能 */
/* ========== CAN Slave Configuration ========== */
#define CFG_CAN_SLAVE_EN        0          /* CAN从站使能 */
#define CFG_CAN_SLAVE_NODE_ID   1          /* CAN从站节点ID */
#define CFG_AXIS_NUM_CAN        0          /* CAN轴数量 */
/* ========== CAN Bitrate Configuration ========== */
#define BITRATE                 1000       /* CAN波特率，主从通用 */

/* ========== FRAM Configuration ========== */
#define CFG_FRAM_SAVE_PERIOD    1000       /* FRAM保存周期(ms) */
#define CFG_FRAM_AUTOSAVE_MAX_BYTES  16000 /* 周期性FRAM自动保存区最大字节数，须小于板载容量并避开OTA配置区 */
#define CFG_CFM_EN              0          /* CFM压力集采模块使能 */

#endif /* BSP_CONFIG_H */
