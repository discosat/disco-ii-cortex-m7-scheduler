#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "FreeRTOS.h"
#include "task.h"

#include "clock_config.h"
#include "pin_mux.h"
#include "board.h"

#include "network_config.h"

StaticTask_t xIdleTaskTCB;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];
StaticTask_t xTimerTaskTCB;
StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
static TaskHandle_t main_task_handle = NULL;

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

static void main_task(void *pvParameters) {
	if (setup_network()) {
		PRINTF("Network setup failed\r\n");
	};

	// for (;;)
    // {
    //     PRINTF("Configuration done - Main task launched\r\n");
    //     vTaskDelay(portMAX_DELAY);
    // }
}

int main(void)
{
    BOARD_InitMemory();

    BOARD_RdcInit();

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("\r\n__BUILT AT %s, %s__\r\n", __TIME__, __DATE__);

    // TODO: initialize RNG ?

    if (xTaskCreate(main_task, "MAIN_TASK", configMINIMAL_STACK_SIZE + 100, NULL, tskIDLE_PRIORITY + 1U, &main_task_handle) != pdPASS) {
        (void)PRINTF("\r\nFailed to create main task\r\n");
        for (;;) {}
    }

    vTaskStartScheduler();
    (void)PRINTF("Failed to start FreeRTOS.\r\n");
    for (;;) {}
}
