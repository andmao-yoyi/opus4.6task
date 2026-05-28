#ifndef USER_COMMON_H
#define USER_COMMON_H

#include "math.h"
#include "stdbool.h"
#include "stdint.h"
#include "string.h"

/* 数据类型统一 */
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

#define ON           0
#define OFF          1

// 设置位
#define SETBIT(x, y) ((x) |= (1U << (y)))
// 清除位
#define CLRBIT(x, y) ((x) &= ~(1U << (y)))
// 读取位
#define GETBIT(x, y) ((x) & (1U << (y)))

/// @brief 打印监控信息
void debug_printf(const char *format, ...);
    
/// @brief 打印任务状态
void print_freertos_task_status(void);

/// @brief 系统锁
void Sys_Lock(void);

/// @brief 系统解锁
void Sys_Unlock(void);

/// @brief 发送消息通知开始执行
void BSP_Notify(void);

/// @brief delay ms
/// @note 该函数用于延时ms，用于延时任务。
void sys_delay_ms(uint32_t ms);

/// @brief get tick
/// @note 该函数用于获取系统tick。
/// @return 返回系统tick
uint32_t sys_get_tick(void);

#endif /* USER_COMMON_H */
