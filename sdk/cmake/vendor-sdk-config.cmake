# vendor_sdk integration entry. Prefer prebuilt libraries; source mode remains optional.
if(NOT VENDOR_SDK_BASE)
    if(DEFINED ENV{VENDOR_SDK_BASE})
        set(VENDOR_SDK_BASE $ENV{VENDOR_SDK_BASE})
    else()
        message(FATAL_ERROR "VENDOR_SDK_BASE is not set")
    endif()
endif()

if(NOT DEFINED BOARD)
    set(BOARD "board6750_ecat" CACHE STRING "Board model: board6750_ecat or board6e_ecat_cfm")
endif()

if(BOARD MATCHES "board6750_")
    set(SOC "6750" CACHE STRING "SoC derived from BOARD")
elseif(BOARD MATCHES "board6e_")
    set(SOC "6e00" CACHE STRING "SoC derived from BOARD")
endif()

set(_vendor_prebuilt_lib "${VENDOR_SDK_BASE}/lib/libvendor_sdk_${BOARD}.a")

if(NOT DEFINED VENDOR_SDK_SOURCE_MODE)
    set(VENDOR_SDK_SOURCE_MODE OFF)
endif()

if(EXISTS "${_vendor_prebuilt_lib}" AND NOT VENDOR_SDK_SOURCE_MODE)
    if(NOT TARGET vendor_sdk)
        add_library(vendor_sdk STATIC IMPORTED)
        set_target_properties(vendor_sdk PROPERTIES IMPORTED_LOCATION "${_vendor_prebuilt_lib}")
        target_include_directories(vendor_sdk INTERFACE "${VENDOR_SDK_BASE}/include")

        target_link_options(vendor_sdk INTERFACE "-Wl,-u,_start")
        target_compile_options(vendor_sdk INTERFACE
            $<$<CONFIG:Debug>:-Og>
            $<$<CONFIG:Release>:-O3>
            $<$<CONFIG:Release>:-funroll-all-loops>
            $<$<CONFIG:RelWithDebInfo>:-O2>
            $<$<CONFIG:RelWithDebInfo>:-funroll-all-loops>
            $<$<CONFIG:MinSizeRel>:-Os>
            -Wall
            -Wundef
            -Wno-format
            -Wno-maybe-uninitialized
            -fomit-frame-pointer
            -fno-builtin
            -ffunction-sections
            -fdata-sections)
    endif()
else()
    if(NOT EXISTS "${VENDOR_SDK_BASE}/CMakeLists.txt")
        message(FATAL_ERROR "vendor_sdk source not found: ${VENDOR_SDK_BASE}")
    endif()
    add_subdirectory("${VENDOR_SDK_BASE}" "${CMAKE_BINARY_DIR}/vendor_sdk_build")
endif()

if(TARGET vendor_sdk)
    target_link_options(vendor_sdk INTERFACE "-Wl,-u,_start")

    set(_vendor_public_compile_defs
        -DCONFIG_FREERTOS=1
        -D__ENABLE_FREERTOS=1
        -D__freertos_irq_stack_top=_stack
        -DUSE_SYSCALL_INTERRUPT_PRIORITY=0
        -DUSE_NONVECTOR_MODE=0
        -DDISABLE_IRQ_PREEMPTIVE=0
        -DCONFIG_DISABLE_GLOBAL_IRQ_ON_STARTUP=1
        -DLWIP_SUPPORT_CUSTOM_PBUF=1
        -DLWIP_DHCP=0
        -DLWIP_TIMEVAL_PRIVATE=0
        -DAGILE_MODBUS_USING_RTU
        -DAGILE_MODBUS_USING_TCP
        -DMODBUS_TCP_SLAVE
        -DUSB_FATFS_ENABLE=1
        -DRMII=1
        -D__USE_DP83848=1
        -D__DISABLE_AUTO_NEGO=0
        -D__ENABLE_ENET_RECEIVE_INTERRUPT=1
        -DportasmHAS_MTIME=1)

    if(BOARD STREQUAL "board6750_ecat" OR BOARD STREQUAL "board6e_ecat_cfm")
        list(APPEND _vendor_public_compile_defs -DCONFIG_ECAT=1)
    else()
        list(APPEND _vendor_public_compile_defs -DCONFIG_ECAT=0)
    endif()
    target_compile_definitions(vendor_sdk INTERFACE ${_vendor_public_compile_defs})

    if(BOARD MATCHES "board6750_" OR BOARD MATCHES "board6e_")
        target_link_options(vendor_sdk INTERFACE "-Wl,--defsym=_noncacheable_size=256K")
    endif()
endif()

if(NOT TARGET vendor_sdk::vendor_sdk)
    add_library(vendor_sdk::vendor_sdk ALIAS vendor_sdk)
endif()
