/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_sgtl5000.h"

/*******************************************************************************
 * Definitations
 ******************************************************************************/

/*! @brief swap byte sequence in  16 bit data */
#define SGTL_SWAP_UINT16_BYTE_SEQUENCE(x) (__REV16(x))

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
status_t SGTL_Init(sgtl_handle_t *handle, sgtl_config_t *config)
{
    assert(handle != NULL);
    assert(config != NULL);

    handle->config = config;
    /* i2c bus initialization */
    if (CODEC_I2C_Init(handle->i2cHandle, config->i2cConfig.codecI2CInstance, SGTL_I2C_BITRATE,
                       config->i2cConfig.codecI2CSourceClock) != kStatus_HAL_I2cSuccess)
    {
        return kStatus_Fail;
    }

    SGTL_WriteReg(handle, CHIP_ANA_POWER, 0x6AFF);

    /* Set the data route */
    SGTL_SetDataRoute(handle, config->route);

    /* Set sgtl5000 to master or slave */
    SGTL_SetMasterSlave(handle, config->master_slave);

    /* Input Volume Control
    Configure ADC left and right analog volume to desired default.
    Example shows volume of 0dB. */
    SGTL_WriteReg(handle, CHIP_ANA_ADC_CTRL, 0x0000U);

    /* Volume and Mute Control
       Configure HP_OUT left and right volume to minimum, unmute.
       HP_OUT and ramp the volume up to desired volume.*/
    SGTL_WriteReg(handle, CHIP_ANA_HP_CTRL, 0x1818U);
    SGTL_ModifyReg(handle, CHIP_ANA_CTRL, 0xFFEFU, 0x0000U);

    /* LINEOUT and DAC volume control */
    SGTL_ModifyReg(handle, CHIP_ANA_CTRL, 0xFEFFU, 0x0000U);

    /* Configure DAC left and right digital volume */
    SGTL_WriteReg(handle, CHIP_DAC_VOL, 0x5C5CU);

    /* Configure ADC volume, reduce 6db. */
    SGTL_WriteReg(handle, CHIP_ANA_ADC_CTRL, 0x0100U);

    /* Unmute DAC */
    SGTL_ModifyReg(handle, CHIP_ADCDAC_CTRL, 0xFFFBU, 0x0000U);
    SGTL_ModifyReg(handle, CHIP_ADCDAC_CTRL, 0xFFF7U, 0x0000U);

    /* Unmute ADC */
    SGTL_ModifyReg(handle, CHIP_ANA_CTRL, 0xFFFEU, 0x0000U);

    /* Set the audio format */
    SGTL_SetProtocol(handle, config->bus);
    SGTL_ConfigDataFormat(handle, config->format.mclk_HZ, config->format.sampleRate, config->format.bitWidth);

    /* sclk valid edge */
    if (config->format.sclkEdge == kSGTL_SclkValidEdgeRising)
    {
        SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_SCLK_INV_CLR_MASK, SGTL5000_I2S_VAILD_RISING_EDGE);
    }
    else
    {
        SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_SCLK_INV_CLR_MASK, SGTL5000_I2S_VAILD_FALLING_EDGE);
    }

    return kStatus_Success;
}

status_t SGTL_Deinit(sgtl_handle_t *handle)
{
    SGTL_DisableModule(handle, kSGTL_ModuleADC);
    SGTL_DisableModule(handle, kSGTL_ModuleDAC);
    SGTL_DisableModule(handle, kSGTL_ModuleDAP);
    SGTL_DisableModule(handle, kSGTL_ModuleI2SIN);
    SGTL_DisableModule(handle, kSGTL_ModuleI2SOUT);
    SGTL_DisableModule(handle, kSGTL_ModuleLineOut);

    return CODEC_I2C_Deinit(handle->i2cHandle);
}

void SGTL_SetMasterSlave(sgtl_handle_t *handle, bool master)
{
    if (master == 1)
    {
        SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_MS_CLR_MASK, SGTL5000_I2S_MASTER);
    }
    else
    {
        SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_MS_CLR_MASK, SGTL5000_I2S_SLAVE);
    }
}

status_t SGTL_EnableModule(sgtl_handle_t *handle, sgtl_module_t module)
{
    status_t ret = kStatus_Success;
    switch (module)
    {
        case kSGTL_ModuleADC:
            SGTL_ModifyReg(handle, CHIP_DIG_POWER, SGTL5000_ADC_ENABLE_CLR_MASK,
                           ((uint16_t)1U << SGTL5000_ADC_ENABLE_SHIFT));
            SGTL_ModifyReg(handle, CHIP_ANA_POWER, SGTL5000_ADC_POWERUP_CLR_MASK,
                           ((uint16_t)1U << SGTL5000_ADC_POWERUP_SHIFT));
            break;
        case kSGTL_ModuleDAC:
            SGTL_ModifyReg(handle, CHIP_DIG_POWER, SGTL5000_DAC_ENABLE_CLR_MASK,
                           ((uint16_t)1U << SGTL5000_DAC_ENABLE_SHIFT));
            SGTL_ModifyReg(handle, CHIP_ANA_POWER, SGTL5000_DAC_POWERUP_CLR_MASK,
                           ((uint16_t)1U << SGTL5000_DAC_POWERUP_SHIFT));
            break;
        case kSGTL_ModuleDAP:
            SGTL_ModifyReg(handle, CHIP_DIG_POWER, SGTL5000_DAP_ENABLE_CLR_MASK,
                           ((uint16_t)1U << SGTL5000_DAP_ENABLE_SHIFT));
            SGTL_ModifyReg(handle, SGTL5000_DAP_CONTROL, SGTL5000_DAP_CONTROL_DAP_EN_CLR_MASK,
                           ((uint16_t)1U << SGTL5000_DAP_CONTROL_DAP_EN_SHIFT));
            break;
        case kSGTL_ModuleI2SIN:
            SGTL_ModifyReg(handle, CHIP_DIG_POWER, SGTL5000_I2S_IN_ENABLE_CLR_MASK,
                           ((uint16_t)1U << SGTL5000_I2S_IN_ENABLE_SHIFT));
            break;
        case kSGTL_ModuleI2SOUT:
            SGTL_ModifyReg(handle, CHIP_DIG_POWER, SGTL5000_I2S_OUT_ENABLE_CLR_MASK,
                           ((uint16_t)1U << SGTL5000_I2S_OUT_ENABLE_SHIFT));
            break;
        case kSGTL_ModuleHP:
            SGTL_ModifyReg(handle, CHIP_ANA_POWER, SGTL5000_HEADPHONE_POWERUP_CLR_MASK,
                           ((uint16_t)1U << SGTL5000_HEADPHONE_POWERUP_SHIFT));
            break;
        case kSGTL_ModuleLineOut:
            SGTL_ModifyReg(handle, CHIP_ANA_POWER, SGTL5000_LINEOUT_POWERUP_CLR_MASK,
                           ((uint16_t)1U << SGTL5000_LINEOUT_POWERUP_SHIFT));
            break;
        default:
            ret = kStatus_InvalidArgument;
            break;
    }
    return ret;
}

status_t SGTL_DisableModule(sgtl_handle_t *handle, sgtl_module_t module)
{
    status_t ret = kStatus_Success;
    switch (module)
    {
        case kSGTL_ModuleADC:
            SGTL_ModifyReg(handle, CHIP_DIG_POWER, SGTL5000_ADC_ENABLE_CLR_MASK,
                           ((uint16_t)0U << SGTL5000_ADC_ENABLE_SHIFT));
            SGTL_ModifyReg(handle, CHIP_ANA_POWER, SGTL5000_ADC_POWERUP_CLR_MASK,
                           ((uint16_t)0U << SGTL5000_ADC_POWERUP_SHIFT));
            break;
        case kSGTL_ModuleDAC:
            SGTL_ModifyReg(handle, CHIP_DIG_POWER, SGTL5000_DAC_ENABLE_CLR_MASK,
                           ((uint16_t)0U << SGTL5000_DAC_ENABLE_SHIFT));
            SGTL_ModifyReg(handle, CHIP_ANA_POWER, SGTL5000_DAC_POWERUP_CLR_MASK,
                           ((uint16_t)0U << SGTL5000_DAC_POWERUP_SHIFT));
            break;
        case kSGTL_ModuleDAP:
            SGTL_ModifyReg(handle, CHIP_DIG_POWER, SGTL5000_DAP_ENABLE_CLR_MASK,
                           ((uint16_t)0U << SGTL5000_DAP_ENABLE_SHIFT));
            SGTL_ModifyReg(handle, SGTL5000_DAP_CONTROL, SGTL5000_DAP_CONTROL_DAP_EN_CLR_MASK,
                           ((uint16_t)0U << SGTL5000_DAP_CONTROL_DAP_EN_SHIFT));
            break;
        case kSGTL_ModuleI2SIN:
            SGTL_ModifyReg(handle, CHIP_DIG_POWER, SGTL5000_I2S_IN_ENABLE_CLR_MASK,
                           ((uint16_t)0U << SGTL5000_I2S_IN_ENABLE_SHIFT));
            break;
        case kSGTL_ModuleI2SOUT:
            SGTL_ModifyReg(handle, CHIP_DIG_POWER, SGTL5000_I2S_OUT_ENABLE_CLR_MASK,
                           ((uint16_t)0U << SGTL5000_I2S_OUT_ENABLE_SHIFT));
            break;
        case kSGTL_ModuleHP:
            SGTL_ModifyReg(handle, CHIP_ANA_POWER, SGTL5000_HEADPHONE_POWERUP_CLR_MASK,
                           ((uint16_t)0U << SGTL5000_HEADPHONE_POWERUP_SHIFT));
            break;
        case kSGTL_ModuleLineOut:
            SGTL_ModifyReg(handle, CHIP_ANA_POWER, SGTL5000_LINEOUT_POWERUP_CLR_MASK,
                           ((uint16_t)0U << SGTL5000_LINEOUT_POWERUP_SHIFT));
            break;
        default:
            ret = kStatus_InvalidArgument;
            break;
    }
    return ret;
}

status_t SGTL_SetDataRoute(sgtl_handle_t *handle, sgtl_route_t route)
{
    status_t ret = kStatus_Success;
    switch (route)
    {
        case kSGTL_RouteBypass:
            /* Bypass means from line-in to HP*/
            SGTL_WriteReg(handle, CHIP_DIG_POWER, 0x0000);
            SGTL_EnableModule(handle, kSGTL_ModuleHP);
            SGTL_ModifyReg(handle, CHIP_ANA_CTRL, SGTL5000_SEL_HP_CLR_MASK, SGTL5000_SEL_HP_LINEIN);
            break;
        case kSGTL_RoutePlayback:
            /* Data route I2S_IN-> DAC-> HP */
            SGTL_EnableModule(handle, kSGTL_ModuleHP);
            SGTL_EnableModule(handle, kSGTL_ModuleDAC);
            SGTL_EnableModule(handle, kSGTL_ModuleI2SIN);
            SGTL_ModifyReg(handle, CHIP_SSS_CTRL, SGTL5000_DAC_SEL_CLR_MASK, SGTL5000_DAC_SEL_I2S_IN);
            SGTL_ModifyReg(handle, CHIP_ANA_CTRL, SGTL5000_SEL_HP_CLR_MASK, SGTL5000_SEL_HP_DAC);
            break;
        case kSGTL_RoutePlaybackandRecord:
            /* I2S IN->DAC->HP  LINE_IN->ADC->I2S_OUT */
            SGTL_EnableModule(handle, kSGTL_ModuleHP);
            SGTL_EnableModule(handle, kSGTL_ModuleDAC);
            SGTL_EnableModule(handle, kSGTL_ModuleI2SIN);
            SGTL_EnableModule(handle, kSGTL_ModuleI2SOUT);
            SGTL_EnableModule(handle, kSGTL_ModuleADC);
            SGTL_ModifyReg(handle, CHIP_SSS_CTRL, SGTL5000_DAC_SEL_CLR_MASK, SGTL5000_DAC_SEL_I2S_IN);
            SGTL_ModifyReg(handle, CHIP_ANA_CTRL, SGTL5000_SEL_HP_CLR_MASK, SGTL5000_SEL_HP_DAC);
            SGTL_ModifyReg(handle, CHIP_ANA_CTRL, SGTL5000_SEL_ADC_CLR_MASK, SGTL5000_SEL_ADC_LINEIN);
            SGTL_ModifyReg(handle, CHIP_SSS_CTRL, SGTL5000_I2S_OUT_SEL_CLR_MASK, SGTL5000_I2S_OUT_SEL_ADC);
            break;
        case kSGTL_RoutePlaybackwithDAP:
            /* I2S_IN->DAP->DAC->HP */
            SGTL_EnableModule(handle, kSGTL_ModuleHP);
            SGTL_EnableModule(handle, kSGTL_ModuleDAC);
            SGTL_EnableModule(handle, kSGTL_ModuleI2SIN);
            SGTL_EnableModule(handle, kSGTL_ModuleDAP);
            SGTL_ModifyReg(handle, CHIP_SSS_CTRL, SGTL5000_DAP_SEL_CLR_MASK, SGTL5000_DAP_SEL_I2S_IN);
            SGTL_ModifyReg(handle, CHIP_SSS_CTRL, SGTL5000_DAC_SEL_CLR_MASK, SGTL5000_DAC_SEL_DAP);
            SGTL_ModifyReg(handle, CHIP_ANA_CTRL, SGTL5000_SEL_HP_CLR_MASK, SGTL5000_SEL_HP_DAC);
            break;
        case kSGTL_RoutePlaybackwithDAPandRecord:
            /* I2S_IN->DAP->DAC->HP,  LINE_IN->ADC->I2S_OUT */
            SGTL_EnableModule(handle, kSGTL_ModuleHP);
            SGTL_EnableModule(handle, kSGTL_ModuleDAC);
            SGTL_EnableModule(handle, kSGTL_ModuleI2SIN);
            SGTL_EnableModule(handle, kSGTL_ModuleI2SOUT);
            SGTL_EnableModule(handle, kSGTL_ModuleADC);
            SGTL_EnableModule(handle, kSGTL_ModuleDAP);
            SGTL_ModifyReg(handle, SGTL5000_DAP_CONTROL, SGTL5000_DAP_CONTROL_DAP_EN_CLR_MASK, 0x0001);
            SGTL_ModifyReg(handle, CHIP_SSS_CTRL, SGTL5000_DAP_SEL_CLR_MASK, SGTL5000_DAP_SEL_I2S_IN);
            SGTL_ModifyReg(handle, CHIP_SSS_CTRL, SGTL5000_DAC_SEL_CLR_MASK, SGTL5000_DAC_SEL_DAP);
            SGTL_ModifyReg(handle, CHIP_ANA_CTRL, SGTL5000_SEL_HP_CLR_MASK, SGTL5000_SEL_HP_DAC);
            SGTL_ModifyReg(handle, CHIP_ANA_CTRL, SGTL5000_SEL_ADC_CLR_MASK, SGTL5000_SEL_ADC_LINEIN);
            SGTL_ModifyReg(handle, CHIP_SSS_CTRL, SGTL5000_I2S_OUT_SEL_CLR_MASK, SGTL5000_I2S_OUT_SEL_ADC);
            break;
        case kSGTL_RouteRecord:
            /* LINE_IN->ADC->I2S_OUT */
            SGTL_EnableModule(handle, kSGTL_ModuleI2SOUT);
            SGTL_EnableModule(handle, kSGTL_ModuleADC);
            SGTL_ModifyReg(handle, CHIP_ANA_CTRL, SGTL5000_SEL_ADC_CLR_MASK, SGTL5000_SEL_ADC_LINEIN);
            SGTL_ModifyReg(handle, CHIP_SSS_CTRL, SGTL5000_I2S_OUT_SEL_CLR_MASK, SGTL5000_I2S_OUT_SEL_ADC);
            break;
        default:
            ret = kStatus_InvalidArgument;
            break;
    }
    return ret;
}

status_t SGTL_SetProtocol(sgtl_handle_t *handle, sgtl_protocol_t protocol)
{
    status_t ret = kStatus_Success;
    switch (protocol)
    {
        case kSGTL_BusI2S:
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_MODE_CLR_MASK, SGTL5000_I2S_MODE_I2S_LJ);
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_LRALIGN_CLR_MASK, SGTL5000_I2S_ONE_BIT_DELAY);
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_SCLK_INV_CLR_MASK, SGTL5000_I2S_VAILD_RISING_EDGE);
            break;
        case kSGTL_BusLeftJustified:
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_MODE_CLR_MASK, SGTL5000_I2S_MODE_I2S_LJ);
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_LRALIGN_CLR_MASK, SGTL5000_I2S_NO_DELAY);
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_SCLK_INV_CLR_MASK, SGTL5000_I2S_VAILD_RISING_EDGE);
            break;
        case kSGTL_BusRightJustified:
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_MODE_CLR_MASK, SGTL5000_I2S_MODE_RJ);
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_SCLK_INV_CLR_MASK, SGTL5000_I2S_VAILD_RISING_EDGE);
            break;
        case kSGTL_BusPCMA:
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_MODE_CLR_MASK, SGTL5000_I2S_MODE_PCM);
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_LRALIGN_CLR_MASK, SGTL5000_I2S_ONE_BIT_DELAY);
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_SCLK_INV_CLR_MASK, SGTL5000_I2S_VAILD_FALLING_EDGE);
            break;
        case kSGTL_BusPCMB:
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_MODE_CLR_MASK, SGTL5000_I2S_MODE_PCM);
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_LRALIGN_CLR_MASK, SGTL5000_I2S_NO_DELAY);
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_SCLK_INV_CLR_MASK, SGTL5000_I2S_VAILD_FALLING_EDGE);
            break;
        default:
            ret = kStatus_InvalidArgument;
            break;
    }
    return ret;
}

status_t SGTL_SetVolume(sgtl_handle_t *handle, sgtl_module_t module, uint32_t volume)
{
    uint16_t vol = 0;
    status_t ret = kStatus_Success;
    switch (module)
    {
        case kSGTL_ModuleADC:
            vol = volume | (volume << 4U);
            ret = SGTL_ModifyReg(handle, CHIP_ANA_ADC_CTRL,
                                 SGTL5000_ADC_VOL_LEFT_CLR_MASK & SGTL5000_ADC_VOL_RIGHT_CLR_MASK, vol);
            break;
        case kSGTL_ModuleDAC:
            vol = volume | (volume << 8U);
            ret = SGTL_WriteReg(handle, CHIP_DAC_VOL, vol);
            break;
        case kSGTL_ModuleHP:
            vol = volume | (volume << 8U);
            ret = SGTL_WriteReg(handle, CHIP_ANA_HP_CTRL, vol);
            break;
        case kSGTL_ModuleLineOut:
            vol = volume | (volume << 8U);
            ret = SGTL_WriteReg(handle, CHIP_LINE_OUT_VOL, vol);
            break;
        default:
            ret = kStatus_InvalidArgument;
            break;
    }
    return ret;
}

uint32_t SGTL_GetVolume(sgtl_handle_t *handle, sgtl_module_t module)
{
    uint16_t vol = 0;
    switch (module)
    {
        case kSGTL_ModuleADC:
            SGTL_ReadReg(handle, CHIP_ANA_ADC_CTRL, &vol);
            vol = (vol & (uint16_t)SGTL5000_ADC_VOL_LEFT_GET_MASK) >> SGTL5000_ADC_VOL_LEFT_SHIFT;
            break;
        case kSGTL_ModuleDAC:
            SGTL_ReadReg(handle, CHIP_DAC_VOL, &vol);
            vol = (vol & (uint16_t)SGTL5000_DAC_VOL_LEFT_GET_MASK) >> SGTL5000_DAC_VOL_LEFT_SHIFT;
            break;
        case kSGTL_ModuleHP:
            SGTL_ReadReg(handle, CHIP_ANA_HP_CTRL, &vol);
            vol = (vol & (uint16_t)SGTL5000_HP_VOL_LEFT_GET_MASK) >> SGTL5000_HP_VOL_LEFT_SHIFT;
            break;
        case kSGTL_ModuleLineOut:
            SGTL_ReadReg(handle, CHIP_LINE_OUT_VOL, &vol);
            vol = (vol & (uint16_t)SGTL5000_LINE_OUT_VOL_LEFT_GET_MASK) >> SGTL5000_LINE_OUT_VOL_LEFT_SHIFT;
            break;
        default:
            vol = 0;
            break;
    }
    return vol;
}

status_t SGTL_SetMute(sgtl_handle_t *handle, sgtl_module_t module, bool mute)
{
    status_t ret = kStatus_Success;
    switch (module)
    {
        case kSGTL_ModuleADC:
            SGTL_ModifyReg(handle, CHIP_ANA_CTRL, SGTL5000_MUTE_ADC_CLR_MASK, mute);
            break;
        case kSGTL_ModuleDAC:
            if (mute)
            {
                SGTL_ModifyReg(handle, CHIP_ADCDAC_CTRL,
                               SGTL5000_DAC_MUTE_LEFT_CLR_MASK & SGTL5000_DAC_MUTE_RIGHT_CLR_MASK, 0x000C);
            }
            else
            {
                SGTL_ModifyReg(handle, CHIP_ADCDAC_CTRL,
                               SGTL5000_DAC_MUTE_LEFT_CLR_MASK & SGTL5000_DAC_MUTE_RIGHT_CLR_MASK, 0x0000);
            }
            break;
        case kSGTL_ModuleHP:
            SGTL_ModifyReg(handle, CHIP_ANA_CTRL, SGTL5000_MUTE_HP_CLR_MASK,
                           ((uint16_t)mute << SGTL5000_MUTE_HP_SHIFT));
            break;
        case kSGTL_ModuleLineOut:
            SGTL_ModifyReg(handle, CHIP_ANA_CTRL, SGTL5000_MUTE_LO_CLR_MASK,
                           ((uint16_t)mute << SGTL5000_MUTE_LO_SHIFT));
            break;
        default:
            ret = kStatus_InvalidArgument;
            break;
    }
    return ret;
}

status_t SGTL_ConfigDataFormat(sgtl_handle_t *handle, uint32_t mclk, uint32_t sample_rate, uint32_t bits)
{
    uint16_t val     = 0;
    uint16_t regVal  = 0;
    uint16_t mul_clk = 0U;
    uint32_t sysFs   = 0U;

    /* Over sample rate can only up to 512, the least to 8k */
    if ((mclk / (MIN(sample_rate * 6U, 96000U)) > 512U) || (mclk / sample_rate < 256U))
    {
        return kStatus_InvalidArgument;
    }

    /* Configure the sample rate */
    switch (sample_rate)
    {
        case 8000:
            if (mclk > 32000U * 512U)
            {
                val   = 0x0038;
                sysFs = 48000;
            }
            else
            {
                val   = 0x0020;
                sysFs = 32000;
            }
            break;
        case 11025:
            val   = 0x0024;
            sysFs = 44100;
            break;
        case 12000:
            val   = 0x0028;
            sysFs = 48000;
            break;
        case 16000:
            if (mclk > 32000U * 512U)
            {
                val   = 0x003C;
                sysFs = 96000;
            }
            else
            {
                val   = 0x0010;
                sysFs = 32000;
            }
            break;
        case 22050:
            val   = 0x0014;
            sysFs = 44100;
            break;
        case 24000:
            if (mclk > 48000U * 512U)
            {
                val   = 0x002C;
                sysFs = 96000;
            }
            else
            {
                val   = 0x0018;
                sysFs = 48000;
            }
            break;
        case 32000:
            val   = 0x0000;
            sysFs = 32000;
            break;
        case 44100:
            val   = 0x0004;
            sysFs = 44100;
            break;
        case 48000:
            if (mclk > 48000U * 512U)
            {
                val   = 0x001C;
                sysFs = 96000;
            }
            else
            {
                val   = 0x0008;
                sysFs = 48000;
            }
            break;
        case 96000:
            val   = 0x000C;
            sysFs = 96000;
            break;
        default:
            return kStatus_InvalidArgument;
    }

    SGTL_ReadReg(handle, CHIP_I2S_CTRL, &regVal);

    /* While as slave, Fs is input */
    if ((regVal & SGTL5000_I2S_MS_GET_MASK) == 0U)
    {
        sysFs = sample_rate;
    }
    mul_clk = mclk / sysFs;
    /* Configure the mul_clk. Sgtl-5000 only support 256, 384 and 512 oversample rate */
    if ((mul_clk / 128U - 2U) > 2U)
    {
        return kStatus_InvalidArgument;
    }
    else
    {
        val |= (mul_clk / 128U - 2U);
    }
    SGTL_WriteReg(handle, CHIP_CLK_CTRL, val);

    /* Data bits configure,sgtl supports 16bit, 20bit 24bit, 32bit */
    SGTL_ModifyReg(handle, CHIP_I2S_CTRL, 0xFEFF, SGTL5000_I2S_SCLKFREQ_64FS);
    switch (bits)
    {
        case 16:
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_DLEN_CLR_MASK, SGTL5000_I2S_DLEN_16);
            break;
        case 20:
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_DLEN_CLR_MASK, SGTL5000_I2S_DLEN_20);
            break;
        case 24:
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_DLEN_CLR_MASK, SGTL5000_I2S_DLEN_24);
            break;
        case 32:
            SGTL_ModifyReg(handle, CHIP_I2S_CTRL, SGTL5000_I2S_DLEN_CLR_MASK, SGTL5000_I2S_DLEN_32);
            break;
        default:
            return kStatus_InvalidArgument;
    }
    return kStatus_Success;
}

status_t SGTL_SetPlay(sgtl_handle_t *handle, uint32_t playSource)
{
    uint16_t regValue = 0U, regBitMask = 0x40U;

    /* headphone source form PGA */
    if (playSource == kSGTL_PlaySourceLineIn)
    {
        regValue = 0x40U;
    }
    /* headphone source from DAC */
    else
    {
        regValue = 0U;
    }

    return SGTL_ModifyReg(handle, CHIP_ANA_CTRL, regBitMask, regValue);
}

status_t SGTL_SetRecord(sgtl_handle_t *handle, uint32_t recordSource)
{
    uint16_t regValue = 0U, regBitMask = 0x4U;

    /* ADC source form LINEIN */
    if (recordSource == kSGTL_RecordSourceLineIn)
    {
        regValue = 0x4U;
    }
    /* ADC source from MIC */
    else
    {
        regValue = 0U;
    }

    return SGTL_ModifyReg(handle, CHIP_ANA_CTRL, regBitMask, regValue);
}

status_t SGTL_WriteReg(sgtl_handle_t *handle, uint16_t reg, uint16_t val)
{
	for (int i = 0; i < 10000; i++) { __NOP(); }
    uint16_t writeValue = SGTL_SWAP_UINT16_BYTE_SEQUENCE(val);

    return CODEC_I2C_Send(handle->i2cHandle, handle->config->slaveAddress, reg, 2U, (uint8_t *)&writeValue, 2U);
}

status_t SGTL_ReadReg(sgtl_handle_t *handle, uint16_t reg, uint16_t *val)
{
	for (int i = 0; i < 10000; i++) { __NOP(); }
    status_t retval = 0;

    uint16_t readValue = 0U;

    retval = CODEC_I2C_Receive(handle->i2cHandle, handle->config->slaveAddress, reg, 2U, (uint8_t *)&readValue, 2U);

    *val = SGTL_SWAP_UINT16_BYTE_SEQUENCE(readValue);

    return retval;
}

status_t SGTL_ModifyReg(sgtl_handle_t *handle, uint16_t reg, uint16_t clr_mask, uint16_t val)
{
    status_t retval = 0;
    uint16_t reg_val;

    /* Read the register value out */
    for (int i = 0; i < 10000; i++) { __NOP(); }
    retval = SGTL_ReadReg(handle, reg, &reg_val);
    if (retval != kStatus_Success)
    {
        return kStatus_Fail;
    }

    /* Modify the value */
    reg_val &= clr_mask;
    reg_val |= val;

    /* Write the data to register */
    for (int i = 0; i < 10000; i++) { __NOP(); }
    retval = SGTL_WriteReg(handle, reg, reg_val);
    if (retval != kStatus_Success)
    {
        return kStatus_Fail;
    }

    return kStatus_Success;
}
