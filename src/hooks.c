#pragma once
#include "tmu.h"

void hook_onehz(void) {
    // TODO: poll GNSS and TMU
}

void hook_init(void) {
    tmu_init();
}
