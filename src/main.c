#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include <stdarg.h>

#include "FreeRTOS.h"
#include "task.h"

#include <csp/csp.h>
#include <csp_proc/proc_server.h>

#include "clock_config.h"
#include "pin_mux.h"
#include "board.h"
#include "lpm.h"

#include "network_config.h"

void csp_print_func(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    PRINTF("%s", buffer);
    va_end(args);
}

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

    int pll_needed[4] = {6, 25, 27, 31};
    for (int i = 0; i < sizeof(pll_needed) / sizeof(pll_needed[0]); i++) {
        CCM->PLL_CTRL[pll_needed[i]].PLL_CTRL = kCLOCK_ClockNeededRun;
    }
    ServiceFlagAddr = ServiceBusy;
    IOMUXC_GPR->GPR22 &= ~IOMUXC_GPR_GPR22_GPR_M7_CPUWAIT(0b0);

    setup_network();

    proc_server_init();
    csp_bind_callback(proc_serve, PROC_PORT_SERVER);

    vTaskStartScheduler();
    (void)PRINTF("Failed to start FreeRTOS.\r\n");
    for (;;) {}
}
