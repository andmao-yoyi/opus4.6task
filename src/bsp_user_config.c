/*
 * bsp_user_config.c
 *
 * 功能： BSP 用户配置
 *
 *  Created on: 2021. 10. 21.
 *      Author: 82109
 */

#include "data_define.h"
#include "vendor_sdk.h"
#include "bsp_config.h"

#if (CFG_CAN_MARSTER_EN != 0) && (CFG_CAN_SLAVE_EN != 0)
#error "CFG_CAN_MARSTER_EN and CFG_CAN_SLAVE_EN cannot be enabled at the same time"
#endif

#if (CFG_CAN_SLAVE_EN != 0) && ((CFG_CAN_SLAVE_NODE_ID == 0) || (CFG_CAN_SLAVE_NODE_ID > 31))
#error "CFG_CAN_SLAVE_NODE_ID must be in range 1..31 when CAN slave is enabled"
#endif

/// @brief  global data
GlobalDataDef GlobalData;

static uint32_t user_fram_save_size(void)
{
    uint32_t save_size = (uint32_t)sizeof(GlobalData.Save);
    uint32_t max_size = save_size;

#ifdef CFG_FRAM_AUTOSAVE_MAX_BYTES
    max_size = (uint32_t)CFG_FRAM_AUTOSAVE_MAX_BYTES;
#endif

    if (save_size > max_size)
    {
        save_size = max_size;
    }
    return save_size;
}

/*
 * 功能： BSP 用户配置
 */
static bsp_user_config_t bsp_user_config = {
    .com1_modbus_en = CFG_COM1_MODBUS_EN,
    .com1_baudrate = CFG_COM1_BAUDRATE,
    .com1_slave_idx = CFG_COM1_SLAVE_IDX,
    .com2_modbus_en = CFG_COM2_MODBUS_EN,
    .com2_baudrate = CFG_COM2_BAUDRATE,
    .com2_slave_idx = CFG_COM2_SLAVE_IDX,
    .tcp_modbus_en = CFG_TCP_MODBUS_EN,
    .tcp_slave_idx = CFG_TCP_SLAVE_IDX,
    .ip_addr = {CFG_TCP_IP_ADDR0, CFG_TCP_IP_ADDR1, CFG_TCP_IP_ADDR2, CFG_TCP_IP_ADDR3},
    .tcp_port = CFG_TCP_PORT,
    .dynlib_en = CFG_DYNLIB_EN,
    .fram_data = NULL,
    .fram_size = 0,
    .fram_save_period = CFG_FRAM_SAVE_PERIOD,
    .axis_num_pluse = CFG_AXIS_NUM_PLUSE,
    .axis_num_ethercat = CFG_AXIS_NUM_ETHERCAT,
    .axis_num_can = CFG_AXIS_NUM_CAN,
    .encoder_num = CFG_ENCODER_NUM,
    .encoder_num_ethercat = CFG_ENCODER_NUM_ETHERCAT,
    .main_input_num = CFG_MAIN_INPUT_NUM,
    .main_output_num = CFG_MAIN_OUTPUT_NUM,
    .ethercat_io_num = CFG_ETHERCAT_IO_NUM,
    .ethercat_input_num = CFG_ETHERCAT_INPUT_NUM,
    .ethercat_output_num = CFG_ETHERCAT_OUTPUT_NUM,
    .axis_cycle_time = CFG_AXIS_CYCLE_TIME,
    .can_master_en = CFG_CAN_MARSTER_EN,
    .can_slave_en = CFG_CAN_SLAVE_EN,
    .can_slave_node_id = CFG_CAN_SLAVE_NODE_ID,
    .can_bitrate = BITRATE,
};

// 用户的配置函数 函数在硬件初始化时被调用 不受maintask线程影响
bsp_user_config_t *user_para_init(void)
{
    // 配置fram自动保存
    bsp_user_config.fram_data = GlobalData.Save.Sys.Sys_Save_Addr;
    bsp_user_config.fram_size = user_fram_save_size();
    bsp_user_config.fram_save_period = CFG_FRAM_SAVE_PERIOD;
    //
    // 配置modbus的地址表
    if (bsp_user_config.com1_modbus_en || bsp_user_config.com2_modbus_en || bsp_user_config.tcp_modbus_en)
    {
        // 初始化map表
        modbus_slave_map_config(GlobalData.Read.Sys.Sys_Read_Addr, 0, READ_MAP_SIZE, GlobalData.Write.Sys.Sys_Write_Addr,
                                READ_MAP_SIZE, READ_MAP_SIZE + WRITE_MAP_SIZE, GlobalData.Save.Sys.Sys_Save_Addr,
                                READ_MAP_SIZE + WRITE_MAP_SIZE,
                                READ_MAP_SIZE + WRITE_MAP_SIZE + (int)(bsp_user_config.fram_size / sizeof(u16)));
    }
    return &bsp_user_config;
}
