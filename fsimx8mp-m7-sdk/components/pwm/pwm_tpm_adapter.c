/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "pwm.h"
#include "fsl_tpm.h"

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/
typedef struct _hal_pwm_handle_struct_t
{
    uint32_t pwmClock_Hz;
    uint8_t instance;
} hal_pwm_handle_struct_t;
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

static TPM_Type *const s_tpmBase[] = TPM_BASE_PTRS;

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
hal_pwm_status_t HAL_PwmInit(hal_pwm_handle_t halPwmHandle, uint8_t instance, uint32_t srcClock_Hz)
{
    tpm_config_t tpmInfo;
    hal_pwm_handle_struct_t *halPwmState = halPwmHandle;

    assert(instance < (sizeof(s_tpmBase) / sizeof(s_tpmBase[0])));
    assert(s_tpmBase[instance]);
    assert(halPwmHandle);
    assert(sizeof(hal_pwm_handle_struct_t) == HAL_PWM_HANDLE_SIZE);
    halPwmState->pwmClock_Hz = srcClock_Hz;
    halPwmState->instance    = instance;
    TPM_GetDefaultConfig(&tpmInfo);
    /* Initialize tpm module */
    TPM_Init(s_tpmBase[instance], (void *)&tpmInfo);

    return kStatus_HAL_PwmSuccess;
}

void HAL_PwmDeinit(hal_pwm_handle_t halPwmHandle)
{
    hal_pwm_handle_struct_t *halPwmState = halPwmHandle;

    assert(halPwmHandle);
    assert(halPwmState->instance < (sizeof(s_tpmBase) / sizeof(s_tpmBase[0])));
    /* DeInitialize tpm module */
    TPM_Deinit(s_tpmBase[halPwmState->instance]);
}

/*! -------------------------------------------------------------------------
 * \brief  Pwm setup.
 *---------------------------------------------------------------------------*/
hal_pwm_status_t HAL_PwmSetupPwm(hal_pwm_handle_t halPwmHandle, uint8_t channel, hal_pwm_setup_config_t *setupConfig)
{
    hal_pwm_handle_struct_t *halPwmState = halPwmHandle;

    assert(halPwmHandle);
    assert(channel <= kTPM_Chnl_7);
    assert(halPwmState->instance < (sizeof(s_tpmBase) / sizeof(s_tpmBase[0])));
    assert(setupConfig);
    tpm_chnl_pwm_signal_param_t pwmChannelConfig = {
        .chnlNumber       = (tpm_chnl_t)channel,
        .level            = (tpm_pwm_level_select_t)setupConfig->level,
        .dutyCyclePercent = setupConfig->dutyCyclePercent,
#if defined(FSL_FEATURE_TPM_HAS_COMBINE) && FSL_FEATURE_TPM_HAS_COMBINE
        .firstEdgeDelayPercent = 0
#endif
    };

    if ((int32_t)kStatus_Success != TPM_SetupPwm(s_tpmBase[halPwmState->instance], (void *)&pwmChannelConfig, 1,
                                                 (tpm_pwm_mode_t)setupConfig->mode, setupConfig->pwmFreq_Hz,
                                                 halPwmState->pwmClock_Hz))
    {
        return kStatus_HAL_PwmFail;
    }
    TPM_StartTimer(s_tpmBase[halPwmState->instance], kTPM_SystemClock);
    return kStatus_HAL_PwmSuccess;
}

/*! -------------------------------------------------------------------------
 * \brief   update the Duty cycle
 *---------------------------------------------------------------------------*/
hal_pwm_status_t HAL_PwmUpdateDutycycle(hal_pwm_handle_t halPwmHandle,
                                        uint8_t channel,
                                        hal_pwm_mode_t mode,
                                        uint8_t dutyCyclePercent)
{
    hal_pwm_handle_struct_t *halPwmState = halPwmHandle;

    assert(halPwmHandle);
    assert(channel <= kTPM_Chnl_7);
    assert(halPwmState->instance < (sizeof(s_tpmBase) / sizeof(s_tpmBase[0])));

    TPM_UpdatePwmDutycycle(s_tpmBase[halPwmState->instance], (tpm_chnl_t)channel, (tpm_pwm_mode_t)mode,
                           dutyCyclePercent);

    return kStatus_HAL_PwmSuccess;
}
