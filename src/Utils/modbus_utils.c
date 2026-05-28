/// modbus通讯工具
#include "modbus_utils.h"
#include "data_define.h"
#include "hpm_interlayer.h"
#include "vendor_sdk.h"

// 读取输入状态
void IO_Update(uint16_t address)
{
    int i = 0;

    // 输入口地址范围 输出口地址范围
    if (!(address >= 310 && address < 390) && !(address >= 1020 && address < 1100))
    {
        return;
    }
    // 清空所有输入状态寄存器
    GSR.InputStatus[0] = 0;
    GSR.InputStatus[1] = 0;
    GSR.InputStatus[2] = 0;
    GSR.InputStatus[3] = 0;
    // 输入信号寄存器赋值
    for (i = 0; i < 32; i++)
    {
        GSR.InputStatus[0] |= (in_get(i) << i);
    }
    for (i = 0; i < 32; i++)
    {
        GSR.InputStatus[1] |= (in_get(i + 32) << i);
    }
    for (i = 0; i < 32; i++)
    {
        GSR.InputStatus[2] |= (in_get(i + 64) << i);
    }
    for (i = 0; i < 32; i++)
    {
        GSR.InputStatus[3] |= (in_get(i + 96) << i);
    }

    // 输出信号寄存器赋值
    GSW.OutputStatus[0] = 0;
    GSW.OutputStatus[1] = 0;
    GSW.OutputStatus[2] = 0;
    GSW.OutputStatus[3] = 0;
    for (i = 0; i < 32; i++)
    {
        GSW.OutputStatus[0] |= (out_get(i) << i);
    }
    for (i = 0; i < 32; i++)
    {
        GSW.OutputStatus[1] |= (out_get(i + 32) << i);
    }
    for (i = 0; i < 32; i++)
    {
        GSW.OutputStatus[2] |= (out_get(i + 64) << i);
    }
    for (i = 0; i < 32; i++)
    {
        GSW.OutputStatus[3] |= (out_get(i + 96) << i);
    }
}

/// @brief 通讯写入输出状态切换
/// @param address 通讯地址
/// @param len 写入长度
void IO_WriteOutputStatus(uint16_t address)
{
    int i = 0;

    // 输出口地址范围
    if (!(address >= 1020 && address < 1099))
    {
        return;
    }
    ///// 根据实际的io数量修改
    for (i = 0; i < 32; i++)
    {
        out_set(i, GSW.OutputStatus[0] >> i & 0x01);
        out_set(i + 32, GSW.OutputStatus[1] >> i & 0x01);
        out_set(i + 64, GSW.OutputStatus[2] >> i & 0x01);
        out_set(i + 96, GSW.OutputStatus[3] >> i & 0x01);
    }
}
