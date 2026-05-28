#ifndef __ALARM_H__
#define __ALARM_H__

#include "vendor_common.h"
#include "hz_alarm.h"

//===================函数声明  ==============
void alarm_check(void);
void alarm_handle(void);

//=================== 报警定义 ==============
// 3级报警
#define ALM3_ShouldRst      3, 0, 0 // 系统需复位 406.00
#define ALM3_AxisErr(n)     3, 0, n // 轴n错误
//复位超时
#define ALM3_RESET_E(a)     3, 2, a //410.0a 轴找零点超时 
#define ALM3_AxisDisable    3, 5, 0 // 412.00 轴断使能   
// 4级报警
#define ALM4_Reboot         4, 0, 0 // 系统需重启 414.00
#define ALM4_AxisErr(n)     4, 0, n // 轴n错误
// 5级报警
#define ALM5_DataErr        5, 0, 0 // 数据错误 422.00
#endif
