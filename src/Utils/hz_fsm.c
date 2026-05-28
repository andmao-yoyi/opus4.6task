#include "hz_fsm.h"
#include <stdio.h>

// 设置cmd
void hz_fsm_cmd(HZ_FSM_Handle *fsm, uint32_t cmd)
{
    // 按照指令优先级控制指令是否生效
    if (cmd > fsm->current_cmd)
    {
        fsm->current_cmd = cmd;
    }
}

// 状态机状态转换
int hz_fsm_transit(HZ_FSM_Handle *fsm)
{
    if (fsm == NULL)
    {
        return -1;
    }
    // 查找状态转换表中是否存在合法的转换
    for (uint16_t i = 0; i < fsm->table_size; i++)
    {
        if (fsm->trans_table[i].current_state == fsm->current_state && fsm->trans_table[i].cmd == fsm->current_cmd)
        {
            // 执行状态转换
            fsm->current_state = fsm->trans_table[i].next_state;
            // 如果存在回调函数，则执行
            if (fsm->trans_table[i].callback != NULL)
            {
                fsm->trans_table[i].callback();
            }
            return 0;
        }
    }

    // 清零cmd 防止重复执行
    fsm->current_cmd = CMD_NONE;

    // 未找到合法的状态转换
    return -1;
}

// 初始化状态机
int hz_fsm_init(HZ_FSM_Handle *fsm, const HZ_FSM_TransItem *trans_table, uint16_t table_size, HZ_FSM_State initial_state)
{
    if (fsm == NULL || trans_table == NULL || table_size == 0)
    {
        return -1;
    }

    fsm->current_state = initial_state;
    fsm->current_cmd = 0;
    fsm->trans_table = trans_table;
    fsm->table_size = table_size;

    return 0;
}

// 获取当前状态
HZ_FSM_State hz_fsm_get_state(const HZ_FSM_Handle *fsm)
{
    if (fsm == NULL)
    {
        return INIT; // 返回初始状态作为默认值
    }
    return fsm->current_state;
}
