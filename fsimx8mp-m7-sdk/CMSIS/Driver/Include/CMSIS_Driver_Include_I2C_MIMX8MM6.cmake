if(NOT CMSIS_DRIVER_INCLUDE_I2C_MIMX8MM6_INCLUDED)

    set(CMSIS_DRIVER_INCLUDE_I2C_MIMX8MM6_INCLUDED true CACHE BOOL "CMSIS_Driver_Include_I2C component is included.")


    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/.
    )

    include(CMSIS_Driver_Include_Common_MIMX8MM6)

endif()
