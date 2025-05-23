/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 * Copyright 2015-2021 NXP.
 *
 * License: LA_OPT_NXP_Software_License
 *
 * This software is owned or controlled by NXP and may
 * only be used strictly in accordance with the applicable license terms.
 * By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that
 * you have read, and that you agree to comply with and are bound by,
 * such license terms. If you do not agree to be bound by the applicable
 * license terms, then you may not retain, install, activate or otherwise
 * use the software.
 *
 * @version 4.1
 *
 * @brief This file contains macros used by the IAR, KEIL and MCUX assembler.
 */

#ifndef ASM_MAC_COMMON_H__
#define ASM_MAC_COMMON_H__

#if defined(__IAR_SYSTEMS_ICC__) || defined(__IAR_SYSTEMS_ASM__) /* IAR */
    #define ASM_PREFIX(x)         x
    #define ASM_LABEL(value)      value
    #define ASM_ALIGN(value)      ALIGNROM  value/4
    #define ASM_PUBLIC(label)     PUBLIC label
    #define ASM_RAM_SECTION(label)   SECTION label : DATA (4)
    #define ASM_CODE_SECTION(label)  SECTION label : CODE (4)
    #define ASM_IMPORT_FUNC(label)   import  label
    #define ASM_END                  END
    #define ASM_COMP_SPECIFIC_DIRECTIVES
    #define ASM_OFFSET     1

    /* Note that these macros should NOT be on the beggining of line when used
     * in assembler code. Prepend allways by at least one space.
     * (was not an issue in EWARM 6.40.x, space seems to be needed in 6.50.x) */
    #define ASM_APSR_MASK
    #define ASM_LITERAL     LTORG
    #define ASM_PUBLIC_BEGIN(name)
    #define ASM_PUBLIC_FUNC(name)
    #define ASM_PUBLIC_END(name)

    #define ASM_STATIC_BEGIN(name)
    #define ASM_STATIC_FUNC(name)
    #define ASM_STATIC_END(name)

    #define ASM_INLINE(value) __asm(value)

#elif defined(__GNUC__) /* For GCC compiler, also for armClang (GCC) */
    #define ASM_PREFIX(x)   x
    #define ASM_ALIGN(value)        .balign value
    #define ASM_PUBLIC(label)       .global ASM_PREFIX(label)
    #define ASM_LABEL(label)        label:
    #define ASM_OFFSET              0

    #define ASM_CODE_SECTION(name)  .section name
    #define ASM_END                 .end

    #define ASM_COMP_SPECIFIC_DIRECTIVES   .syntax unified
    #define ASM_REG(APSR)   APSR_nzcvq

    #define ASM_LITERAL
    #define ASM_PUBLIC_BEGIN(name) .thumb_func
    #define ASM_PUBLIC_FUNC(name)
    #define ASM_PUBLIC_END(name)

    #define ASM_INLINE(value) __asm(value)
#endif

#endif /* ASM_MAC_COMMON_H__ */
