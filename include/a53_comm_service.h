#pragma once
#include "param_config.h"

void a53_service_init();
void a53_service_deinit();

int wake_a53_callback(struct param_s *param, int offset);
