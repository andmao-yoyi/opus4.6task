# =============================================================================
# 源文件配置 - 自动搜索，无需手动添加
# =============================================================================

set(APP_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 自动搜索src目录下所有.c文件
aux_source_directory(${APP_ROOT_DIR}/src src)
aux_source_directory(${APP_ROOT_DIR}/src/Common Common)
aux_source_directory(${APP_ROOT_DIR}/src/Utils Utils)
aux_source_directory(${APP_ROOT_DIR}/src/Module Module)

# 应用层不再暴露 startup 源码，启动逻辑由 SDK 静态库内置
set(STARTUP_SOURCES)

# 合并所有源文件
set(APP_ALL_SOURCES ${src} ${Common} ${Utils} ${Module} ${STARTUP_SOURCES})

# 头文件搜索路径
set(APP_INCLUDE_DIRS
    ${APP_ROOT_DIR}/src
    ${APP_ROOT_DIR}/src/Common
    ${APP_ROOT_DIR}/src/Utils
    ${APP_ROOT_DIR}/src/Module
    ${APP_ROOT_DIR}/sdk/include
)
