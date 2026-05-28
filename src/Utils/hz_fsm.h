#ifndef HZ_FSM_H
#define HZ_FSM_H

#include <stdint.h>

/// @brief 状态机的几种状态
typedef enum
{
    INIT,  // 0 初始态
    STOP,  // 1 停止
    RUN,   // 2 运行
    RESET, // 3 复位
    SCRAM, // 4 急停
    PAUSED // 5 暂停
} HZ_FSM_State;

/// @brief 状态机的几种命令
typedef enum
{
    CMD_NONE,  //  0无命令
    CMD_RUN,   //  1运行
    CMD_RESET, //  2复位
    CMD_PAUSE, //  3暂停
    CMD_STOP,  //  4停止
    CMD_SCRAM  //  5急停
} HZ_FSM_CMD_Type;

/// @brief 状态切换后的回调函数
typedef void (*HZ_FSM_Callback)(void);

/// @brief 状态转换表项结构
typedef struct
{
    HZ_FSM_State    current_state; // 当前状态
    HZ_FSM_CMD_Type cmd;           // 指令
    HZ_FSM_State    next_state;    // 下一个状态
    HZ_FSM_Callback callback;      // 状态转换时的回调函数
} HZ_FSM_TransItem;

/// @brief 状态机结构
typedef struct
{
    HZ_FSM_State            current_state; // 当前状态
    const HZ_FSM_TransItem *trans_table;   // 状态转换表
    uint16_t                table_size;    // 转换表大小
    HZ_FSM_CMD_Type         current_cmd;   // 当前命令
} HZ_FSM_Handle;

extern HZ_FSM_Handle fsm;

/// @brief 初始化状态机
/// @param fsm 状态机句柄
/// @param trans_table 状态转换表
/// @param table_size 转换表大小
/// @param initial_state 初始状态
/// @return 0:成功 其他:失败
int hz_fsm_init(HZ_FSM_Handle *fsm, const HZ_FSM_TransItem *trans_table, uint16_t table_size, HZ_FSM_State initial_state);

/// @brief 获取当前状态
/// @param fsm 状态机句柄
/// @return 当前状态
HZ_FSM_State hz_fsm_get_state(const HZ_FSM_Handle *fsm);

/// @brief 状态机状态转换
/// @param fsm 状态机句柄
/// @return 0:成功 其他:失败
int hz_fsm_transit(HZ_FSM_Handle *fsm);

/// @brief 发送命令
/// @param fsm 状态机句柄
/// @param cmd 命令
void hz_fsm_cmd(HZ_FSM_Handle *fsm, uint32_t cmd);


#endif // HZ_FSM_H
