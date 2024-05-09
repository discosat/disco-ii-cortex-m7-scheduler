#pragma once

#include "gnss.h"
#include "tmu.h"
#include "param_config.h"
#include "sys_comm_service.h"
#include "clock_config.h"
#include "can_iface.h"

static int a53_enter_suspend_flag = 1;

void hook_onehz(void) {
    // GNSS
    int ret = gnss_poll();
    if (ret != 0) {
        // TODO: handle error
    }

    // TMU
    status_t tmu_status;
    int8_t temp = poll_tmu(&tmu_status);
    if (tmu_status != kStatus_Success) {
        // TODO: handle error
    } else {
        uint16_t temp_K = (uint16_t)temp + 273;
        param_set_uint16(&tmu_reading, temp_K);
    }

    // Power / clocks management
    param_set_uint8(&a53_status, (GPC->SLPCR & GPC_SLPCR_EN_DSM_MASK) >> GPC_SLPCR_EN_DSM_SHIFT ? 0 : 1);
    if (param_get_uint8(&a53_status) == 0) {
        if (a53_enter_suspend_flag) {
            BOARD_A53WFIClockReenable();
            iface_can_init();
            a53_enter_suspend_flag = 0;
        }
    } else {
        a53_enter_suspend_flag = 1;
    }

    // System registers that shouldn't be accessed when A53 is in WFI mode
    if (param_get_uint8(&a53_status) == 1) {
        _GPIO1__DR = GPIO1->DR;
        _GPIO1__GDIR = GPIO1->GDIR;
        _GPIO2__DR = GPIO2->DR;
        _GPIO2__GDIR = GPIO2->GDIR;
        _GPIO3__GDIR = GPIO3->GDIR;
        _GPIO4__GDIR = GPIO4->GDIR;
        _GPIO5__GDIR = GPIO5->GDIR;
        _MUB__SR = MUB->SR;
        _RDC__STAT = RDC->STAT;
        _TMU__TSCR = TMU->TSCR;
        _TMU__TRITSR = TMU->TRITSR;
        _TMU__TRATSR = TMU->TRATSR;
    }
}

void hook_init(void) {
    gnss_init();
    tmu_init();
    sys_comm_service_init();
}
