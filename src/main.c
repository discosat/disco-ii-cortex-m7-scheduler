/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_ns.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "FreeRTOS.h"
#include "task.h"

#include "lpm.h"
#include "fsl_uart.h"
#include "fsl_rdc.h"
#include "rsc_table.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define RDC_DISABLE_A53_ACCESS 0xFC
#define RPMSG_LITE_LINK_ID            (RL_PLATFORM_IMX8MP_M7_USER_LINK_ID)
#define RPMSG_LITE_SHMEM_BASE         (VDEV0_VRING_BASE)
#define RPMSG_LITE_NS_ANNOUNCE_STRING "rpmsg-openamp-demo-channel"
// #define RPMSG_LITE_MASTER_IS_LINUX

#define APP_DEBUG_UART_BAUDRATE (115200U) /* Debug console baud rate. */
#define APP_TASK_STACK_SIZE (256U)
#ifndef LOCAL_EPT_ADDR
#define LOCAL_EPT_ADDR (30U)
#endif
#define APP_RPMSG_READY_EVENT_DATA (1U)

typedef struct the_message
{
    uint32_t DATA;
} THE_MESSAGE, *THE_MESSAGE_PTR;

static volatile THE_MESSAGE msg = {0};
#ifdef RPMSG_LITE_MASTER_IS_LINUX
static char helloMsg[13];
#endif /* RPMSG_LITE_MASTER_IS_LINUX */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_PeripheralRdcSetting(void)
{
    rdc_domain_assignment_t assignment = {0};
    rdc_periph_access_config_t periphConfig;

    assignment.domainId = BOARD_DOMAIN_ID;

    /* Only configure the RDC if the RDC peripheral write access is allowed. */
    if ((0x1U & RDC_GetPeriphAccessPolicy(RDC, kRDC_Periph_RDC, assignment.domainId)) != 0U)
    {

        RDC_GetDefaultPeriphAccessConfig(&periphConfig);
        /* Do not allow the A53 domain(domain0) to access the following peripherals */
        periphConfig.policy = RDC_DISABLE_A53_ACCESS;
        periphConfig.periph = kRDC_Periph_UART4;  // TODO UART3 or UART4 ??
        RDC_SetPeriphAccessConfig(RDC, &periphConfig);
        periphConfig.periph = kRDC_Periph_GPT1;
        RDC_SetPeriphAccessConfig(RDC, &periphConfig);
    }
}

static TaskHandle_t app_task_handle = NULL;

static void app_nameservice_isr_cb(uint32_t new_ept, const char *new_ept_name, uint32_t flags, void *user_data)
{
}

#ifdef MCMGR_USED
/*!
 * @brief Application-specific implementation of the SystemInitHook() weak function.
 */
void SystemInitHook(void)
{
    /* Initialize MCMGR - low level multicore management library. Call this
       function as close to the reset entry as possible to allow CoreUp event
       triggering. The SystemInitHook() weak function overloading is used in this
       application. */
    (void)MCMGR_EarlyInit();
}
#endif /* MCMGR_USED */

static void app_task(void *param)
{
    volatile uint32_t remote_addr;
    struct rpmsg_lite_endpoint *volatile my_ept;
    volatile rpmsg_queue_handle my_queue;
    struct rpmsg_lite_instance *volatile my_rpmsg;
    volatile rpmsg_ns_handle ns_handle;
    char ch;

    /* Print the initial banner */
    (void)PRINTF("\r\nRPMSG Ping-Pong FreeRTOS RTOS API Demo...\r\n");

#ifdef MCMGR_USED
    uint32_t startupData;
    mcmgr_status_t status;

    /* Get the startup data */
    do
    {
        status = MCMGR_GetStartupData(&startupData);
    } while (status != kStatus_MCMGR_Success);

    my_rpmsg = rpmsg_lite_remote_init((void *)(char *)startupData, RPMSG_LITE_LINK_ID, RL_NO_FLAGS);

    /* Signal the other core we are ready by triggering the event and passing the APP_RPMSG_READY_EVENT_DATA */
    (void)MCMGR_TriggerEvent(kMCMGR_RemoteApplicationEvent, APP_RPMSG_READY_EVENT_DATA);
#else
    (void)PRINTF("RPMSG Share Base Addr is 0x%x\r\n", RPMSG_LITE_SHMEM_BASE);
    my_rpmsg = rpmsg_lite_remote_init((void *)RPMSG_LITE_SHMEM_BASE, RPMSG_LITE_LINK_ID, RL_NO_FLAGS);
#endif /* MCMGR_USED */
    while (0 == rpmsg_lite_is_link_up(my_rpmsg))
    {
    }
    (void)PRINTF("Link is up!\r\n");

    my_queue  = rpmsg_queue_create(my_rpmsg);
    my_ept    = rpmsg_lite_create_ept(my_rpmsg, LOCAL_EPT_ADDR, rpmsg_queue_rx_cb, my_queue);
    ns_handle = rpmsg_ns_bind(my_rpmsg, app_nameservice_isr_cb, ((void *)0));
    /* Introduce some delay to avoid NS announce message not being captured by the master side.
       This could happen when the remote side execution is too fast and the NS announce message is triggered
       before the nameservice_isr_cb is registered on the master side. */
    SDK_DelayAtLeastUs(1000000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    (void)rpmsg_ns_announce(my_rpmsg, my_ept, RPMSG_LITE_NS_ANNOUNCE_STRING, (uint32_t)RL_NS_CREATE);
    (void)PRINTF("Nameservice announce sent.\r\n");

#ifdef RPMSG_LITE_MASTER_IS_LINUX
    /* Wait Hello handshake message from Remote Core. */
    (void)rpmsg_queue_recv(my_rpmsg, my_queue, (uint32_t *)&remote_addr, helloMsg, sizeof(helloMsg), ((void *)0),
                           RL_BLOCK);
#endif /* RPMSG_LITE_MASTER_IS_LINUX */

    while (msg.DATA <= 100U)
    {
        (void)PRINTF("Waiting for ping...\r\n");
        (void)rpmsg_queue_recv(my_rpmsg, my_queue, (uint32_t *)&remote_addr, (char *)&msg, sizeof(THE_MESSAGE),
                               ((void *)0), RL_BLOCK);
        msg.DATA++;
        while (1)
        {
            ch = GETCHAR();
            if(ch == 'c')
                break;
        }
        (void)PRINTF("Sending pong...\r\n");
        (void)rpmsg_lite_send(my_rpmsg, my_ept, remote_addr, (char *)&msg, sizeof(THE_MESSAGE), RL_BLOCK);
    }

    (void)PRINTF("Ping pong done, deinitializing...\r\n");

    (void)rpmsg_lite_destroy_ept(my_rpmsg, my_ept);
    my_ept = ((void *)0);
    (void)rpmsg_queue_destroy(my_rpmsg, my_queue);
    my_queue = ((void *)0);
    (void)rpmsg_ns_unbind(my_rpmsg, ns_handle);
    (void)rpmsg_lite_deinit(my_rpmsg);
    msg.DATA = 0U;

    (void)PRINTF("Looping forever...\r\n");

    /* End of the example */
    for (;;)
    {
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i = 0;
    /* Initialize standard SDK demo application pins */
    /* M7 has its local cache and enabled by default,
     * need to set smart subsystems (0x28000000 ~ 0x3FFFFFFF)
     * non-cacheable before accessing this address region */
    BOARD_InitMemory();

    /* Board specific RDC settings */
    BOARD_RdcInit();
    BOARD_PeripheralRdcSetting();

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    PRINTF("\r\n__BUILT AT %s, %s__\r\n", __TIME__, __DATE__);


    // CLOCK_SetRootMux(kCLOCK_RootI2c3, kCLOCK_I2cRootmuxSysPll1Div5); /* Set I2C source to SysPLL1 Div5 160MHZ */
    // CLOCK_SetRootDivider(kCLOCK_RootI2c3, 1U, 10U);                  /* Set root clock to 160MHZ / 10 = 16MHZ */
    // CLOCK_SetRootMux(kCLOCK_RootGpt1, kCLOCK_GptRootmuxOsc24M);      /* Set GPT source to Osc24 MHZ */
    // CLOCK_SetRootDivider(kCLOCK_RootGpt1, 1U, 1U);

    for (i = 0; i < 39; i++)
    {
        CCM->PLL_CTRL[i].PLL_CTRL = kCLOCK_ClockNeededRun;
    }

    copyResourceTable();

#ifdef MCMGR_USED
    /* Initialize MCMGR before calling its API */
    (void)MCMGR_Init();
#endif /* MCMGR_USED */

    ServiceFlagAddr = ServiceBusy;
    if (xTaskCreate(app_task, "APP_TASK", APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1U, &app_task_handle) != pdPASS)
    {
        (void)PRINTF("\r\nFailed to create application task\r\n");
        for (;;)
        {
        }
    }

    vTaskStartScheduler();

    (void)PRINTF("Failed to start FreeRTOS on core0.\r\n");
    for (;;)
    {
    }
}
