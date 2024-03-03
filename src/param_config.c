#include "param_config.h"
#include "vmem_config.h"
#include "gnss.h"
#include "tmu.h"
#include "wake_a53_service.h"

/*
PARAM_DEFINE_STATIC_VMEM(
    id,           // unique parameter id, from param_config.h
    name,         // param_t variable name, from param_config.h
    type,         // Available types in enum param_type_e in param.h
    array_count,  // Number of elements/(bytes?) in the array. -1 for single values
    array_step,   // ??maybe element size in bytes. 0 for single values, 1 for arrays
    flags,        // See param.h; PM_SYSCONF for system/network config, PM_CONF for user config, PM_READONLY for read-only
    callback,     // Callback function for when the value is set
    unit,         // ?? Possibly the unit of the value, e.g. "m/s"
    vmem_name,    // Name of the vmem, without "vmem_" prefix Defined in vmem_config.h. Initialized in main.c
    vmem_address, // Offset in the vmem, from vmem_config.h
    docstr        // Documentation string (for param info)
)
*/

PARAM_DEFINE_STATIC_VMEM(
    PARAMID_GNSS_READING,
    gnss_reading,
    PARAM_TYPE_DOUBLE, // Verify double-precision float works
    8, // 2 doubles
    1,
    PM_READONLY,
    NULL,
    "N, E",
    config,
    VMEM_CONF_GNSS_READING,
    "GNSS GPS coordinates"
);

PARAM_DEFINE_STATIC_VMEM(
    PARAMID_TMU_READING,
    tmu_reading,
    PARAM_TYPE_UINT16,
    -1,
    0,
    PM_READONLY,
    NULL,
    "K",
    config,
    VMEM_CONF_TMU_READING,
    "TMU temperature reading"
);

PARAM_DEFINE_STATIC_VMEM(
    PARAMID_WAKE_A53,
    wake_a53,
    PARAM_TYPE_UINT8,
    -1,
    0,
    PM_CONF,
    wake_a53_callback,
    NULL,
    config,
    VMEM_CONF_WAKE_A53,
    "Send wake signal to A53 peer core"
);
