/* NXP i.MX8MP with MCUXpresso SDK */
// #include <core_cm7.h>
#include <csp/csp_hooks.h>

void csp_reboot_hook(void) {
    // NVIC_SystemReset();
}

void csp_shutdown_hook(void) {
    // NVIC_SystemReset();
}
