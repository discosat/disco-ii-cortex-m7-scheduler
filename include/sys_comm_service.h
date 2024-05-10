#pragma once
#include "param_config.h"

void sys_comm_service_init();
void sys_comm_service_deinit();

int wake_a53_callback(struct param_s * param, int offset);
