#include <csp/interfaces/csp_if_kiss.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "fsl_uart.h"
#include "fsl_uart_sdma.h"
#include "fsl_rdc.h"

#include "kiss_iface.h"
#include "board.h"

// maybe try UART1 during debug? BOARD_InitDebugConsole(); might be problematic
// UART3=UART_D for dev board and UART4=UART_B for custom PCB
#define KISS_UART          UART3
#define KISS_UART_CLK_FREQ \
    (CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / (CLOCK_GetRootPreDivider(kCLOCK_RootUart3)) / \
        (CLOCK_GetRootPostDivider(kCLOCK_RootUart3)) / 10)
#define KISS_UART_IRQn          UART3_IRQn
#define KISS_UART_DMA_BASEADDR SDMAARM3
#define UART_RX_DMA_CHANNEL 1U
#define UART_TX_DMA_CHANNEL 2U
#define UART_RX_DMA_REQUEST (22)
#define UART_TX_DMA_REQUEST (23)
#define KISS_UART_IRQHandler UART3_IRQHandler
#define BUFFER_LENGTH 512

void UART_UserCallback(UART_Type *base, uart_sdma_handle_t *handle, status_t status, void *userData);

AT_NONCACHEABLE_SECTION_ALIGN(uart_sdma_handle_t g_uartSdmaHandle, 4);
AT_NONCACHEABLE_SECTION_ALIGN(sdma_handle_t g_uartTxSdmaHandle, 4);
AT_NONCACHEABLE_SECTION_ALIGN(sdma_handle_t g_uartRxSdmaHandle, 4);

AT_NONCACHEABLE_SECTION_ALIGN(sdma_context_data_t context_Tx, 4);
AT_NONCACHEABLE_SECTION_ALIGN(sdma_context_data_t context_Rx, 4);

AT_NONCACHEABLE_SECTION_ALIGN(uart_transfer_t sendXfer, 4);
AT_NONCACHEABLE_SECTION_ALIGN(uart_transfer_t receiveXfer, 4);

// AT_NONCACHEABLE_SECTION_ALIGN(uint8_t g_txBuffer[BUFFER_LENGTH], 4);
AT_NONCACHEABLE_SECTION_ALIGN(uint8_t g_rxBuffer[BUFFER_LENGTH], 4);

// volatile bool rxBufferEmpty = true;
// volatile bool txBufferFull  = false;
volatile bool txOnGoing     = false;
volatile bool rxOnGoing     = false;
volatile uint32_t dataSize  = 0U;

static status_t status;
static uart_config_t config;
static sdma_config_t sdmaConfig;
static uart_transfer_t xfer;

static int kiss_driver_tx(void *driver_data, const unsigned char * data, size_t data_length);

csp_kiss_interface_data_t ifdata = {
    .tx_func = kiss_driver_tx,
};

csp_iface_t csp_if_kiss = {
    .name = "KISS",
    .addr = 3,
    .netmask = 8,
    .interface_data = &ifdata,
};

static StaticTask_t xKissTaskTCB;
static StackType_t uxKissTaskStack[1024];
static TaskHandle_t kiss_task_handle = NULL;

static StaticSemaphore_t kiss_lock_buf;
static SemaphoreHandle_t kiss_lock = NULL;

void csp_usart_lock(void * driver_data) {
	xSemaphoreTake(kiss_lock, 100);
}

void csp_usart_unlock(void * driver_data) {
	xSemaphoreGive(kiss_lock);
}

void KISS_UART_IRQHandler(uint32_t giccIar, void *param) {
    BaseType_t xTaskWoken = 0;
	vTaskNotifyGiveFromISR(kiss_task_handle, &xTaskWoken);
	if (xTaskWoken) {
        portYIELD();
    }

    /* if receiver awake flag was detected. */
    if ((UART_GetStatusFlag(KISS_UART, kUART_WakeFlag)) && (UART_GetEnabledInterrupts(KISS_UART) & kUART_WakeEnable))
    {
        /* Enable the IDLE line detected interrupt. */
        UART_ClearStatusFlag(KISS_UART, kUART_IdleFlag);
        UART_EnableInterrupts(KISS_UART, kUART_RxDmaIdleEnable);
        /* Disable the receiver awake interrupt for next transfer. */
        UART_DisableInterrupts(KISS_UART, kUART_WakeEnable);
        UART_ClearStatusFlag(KISS_UART, kUART_WakeFlag);
    }
    UART_TransferSdmaHandleIRQ(KISS_UART, &g_uartSdmaHandle);
    __DSB();
}

void UART_UserCallback(UART_Type *base, uart_sdma_handle_t *handle, status_t status, void *userData) {
    userData = userData;

    if (kStatus_UART_TxIdle == status) {
        // txBufferFull = false;
        txOnGoing = false;
    }

    if (kStatus_UART_RxIdle == status) {
        // rxBufferEmpty = false;
        rxOnGoing = false;

        csp_kiss_rx(&csp_if_kiss, userData, dataSize, NULL);
        dataSize = BUFFER_LENGTH;

        if (
            (UART_GetStatusFlag(KISS_UART, kUART_IdleFlag)) &&
            (UART_GetEnabledInterrupts(KISS_UART) & kUART_RxDmaIdleEnable)
        ) {
            dataSize = SDMA_GetTransferredBytes(&g_uartRxSdmaHandle);
            UART_DisableInterrupts(KISS_UART, kUART_RxDmaIdleEnable);
        }
    }
}

void kiss_uart_init(void) {
    // mark critical section because of RDC and DMA work
    portENTER_CRITICAL();

    if ((0x1U & RDC_GetPeriphAccessPolicy(RDC, kRDC_Periph_RDC, RDC_GetCurrentMasterDomainId(RDC))) != 0U) {
        /*set SDMA1 PERIPH to M7 Domain(DID=1),due to UART not be accessible by DID=0 by default*/
        rdc_domain_assignment_t assignment = {0};
        assignment.domainId                = BOARD_DOMAIN_ID;
        RDC_SetMasterDomainAssignment(RDC, kRDC_Master_SDMA1_PERIPH, &assignment);
    }

    UART_GetDefaultConfig(&config);
    config.baudRate_Bps    = (115200U);
    config.rxFifoWatermark = 1;
    config.enableTx        = true;
    config.enableRx        = true;

    status = UART_Init(KISS_UART, &config, KISS_UART_CLK_FREQ);
    if (kStatus_Success != status) {
        return kStatus_Fail;
    }

    SDMA_GetDefaultConfig(&sdmaConfig);
    SDMA_Init(KISS_UART_DMA_BASEADDR, &sdmaConfig);
    SDMA_CreateHandle(&g_uartTxSdmaHandle, KISS_UART_DMA_BASEADDR, UART_TX_DMA_CHANNEL, &context_Tx);
    SDMA_CreateHandle(&g_uartRxSdmaHandle, KISS_UART_DMA_BASEADDR, UART_RX_DMA_CHANNEL, &context_Rx);
    SDMA_SetChannelPriority(KISS_UART_DMA_BASEADDR, UART_TX_DMA_CHANNEL, 3U);
    SDMA_SetChannelPriority(KISS_UART_DMA_BASEADDR, UART_RX_DMA_CHANNEL, 4U);

    UART_TransferCreateHandleSDMA(
        KISS_UART,
        &g_uartSdmaHandle,
        UART_UserCallback,
        NULL,
        &g_uartTxSdmaHandle,
        &g_uartRxSdmaHandle,
        UART_TX_DMA_REQUEST,
        UART_RX_DMA_REQUEST
    );

    portEXIT_CRITICAL();
}

static void kiss_task(void *pvParameters) {
    receiveXfer.data = g_rxBuffer;
    receiveXfer.dataSize = BUFFER_LENGTH;

    UART_SetIdleCondition(KISS_UART, kUART_IdleFor16Frames);
    UART_DisableInterrupts(KISS_UART, kUART_AllInterruptsEnable);

    EnableIRQ(KISS_UART_IRQn);

    while(1) {
		if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == 0) {
            continue;
        }

        if (UART_GetStatusFlag(KISS_UART, kUART_IdleFlag)) {
            UART_ClearStatusFlag(KISS_UART, kUART_WakeFlag);
            UART_EnableInterrupts(KISS_UART, kUART_WakeEnable);
        }

        if (!rxOnGoing) {
            rxOnGoing = true;
            UART_ReceiveSDMA(KISS_UART, &g_uartSdmaHandle, &receiveXfer);
        }
	}
}

static int kiss_driver_tx(void *driver_data, const unsigned char * data, size_t data_length) {
    if (!txOnGoing) {
        sendXfer.data = data;
        sendXfer.dataSize = data_length;
        txOnGoing = true;
        UART_SendSDMA(KISS_UART, &g_uartSdmaHandle, &sendXfer);
    }
    return CSP_ERR_NONE;
}

void iface_kiss_init(void) {
	kiss_task_handle = xTaskCreateStatic(
        kiss_task,
        "KISS_TASK",
        sizeof(uxKissTaskStack) / sizeof(StackType_t),
        NULL,
        tskIDLE_PRIORITY + 3U,
        uxKissTaskStack,
        &xKissTaskTCB
    );

	if (kiss_lock == NULL) {
		kiss_lock = xSemaphoreCreateMutexStatic(&kiss_lock_buf);
	}

    csp_kiss_add_interface(&csp_if_kiss);
}
