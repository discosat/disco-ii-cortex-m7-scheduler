#include <vmem/vmem_ram.h>
#include <param/param_commands.h>
#include <param/param_scheduler.h>

VMEM_DEFINE_STATIC_RAM(config, "config", 2500);
VMEM_DEFINE_STATIC_RAM(commands, "commands", 1500);
VMEM_DEFINE_STATIC_RAM(schedule, "schedule", 1500);
