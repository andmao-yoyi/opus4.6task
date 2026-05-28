cmake_minimum_required(VERSION 3.13)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_STANDARD 11)

# =============================================================================
# 项目配置
# =============================================================================
project(hpm_user_app C ASM)

set(APP_NAME demo)
add_executable(${APP_NAME}.elf)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/output)
# 设置输出文件名
set(APP_NAME demo)
set(APP_ELF_NAME ${APP_NAME}.elf)
set(APP_BIN_NAME ${APP_NAME}.bin)
set(APP_MAP_NAME ${APP_NAME}.map)
set(APP_ASM_NAME ${APP_NAME}.asm)
set(APP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "application source directory")
set(APP_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "application binary directory")

# =============================================================================
# SDK 库配置
# =============================================================================
set(VENDOR_SDK_BASE "${CMAKE_CURRENT_LIST_DIR}/sdk" CACHE PATH "prebuilt vendor_sdk package")

option(VENDOR_SDK_PULL_LATEST "Pull latest sdk/lib/*.a from Package Registry before configure" ON)
option(VENDOR_SDK_AUTO_SYNC "Auto sync sdk/lib/*.a from Package Registry before configure" ON)
if(NOT VENDOR_SDK_AUTO_SYNC)
    set(VENDOR_SDK_PULL_LATEST OFF)
endif()
set(VENDOR_SDK_SYNC_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/tool/sdk_registry/sync_sdk_from_registry.ps1" CACHE FILEPATH
    "PowerShell script used to sync prebuilt SDK libraries from package registry")
set(VENDOR_SDK_SYNC_CONFIG "${CMAKE_CURRENT_LIST_DIR}/tool/sdk_registry/registry.config.json" CACHE FILEPATH
    "Local registry config file for SDK library sync")
if(DEFINED BOARD AND NOT BOARD STREQUAL "")
    set(VENDOR_SDK_SYNC_BOARD "${BOARD}")
else()
    set(VENDOR_SDK_SYNC_BOARD "board6750_ecat")
endif()

if(VENDOR_SDK_PULL_LATEST)
    find_program(VENDOR_POWERSHELL_EXECUTABLE NAMES pwsh powershell)
    if(VENDOR_POWERSHELL_EXECUTABLE AND EXISTS "${VENDOR_SDK_SYNC_SCRIPT}")
        message(STATUS "SDK sync (configure): board=${VENDOR_SDK_SYNC_BOARD}")
        execute_process(
            COMMAND ${VENDOR_POWERSHELL_EXECUTABLE}
                    -NoProfile
                    -ExecutionPolicy Bypass
                    -File "${VENDOR_SDK_SYNC_SCRIPT}"
                    -ConfigPath "${VENDOR_SDK_SYNC_CONFIG}"
                    -SdkBase "${VENDOR_SDK_BASE}"
                    -Board "${VENDOR_SDK_SYNC_BOARD}"
            RESULT_VARIABLE VENDOR_SDK_SYNC_RESULT
        )

        if(NOT VENDOR_SDK_SYNC_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to sync SDK libraries from Package Registry. "
                                "Set VENDOR_SDK_PULL_LATEST=0 to skip.")
        endif()

        add_custom_target(vendor_sdk_sync
            COMMAND ${VENDOR_POWERSHELL_EXECUTABLE}
                    -NoProfile
                    -ExecutionPolicy Bypass
                    -File "${VENDOR_SDK_SYNC_SCRIPT}"
                    -ConfigPath "${VENDOR_SDK_SYNC_CONFIG}"
                    -SdkBase "${VENDOR_SDK_BASE}"
                    -Board "${VENDOR_SDK_SYNC_BOARD}"
            COMMENT "Sync SDK libraries from Package Registry"
            USES_TERMINAL
            VERBATIM
        )
        add_dependencies(${APP_NAME}.elf vendor_sdk_sync)
    else()
        message(STATUS "SDK sync skipped. Missing PowerShell/script. "
                       "Expected script: ${VENDOR_SDK_SYNC_SCRIPT}")
    endif()
endif()

# 方式一：通过 vendor-sdk-config.cmake 统一接入（推荐）
include(${VENDOR_SDK_BASE}/cmake/vendor-sdk-config.cmake)

# 方式二：直接导入预编译库（可选）
# set(VENDOR_SDK_LIB "${VENDOR_SDK_BASE}/lib/libvendor_sdk.a")
# if(NOT EXISTS ${VENDOR_SDK_LIB})
#     message(FATAL_ERROR "libvendor_sdk.a not found: ${VENDOR_SDK_LIB}")
# endif()
# add_library(vendor_sdk STATIC IMPORTED)
# set_target_properties(vendor_sdk PROPERTIES IMPORTED_LOCATION ${VENDOR_SDK_LIB})
# target_include_directories(vendor_sdk INTERFACE ${VENDOR_SDK_BASE}/include)

# =============================================================================
# 源文件配置
# =============================================================================
include(${CMAKE_CURRENT_LIST_DIR}/cmake/app_sources.cmake)

target_sources(${APP_NAME}.elf PRIVATE ${APP_ALL_SOURCES})

target_include_directories(${APP_NAME}.elf PRIVATE ${APP_INCLUDE_DIRS})

# =============================================================================
# 编译配置
# =============================================================================
string(TIMESTAMP BUILD_TIMESTAMP "%s")

# 这里显式保留应用层自己的时间戳与启动模式宏，和源码仓库当前入口保持一致。
target_compile_definitions(${APP_NAME}.elf PRIVATE
    -DCONFIG_AXIS_ACCEPT_TEST=0
    -DFLASH_XIP=1
    -DTIMESTAMP=${BUILD_TIMESTAMP})

target_compile_options(${APP_NAME}.elf PRIVATE
    -mabi=ilp32f
    # 应用层与打包库统一声明 zicsr，避免两边对 CSR 指令集的假设不一致。
    -march=rv32imafc_zicsr_zifencei)

# =============================================================================
# 链接配置
# =============================================================================
# 使用 --whole-archive 确保 SDK 库中所有 .o 都被链接进来
# 包括 c_start.c.obj 中的强符号 c_startup（覆盖 reset.c.obj 中的弱符号版本）
target_link_libraries(${APP_NAME}.elf PRIVATE
    "-specs=nano.specs"
    "-u _printf_float"
    "-static"
    "-nostartfiles"
    "-Wl,--gc-sections"
    "-Wl,--print-memory-usage"
    "-Wl,-Map=${EXECUTABLE_OUTPUT_PATH}/${APP_MAP_NAME}"
    "-Wl,--whole-archive"
    # 保持和源码仓库一致的顺序，让数学库与预编译 SDK 一起处于整库保留区间。
    "-lm"
    vendor_sdk::vendor_sdk
    "-Wl,--no-whole-archive"
)

# 根据 SOC 型号选择链接脚本
set(SOC_LD_SUFFIX ${SOC})
if(${SOC} STREQUAL "6e00")
    set(SOC_LD_SUFFIX "6e80")
endif()
set(LINK_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/linker/hpm${SOC_LD_SUFFIX}_xip_boot.ld)
message(LINK_SCRIPT: ${SOC_LD_SUFFIX}_xip_boot.ld)
if(NOT EXISTS ${LINK_SCRIPT})
    message(FATAL_ERROR "Linker script not found for SOC=${SOC}: ${LINK_SCRIPT}")
endif()

# 预编译包模式下，这些链接脚本符号需要由应用工程显式补齐，才能和源码仓库保持一致。
if(NOT DEFINED APP_STACK_SIZE)
    set(APP_STACK_SIZE 16K CACHE STRING "应用主栈大小（传递给链接脚本 _stack_size）")
endif()
if(NOT DEFINED APP_HEAP_SIZE)
    set(APP_HEAP_SIZE 8K CACHE STRING "应用堆大小（传递给链接脚本 _heap_size）")
endif()
if(NOT DEFINED APP_FLASH_SIZE)
    if(SOC STREQUAL "6750")
        set(APP_FLASH_SIZE 2M CACHE STRING "应用可用 Flash 大小（传递给链接脚本 _flash_size）")
    else()
        set(APP_FLASH_SIZE 1M CACHE STRING "应用可用 Flash 大小（传递给链接脚本 _flash_size）")
    endif()
endif()

target_link_options(${APP_NAME}.elf PRIVATE
    -mabi=ilp32f
    # 链接阶段同样使用与源码仓库一致的 ISA 声明，避免编译与链接参数脱节。
    -march=rv32imafc_zicsr_zifencei
    -Wl,--no-warn-rwx-segments
    -Wl,--defsym=_stack_size=${APP_STACK_SIZE}
    -Wl,--defsym=_heap_size=${APP_HEAP_SIZE}
    -Wl,--defsym=_flash_size=${APP_FLASH_SIZE}
    -Wl,-T,${LINK_SCRIPT})

# 生成bin文件和asm文件
set(APP_DEBUG 0)
message(STATUS "LINK_SCRIPT: ${LINK_SCRIPT}")
target_compile_definitions(${APP_ELF_NAME} PUBLIC -DTIMESTAMP=${BUILD_TIMESTAMP})
message(STATUS "LINKER_SYMBOLS: _stack_size=${APP_STACK_SIZE}, _heap_size=${APP_HEAP_SIZE}, _flash_size=${APP_FLASH_SIZE}")

message(STATUS "CMAKE_OBJCOPY: ${CMAKE_OBJCOPY}")
message(STATUS "CMAKE_OBJDUMP: ${CMAKE_OBJDUMP}")
add_custom_command(
    # TARGET ${APP_ELF_NAME}
    OUTPUT ${EXECUTABLE_OUTPUT_PATH}/${APP_BIN_NAME}
    COMMAND "${CMAKE_OBJCOPY}" -O binary -S ${EXECUTABLE_OUTPUT_PATH}/${APP_ELF_NAME} ${EXECUTABLE_OUTPUT_PATH}/${APP_BIN_NAME}
    DEPENDS ${APP_ELF_NAME}
)

add_custom_command(
    # TARGET ${APP_ELF_NAME}
    OUTPUT ${EXECUTABLE_OUTPUT_PATH}/${APP_ASM_NAME}
    COMMAND "${CMAKE_OBJDUMP}" -S -d ${EXECUTABLE_OUTPUT_PATH}/${APP_ELF_NAME} > ${EXECUTABLE_OUTPUT_PATH}/${APP_ASM_NAME}
    DEPENDS ${APP_ELF_NAME}
)

# 确定环境变量中的gdb是否存在
message(STATUS "GNURISCV_TOOLCHAIN_PATH: $ENV{GNURISCV_TOOLCHAIN_PATH}")


# 生成OTA文件
if(APP_DEBUG EQUAL 0)
    # 查找Python3解释器，但不是必需的（即不使用REQUIRED关键字）
    find_package(Python3 COMPONENTS Interpreter)
    # 定义Python脚本路径
    set(ADD_HEADER_SCRIPT ${CMAKE_SOURCE_DIR}/tool/ota_pack_tool/pack_ota.py)
    set(OTA_BIN_NAME ${BOARD}.bin)
    set(USB_DEV_OTA_BIN_NAME update_sign_usb.bin)

    # 确保Python脚本存在
    if(NOT EXISTS ${ADD_HEADER_SCRIPT})
        message(FATAL_ERROR "Python script for adding header not found: ${ADD_HEADER_SCRIPT}")
    endif()

    # 添加自定义命令以在生成bin文件后添加头部信息
    add_custom_command(
        OUTPUT ${EXECUTABLE_OUTPUT_PATH}/${OTA_BIN_NAME}
        COMMAND ${Python3_EXECUTABLE} ${ADD_HEADER_SCRIPT} 4 ${EXECUTABLE_OUTPUT_PATH}/${APP_BIN_NAME} ${EXECUTABLE_OUTPUT_PATH}/${OTA_BIN_NAME} ${EXECUTABLE_OUTPUT_PATH}/${USB_DEV_OTA_BIN_NAME}
        DEPENDS ${EXECUTABLE_OUTPUT_PATH}/${APP_BIN_NAME} ${ADD_HEADER_SCRIPT}
        COMMENT "Adding header to binary file"
        VERBATIM
    )

    # 创建一个自定义目标来确保添加头部信息的操作被执行
    add_custom_target(
        AddHeaderToBin ALL
        DEPENDS ${EXECUTABLE_OUTPUT_PATH}/${OTA_BIN_NAME}
    )
endif()

