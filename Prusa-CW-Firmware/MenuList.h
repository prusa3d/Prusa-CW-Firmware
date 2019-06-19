//! @file
//! @date Apr 12, 2019
//! @author Marek Bel

#ifndef MENULIST_H
#define MENULIST_H

#include "PrusaLcd.h"
#include <stdint.h>

struct Scrolling_item
{
   const char *caption;
   bool visible;
   PrusaLcd::Terminator last_symbol;
};

typedef Scrolling_item Scrolling_items[9];

uint_least8_t scrolling_list(const Scrolling_items &items);
void scrolling_list_set(uint8_t index);

#endif /* MENULIST_H */
