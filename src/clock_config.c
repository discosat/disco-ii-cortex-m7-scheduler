/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "clock_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Fractional PLLs: Fout = ((mainDiv+dsm/65536) * refSel) / (preDiv * 2^ postDiv) */

/* Integer PLLs: Fout = (mainDiv * refSel) / (preDiv * 2^ postDiv) */
/* SYSTEM PLL1 configuration */
const ccm_analog_integer_pll_config_t g_sysPll1Config = {
	.refSel = kANALOG_PllRefOsc24M, /*!< PLL reference OSC24M */
	.mainDiv = 400U,
	.preDiv = 3U,
	.postDiv = 2U, /*!< SYSTEM PLL1 frequency  = 800MHZ */
};

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_BootClockRUN(void) {
	CLOCK_SetRootMux(kCLOCK_RootAhb, kCLOCK_AhbRootmuxOsc24M);
	CLOCK_SetRootMux(kCLOCK_RootM7, kCLOCK_M7RootmuxOsc24M);

	CLOCK_SetRootDivider(kCLOCK_RootM7, 1U, 1U);              /* Set root clock to 800M */
	CLOCK_SetRootMux(kCLOCK_RootM7, kCLOCK_M7RootmuxSysPll1); /* switch cortex-m7 to SYSTEM PLL1 */

	// CLOCK_SetRootDivider(kCLOCK_RootQspi, 1U, 2U);              /* Set root clock to 800M */
	// CLOCK_SetRootMux(kCLOCK_RootM7, kCLOCK_M7RootmuxSysPll1); /* switch QSPI to SYSTEM PLL1 */

	CLOCK_SetRootDivider(kCLOCK_RootAhb, 1U, 1U);                   /* Set root clock freq to 133M / 1= 133MHZ */
	CLOCK_SetRootMux(kCLOCK_RootAhb, kCLOCK_AhbRootmuxSysPll1Div6); /* switch AHB to SYSTEM PLL1 DIV6 */

	CLOCK_SetRootMux(kCLOCK_RootUart1, kCLOCK_UartRootmuxSysPll1Div10); /* Set UART1 source to SysPLL1 Div10 80MHZ */
	CLOCK_SetRootDivider(kCLOCK_RootUart1, 1U, 1U);                     /* Set root clock to 80MHZ/ 1= 80MHZ */

	CLOCK_SetRootMux(kCLOCK_RootUart3, kCLOCK_UartRootmuxSysPll1Div10); /* Set UART3 source to SysPLL1 Div10 80MHZ */
	CLOCK_SetRootDivider(kCLOCK_RootUart3, 1U, 1U);                     /* Set root clock to 80MHZ/ 1= 80MHZ */

	CLOCK_EnableClock(kCLOCK_Rdc);   /* Enable RDC clock */
	CLOCK_EnableClock(kCLOCK_Ocram); /* Enable Ocram clock */

	/* The purpose to enable the following modules clock is to make sure the M7 core could work normally when A53 core
	 * enters the low power status.*/
	CLOCK_EnableClock(kCLOCK_Sim_m);
	CLOCK_EnableClock(kCLOCK_Sim_main);
	CLOCK_EnableClock(kCLOCK_Sim_s);
	CLOCK_EnableClock(kCLOCK_Sim_wakeup);
	CLOCK_EnableClock(kCLOCK_Debug);
	CLOCK_EnableClock(kCLOCK_Dram);
	CLOCK_EnableClock(kCLOCK_Sec_Debug);

	/* Update core clock */
	SystemCoreClockUpdate();
}

void BOARD_A53WFIClockReenable(void) {
	CLOCK_SetRootMux(kCLOCK_RootUart1, kCLOCK_UartRootmuxSysPll1Div10);
	CLOCK_SetRootDivider(kCLOCK_RootUart1, 1U, 1U);

	CLOCK_SetRootMux(kCLOCK_RootUart3, kCLOCK_UartRootmuxSysPll1Div10);
	CLOCK_SetRootDivider(kCLOCK_RootUart3, 1U, 1U);
}
