#pragma once
#include "fsl_debug_console.h"

void wake_a53_service_init() {}

void wake_a53_callback(struct param_s *param, int offset) {
    PRINTF("DEBUG_wake_a53_callback\n");
}
