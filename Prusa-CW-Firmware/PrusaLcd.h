//! @file
//! @date Apr 12, 2019
//! @author Marek Bel

#ifndef PRUSALCD_H
#define PRUSALCD_H

//#include <LiquidCrystal.h>
#include "LiquidCrystal_Prusa.h"

class PrusaLcd : public LiquidCrystal_Prusa
{
public:
    using LiquidCrystal_Prusa::LiquidCrystal_Prusa;

    //! Print n characters from null terminated string c
    //! if there are not enough characters, prints ' ' for remaining n.
    void printClear(const char *c, uint_least8_t n)
    {
        for (uint_least8_t i = 0; i < n; ++i)
        {
            if (*c)
            {
                print(*c);
                ++c;
            }
            else
            {
                print(' ');
            }
        }
    }

};


#endif /* PRUSALCD_H */
