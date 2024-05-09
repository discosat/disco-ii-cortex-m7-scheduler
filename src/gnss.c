#include "fsl_uart.h"
#include <stdint.h>

#include <FreeRTOS.h>
#include <task.h>

#include "tinygps.h"

#include "gnss.h"
#include "param_config.h"

#define GNSS_UART            UART3
#define GNSS_UART_CLK_FREQ \
    CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / (CLOCK_GetRootPreDivider(kCLOCK_RootUart3)) / \
        (CLOCK_GetRootPostDivider(kCLOCK_RootUart3)) / 10
#define GNSS_UART_BAUDRATE   115200U
#define GNSS_IRQn            UART3_IRQn
#define GNSS_UART_IRQHandler UART3_IRQHandler

#define UART_RX_BUFFER_SIZE 256

volatile uint16_t uart_rx_read_ptr = 0;
volatile uint16_t uart_rx_write_ptr = 0;
uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];

static StaticTask_t xGNSSRXTaskTCB;
static StackType_t uxGNSSRXTaskStack[256];
static TaskHandle_t gnss_rx_task_handle = NULL;

void GNSS_UART_IRQHandler(void) {
    uint8_t data;

    if ((UART_GetStatusFlag(GNSS_UART, kUART_RxDataReadyFlag)) || (UART_GetStatusFlag(GNSS_UART, kUART_RxOverrunFlag)))
    {
        data = UART_ReadByte(GNSS_UART);
        uart_rx_buffer[uart_rx_write_ptr] = data;
        uart_rx_write_ptr = (uart_rx_write_ptr + 1) % UART_RX_BUFFER_SIZE;
    }
    SDK_ISR_EXIT_BARRIER;
}

static void gnss_rx_task(void *pvParameters) {
    // Deferred parsing of the data on the UART line
    while (1) {
        if (uart_rx_read_ptr != uart_rx_write_ptr) {
            gps_encode(uart_rx_buffer[uart_rx_read_ptr]);
            uart_rx_read_ptr = (uart_rx_read_ptr + 1) % UART_RX_BUFFER_SIZE;
        } else {
            vTaskDelay(5);
        }
    }
}

void gnss_init() {
    status_t status;
    uart_config_t config;
    
    UART_GetDefaultConfig(&config);
    config.baudRate_Bps = GNSS_UART_BAUDRATE;
    config.enableTx = false;
    config.enableRx = true;

    status = UART_Init(GNSS_UART, &config, GNSS_UART_CLK_FREQ);
    if (kStatus_Success != status) {
        // TODO: handle error
    }

    NVIC_SetPriority(GNSS_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    UART_EnableInterrupts(GNSS_UART, kUART_RxDataReadyEnable | kUART_RxOverrunEnable);
    status = EnableIRQ(GNSS_IRQn);
    if (kStatus_Success != status) {
        // TODO: handle error
    }

    gnss_rx_task_handle = xTaskCreateStatic(
        gnss_rx_task,
        "GNSS_RX_TASK",
        sizeof(uxGNSSRXTaskStack) / sizeof(StackType_t),
        NULL,
        tskIDLE_PRIORITY + 3U,
        uxGNSSRXTaskStack,
        &xGNSSRXTaskTCB
    );
}

int gnss_poll() {
    float flat, flon;
    unsigned long age;
    unsigned long gps_date, gps_time;

    gps_f_get_position(&flat, &flon, &age);    
    gps_get_datetime(&gps_date, &gps_time, &age);

    float speed = gps_f_speed_mps();
    float alt = gps_f_altitude();
    float course = gps_f_course();

    param_set_float(&gnss_lat, flat);
    param_set_float(&gnss_lon, flon);
    param_set_uint32(&gnss_age, age);
    param_set_uint32(&gnss_date, gps_date);
    param_set_uint32(&gnss_time, gps_time);
    param_set_float(&gnss_speed, speed);
    param_set_float(&gnss_alt, alt);
    param_set_float(&gnss_course, course);

    return 0;
}
