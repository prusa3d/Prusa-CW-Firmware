#pragma once

#ifdef CW1_HW
#include "cw1.h"
#define MAX_TARGET_TEMP_C			40
#endif

#ifdef CW1S_HW
#include "cw1s.h"
#define MAX_TARGET_TEMP_C			60
#define HEATING_ON_FAN1_SPEED		100
#endif

#define MIN_TARGET_TEMP_C			20

#define MIN_TARGET_TEMP_F	MIN_TARGET_TEMP_C * 1.8 + 32
#define MAX_TARGET_TEMP_F	MAX_TARGET_TEMP_C * 1.8 + 32
