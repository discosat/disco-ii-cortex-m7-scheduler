#include <csp/csp.h>
#include <param/param_server.h>
#include <vmem/vmem_file.h>
#include <vmem/vmem_server.h>

#include "FreeRTOS.h"
#include "task.h"
#include "fsl_debug_console.h"

#include "network_config.h"
#include "hooks.h"
#include "vmem_config.h"
#include "param_config.h"
#include "kiss_iface.h"

StaticTask_t xVmemServerTaskTCB;
StackType_t uxVmemServerTaskStack[1024];
static TaskHandle_t vmem_server_task_handle = NULL;

StaticTask_t xRouterTaskTCB;
StackType_t uxRouterTaskStack[1024];
static TaskHandle_t router_task_handle = NULL;

StaticTask_t xOneHzTaskTCB;
StackType_t uxOneHzTaskStack[1024];
static TaskHandle_t onehz_task_handle = NULL;

void vmem_server_task(void *pvParameters) {
    vmem_server_loop(pvParameters);
    return NULL;
}

void router_task(void *pvParameters) {
    while (1) {
        csp_route_work();
    }
    return NULL;
}

static void onehz_task(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) {
        // TODO: Consider adding watchdog timer and feed here (maybe also other places?)
        hook_onehz();
        vTaskDelayUntil(&xLastWakeTime, 1 * configTICK_RATE_HZ);
    }
    return NULL;
}

static void setup_csp_tasks(void) {
    vmem_server_task_handle = xTaskCreateStatic(
        vmem_server_task,
        "VMEM_SERVER_TASK",
        sizeof(uxVmemServerTaskStack) / sizeof(StackType_t),
        NULL,
        tskIDLE_PRIORITY + 1U,
        uxVmemServerTaskStack,
        &xVmemServerTaskTCB
    );
    if (vmem_server_task_handle == NULL) {
        (void)PRINTF("\r\nFailed to create vmem server task\r\n");
    }

    router_task_handle = xTaskCreateStatic(
        router_task,
        "ROUTER_TASK",
        sizeof(uxRouterTaskStack) / sizeof(StackType_t),
        NULL,
        tskIDLE_PRIORITY + 2U,
        uxRouterTaskStack,
        &xRouterTaskTCB
    );
    if (router_task_handle == NULL) {
        (void)PRINTF("\r\nFailed to create router task\r\n");
        for (;;) {}
    }

    onehz_task_handle = xTaskCreateStatic(
        onehz_task,
        "ONEHZ_TASK",
        sizeof(uxOneHzTaskStack) / sizeof(StackType_t),
        NULL,
        tskIDLE_PRIORITY + 3U,
        uxOneHzTaskStack,
        &xOneHzTaskTCB
    );
    if (onehz_task_handle == NULL) {
        (void)PRINTF("\r\nFailed to create onehz task\r\n");
    }
}

// static void iface_can_init(void) {
//     csp_iface_t* iface;
//     // CAN init - csp_can_add_interface ?
// 
//     iface->is_default = ;
//     iface->addr = ;
//     iface->netmask = ;
//     iface->name = "CAN";
// }

int setup_network(void) {
    csp_conf.hostname = "DISCO-II-scheduler";
    csp_conf.model = "PicoCoreMX8MP-Cortex-M7";
    csp_conf.revision = "Rev1.10";
	csp_conf.version = 2;
	csp_conf.dedup = CSP_DEDUP_OFF;

    csp_init();
    
    csp_cmp_set_memcpy((csp_memcpy_fnc_t) vmem_memcpy);

    // iface_can_init();

    kiss_uart_init();
    iface_kiss_init();

    // TODO: maybe ?
    // csp_rt_init();
    // csp_rtable_set(0, 0, csp_iflist_get_by_index(dfl_if), CSP_NO_VIA_ADDRESS);

    setup_csp_tasks();

    csp_bind_callback(csp_service_handler, CSP_ANY);
    csp_bind_callback(param_serve, PARAM_PORT_SERVER);

    // serial_init ?

    hook_init();

    return 0;
}
