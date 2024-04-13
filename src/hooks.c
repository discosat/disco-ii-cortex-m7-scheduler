#pragma once
#include "tmu.h"
#include "param_config.h"
#include "sys_comm_service.h"

void hook_onehz(void) {
    // TODO: poll GNSS

    status_t tmu_status;
    int8_t temp = poll_tmu(&tmu_status);
    if (tmu_status != kStatus_Success) {
        // TODO: handle error
    } else {
        uint16_t temp_K = (uint16_t)temp + 273;
        param_set_uint16(&tmu_reading, temp_K);
    }
}

void hook_init(void) {
    tmu_init();
    sys_comm_service_init();
}
