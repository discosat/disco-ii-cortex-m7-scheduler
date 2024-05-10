#include "fsl_mu.h"

#include "board.h"
#include "sys_comm_service.h"

#include <csp/csp.h>

void sys_comm_service_init() {
	MU_Init(MUB);
}

void sys_comm_service_deinit() {
	MU_Deinit(MUB);
}

int wake_a53_callback(struct param_s * param, int offset) {
	MU_TriggerInterrupts(MUB, kMU_GenInt0InterruptTrigger);

	return CSP_ERR_NONE;
}
