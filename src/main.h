//! @file
//! @date Apr 12, 2019
//! @author Marek Bel

#pragma once

#include <stdint.h>
#include "LiquidCrystal_Prusa.h"

extern volatile uint8_t rotary_diff;
extern LiquidCrystal_Prusa lcd;

void print_menu_cursor(uint8_t line);
