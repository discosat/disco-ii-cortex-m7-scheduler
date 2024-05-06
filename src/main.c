#include "fsl_device_registers.h"
#include "fsl_debug_console.h"

#include "FreeRTOS.h"
#include "task.h"

#include <csp/csp.h>
#include <csp_proc/proc_server.h>

#include "clock_config.h"
#include "pin_mux.h"
#include "board.h"

#include "network_config.h"

StaticTask_t xIdleTaskTCB;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];
StaticTask_t xTimerTaskTCB;
StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

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

int main(void)
{
    BOARD_InitMemory();

    BOARD_RdcInit();

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_PeripheralRdcSetting();

    BOARD_InitDebugConsole();

    PRINTF("\r\n__BUILT AT %s, %s__\r\n", __TIME__, __DATE__);

    setup_network();

    proc_server_init();
    csp_bind_callback(proc_serve, PROC_PORT_SERVER);

    vTaskStartScheduler();
    (void)PRINTF("Failed to start FreeRTOS.\r\n");
    for (;;) {}
}
