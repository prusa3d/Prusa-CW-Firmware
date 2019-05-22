//! @file
//! @date Apr 12, 2019
//! @author Marek Bel

#ifndef CW_MAIN_H
#define CW_MAIN_H
#include <stdint.h>
#include "PrusaLcd.h"

extern uint8_t menu_position;
extern volatile uint8_t rotary_diff;
extern PrusaLcd lcd;

void print_menu_cursor(uint8_t line);


#endif /* CW_MAIN_H */
