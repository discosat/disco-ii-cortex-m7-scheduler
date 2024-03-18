#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_ns.h"
#include "FreeRTOS.h"
#include "task.h"

#include "board.h"
#include "rsc_table.h"
#include "a53_comm_service.h"

#include <csp/csp.h>

#define RPMSG_LITE_SHMEM_BASE         (VDEV0_VRING_BASE)
#define RPMSG_LITE_LINK_ID            (RL_PLATFORM_IMX8MP_M7_USER_LINK_ID)
#define RPMSG_LITE_NS_ANNOUNCE_STRING "rpmsg-virtual-tty-channel-1"
#ifndef LOCAL_EPT_ADDR
#define LOCAL_EPT_ADDR (30)
#endif

volatile uint8_t RPMSG_READY = 0;

static TaskHandle_t a53_service_task_handle = NULL;

struct rpmsg_lite_instance *volatile rpmsg_instance;
struct rpmsg_lite_endpoint *volatile rpmsg_endpoint;


static void a53_service_task(void *param) {
    volatile rpmsg_queue_handle rpmsg_queue;

    rpmsg_instance = rpmsg_lite_remote_init((void *)RPMSG_LITE_SHMEM_BASE, RPMSG_LITE_LINK_ID, RL_NO_FLAGS);
    while (0 == rpmsg_lite_is_link_up(rpmsg_instance)) {}

    rpmsg_queue = rpmsg_queue_create(rpmsg_instance);
    rpmsg_endpoint = rpmsg_lite_create_ept(rpmsg_instance, LOCAL_EPT_ADDR, rpmsg_queue_rx_cb, rpmsg_queue);
    (void)rpmsg_ns_announce(rpmsg_instance, rpmsg_endpoint, RPMSG_LITE_NS_ANNOUNCE_STRING, RL_NS_CREATE);

    RPMSG_READY = 1;
    for (;;) {
        vTaskSuspend(NULL);
    }
}

void a53_service_init() {
    copyResourceTable();

    if (xTaskCreate(a53_service_task, "A53_TASK", 256, NULL, tskIDLE_PRIORITY + 2, &a53_service_task_handle) != pdPASS) {
        for (;;) {}  // infinite loop, watchdog will trigger
    }
}


void a53_service_deinit() {
    vTaskDelete(a53_service_task_handle);
    rpmsg_lite_destroy_ept(rpmsg_instance, rpmsg_endpoint);
    rpmsg_lite_deinit(rpmsg_instance);
}


int wake_a53_callback(struct param_s *param, int offset) {
    if (RPMSG_READY == 0) {
        return CSP_ERR_TX;
    }

    volatile uint32_t remote_addr;
    uint32_t len;
    uint32_t size;
    void *tx_buf;
    int32_t result;

    tx_buf = rpmsg_lite_alloc_tx_buffer(rpmsg_instance, &size, RL_BLOCK);
    result = rpmsg_lite_send_nocopy(rpmsg_instance, rpmsg_endpoint, remote_addr, tx_buf, len);  // pings with a 0-length message
    if (result != 0) {
        return CSP_ERR_TX;
    }
    return CSP_ERR_NONE;
}
