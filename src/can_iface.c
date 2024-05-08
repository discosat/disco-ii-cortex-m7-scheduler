#include <FreeRTOS.h>
#include <semphr.h>
#include "fsl_flexcan.h"
#include "fsl_debug_console.h"

#include <csp/csp.h>
#include <csp/csp_id.h>
#include <csp/interfaces/csp_if_can.h>

#include "can_iface.h"

#define CSPCAN FLEXCAN1
#define CSPCAN_CLK_FREQ \
    (CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / (CLOCK_GetRootPreDivider(kCLOCK_RootFlexCan1)) / \
     (CLOCK_GetRootPostDivider(kCLOCK_RootFlexCan1)))
#define CSPCAN_IRQn CAN_FD1_IRQn

#define RX_MESSAGE_BUFFER_NUM (9)
#define TX_MESSAGE_BUFFER_NUM (8)
#define PROMISCUOUS_MODE (0)

#define ACQUIRE_TX_LOCK_TIMEOUT_MS 5000

flexcan_handle_t flexcanHandle;
volatile bool rxComplete = true;
SemaphoreHandle_t txInitSemaphore, txCompleteSemaphore;
flexcan_mb_transfer_t rxXfer, txXfer;
flexcan_frame_t txFrame, rxFrame;

static StaticTask_t xCANTaskTCB;
static StackType_t uxCANTaskStack[1024];
static TaskHandle_t can_task_handle = NULL;

static int csp_can_tx_frame(void *driver_data, uint32_t id, const uint8_t * data, uint8_t dlc);

csp_can_interface_data_t ifdata = {
    .tx_func = csp_can_tx_frame,
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
        case kStatus_FLEXCAN_TxBusy:
            if (TX_MESSAGE_BUFFER_NUM == result) {
                xSemaphoreGiveFromISR(txCompleteSemaphore, &xTaskWoken);
            }
            FLEXCAN_ClearStatusFlags(CSPCAN, FLEXCAN_ERROR_AND_STATUS_INIT_FLAG); // FLEXCAN_ERROR_AND_STATUS_INIT_FLAG covers all relevant interrupt flags (excluding the wake up flag)
            break;

        case kStatus_FLEXCAN_TxIdle:
        case kStatus_FLEXCAN_TxSwitchToRx:
            if (TX_MESSAGE_BUFFER_NUM == result) {
                xSemaphoreGiveFromISR(txCompleteSemaphore, &xTaskWoken);
            }
            break;

        case kStatus_FLEXCAN_RxBusy:
        case kStatus_FLEXCAN_RxFifoBusy:
            FLEXCAN_ClearStatusFlags(CSPCAN, FLEXCAN_ERROR_AND_STATUS_INIT_FLAG);
            break;

        case kStatus_FLEXCAN_RxIdle:
        case kStatus_FLEXCAN_RxFifoIdle:
            if (RX_MESSAGE_BUFFER_NUM == result) {
                rxComplete = true;
                vTaskNotifyGiveFromISR(can_task_handle, &xTaskWoken); // Deferred parsing of the frame
            }
            break;

        case kStatus_FLEXCAN_RxOverflow:
        case kStatus_FLEXCAN_RxFifoOverflow:
            if (RX_MESSAGE_BUFFER_NUM == result) {
                rxComplete = true;
                vTaskNotifyGiveFromISR(can_task_handle, &xTaskWoken);
            }
            FLEXCAN_ClearStatusFlags(CSPCAN, FLEXCAN_ERROR_AND_STATUS_INIT_FLAG);
            break;

        case kStatus_FLEXCAN_RxFifoWarning:
            FLEXCAN_ClearStatusFlags(CSPCAN, FLEXCAN_ERROR_AND_STATUS_INIT_FLAG);
            break;

        case kStatus_FLEXCAN_ErrorStatus:
            FLEXCAN_ClearStatusFlags(CSPCAN, FLEXCAN_ERROR_AND_STATUS_INIT_FLAG);
            break;

        case kStatus_FLEXCAN_WakeUp:
            FLEXCAN_ClearStatusFlags(CSPCAN, FLEXCAN_WAKE_UP_FLAG);
            break;

        case kStatus_FLEXCAN_RxRemote:
            // TODO: handle
            break;

#if (defined(FSL_FEATURE_FLEXCAN_HAS_ENHANCED_RX_FIFO) && FSL_FEATURE_FLEXCAN_HAS_ENHANCED_RX_FIFO)
        case kStatus_FLEXCAN_RxFifoUnderflow:
            FLEXCAN_ClearStatusFlags(CSPCAN, FLEXCAN_WAKE_UP_FLAG);
            break;
#endif

        case kStatus_FLEXCAN_UnHandled:
        default:
            FLEXCAN_ClearStatusFlags(CSPCAN, (uint32_t)(-1));
            // Release locks on unhandled case
            rxComplete = true;
            xSemaphoreGiveFromISR(txCompleteSemaphore, &xTaskWoken);
            vTaskNotifyGiveFromISR(can_task_handle, &xTaskWoken);
            csp_print("status: %d, result: %d\r\n", status, result);
            break;
    }
    portYIELD_FROM_ISR(xTaskWoken);
}

static int csp_can_tx_frame(void *driver_data, uint32_t id, const uint8_t * data, uint8_t dlc) {
    if (xSemaphoreTake(txInitSemaphore, pdMS_TO_TICKS(ACQUIRE_TX_LOCK_TIMEOUT_MS)) != pdTRUE) {
        return CSP_ERR_TX;
    }

    for (uint8_t i = 0; i < dlc; i++) {
        switch(i) {
            case 0: txFrame.dataByte0 = data[i]; break;
            case 1: txFrame.dataByte1 = data[i]; break;
            case 2: txFrame.dataByte2 = data[i]; break;
            case 3: txFrame.dataByte3 = data[i]; break;
            case 4: txFrame.dataByte4 = data[i]; break;
            case 5: txFrame.dataByte5 = data[i]; break;
            case 6: txFrame.dataByte6 = data[i]; break;
            case 7: txFrame.dataByte7 = data[i]; break;
        }
    }

    txFrame.id = FLEXCAN_ID_EXT(id);
    txFrame.format = (uint8_t)kFLEXCAN_FrameFormatExtend;
    txFrame.type = (uint8_t)kFLEXCAN_FrameTypeData;
    txFrame.length = dlc;

    txXfer.mbIdx = (uint8_t)TX_MESSAGE_BUFFER_NUM;
    txXfer.frame = &txFrame;

    status_t txStatus = FLEXCAN_TransferSendNonBlocking(CSPCAN, &flexcanHandle, &txXfer);

    if (xSemaphoreTake(txCompleteSemaphore, pdMS_TO_TICKS(ACQUIRE_TX_LOCK_TIMEOUT_MS)) != pdTRUE) {
        xSemaphoreGive(txInitSemaphore);
        return CSP_ERR_TX;
    }

    xSemaphoreGive(txInitSemaphore);
    if (txStatus == kStatus_Fail) {
        return CSP_ERR_TX;
    }

    return CSP_ERR_NONE;
}

static void can_task(void *pvParameters) {
    rxXfer.mbIdx = (uint8_t)RX_MESSAGE_BUFFER_NUM;
    rxXfer.frame = &rxFrame;
    (void)FLEXCAN_TransferReceiveNonBlocking(CSPCAN, &flexcanHandle, &rxXfer);

    while(1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

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
    txInitSemaphore = xSemaphoreCreateBinary();
    txCompleteSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(txInitSemaphore);

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
        tskIDLE_PRIORITY + 3U,
        uxCANTaskStack,
        &xCANTaskTCB
    );

    csp_can_add_interface(&csp_if_can);
}
