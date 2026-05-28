#include "hz_alarm.h"
#include "data_define.h"

// 定义报警系统的边界常量
#define ALARM_MAX_LEVEL    5
#define ALARM_WORDS_PER_LEVEL  4
#define ALARM_BITS_PER_WORD    32

/// @brief 报警分成5个等级 每个等级占用4个u32 每一位代表一个报警
/// @param level 报警等级(1-5)
/// @param word 字索引(0-3)
/// @param bit 位索引(0-31)
void AlarmSetBit(u8 level, u8 word, u8 bit)
{
    // 参数边界检查
    if (level < 1 || level > ALARM_MAX_LEVEL || 
        word >= ALARM_WORDS_PER_LEVEL || 
        bit >= ALARM_BITS_PER_WORD)
    {
        return;
    }

    // 报警分成5个等级 每个等级占用4个u32 每一位代表一个报警
    GSR.ErrorCode[(level - 1) * ALARM_WORDS_PER_LEVEL + word] |= (1UL << bit);
}

/// @brief 清除报警
/// @param level 报警等级(1-5)
/// @param word 字索引(0-3)
/// @param bit 位索引(0-31)
void AlarmClearBit(u8 level, u8 word, u8 bit)
{
    // 参数边界检查
    if (level < 1 || level > ALARM_MAX_LEVEL || 
        word >= ALARM_WORDS_PER_LEVEL || 
        bit >= ALARM_BITS_PER_WORD)
    {
        return;
    }

    GSR.ErrorCode[(level - 1) * ALARM_WORDS_PER_LEVEL + word] &= ~(1UL << bit);
}

/// @brief 获取报警
/// @param level 报警等级(1-5)
/// @param word 字索引(0-3)
/// @param bit 位索引(0-31)
/// @return 1:有报警 0:无报警 -1:参数错误
s32 AlarmGetBit(u8 level, u8 word, u8 bit)
{
    // 参数边界检查
    if (level < 1 || level > ALARM_MAX_LEVEL || 
        word >= ALARM_WORDS_PER_LEVEL || 
        bit >= ALARM_BITS_PER_WORD)
    {
        return -1;
    }

    return (GSR.ErrorCode[(level - 1) * ALARM_WORDS_PER_LEVEL + word] >> bit) & 1U;
}

/// @brief 获取报警等级
/// @return 1-5:报警等级 0:无报警
u8 AlarmGetLevel(void)
{
    // 从最高级别开始检查，找到最高级别的报警
    for (u8 level = ALARM_MAX_LEVEL; level >= 1; level--)
    {
        u8 baseIdx = (level - 1) * ALARM_WORDS_PER_LEVEL;
        // 检查该等级的所有4个u32是否有报警
        for (u8 word = 0; word < ALARM_WORDS_PER_LEVEL; word++)
        {
            if (GSR.ErrorCode[baseIdx + word] != 0)
            {
                return level;
            }
        }
    }

    // 无报警返回0
    return 0;
}

/// @brief 清除所有报警
void AlarmClearAll(void)
{
    // 使用sizeof确保不会越界
    memset(GSR.ErrorCode, 0, sizeof(GSR.ErrorCode));
    GSR.ErrorLevel = 0;
}
