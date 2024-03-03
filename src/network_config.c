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

static TaskHandle_t vmem_server_task_handle = NULL;
static TaskHandle_t router_task_handle = NULL;
static TaskHandle_t onehz_task_handle = NULL;

void* vmem_server_task(void *pvParameters) {
    vmem_server_loop(pvParameters);
    return NULL;
}

void* router_task(void *pvParameters) {
    while (1) {
        csp_route_work();
    }
    return NULL;
}

static void* onehz_task(void *pvParameters) {
    while (1) {
        hook_onehz();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    return NULL;
}

static void csp_init_fun(void) {
    csp_conf.hostname = "DISCO-II-scheduler";
    csp_conf.model = "PicoCoreMX8MP-Cortex-M7";
    csp_conf.revision = "Rev1.10";
	csp_conf.version = 2;
	csp_conf.dedup = CSP_DEDUP_OFF;

    csp_init();

    csp_bind_callback(csp_service_handler, CSP_ANY);
    csp_bind_callback(param_serve, PARAM_PORT_SERVER);

    if (xTaskCreate(vmem_server_task, "VMEM_SERVER_TASK", 512U, NULL, tskIDLE_PRIORITY + 1U, vmem_server_task_handle) != pdPASS) {
        (void)PRINTF("\r\nFailed to create vmem server task\r\n");
    }

    if (xTaskCreate(router_task, "ROUTER_TASK", 512U, NULL, tskIDLE_PRIORITY + 1U, router_task_handle) != pdPASS) {
        (void)PRINTF("\r\nFailed to create router task\r\n");
        for (;;) {}
    }

    if (xTaskCreate(onehz_task, "ONEHZ_TASK", 512U, NULL, tskIDLE_PRIORITY + 1U, onehz_task_handle) != pdPASS) {
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

// static void iface_kiss_init(void) {
//     csp_iface_t* iface;
//     // KISS init - csp_kiss_add_interface ?
//     iface->is_default = ;
//     iface->addr = ;
//     iface->netmask = ;
//     iface->name = "KISS";
// }

int setup_network(void) {
    // vmem_file_init(&vmem_config); // TODO: flash storage

    // iface_can_init();
    // iface_kiss_init();
    
    csp_init_fun();

    hook_init();

    return 0;
}
