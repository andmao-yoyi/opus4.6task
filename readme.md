# HPM_USER_PROJECT

## 工程目录说明

| 文件夹 | 说明 |
|--------|------|
| `src/` | 应用层源代码 |
| `src/Common/` | 公共模块 |
| `src/Module/` | 功能模块 |
| `src/Utils/` | 工具函数 |
| `sdk/` | SDK预编译库和头文件 |
| `linker/` | 链接脚本 |
| `openocd/` | 调试配置文件 |
| `tool/` | OTA打包工具 |
| `cmake/` | CMake配置 |

---

## src/Common/ - 公共模块

| 文件 | 说明 |
|------|------|
| `hpm_appheader.c/h` | 固件头结构体定义（OTA升级用） |
| `logic.c/h` | 逻辑控制核心 |
| `alarm.c/h` | 报警处理 |
| `teach.c/h` | 点位示教功能 |
| `data_define.h` | 数据结构定义 |
| `app_init.c` | 应用初始化 |

---

## src/Module/ - 功能模块

| 文件 | 说明 |
|------|------|
| `terminal_tool.c/h` | 串口终端命令工具 |
| `reset_axis.c/h` | 轴复位功能 |
| `dev_reset.c` | 设备复位 |

---

## src/Utils/ - 工具函数

| 文件 | 说明 |
|------|------|
| `Jog.c/h` | JOG运动控制 |
| `lock.c/h` | 互斥锁 |
| `hz_fsm.c/h` | 有限状态机 |
| `modbus_utils.c/h` | Modbus通信工具 |
| `queue_tool.c/h` | 队列工具 |
| `uph.c/h` | 采样/滤波 |
| `utils.c/h` | 通用工具函数 |

---

## sdk/ - SDK库

| 文件夹 | 说明 |
|--------|------|
| `lib/` | 预编译SDK库（按芯片型号） |
| `include/` | SDK头文件 |
| `cmake/` | CMake集成配置 |

---

## tool/ - 工具脚本

| 文件 | 说明 |
|------|------|
| `ota_pack_tool/` | OTA固件打包工具 |
| `ota_pack_tool/pack_ota.py` | 打包脚本（含国密SM2/SM3/SM4） |

---

## 支持的芯片型号

| BOARD | SOC | 链接脚本 |
|-------|-----|----------|
| `board6750_ecat` | 6750 | `hpm6750_xip_boot.ld` |
| `board6e_ecat_cfm` | 6e00 | `hpm6e80_xip_boot.ld` |

---

## 中断优先级

| 中断源 | 优先级 |
|--------|--------|
| IRQn_GPIO0_B | 3 |
| IRQn_GPTMR4 | 1 |
| IRQn_UART1/UART3 | 1 |
| IRQn_ENET1 | 1 |
| IRQn_USB0 | 1 |

---

## 固件头信息

固件二进制文件包含92字节的固件头（`hpm_app_header_t`），位于文件起始位置：

| 偏移 | 大小 | 字段 | 说明 |
|------|------|------|------|
| 0x00 | 4B | magic | 固件标识 (0xBEAF5AA5) |
| 0x04 | 4B | touchid | ID信息 |
| 0x08 | 4B | device | 设备信息 |
| 0x0C | 4B | len | 固件长度（运行时填写） |
| 0x10 | 4B | version | 编译时间戳 |
| 0x14 | 1B | type | 升级类型 |
| 0x15 | 1B | hash_enable | Hash校验开关 |
| 0x16 | 1B | pwr_hash | 启动校验开关 |
| 0x17 | 1B | hash_type | Hash算法类型 |
| 0x18 | 4B | reserved | 保留 |
| 0x1C | 64B | hash_data | Hash值 |


## 如何针对不同型号芯片切换工程

首先进入CMake扩展，在项目状态-->配置-->设置变量，点击铅笔符号（选择变量）根据芯片型号选择对应的变量，点击应用即可。
