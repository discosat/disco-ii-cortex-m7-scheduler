#include <param/param.h>

uint32_t _serial0;

#define PARAMID_SERIAL0 61

PARAM_DEFINE_STATIC_RAM(PARAMID_SERIAL0, serial0, PARAM_TYPE_INT32, -1, 0, PM_HWREG, NULL, "", &_serial0, NULL);

void serial_init(void) {
	_serial0 = __TIME__[7] - '0' + (__TIME__[6] - '0') * 10 + (__TIME__[4] - '0') * 60 + (__TIME__[3] - '0') * 600 + (__TIME__[1] - '0') * 3600 + (__TIME__[0] - '0') * 36000;
}

uint32_t serial_get(void) {
	return _serial0;
}
