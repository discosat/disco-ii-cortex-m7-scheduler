#pragma once

#include <vmem/vmem.h>

extern vmem_t vmem_config;

#define VMEM_CONF_GNSS_READING 0x00    // 8 bytes
#define VMEM_CONF_TMU_READING 0x08    // 2 bytes
#define VMEM_CONF_WAKE_A53 0x10    // 1 byte
