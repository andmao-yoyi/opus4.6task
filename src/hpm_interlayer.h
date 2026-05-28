#ifndef _HPM_INTERLAYER_H_
#define _HPM_INTERLAYER_H_

#include "vendor_common.h"
#include "data_define.h"
#include "vendor_sdk.h"

typedef enum
{
    CLOSING = 0,
    CLOSEDONE = 1,
    OPENING = 2,
    OPENDONE = 3
} OUT_STATUS;

//=============================================================================
// 函数声明
//=============================================================================
/// @brief 初始化输出状态管理器
/// @param open_time 打开时间数组首地址（大小为OUT_MAX_NUM）
/// @param close_time 关闭时间数组首地址（大小为OUT_MAX_NUM）
void out_status_init(uint16_t *open_time, uint16_t *close_time);

/// @brief 读取输入状态
/// @param idx 输入索引
/// @return 输入状态
int in_get(int idx);

/// @brief 读取输出状态
/// @param idx 输出索引
/// @return 输出状态
int out_get(int idx);

/// @brief 设置输出状态
/// @note 该函数使用时会自动更新时间戳
/// @param idx 输出索引
/// @param value 输出状态
/// @return 0成功 1失败
int out_set(int idx, int value);

float spd_rate(float rate);

float spd_rate1000(float rate);

/// @brief 设置逻辑轴映射表
/// @param list 逻辑轴列表，按物理轴顺序排列
/// @param count 列表数量
/// @note 使用示例：
/// @note static const u32 axis_list_mode_a[] = { axis1, axis2, axis3 };
/// @note axis_map_set_list(axis_list_mode_a, sizeof(axis_list_mode_a) / sizeof(axis_list_mode_a[0]));
void axis_map_set_list(const uint32_t *list, uint32_t count);

uint32_t axis_map_to_logic(uint32_t phy_axis, uint32_t *logic_axis);

/// @brief 获取输出状态
/// @param idx 输出索引
/// @return 输出状态
OUT_STATUS out_status(int idx);

/// @brief 获取实际轴数量
/// @return 轴数量
uint32_t get_total_axis_num(void);

/// @brief 获取实际输入口数量
/// @return 输入口数量
uint32_t get_total_in_num(void);

/// @brief 获取实际输出口数量
/// @return 输出口数量
uint32_t get_total_out_num(void);

//==========================================================================================
// 轴接口（应用层封装，调用底层 axis_* 接口）
//==========================================================================================

/// @brief 轴参数加载
/// @param param 轴参数
void mc_param_load(axis_data *param);

/// @brief 轴停止
int mc_stop(uint32_t ax);

/// @brief 轴减速停止
int mc_stop_dec(uint32_t ax);

/// @brief 轴复位
int mc_reset(uint32_t ax);

/// @brief 轴回零操作（使用预加载的回零参数）
/// @param ax 轴索引
/// @return 0成功 -1失败
int mc_home(uint32_t ax);

/// @brief 设置轴回零慢速
/// @param ax 轴索引
/// @param search_zero_spd 回零慢速(单位 unit/s)
/// @return 0成功 -1失败
int mc_set_home_search_zero_spd(uint32_t ax, float search_zero_spd);

/// @brief 轴绝对位置运动（使用预加载的速度参数）
/// @param ax 轴索引
/// @param pos 目标位置（用户单位）
/// @return 0成功 -1失败
int mc_move_abs(uint32_t ax, float pos);

/// @brief 轴相对位置运动（使用预加载的速度参数）
/// @param ax 轴索引
/// @param pos 相对位移量（用户单位）
/// @return 0成功 -1失败
int mc_move_rel(uint32_t ax, float pos);

/// @brief 临时重载单轴运行速度
/// @param ax 轴索引
/// @param new_speed 新速度（用户单位/s）
/// @return 0成功 -1失败
int mc_reload_speed(uint32_t ax, float new_speed);

/// @brief 轴相对位置运动（指定速度比例）
/// @param ax 轴索引
/// @param pos 相对位移量（用户单位）
/// @param rate 速度比例因子
/// @return 0成功 -1失败
int mc_move_rel_rate(uint32_t ax, float pos, float rate);

/// @brief 轴绝对位置运动（指定速度比例）
/// @param ax 轴索引
/// @param pos 目标位置（用户单位）
/// @param rate 速度比例因子
/// @return 0成功 -1失败
int mc_move_abs_rate(uint32_t ax, float pos, float rate);

/// @brief 设置轴运动方向
/// @param idx 轴索引
/// @param dir 方向（1:正向 / 0:反向）
/// @return 0成功 -1失败
int mc_set_dir(uint32_t idx, int dir);

/// @brief 获取轴运动方向
/// @param idx 轴索引
/// @return 方向（1:正向 / 0:反向）
int mc_get_dir(uint32_t idx);

/// @brief 设置轴使能状态
/// @param idx 轴索引
/// @param en 使能状态（1:使能 / 0:失能）
/// @return 0成功 -1失败
int mc_set_en(uint32_t idx, int en);

/// @brief 设置轴每转脉冲数（PPR）
/// @param idx 轴索引
/// @param pulse 每转脉冲数
/// @return 0成功 -1失败
int mc_set_ppr(uint32_t idx, uint32_t pulse);

/// @brief 获取轴每转脉冲数（PPR）
/// @param idx 轴索引
/// @return 每转脉冲数
int mc_get_ppr(uint32_t idx);

/// @brief 设置轴当前位置（脉冲数）
/// @param idx 轴索引
/// @param pos 位置值（脉冲）
/// @return 0成功 -1失败
int mc_set_cur_pos(uint32_t idx, int32_t pos);

/// @brief 设置轴当前位置（用户单位）
/// @param idx 轴索引
/// @param pos 位置值（用户单位）
/// @return 0成功 -1失败
int mc_set_cur_unit_pos(uint32_t idx, float pos);

/// @brief 获取轴状态
/// @param ax 轴索引
/// @return 轴状态（位掩码）
int mc_get_status(uint32_t ax);

/// @brief 检查轴是否就绪
/// @param ax 轴索引
/// @return 1:就绪 / 0:未就绪
int mc_ready(uint32_t ax);

/// @brief 获取轴当前位置（脉冲数）
/// @param idx 轴索引
/// @return 当前位置（脉冲）
int mc_get_cur_pos(uint32_t idx);

/// @brief 获取轴当前位置（用户单位）
/// @param idx 轴索引
/// @return 当前位置（用户单位）
float mc_get_cur_unit_pos(uint32_t idx);

/// @brief 获取驱动器当前位置（用户单位）
/// @param idx 轴索引
/// @return 当前位置（用户单位）
float mc_get_cur_unit_pos_ex(uint32_t idx);

/// @brief 获取轴当前速度
/// @param idx 轴索引
/// @return 当前速度
int32_t mc_get_cur_spd(uint32_t idx);

/// @brief 获取轴每单位脉冲数
/// @param idx 轴索引
/// @return 每单位脉冲数
uint32_t mc_get_pulse_per_unit(uint32_t idx);

/// @brief 获取轴当前速度配置
/// @param idx 轴索引
/// @param profile 速度配置结构体指针（输出）
/// @return 0成功 -1失败
int32_t mc_get_cur_spd_config(uint32_t idx, single_move_config_t *profile);

/// @brief 获取轴错误状态
/// @param idx 轴索引
/// @return 错误状态码
axis_err_t mc_get_error_status(uint32_t idx);

/// @brief 获取轴当前转矩
/// @param idx 轴索引
/// @return 当前转矩值
int mc_get_cur_torque(uint32_t idx);

/// @brief 设置轴转矩限制
/// @param idx 轴索引
/// @param forward_torque 正向转矩限制
/// @param reverse_torque 反向转矩限制
/// @return 0成功 -1失败
int mc_set_torque_limit(uint32_t idx, uint16_t forward_torque, uint16_t reverse_torque);

/// @brief 设置虚拟位置（视觉飞拍）
/// @param idx 轴索引
/// @param pos 位置值（用户单位）
/// @param puls_time 脉冲时间
/// @return 0成功 -1失败
int mc_set_visual_pos(uint32_t idx, float pos, uint8_t puls_time);

/// @brief 加载轴限位配置
/// @param ax 轴索引
/// @param limit_config 限位配置结构体指针
/// @return 0成功 -1失败
int mc_load_limit_config(uint32_t ax, limit_config_t *limit_config);

/// @brief 加载轴回零配置
/// @param ax 轴索引
/// @param home_config 回零配置结构体指针
/// @return 0成功 -1失败
int mc_load_home_config(uint32_t ax, home_config_t *home_config);

/// @brief 获取轴的默认速度配置
void mc_get_default_profile(uint32_t idx, single_move_config_t *profile);

/// @brief 获取轴的默认回零配置
void mc_get_default_home_config(uint32_t idx, home_config_t *home_cfg);

/// @brief 配置轴组成员
/// @param group_id 轴组ID
/// @param axis_list 轴号数组
/// @param axis_num 轴数量
/// @return 0:成功 -1:失败
int mc_group_config(uint32_t group_id, const uint32_t *axis_list, uint8_t axis_num);

/// @brief 清除轴组配置
/// @param group_id 轴组ID
/// @return 0:成功 -1:失败
int mc_group_clear_config(uint32_t group_id);

/// @brief 轴组复位（清除错误状态，保留配置）
/// @param group_id 轴组ID
/// @return 0:成功 -1:失败
int mc_group_reset(uint32_t group_id);

/// @brief 设置轴组运动轨迹
/// @param group_id 轴组ID
/// @param motion_type 运动类型
/// @param motion_data 运动参数指针
/// @param motion_len 运动参数长度
/// @return 0:成功 -1:失败
int mc_group_set_motion(uint32_t group_id, uint32_t motion_type, const void *motion_data, uint32_t motion_len);

/// @brief 启动轴组运动
/// @param group_id 轴组ID
/// @return 0:成功 -1:失败
int mc_group_start(uint32_t group_id);

/// @brief 停止轴组运动
/// @param group_id 轴组ID
/// @return 0:成功 -1:失败
int mc_group_stop(uint32_t group_id);

/// @brief 轴组运行状态查询
/// @param group_id 轴组ID
/// @return 0:空闲 1:运行中 -1:失败
int mc_group_state(uint32_t group_id);

/// @brief 获取轴组最近一次错误码
/// @param group_id 轴组ID
/// @return 0:无错误 其他:错误码
int mc_group_last_error(uint32_t group_id);

/// @brief 设置RTC时间
/// @param hour 时（0-23）
/// @param min 分（0-59）
/// @param sec 秒（0-59）
/// @param ampm 无效参数（填0）
/// @return 0成功
int hz_rtc_set_time(u16 hour, u16 min, u16 sec, u16 ampm);

/// @brief 设置RTC日期
/// @param year 年
/// @param month 月（1-12）
/// @param date 日（1-31）
/// @param week 无效参数（填0）
/// @return 0成功
int hz_rtc_set_date(u16 year, u16 month, u16 date, u16 week);

/// @brief 获取RTC时间
/// @param hour 时（输出）
/// @param min 分（输出）
/// @param sec 秒（输出）
/// @param ampm 无效参数（填NULL）
void hz_rtc_get_time(u16 *hour, u16 *min, u16 *sec, u16 *ampm);

/// @brief 获取RTC日期
/// @param year 年（输出）
/// @param month 月（输出）
/// @param date 日（输出）
/// @param week 无效参数（填NULL）
void hz_rtc_get_date(u16 *year, u16 *month, u16 *date, u16 *week);
#endif /* _HPM_INTERLAYER_H_ */

