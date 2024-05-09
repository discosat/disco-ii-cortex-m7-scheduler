#pragma once

#include <vmem/vmem.h>

extern vmem_t vmem_config;

#define VMEM_CONF_WAKE_A53 0x00    // 1 uint8 = 1 byte
#define VMEM_CONF_A53_STATUS 0x01    // 1 uint8 = 1 byte
#define VMEM_CONF_TMU_READING 0x02    // 1 uint16 = 2 bytes
#define VMEM_GNSS_LAT 0x04    // 1 float = 4 bytes
#define VMEM_GNSS_LON 0x08    // 1 float = 4 bytes
#define VMEM_GNSS_AGE 0x0C    // 1 uint32 = 4 bytes
#define VMEM_GNSS_DATE 0x10    // 1 uint32 = 4 bytes
#define VMEM_GNSS_TIME 0x14    // 1 uint32 = 4 bytes
#define VMEM_GNSS_SPEED 0x18    // 1 float = 4 bytes
#define VMEM_GNSS_ALT 0x1C    // 1 float = 4 bytes
#define VMEM_GNSS_COURSE 0x20    // 1 float = 4 bytes
