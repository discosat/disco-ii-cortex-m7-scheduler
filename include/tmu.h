#pragma once
#include "fsl_tmu.h"

void tmu_init();
void tmu_deinit();

int8_t poll_tmu(status_t * status);
