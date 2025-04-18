include_guard()
message("component_serial_manager_uart component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/fsl_component_serial_port_uart.c
)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)


#OR Logic component
if(CONFIG_USE_driver_iuart_MIMX8ML8)
     include(driver_iuart_MIMX8ML8)
endif()
if(NOT (CONFIG_USE_driver_iuart_MIMX8ML8))
    message(WARNING "Since driver_iuart_MIMX8ML8 is not included at first or config in config.cmake file, use driver_uart_MIMX8ML8 by default.")
    include(driver_uart_MIMX8ML8)
endif()

include(component_serial_manager_MIMX8ML8)

