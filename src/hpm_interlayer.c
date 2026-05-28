/**
 * @file hpm_interlayer.c
 * @author Application Team
 * @brief
 * hpm芯片接口设备兼容层
 *
 *
 *
 * @version 0.1
 * @date 2024-12-26
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "hpm_interlayer.h"
#include "alarm.h"
#include "iodefine.h"

//=============================================================================
// 定义区
//=============================================================================
struct _out_status
{
    uint32_t  timestamp[OUT_MAX_NUM]; // 时间戳
    uint32_t  value[OUT_MAX_NUM];     // 状态
    uint16_t *open_time;              // 打开时间
    uint16_t *close_time;             // 关闭时间
};

struct _out_status out_status_data;

//=============================================================================
// 函数声明
//=============================================================================

//=============================================================================
// 代码区
//=============================================================================

//==========================================================================================
// io接口
//==========================================================================================

/// @brief 初始化
/// @param open_time 打开时间
/// @param close_time 关闭时间
void out_status_init(uint16_t *open_time, uint16_t *close_time)
{
    uint32_t out_num = get_total_out_num();

    // 运行期保护：实际输出数量不能超过应用层数组上限
    if (out_num > OUT_MAX_NUM)
    {
        out_num = OUT_MAX_NUM;
    }

    out_status_data.open_time = open_time;
    out_status_data.close_time = close_time;
    // 仅初始化实际输出点，避免按最大点位遍历造成额外开销
    for (uint32_t i = 0; i < out_num; i++)
    {
        write_digital_output(i, OFF);
        // 初始化时间戳
        out_status_data.timestamp[i] = sys_get_tick();
        // 获取真实输出状态
        out_status_data.value[i] = out_get(i);
    }
}

// io接口兼容层
/// @brief 读取输入状态
/// @param idx 输入索引
/// @return 输入状态
int in_get(int idx)
{
    return read_digital_input(idx);
}

/// @brief 读取输出状态
/// @param idx 输出索引
/// @return 输出状态
int out_get(int idx)
{
    return read_digital_output(idx);
}

/// @brief 设置输出状态
/// @note 该函数使用时会自动代入时间戳
/// @param idx 输出索引
/// @param value 输出状态
/// @return 0
int out_set(int idx, int value)
{
    if (idx >= OUT_MAX_NUM)
    {
        return 1;
    }
    if (value != out_status_data.value[idx])
    {
        // 更新时间戳
        out_status_data.timestamp[idx] = sys_get_tick();
    }
    out_status_data.value[idx] = value;
    return write_digital_output(idx, value);
}

/// @brief 获取输出状态
/// @param idx 输出索引
/// @return 输出状态
OUT_STATUS out_status(int idx)
{
    if (idx >= OUT_MAX_NUM)
    {
        return CLOSEDONE;
    }
    if (out_status_data.value[idx] == ON)
    {
        // 计算当前时间与打开时间的时间差大于open_time[idx]
        if (sys_get_tick() - out_status_data.timestamp[idx] >= out_status_data.open_time[idx])
        {
            return OPENDONE;
        }
        else
        {
            return OPENING;
        }
    }
    else
    {
        // 计算当前时间与关闭时间的时间差大于close_time[idx]
        if (sys_get_tick() - out_status_data.timestamp[idx] >= out_status_data.close_time[idx])
        {
            return CLOSEDONE;
        }
        else
        {
            return CLOSING;
        }
    }
}

/**
 * @brief 速度比例计算，不同项目可按配置调整移动速度
 *
 * @param rate 比例值，按项目配置解释比例基准
 * @return float
 */
float spd_rate(float rate)
{
    // 100份
    return rate / 100.0f;
}

float spd_rate1000(float rate)
{
    // 1000份
    return rate / 1000.0f;
}

//==========================================================================================
// 轴接口
//==========================================================================================
// 缺失轴报警：逻辑轴不存在或超出物理轴数量时触发
static void axis_alarm_missing(uint32_t logic_axis)
{
    uint32_t alarm_axis = logic_axis + 1;
    if (alarm_axis < 32U)
    {
        AlarmSetBit(ALM3_AxisErr(alarm_axis));
    }
    else
    {
        AlarmSetBit(ALM3_AxisDisable);
    }
}

// 当前启用的逻辑轴列表（按物理轴顺序排列）
static const uint32_t *axis_logic_list = NULL;
// 列表数量
static uint32_t axis_logic_list_count = 0;
// 映射版本号：外部切换表时递增触发重建
static uint32_t axis_map_version = 0;

void axis_map_set_list(const uint32_t *list, uint32_t count)
{
    // 仅记录指针与数量，映射在首次使用时延迟构建
    axis_logic_list = list;
    axis_logic_list_count = count;
    axis_map_version++;
}

// 逻辑轴映射到物理轴（缺失轴可选择是否报警）
static int axis_map_to_phy(uint32_t logic_axis, uint32_t *phy_axis, int alarm_on_missing)
{
    static uint32_t axis_logic_to_phy[AXIS_MAX_NUM];
    static int axis_map_inited = 0;
    static uint32_t axis_map_cached_version = 0;
    if (!axis_map_inited || axis_map_cached_version != axis_map_version)
    {
        // 初始化映射表，未配置的轴标记为无效
        for (uint32_t i = 0; i < AXIS_MAX_NUM; i++)
        {
            axis_logic_to_phy[i] = AXIS_MAX_NUM;
        }
        const uint32_t *axis_logic_used_list = axis_logic_list;
        uint32_t list_count = axis_logic_list_count;
        if (axis_logic_used_list == NULL || list_count == 0)
        {
            // 未设置运行期表，使用默认表作为兜底
            static const uint32_t default_axis_logic_used_list[] = AXIS_LOGIC_USED_LIST_DEFAULT;
            axis_logic_used_list = default_axis_logic_used_list;
            list_count = sizeof(default_axis_logic_used_list) / sizeof(default_axis_logic_used_list[0]);
        }
        uint32_t phy_index = 0;
        for (uint32_t i = 0; i < list_count; i++)
        {
            uint32_t logic = axis_logic_used_list[i];
            if (logic < AXIS_MAX_NUM)
            {
                // 列表顺序即物理轴顺序
                axis_logic_to_phy[logic] = phy_index;
                phy_index++;
            }
        }
        axis_map_inited = 1;
        axis_map_cached_version = axis_map_version;
    }
    if (logic_axis >= AXIS_MAX_NUM)
    {
        if (alarm_on_missing)
        {
            axis_alarm_missing(logic_axis);
        }
        return 0;
    }
    uint32_t total_phy = pBsp->axis_num_pluse + pBsp->axis_num_ethercat;
    uint32_t mapped = axis_logic_to_phy[logic_axis];
    if (mapped >= total_phy)
    {
        if (alarm_on_missing)
        {
            axis_alarm_missing(logic_axis);
        }
        return 0;
    }
    *phy_axis = mapped;
    return 1;
}

// 物理轴到逻辑轴映射
uint32_t axis_map_to_logic(uint32_t phy_axis, uint32_t *logic_axis)
{
    static uint32_t phy_to_logic_map[AXIS_MAX_NUM] = {0};
    static uint32_t map_initialized = 0;
    static uint32_t phy_map_cached_version = 0;

    if (!map_initialized || phy_map_cached_version != axis_map_version)
    {
        for (uint32_t i = 0; i < AXIS_MAX_NUM; i++)
        {
            phy_to_logic_map[i] = AXIS_MAX_NUM;
        }
        for (uint32_t i = 0; i < AXIS_MAX_NUM; i++)
        {
            uint32_t phy_axis_num;
            if (axis_map_to_phy(i, &phy_axis_num, 0))
            {
                phy_to_logic_map[phy_axis_num] = i;
            }
        }
        map_initialized = 1;
        phy_map_cached_version = axis_map_version;
    }
    // 检查物理轴号是否有效
    if (phy_axis >= AXIS_MAX_NUM)
    {
        return 0; // 返回错误
    }
    // 获取对应的逻辑轴号
    *logic_axis = phy_to_logic_map[phy_axis];
    // 检查映射是否有效
    if (*logic_axis >= AXIS_MAX_NUM)
    {
        return 0; // 返回错误
    }
    return 1; // 返回成功
}

//==========================================================================================
// 轴参数缓存（用于应用层接口默认参数）
//==========================================================================================
static axis_data *s_axis_param = NULL;
static float      s_axis_ppu_cache[AXIS_MAX_NUM];
static uint8_t    s_axis_ppu_cache_valid[AXIS_MAX_NUM];
static float      s_axis_home_search_zero_spd[AXIS_MAX_NUM];

static void axis_init_home_search_zero_spd_cache(void)
{
    static uint8_t inited = 0U;

    if (inited)
    {
        return;
    }

    for (uint32_t i = 0; i < AXIS_MAX_NUM; i++)
    {
        s_axis_home_search_zero_spd[i] = 1.0f;
    }
    inited = 1U;
}

/// @brief 根据地址表中的 MPR/PPR 解析脉冲当量
/// @param param 单轴参数
/// @return 脉冲当量（pulse/unit）
static float axis_resolve_pulse_per_unit(const axis_data *param)
{
    float ppu = 1.0f;

    if (param != NULL && param->Config.MPR > VENDOR_AXIS_PPU_EPS)
    {
        ppu = (float)param->Config.PPR / param->Config.MPR;
    }
    if (ppu <= VENDOR_AXIS_PPU_EPS)
    {
        ppu = 1.0f;
    }
    return ppu;
}

/// @brief 判断脉冲当量是否需要重新下发
/// @param idx 轴索引
/// @param ppu 新的脉冲当量
/// @return 1 需要下发 0 无需下发
static int axis_need_update_pulse_per_unit(uint32_t idx, float ppu)
{
    float diff = 0.0f;

    if (idx >= AXIS_MAX_NUM)
    {
        return 0;
    }
    if (!s_axis_ppu_cache_valid[idx])
    {
        return 1;
    }

    diff = ppu - s_axis_ppu_cache[idx];
    return ((diff > VENDOR_AXIS_PPU_EPS) || (diff < -VENDOR_AXIS_PPU_EPS)) ? 1 : 0;
}

/// @brief 获取轴的默认速度配置
void mc_get_default_profile(uint32_t idx, single_move_config_t *profile)
{
    if (!profile || !s_axis_param || idx >= AXIS_MAX_NUM)
    {
        return;
    }

    profile->pulse_per_unit = axis_resolve_pulse_per_unit(&s_axis_param[idx]);
    profile->max_vel = s_axis_param[idx].speed.max_speed;
    profile->acc_time = (int32_t)s_axis_param[idx].speed.acc_time;
    profile->dec_time = (int32_t)s_axis_param[idx].speed.dec_time;
    profile->acc_model = 0; // 默认T型加减速
}

/// @brief 获取轴的默认回零配置
void mc_get_default_home_config(uint32_t idx, home_config_t *home_cfg)
{
    if (!home_cfg || !s_axis_param || idx >= AXIS_MAX_NUM)
    {
        return;
    }
    axis_init_home_search_zero_spd_cache();
    home_cfg->home_type = s_axis_param[idx].Config.home_type;
    home_cfg->org_num = s_axis_param[idx].Config.org_idx;
    home_cfg->org_lev = s_axis_param[idx].Config.set.org_single_lev;
    home_cfg->search_spd = s_axis_param[idx].speed.home_vel;
    home_cfg->search_zero_spd = s_axis_home_search_zero_spd[idx];
    home_cfg->org_offset = s_axis_param[idx].Config.org_offset;
    home_cfg->timeout_ms = 0U; // 地址表暂未提供该字段，先交给底层默认兜底值
}

/// @brief 配置地址表中的轴参数
/// @param param
void mc_param_load(axis_data *param)
{
    int            i = 0;
    limit_config_t limit_config = {0};
    home_config_t  home_config = {0};
    float          pulse_per_unit = 1.0f;

    axis_init_home_search_zero_spd_cache();

    // 保存参数指针，供应用层接口使用
    s_axis_param = param;

    // 获取轴数量
    int axis_num = pBsp->axis_num_pluse + pBsp->axis_num_ethercat;
    if (axis_num >= AXIS_MAX_NUM)
    {
        axis_num = AXIS_MAX_NUM;
    }
    for (i = 0; i < axis_num; i++)
    {
        uint32_t phy_axis = 0;
        if (!axis_map_to_phy(i, &phy_axis, 0))
        {
            continue;
        }
        // 每个轴都重新组装一份配置，避免上一个轴的字段残留到下一个轴。
        memset(&limit_config, 0, sizeof(limit_config));
        memset(&home_config, 0, sizeof(home_config));

        // 脉冲当量只在参数变化时下发，避免逻辑周期内重复投递同一参数。
        pulse_per_unit = axis_resolve_pulse_per_unit(&param[i]);
        if (axis_need_update_pulse_per_unit((uint32_t)i, pulse_per_unit))
        {
            if (axis_set_pulse_per_unit((uint32_t)phy_axis, pulse_per_unit) == 0)
            {
                s_axis_ppu_cache[i] = pulse_per_unit;
                s_axis_ppu_cache_valid[i] = 1U;
            }
        }
        // 轴报警设置
        if (param[i].Config.set.alm_en)
        {
            limit_config.alm_lvl = param[i].Config.set.alm_single_lev;
            limit_config.alm_single_idx = param[i].Config.alm_ext_idx;
        }
        else
        {
            limit_config.alm_lvl = 3; // 无效
        }
        // GSS 中的软件限位用用户单位保存，这里统一换算成脉冲再下发到底层。
        limit_config.software_pos_limit_en = param[i].Config.set.soft_pos_limit ? 1U : 0U;
        limit_config.software_neg_limit_en = param[i].Config.set.soft_neg_limit ? 1U : 0U;
        limit_config.software_limit_max = (int32_t)(param[i].Config.max_pos * pulse_per_unit);
        limit_config.software_limit_min = (int32_t)(param[i].Config.min_pos * pulse_per_unit);
        limit_config.hardware_pos_limit_en = param[i].Config.set.hardware_pos_limit ? 1U : 0U;
        limit_config.hardware_neg_limit_en = param[i].Config.set.hardware_neg_limit ? 1U : 0U;
        limit_config.hardware_limit_neg_idx = param[i].Config.neg_limit_idx;
        limit_config.hardware_limit_pos_idx = param[i].Config.pos_limit_idx;
        // 硬件限位有效电平从地址表配置透传到底层。
        limit_config.hardware_pos_limit_lev = param[i].Config.set.hardware_pos_limit_lev ? 1U : 0U;
        limit_config.hardware_neg_limit_lev = param[i].Config.set.hardware_neg_limit_lev ? 1U : 0U;
        // 轴的限位设置
        axis_load_limit_config(phy_axis, &limit_config);
        // 轴的原点参数设置（仅配置信号，速度由回零命令携带）
        home_config.home_type = param[i].Config.home_type;
        home_config.org_num = param[i].Config.org_idx;
        home_config.org_lev = param[i].Config.set.org_single_lev;
        home_config.search_spd = param[i].speed.home_vel;
        home_config.search_zero_spd = s_axis_home_search_zero_spd[i];
        home_config.org_offset = param[i].Config.org_offset;
        axis_load_home_config(phy_axis, &home_config);
    }
}

//==========================================================================================
// 轴应用层接口实现（mc_* 封装 axis_* 底层接口）
//==========================================================================================

/// @brief 停止轴运动（立即停止）
/// @param ax 轴索引
/// @return 执行结果
int mc_stop(uint32_t ax)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(ax, &phy_axis, 0))
    {
        return -1;
    }
    return axis_stop(phy_axis);
}

/// @brief 停止轴运动（减速停止）
/// @param ax 轴索引
/// @return 执行结果
int mc_stop_dec(uint32_t ax)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(ax, &phy_axis, 0))
    {
        return -1;
    }
    return axis_stop_dec(phy_axis);
}

/// @brief 复位轴
/// @param ax 轴索引
/// @return 执行结果
int mc_reset(uint32_t ax)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(ax, &phy_axis, 0))
    {
        return -1;
    }
    return axis_reset(phy_axis);
}

/// @brief 轴回零操作（使用默认回零配置）
/// @param ax 轴索引
/// @return 执行结果
int mc_home(uint32_t ax)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(ax, &phy_axis, 0))
    {
        return -1;
    }
    home_config_t home_cfg = {0};
    mc_get_default_home_config(ax, &home_cfg);
    return axis_home(phy_axis, &home_cfg);
}

/// @brief 设置轴回零慢速
/// @param ax 轴索引
/// @param search_zero_spd 回零慢速(单位 unit/s)
/// @return 0成功 -1失败
int mc_set_home_search_zero_spd(uint32_t ax, float search_zero_spd)
{
    if (ax >= AXIS_MAX_NUM || search_zero_spd <= 0.0f)
    {
        return -1;
    }
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(ax, &phy_axis, 0))
    {
        return -1;
    }
    axis_init_home_search_zero_spd_cache();
    s_axis_home_search_zero_spd[phy_axis] = search_zero_spd;
    return 0;
}

/// @brief 轴绝对位置运动（使用默认速度配置）
/// @param ax 轴索引
/// @param pos 目标位置（单位）
/// @return 执行结果
int mc_move_abs(uint32_t ax, float pos)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(ax, &phy_axis, 0))
    {
        return -1;
    }
    single_move_config_t profile = {0};
    mc_get_default_profile(ax, &profile);
    return axis_move_abs(phy_axis, pos, &profile);
}

/// @brief 轴相对位置运动（使用默认速度配置）
/// @param ax 轴索引
/// @param pos 相对位移量（单位）
/// @return 执行结果
int mc_move_rel(uint32_t ax, float pos)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(ax, &phy_axis, 0))
    {
        return -1;
    }
    single_move_config_t profile = {0};
    mc_get_default_profile(ax, &profile);
    return axis_move_rel(phy_axis, pos, &profile);
}

/// @brief 临时重载单轴运行速度
/// @param ax 轴索引
/// @param new_speed 新速度（用户单位/s）
/// @return 执行结果
int mc_reload_speed(uint32_t ax, float new_speed)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(ax, &phy_axis, 0))
    {
        return -1;
    }
    return axis_reload_speed(phy_axis, new_speed);
}

/// @brief 轴相对位置运动（指定速度比例）
/// @param ax 轴索引
/// @param pos 相对位移量（单位）
/// @param rate 速度比例因子（0.0-1.0）
/// @return 执行结果
int mc_move_rel_rate(uint32_t ax, float pos, float rate)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(ax, &phy_axis, 0))
    {
        return -1;
    }
    single_move_config_t profile = {0};
    mc_get_default_profile(ax, &profile);
    // 在应用层做速度比例换算
    profile.max_vel *= rate;
    return axis_move_rel(phy_axis, pos, &profile);
}

/// @brief 轴绝对位置运动（指定速度比例）
/// @param ax 轴索引
/// @param pos 目标位置（单位）
/// @param rate 速度比例因子（0.0-1.0）
/// @return 执行结果
int mc_move_abs_rate(uint32_t ax, float pos, float rate)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(ax, &phy_axis, 0))
    {
        return -1;
    }
    single_move_config_t profile = {0};
    mc_get_default_profile(ax, &profile);
    // 在应用层做速度比例换算
    profile.max_vel *= rate;
    return axis_move_abs(phy_axis, pos, &profile);
}

/// @brief 设置轴运动方向
/// @param idx 轴索引
/// @param dir 方向（1: 正向 / 0: 反向）
/// @return 执行结果
int mc_set_dir(uint32_t idx, int dir)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(idx, &phy_axis, 0))
    {
        return -1;
    }
    return axis_set_dir(phy_axis, dir);
}

/// @brief 获取轴运动方向
/// @param idx 轴索引
/// @return 方向（1: 正向 / 0: 反向）
int mc_get_dir(uint32_t idx)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(idx, &phy_axis, 0))
    {
        return -1;
    }
    return axis_get_dir(phy_axis);
}

/// @brief 设置轴使能状态
/// @param idx 轴索引
/// @param en 使能状态（1: 使能 / 0: 失能）
/// @return 执行结果
int mc_set_en(uint32_t idx, int en)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(idx, &phy_axis, 0))
    {
        return -1;
    }
    return axis_set_en(phy_axis, en);
}

/// @brief 设置轴每转脉冲数（PPR）
/// @param idx 轴索引
/// @param pulse 每转脉冲数
/// @return 执行结果
int mc_set_ppr(uint32_t idx, uint32_t pulse)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(idx, &phy_axis, 0))
    {
        return -1;
    }
    return axis_set_ppr(phy_axis, pulse);
}

/// @brief 获取轴每转脉冲数（PPR）
/// @param idx 轴索引
/// @return 每转脉冲数
int mc_get_ppr(uint32_t idx)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(idx, &phy_axis, 0))
    {
        return -1;
    }
    return axis_get_ppr(phy_axis);
}

/// @brief 设置轴当前位置（脉冲数）
/// @param idx 轴索引
/// @param pos 位置值（脉冲）
/// @return 执行结果
int mc_set_cur_pos(uint32_t idx, int32_t pos)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(idx, &phy_axis, 0))
    {
        return -1;
    }
    return axis_set_cur_pos(phy_axis, pos);
}

/// @brief 设置轴当前位置（单位）
/// @param idx 轴索引
/// @param pos 位置值（单位）
/// @return 执行结果
int mc_set_cur_unit_pos(uint32_t idx, float pos)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(idx, &phy_axis, 0))
    {
        return -1;
    }
    return axis_set_cur_unit_pos(phy_axis, pos);
}

/// @brief 获取轴状态
/// @param ax 轴索引
/// @return 轴状态（位掩码）
int mc_get_status(uint32_t ax)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(ax, &phy_axis, 0))
    {
        return -1;
    }
    return axis_get_status(phy_axis);
}

/// @brief 检查轴是否就绪
/// @param ax 轴索引
/// @return 1: 就绪 / 0: 未就绪
int mc_ready(uint32_t ax)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(ax, &phy_axis, 0))
    {
        return -1;
    }
    return axis_ready(phy_axis);
}

/// @brief 获取轴当前位置（脉冲数）
/// @param idx 轴索引
/// @return 当前位置（脉冲）
int mc_get_cur_pos(uint32_t idx)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(idx, &phy_axis, 0))
    {
        return -1;
    }
    return axis_get_cur_pos(phy_axis);
}

/// @brief 获取轴当前位置（单位）
/// @param idx 轴索引
/// @return 当前位置（单位）
float mc_get_cur_unit_pos(uint32_t idx)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(idx, &phy_axis, 0))
    {
        return -1;
    }
    return axis_get_cur_unit_pos(phy_axis);
}

/// @brief 获取轴当前位置（用户单位，扩展版）
/// @param idx 轴索引
/// @return 当前位置（用户单位）
float mc_get_cur_unit_pos_ex(uint32_t idx)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(idx, &phy_axis, 0))
    {
        return -1;
    }
    return axis_get_cur_unit_pos_ex(phy_axis);
}

/// @brief 获取轴当前速度
/// @param idx 轴索引
/// @return 当前速度
int32_t mc_get_cur_spd(uint32_t idx)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(idx, &phy_axis, 0))
    {
        return -1;
    }
    return axis_get_cur_spd(phy_axis);
}

/// @brief 获取轴每单位脉冲数
/// @param idx 轴索引
/// @return 每单位脉冲数
uint32_t mc_get_pulse_per_unit(uint32_t idx)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(idx, &phy_axis, 0))
    {
        return -1;
    }
    return axis_get_pulse_per_unit(phy_axis);
}

/// @brief 获取轴当前速度配置
/// @param idx 轴索引
/// @param profile 速度配置结构体指针（输出）
/// @return 0成功 -1失败
int32_t mc_get_cur_spd_config(uint32_t idx, single_move_config_t *profile)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(idx, &phy_axis, 0))
    {
        return -1;
    }
    return axis_get_cur_spd_config(phy_axis, profile);
}

/// @brief 获取轴错误状态
/// @param idx 轴索引
/// @return 错误状态码
axis_err_t mc_get_error_status(uint32_t idx)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(idx, &phy_axis, 0))
    {
        return -1;
    }
    return axis_get_error_status(phy_axis);
}

/// @brief 获取轴当前转矩
/// @param idx 轴索引
/// @return 当前转矩值
int mc_get_cur_torque(uint32_t idx)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(idx, &phy_axis, 0))
    {
        return -1;
    }
    return axis_get_cur_torque(phy_axis);
}

/// @brief 设置轴转矩限制
/// @param idx 轴索引
/// @param forward_torque 正向转矩限制
/// @param reverse_torque 反向转矩限制
/// @return 执行结果
int mc_set_torque_limit(uint32_t idx, uint16_t forward_torque, uint16_t reverse_torque)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(idx, &phy_axis, 0))
    {
        return -1;
    }
    return axis_set_torque_limit(phy_axis, forward_torque, reverse_torque);
}

/// @brief 设置轴虚拟位置
/// @param idx 轴索引
/// @param pos 位置值（单位）
/// @param puls_time 脉冲时间
/// @return 执行结果
int mc_set_visual_pos(uint32_t idx, float pos, uint8_t puls_time)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(idx, &phy_axis, 0))
    {
        return -1;
    }
    // 兼容旧应用层单点飞拍接口：默认只配置一个位置，并按双向穿越触发。
    float pos_list[1] = { pos };
    int direction_list[1] = { 3 };
    return axis_set_visual_pos(phy_axis, pos_list, direction_list, 1, puls_time);
}

/// @brief 加载轴限位配置
/// @param ax 轴索引
/// @param limit_config 限位配置结构体指针
/// @return 执行结果
int mc_load_limit_config(uint32_t ax, limit_config_t *limit_config)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(ax, &phy_axis, 0))
    {
        return -1;
    }
    return axis_load_limit_config(phy_axis, limit_config);
}

/// @brief 加载轴回零配置
/// @param ax 轴索引
/// @param home_config 回零配置结构体指针
/// @return 执行结果
int mc_load_home_config(uint32_t ax, home_config_t *home_config)
{
    uint32_t phy_axis = 0;
    if (!axis_map_to_phy(ax, &phy_axis, 0))
    {
        return -1;
    }
    return axis_load_home_config(phy_axis, home_config);
}

/// @brief 配置轴组成员
/// @param group_id 轴组ID
/// @param axis_list 轴号数组
/// @param axis_num 轴数量
/// @return 0:成功 -1:失败
int mc_group_config(uint32_t group_id, const uint32_t *axis_list, uint8_t axis_num)
{
    if (axis_num > MAX_GROUP_AXIS_NUM) return -1;
    uint32_t phy_axis[MAX_GROUP_AXIS_NUM] = {0};
    for (int i = 0; i < axis_num; i++)
    {
        if (!axis_map_to_phy(axis_list[i], &phy_axis[i], 0))
        {
            return -1;
        }
    }
    return axis_group_config(group_id, phy_axis, axis_num);
}

/// @brief 清除轴组配置
/// @param group_id 轴组ID
/// @return 0:成功 -1:失败
int mc_group_clear_config(uint32_t group_id)
{
    return axis_group_clear_config(group_id);
}

/// @brief 轴组复位
/// @param group_id 轴组ID
/// @return 0:成功 -1:失败
int mc_group_reset(uint32_t group_id)
{
    return axis_group_reset(group_id);
}

/// @brief 设置轴组运动轨迹
/// @param group_id 轴组ID
/// @param motion_type 运动类型
/// @param motion_data 运动参数指针
/// @param motion_len 运动参数长度
/// @return 0:成功 -1:失败
int mc_group_set_motion(uint32_t group_id, uint32_t motion_type, const void *motion_data, uint32_t motion_len)
{
    return axis_group_set_motion(group_id, motion_type, motion_data, motion_len);
}

/// @brief 启动轴组运动
/// @param group_id 轴组ID
/// @return 0:成功 -1:失败
int mc_group_start(uint32_t group_id)
{
    return axis_group_start(group_id);
}

/// @brief 停止轴组运动
/// @param group_id 轴组ID
/// @return 0:成功 -1:失败
int mc_group_stop(uint32_t group_id)
{
    return axis_group_stop(group_id);
}

/// @brief 轴组运行状态查询
/// @param group_id 轴组ID
/// @return 0:空闲 1:运行中 -1:失败
int mc_group_state(uint32_t group_id)
{
    return axis_group_state(group_id);
}

/// @brief 获取轴组最近一次错误码
/// @param group_id 轴组ID
/// @return 0:无错误 其他:错误码
int mc_group_last_error(uint32_t group_id)
{
    return axis_group_last_error(group_id);
}

/// @brief 获取实际轴数量
/// @return 轴数量
uint32_t get_total_axis_num(void)
{
    return pBoardResource->read_axis_num;
}

/// @brief 获取实际输入口数量
/// @return 输入口数量
uint32_t get_total_in_num(void)
{
    return pBoardResource->read_input_num;
}

/// @brief 获取实际输出口数量
/// @return 输出口数量
uint32_t get_total_out_num(void)
{
    return pBoardResource->read_output_num;
}

/// @brief 设置RTC时间
/// @param hour 时（0-23）
/// @param min 分（0-59）
/// @param sec 秒（0-59）
/// @param ampm 无效参数（填0）
/// @return 0成功
int hz_rtc_set_time(u16 hour, u16 min, u16 sec, u16 ampm)
{
    rtc_datetime_t rtc_time = {0};
    rtc_get_datetime(&rtc_time);
    rtc_time.hour = hour;
    rtc_time.minute = min;
    rtc_time.second = sec;
    rtc_set_datetime(&rtc_time);
    return 0;
}

/// @brief 设置RTC日期
/// @param year 年
/// @param month 月（1-12）
/// @param date 日（1-31）
/// @param week 无效参数（填0）
/// @return 0成功
int hz_rtc_set_date(u16 year, u16 month, u16 date, u16 week)
{
    rtc_datetime_t rtc_time = {0};
    rtc_get_datetime(&rtc_time);
    rtc_time.year = year;
    rtc_time.month = month;
    rtc_time.day = date;
    rtc_set_datetime(&rtc_time);
    return 0;
}

/// @brief 获取RTC时间
/// @param hour 时（输出）
/// @param min 分（输出）
/// @param sec 秒（输出）
/// @param ampm 无效参数（填NULL）
void hz_rtc_get_time(u16 *hour, u16 *min, u16 *sec, u16 *ampm)
{
    rtc_datetime_t rtc_time = {0};
    rtc_get_datetime(&rtc_time);
    *hour = rtc_time.hour;
    *min = rtc_time.minute;
    *sec = rtc_time.second;
}

/// @brief 获取RTC日期
/// @param year 年（输出）
/// @param month 月（输出）
/// @param date 日（输出）
/// @param week 无效参数（填NULL）
void hz_rtc_get_date(u16 *year, u16 *month, u16 *date, u16 *week)
{
    rtc_datetime_t rtc_time = {0};
    rtc_get_datetime(&rtc_time);
    *year = rtc_time.year;
    *month = rtc_time.month;
    *date = rtc_time.day;
    *week = 0;
}
