# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.10.0)

# THE VERSION NUMBER
SET (VERSION_MAJOR 1)
SET (VERSION_MINOR 0)

# ENABLE ASM
ENABLE_LANGUAGE(ASM)

SET(CMAKE_STATIC_LIBRARY_PREFIX)
SET(CMAKE_STATIC_LIBRARY_SUFFIX)

SET(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
SET(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

SET(RepoRoot /disco-ii-cortex-m7-scheduler)
SET(ProjDirPath ${RepoRoot}/src/armgcc)
SET(BaseDirPath ${RepoRoot}/fsimx8mp-m7-sdk)

#SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})

project(disco_scheduler)

set(MCUX_SDK_PROJECT_NAME disco_scheduler.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

set(SchedulerSources
    ${RepoRoot}/src/main.c
    ${RepoRoot}/src/FreeRTOSConfig.h
    ${RepoRoot}/src/pin_mux.c
    ${RepoRoot}/src/pin_mux.h
    ${RepoRoot}/src/lpm.c
    ${RepoRoot}/src/board.c
    ${RepoRoot}/src/clock_config.c
    ${RepoRoot}/src/fsl_iomuxc.h
    ${RepoRoot}/src/gnss.c
    ${RepoRoot}/src/hooks.c
    ${RepoRoot}/src/network_config.c
    ${RepoRoot}/src/param_config.c
    ${RepoRoot}/src/serial.c
    ${RepoRoot}/src/si_lock.c
    ${RepoRoot}/src/vmem_config.c
    ${RepoRoot}/src/can_iface.c
    ${RepoRoot}/src/csp_hook_fsl_mx8mp_m7.c
    ${RepoRoot}/src/tinygps.c
    ${RepoRoot}/src/tmu.c
    ${RepoRoot}/src/sys_comm_service.c
)

set(CSPLibPath ${RepoRoot}/lib/csp)
set(CSP_QFIFO_LEN 15 CACHE STRING "Length of incoming queue for router task")
set(CSP_PORT_MAX_BIND 16 CACHE STRING "Length of incoming queue for router task")
set(CSP_CONN_RXQUEUE_LEN 16 CACHE STRING "Number of packets in connection queue")
set(CSP_CONN_MAX 8 CACHE STRING "Number of new connections on socket queue")
set(CSP_BUFFER_SIZE 256 CACHE STRING "Bytes in each packet buffer")
set(CSP_BUFFER_COUNT 15 CACHE STRING "Number of total packet buffers")
set(CSP_RDP_MAX_WINDOW 5 CACHE STRING "Max window size for RDP")
set(CSP_RTABLE_SIZE 10 CACHE STRING "Number of elements in routing table")

option(CSP_HAVE_STDIO "OS provides C Standard I/O functions" ON)
option(CSP_ENABLE_CSP_PRINT "Enable csp_print() function" ON)
option(CSP_PRINT_STDIO "Use vprintf() for csp_print() function" ON)
option(CSP_USE_RDP "Reliable Datagram Protocol" ON)
option(CSP_USE_HMAC "Hash-based message authentication code" ON)
option(CSP_USE_PROMISC "Promiscious mode" ON)
option(CSP_USE_DEDUP "Packet deduplication" ON)
option(CSP_USE_RTABLE "Use routing table" OFF)

configure_file(${CSPLibPath}/csp_autoconfig.h.in ${CSPLibPath}/include/csp/autoconfig.h)
set(CSPLibSources
    ${CSPLibPath}/src/csp_bridge.c
    ${CSPLibPath}/src/csp_buffer.c
    ${CSPLibPath}/src/csp_conn.c
    ${CSPLibPath}/src/csp_crc32.c
    ${CSPLibPath}/src/csp_debug.c
    ${CSPLibPath}/src/csp_dedup.c
    ${CSPLibPath}/src/csp_hex_dump.c
    ${CSPLibPath}/src/csp_id.c
    ${CSPLibPath}/src/csp_iflist.c
    ${CSPLibPath}/src/csp_init.c
    ${CSPLibPath}/src/csp_io.c
    ${CSPLibPath}/src/csp_port.c
    ${CSPLibPath}/src/csp_promisc.c
    ${CSPLibPath}/src/csp_qfifo.c
    ${CSPLibPath}/src/csp_route.c
    ${CSPLibPath}/src/csp_rtable_cidr.c
    ${CSPLibPath}/src/csp_rtable_stdio.c
    ${CSPLibPath}/src/csp_service_handler.c
    ${CSPLibPath}/src/csp_services.c
    ${CSPLibPath}/src/csp_rdp.c
    ${CSPLibPath}/src/csp_rdp_queue.c

    ${CSPLibPath}/src/arch/freertos/csp_queue.c
    ${CSPLibPath}/src/arch/freertos/csp_clock.c
    ${CSPLibPath}/src/arch/freertos/csp_semaphore.c
    ${CSPLibPath}/src/arch/freertos/csp_system.c
    ${CSPLibPath}/src/arch/freertos/csp_time.c

    ${CSPLibPath}/src/crypto/csp_hmac.c
    ${CSPLibPath}/src/crypto/csp_sha1.c

    ${CSPLibPath}/src/drivers/usart/usart_kiss.c

    ${CSPLibPath}/src/interfaces/csp_if_lo.c
    ${CSPLibPath}/src/interfaces/csp_if_kiss.c
    ${CSPLibPath}/src/interfaces/csp_if_i2c.c
    ${CSPLibPath}/src/interfaces/csp_if_tun.c
    ${CSPLibPath}/src/interfaces/csp_if_can.c
    ${CSPLibPath}/src/interfaces/csp_if_can_pbuf.c
)

set(ParamLibPath ${RepoRoot}/lib/param)
set(PARAM_STORE_FILE OFF CACHE BOOL "Store parameters in file")
set(PARAM_STORE_VMEM ON CACHE BOOL "Store parameters in virtual memory")
set(PARAM_STORE_LOG OFF CACHE BOOL "Log parameter store operations")
set(PARAM_HAVE_SYS_QUEUE OFF CACHE BOOL "Have system queue")
set(PARAM_LIST_DYNAMIC ON CACHE BOOL "List dynamic parameters")
set(PARAM_LIST_POOL OFF CACHE BOOL "List pool parameters")
set(PARAM_HAVE_SCHEDULER ON CACHE BOOL "Have scheduler")
set(PARAM_HAVE_COMMANDS ON CACHE BOOL "Have commands")
set(MPACK_FLOAT 1 CACHE STRING "MPACK float support")
set(MPACK_STDIO 0 CACHE STRING "MPACK stdio support")

file(WRITE ${ParamLibPath}/include/libparam.h
"#ifndef LIBPARAM_H
#define LIBPARAM_H

#define PARAM_STORE_FILE ${PARAM_STORE_FILE}
#define PARAM_STORE_VMEM ${PARAM_STORE_VMEM}
#define PARAM_STORE_LOG ${PARAM_STORE_LOG}
#define PARAM_HAVE_SYS_QUEUE ${PARAM_HAVE_SYS_QUEUE}
#define PARAM_LIST_DYNAMIC ${PARAM_LIST_DYNAMIC}
#define PARAM_LIST_POOL ${PARAM_LIST_POOL}
#define PARAM_HAVE_SCHEDULER ${PARAM_HAVE_SCHEDULER}
#define PARAM_HAVE_COMMANDS ${PARAM_HAVE_COMMANDS}
#define MPACK_FLOAT ${MPACK_FLOAT}
#define MPACK_STDIO ${MPACK_STDIO}

#endif // LIBPARAM_H
")

set(ParamLibSources
    ${ParamLibPath}/src/param/list/param_list.c
    ${ParamLibPath}/src/param/param_client.c
    ${ParamLibPath}/src/param/param_serializer.c
    ${ParamLibPath}/src/param/param_server.c
    ${ParamLibPath}/src/param/param_string.c
    ${ParamLibPath}/src/param/param_queue.c
    ${ParamLibPath}/src/param/param_wildcard.c
    ${ParamLibPath}/src/param/param.c
    ${ParamLibPath}/src/mpack/mpack.c
    ${ParamLibPath}/src/vmem/vmem_client.c
    ${ParamLibPath}/src/vmem/vmem_ram.c
    ${ParamLibPath}/src/vmem/vmem_crc32.c
    ${ParamLibPath}/src/vmem/vmem_server.c
    ${ParamLibPath}/src/vmem/vmem.c
    ${ParamLibPath}/src/objstore/objstore.c
)

if(PARAM_HAVE_SCHEDULER)
    list(APPEND ParamLibSources
        ${ParamLibPath}/src/param/scheduler/param_scheduler.c
    )
endif()

if(PARAM_HAVE_COMMANDS)
    list(APPEND ParamLibSources
        ${ParamLibPath}/src/param/commands/param_commands.c
    )
endif()

set(CSPProcLibPath ${RepoRoot}/lib/csp_proc)
set(CSPProcLibSources
    ${CSPProcLibPath}/src/proc_pack.c
    ${CSPProcLibPath}/src/proc_server.c
    ${CSPProcLibPath}/src/store/proc_store_static.c
    ${CSPProcLibPath}/src/platform/proc_memory_FreeRTOS.c
    ${CSPProcLibPath}/src/platform/proc_mutex_FreeRTOS.c
    ${CSPProcLibPath}/src/runtime/proc_runtime_instructions_common.c
    ${CSPProcLibPath}/src/runtime/proc_runtime_instructions_FreeRTOS.c
    ${CSPProcLibPath}/src/runtime/proc_runtime_FreeRTOS.c
    ${CSPProcLibPath}/src/proc_analyze.c
)

add_executable(${MCUX_SDK_PROJECT_NAME} 
${SchedulerSources}
${CSPLibSources}
${ParamLibSources}
${CSPProcLibSources}
)
target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PRIVATE CSP_FREERTOS=1)
target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PRIVATE MAX_INSTRUCTIONS=50)
target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PRIVATE MAX_PROC_SLOT=50)
target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PRIVATE RESERVED_PROC_SLOTS=25)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${RepoRoot}/include
    ${RepoRoot}/src
    ${CSPLibPath}/include
    ${CSPLibPath}/src
    ${ParamLibPath}/include
    ${ParamLibPath}/src
    ${CSPProcLibPath}/include
    /usr/local/arm/gcc-arm-none-eabi-10.3-2021.10/arm-none-eabi/include/machine
)
set(CMAKE_MODULE_PATH
    ${BaseDirPath}/middleware/multicore
    ${BaseDirPath}/rtos/freertos
    ${BaseDirPath}/devices/MIMX8ML8/drivers
    ${BaseDirPath}/devices/MIMX8ML8
    ${BaseDirPath}/devices/MIMX8ML8/utilities
    ${BaseDirPath}/components/uart
    ${BaseDirPath}/components/serial_manager
    ${BaseDirPath}/components/lists
    ${BaseDirPath}/CMSIS/Core/Include
)

# include modules
include(middleware_freertos-kernel_heap_4_MIMX8ML8)

include(driver_clock_MIMX8ML8)

include(driver_flexcan_MIMX8ML8)

include(driver_tmu_2_MIMX8ML8)

include(driver_mu_MIMX8ML8)

include(middleware_freertos-kernel_MIMX8ML8)

include(driver_common_MIMX8ML8)

include(driver_rdc_MIMX8ML8)

include(driver_gpt_MIMX8ML8)

include(driver_audiomix_MIMX8ML8)

include(device_MIMX8ML8_CMSIS_MIMX8ML8)

include(utility_debug_console_MIMX8ML8)

include(component_iuart_adapter_MIMX8ML8)

include(component_serial_manager_MIMX8ML8)

include(component_lists_MIMX8ML8)

include(component_serial_manager_uart_MIMX8ML8)

include(driver_iuart_MIMX8ML8)

include(device_MIMX8ML8_startup_MIMX8ML8)

include(utility_assert_MIMX8ML8)

include(middleware_freertos-kernel_extension_MIMX8ML8)

include(CMSIS_Include_core_cm_MIMX8ML8)

include(utilities_misc_utilities_MIMX8ML8)

include(device_MIMX8ML8_system_MIMX8ML8)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/disco_scheduler.bin)

