#pragma once

#include <vmem/vmem.h>

extern vmem_t vmem_config;

#define VMEM_CONF_GNSS_READING 0x00    // 2 double = 16 bytes
#define VMEM_CONF_TMU_READING 0x10    // 1 uint16 = 2 bytes
#define VMEM_CONF_WAKE_A53 0x12    // 1 uint8 = 1 byte
#define VMEM_CONF_A53_STATUS 0x13    // 1 uint8 = 1 byte
