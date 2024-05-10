#include "FreeRTOS.h"
#include "semphr.h"
#include <param/param_scheduler.h>

SemaphoreHandle_t si_lock;

void si_lock_init(void) {
	si_lock = xSemaphoreCreateMutex();
	if (si_lock == NULL) {
		// Handle error: Mutex creation failed
	}
}

void si_lock_take(void) {
	if (xSemaphoreTake(si_lock, portMAX_DELAY) != pdTRUE) {
		// Handle error: Failed to take mutex
	}
}

void si_lock_give(void) {
	if (xSemaphoreGive(si_lock) != pdTRUE) {
		// Handle error: Failed to give mutex
	}
}
