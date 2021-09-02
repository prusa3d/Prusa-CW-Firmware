#include "wrappers.h"
#include "device.h"

bool is_cover_closed() {
	return hw.is_cover_closed();
}

bool is_tank_inserted() {
	return hw.is_tank_inserted();
}

