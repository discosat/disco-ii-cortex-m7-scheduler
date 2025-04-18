/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _LPM_H_
#define _LPM_H_

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SYSTICK_BASE    GPT1
#define SYSTICK_IRQn    GPT1_IRQn
#define SYSTICK_HANDLER GPT1_IRQHandler
#define SYSTICK_CLOCK \
	24000000 / (CLOCK_GetRootPreDivider(kCLOCK_RootGpt1)) / (CLOCK_GetRootPostDivider(kCLOCK_RootGpt1))

/* Using SRC_GPR10 register to sync the tasks status with A core */
#define ServiceFlagAddr SRC->GPR10
/* The flags,ServiceBusy and ServiceIdle, shows if the service task is running or not.
 * If the task is runing, A core should not put DDR in self-refresh mode after A core enters supsend.
 */
#define ServiceBusy (0x5555U)
#define ServiceIdle (0x0U)

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/*!
 * @brief Configure the system tick(GPT) before entering the low power mode.
 * @return Return the sleep time ticks.
 */
uint32_t LPM_EnterTicklessIdle(uint32_t timeoutMilliSec, uint64_t * pCounter);
/*!
 * @brief Configure the system tick(GPT) after exist the low power mode.
 */
void LPM_ExitTicklessIdle(uint32_t timeoutTicks, uint64_t timeoutCounter);
/*!
 * @brief This function is used to increase the count of the block event.
 */
void LPM_IncreseBlockSleepCnt(void);
/*!
 * @brief This function is used to decrease the count of the block event.
 */
void LPM_DecreaseBlockSleepCnt(void);
/*!
 * @brief This function is used to judge if the system could enter low power mode.
 * @return Return true if there is no block event exists.
 */
bool LPM_AllowSleep(void);

/*
 * Clock Speed of M core
 */
typedef enum lpm_mcore_clock_speed {
	LPM_MCORE_HIGH_FREQ,
	LPM_MCORE_LOW_FREQ
} LPM_MCORE_CLOCK_SPEED;
void LPM_MCORE_ChangeMCoreClock(LPM_MCORE_CLOCK_SPEED target);

/*
 * LPM state of M core
 */
typedef enum lpm_power_status_mcore {
	LPM_MCORE_STATE_RUN,
	LPM_MCORE_STATE_WAIT,
	LPM_MCORE_STATE_STOP,
} LPM_POWER_STATUS_MCORE;
void LPM_MCORE_SetPowerStatus(GPC_Type * base, LPM_POWER_STATUS_MCORE targetPowerMode);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/
#endif /* _LPM_H_ */
