/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOARD_H_
#define _BOARD_H_
#include "clock_config.h"
#include "fsl_clock.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief The board name */
#define BOARD_NAME        "PicoCoreMX8MP"
#define MANUFACTURER_NAME "F&S"
#define BOARD_DOMAIN_ID   (1U)
/* The UART to use for debug messages. */
#define BOARD_DEBUG_UART_TYPE     kSerialPort_Uart
#define BOARD_DEBUG_UART_BAUDRATE (115200U)
#define BOARD_DEBUG_UART_BASEADDR UART1_BASE
#define BOARD_DEBUG_UART_INSTANCE (1U)
#define BOARD_DEBUG_UART_CLK_FREQ                                                           \
	CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / (CLOCK_GetRootPreDivider(kCLOCK_RootUart1)) / \
		(CLOCK_GetRootPostDivider(kCLOCK_RootUart1)) / 10
#define BOARD_UART_IRQ         UART1_IRQn
#define BOARD_UART_IRQ_HANDLER UART1_IRQHandler

#define BOARD_GPC_BASEADDR GPC
#define BOARD_MU_IRQ       MU1_M7_IRQn

#define BOARD_CODEC_I2C            I2C6
#define BOARD_CODEC_I2C_INSTANCE   (6U)
#define BOARD_CODEC_I2C_CLOCK_FREQ (16000000U)

/* Shared memory base for RPMsg communication. */
#define VDEV0_VRING_BASE      (0x55000000U)
#define RESOURCE_TABLE_OFFSET (0xFF000)

#define RDC_DISABLE_A53_ACCESS 0xFC

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/

void BOARD_InitDebugConsole(void);
void BOARD_InitMemory(void);
void BOARD_RdcInit(void);
void BOARD_PeripheralRdcSetting(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
