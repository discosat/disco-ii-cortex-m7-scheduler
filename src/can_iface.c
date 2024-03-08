#include <FreeRTOS.h>
#include <semphr.h>
#include "fsl_flexcan.h"
#include "fsl_debug_console.h"

#include <csp/csp.h>
#include <csp/csp_id.h>
#include <csp/interfaces/csp_if_can.h>

#include "clock_config.h"
#include "can_iface.h"

#define CSPCAN FLEXCAN1
#define CSPCAN_CLK_FREQ \
    (CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / (CLOCK_GetRootPreDivider(kCLOCK_RootFlexCan1)) / \
     (CLOCK_GetRootPostDivider(kCLOCK_RootFlexCan1)))
#define CSPCAN_IRQn CAN_FD1_IRQn

#define RX_MESSAGE_BUFFER_NUM (9)
#define TX_MESSAGE_BUFFER_NUM (8)
#define PROMISCUOUS_MODE (0)

flexcan_handle_t flexcanHandle;
volatile bool wokenUp = false;
volatile bool rxComplete = true;
flexcan_mb_transfer_t rxXfer;
flexcan_frame_t txFrame, rxFrame;

static StaticTask_t xCANTaskTCB;
static StackType_t uxCANTaskStack[1024];
static TaskHandle_t can_task_handle = NULL;

static int csp_can_tx_func(void *driver_data, uint32_t id, const uint8_t * data, uint8_t dlc);

csp_can_interface_data_t ifdata = {
    .tx_func = csp_can_tx_func,
    .pbufs = NULL,
};

csp_iface_t csp_if_can = {
    .name = "CAN",
    .addr = 4,
    .netmask = 8,
    .interface_data = &ifdata,
};

static FLEXCAN_CALLBACK(flexcan_callback) {
    int xTaskWoken = pdFALSE;
    switch (status) {
        case kStatus_FLEXCAN_RxIdle:
            if (RX_MESSAGE_BUFFER_NUM == result) {
                rxComplete = true;
                vTaskNotifyGiveFromISR(can_task_handle, &xTaskWoken); // Deferred parsing of the frame
            }
            break;

        case kStatus_FLEXCAN_RxOverflow:
            // TODO: error code to CSP ?
            if (RX_MESSAGE_BUFFER_NUM == result) {
                rxComplete = true;
            }

        case kStatus_FLEXCAN_WakeUp:
            wokenUp = true;
            break;

        default:
            PRINTF("status: %d, result: %d\r\n", status, result);
            break;
    }
    portYIELD_FROM_ISR(xTaskWoken);
}

static int csp_can_tx_func(void *driver_data, uint32_t id, const uint8_t * data, uint8_t dlc) {
    uint8_t bytesSent = 0;

    while (bytesSent < dlc) {
        for (uint8_t i = 0; i < 8 && i < (dlc - bytesSent); i++) {
            switch(i) {
                case 0: txFrame.dataByte0 = data[bytesSent + i]; break;
                case 1: txFrame.dataByte1 = data[bytesSent + i]; break;
                case 2: txFrame.dataByte2 = data[bytesSent + i]; break;
                case 3: txFrame.dataByte3 = data[bytesSent + i]; break;
                case 4: txFrame.dataByte4 = data[bytesSent + i]; break;
                case 5: txFrame.dataByte5 = data[bytesSent + i]; break;
                case 6: txFrame.dataByte6 = data[bytesSent + i]; break;
                case 7: txFrame.dataByte7 = data[bytesSent + i]; break;
            }
        }

        txFrame.id = FLEXCAN_ID_EXT(id);
        txFrame.format = (uint8_t)kFLEXCAN_FrameFormatExtend;
        txFrame.type = (uint8_t)kFLEXCAN_FrameTypeData;
        txFrame.length = (dlc - bytesSent) < 8 ? dlc - bytesSent : 8;

        status_t txStatus = FLEXCAN_TransferSendBlocking(CSPCAN, (uint8_t)TX_MESSAGE_BUFFER_NUM, &txFrame);  // Using blocking transfer since it suffices for now and non-blocking would be fairly complex and prone to error because of the discprency between CAN frame size and the size of CSP packets
        if (txStatus == kStatus_Fail) {
            return CSP_ERR_TX;
        }

        bytesSent += txFrame.length;
    }

    return CSP_ERR_NONE;
}

static void can_task(void *pvParameters) {
    rxXfer.mbIdx = (uint8_t)RX_MESSAGE_BUFFER_NUM;
    rxXfer.frame = &rxFrame;
    (void)FLEXCAN_TransferReceiveNonBlocking(CSPCAN, &flexcanHandle, &rxXfer);

    while(1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (wokenUp) {}  // TODO: do something with this ?

        if (rxComplete) {
            (void)FLEXCAN_TransferReceiveNonBlocking(CSPCAN, &flexcanHandle, &rxXfer);
            rxComplete = false;

            int xTaskWoken = pdFALSE;
            uint8_t rxData[8] = {rxFrame.dataByte0, rxFrame.dataByte1, rxFrame.dataByte2, rxFrame.dataByte3, rxFrame.dataByte4, rxFrame.dataByte5, rxFrame.dataByte6, rxFrame.dataByte7};
            csp_can_rx(&csp_if_can, rxFrame.id, rxData, rxFrame.length, xTaskWoken); // Deferred parsing of the frame

            if (xTaskWoken) {
                portYIELD();
            }
        }
    }
}

csp_iface_t * iface_can_init(void) {    
    flexcan_config_t flexcanConfig;
    flexcan_rx_mb_config_t mbConfig;

    CLOCK_SetRootMux(kCLOCK_RootFlexCan1, kCLOCK_FlexCanRootmuxSysPll1);
    CLOCK_SetRootDivider(kCLOCK_RootFlexCan1, 2U, 5U);

    FLEXCAN_GetDefaultConfig(&flexcanConfig);

    flexcan_timing_config_t timing_config;
    memset(&timing_config, 0, sizeof(flexcan_timing_config_t));

    if (
        FLEXCAN_CalculateImprovedTimingValues(
            CSPCAN,
            flexcanConfig.baudRate,
            CSPCAN_CLK_FREQ,
            &timing_config
        )
    ) {
        memcpy(&(flexcanConfig.timingConfig), &timing_config, sizeof(flexcan_timing_config_t));
    }

    FLEXCAN_Init(CSPCAN, &flexcanConfig, CSPCAN_CLK_FREQ);

    NVIC_SetPriority(CSPCAN_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    FLEXCAN_TransferCreateHandle(CSPCAN, &flexcanHandle, flexcan_callback, NULL);

    if (PROMISCUOUS_MODE) {
        FLEXCAN_SetRxMbGlobalMask(CSPCAN, 0);
    } else {
        uint32_t mask = FLEXCAN_RX_MB_EXT_MASK(csp_if_can.addr, 0, 0);
        mask &= (CFP2_DST_MASK << CFP2_DST_OFFSET);
        FLEXCAN_SetRxMbGlobalMask(CSPCAN, mask);
    }

    mbConfig.format = kFLEXCAN_FrameFormatExtend;
    mbConfig.type = kFLEXCAN_FrameTypeData;
    mbConfig.id = csp_if_can.addr << CFP2_DST_OFFSET;

    FLEXCAN_SetRxMbConfig(CSPCAN, RX_MESSAGE_BUFFER_NUM, &mbConfig, true);
    FLEXCAN_SetTxMbConfig(CSPCAN, TX_MESSAGE_BUFFER_NUM, true);

    can_task_handle = xTaskCreateStatic(
        can_task,
        "CAN_TASK",
        sizeof(uxCANTaskStack) / sizeof(StackType_t),
        NULL,
        tskIDLE_PRIORITY + 3U,  // decrease to 3U?
        uxCANTaskStack,
        &xCANTaskTCB
    );

    csp_can_add_interface(&csp_if_can);
}
