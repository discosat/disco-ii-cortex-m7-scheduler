#include "tmu.h"

static tmu_config_t config;

void tmu_init() {
	TMU_GetDefaultConfig(&config);
	config.averageLPF = kTMU_AverageLowPassFilter0_5;
	config.probeSelect = kTMU_ProbeSelectMainProbe;  // TODO: also add second probe near A53? Need to check both probes can be active seperately. kTMU_ProbeSelectRemoteProbe

	TMU_Init(TMU, &config);
}

void tmu_deinit() {
	TMU_Deinit(TMU);
}

int8_t poll_tmu(status_t * status) {
	int8_t temp;

	*status = TMU_GetAverageTemperature(TMU, config.probeSelect, &temp);
	if (kStatus_Success == *status) {
		if (temp < -40 || temp > 125) {
			*status = kStatus_OutOfRange;
		} else {
			return temp;
		}
	}
	return temp;
}
