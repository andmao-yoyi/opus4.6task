#ifdef TIMESTAMP
#undef TIMESTAMP
#endif

#include "logic.h"
#include "vendor_common.h"
#include "vendor_sdk.h"
#include "bsp_config.h"
#include "hz_fsm.h"
#include "modbus_utils.h"

//=============================================================================
// 定义
//=============================================================================
const board_resource_t *pBoardResource = NULL;

extern void app_init(void);
void        app_loop(void *);
// 初始化函数
const board_resource_t *task_init(void *pvPara)
{
    bsp_user_config_t *cfg = (bsp_user_config_t *)pvPara;
    // 读取配置文件 确定数据是否有写入过
    GUR.HaveToSetData = fram_read_block(0, cfg->fram_data, cfg->fram_size);
    //  初始化任务
    bsp_task_create(pvPara);
    modbus_task_create(pvPara);
    can_task_create(pvPara);
    return fpga_task_create(pvPara);
}

//=============================================================================
// 函数：main_task
// user app主程序
//=============================================================================
void main_task(void *pvPara)
{
    bsp_user_config_size_check(sizeof(bsp_user_config_t));
    pBoardResource = task_init(pvPara);
    app_loop(pvPara);
}

/// @brief 应用程序开始执行
/// @param
void app_loop(void *pvPara)
{
    app_init();
    while (1)
    {
#if CONFIG_AXIS_ACCEPT_TEST
        // 验收模式下只保留状态刷新，避免业务逻辑反复覆盖测试参数。
        system_data_update();
        sys_delay_ms(20);
#else
        //逻辑运行
        logic();
        //更新系统变量
        system_data_update();
#endif
    }
}


void cfm_save_restore(void *cfm_save_ptr, uint32_t len)
{
    if (cfm_save_ptr && len > 0)
    {
        #if (defined(CFG_CFM_EN) && CFG_CFM_EN == 1)
        memcpy(cfm_save_ptr, GlobalData.Save.User.Data.cfm_reserved, len);
        #else
        debug_printf("CFM is not enabled!\n");
        #endif
    }
}

void cfm_save_persist(void *cfm_save_ptr, uint32_t len)
{
    if (cfm_save_ptr && len > 0)
    {
        #if (defined(CFG_CFM_EN) && CFG_CFM_EN == 1)
        memcpy(GlobalData.Save.User.Data.cfm_reserved, cfm_save_ptr, len);
        #else
        debug_printf("CFM is not enabled!\n");
        #endif
    }
}
