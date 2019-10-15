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

    enum class Terminator : uint_least8_t
    {
        none,
        back,
        right,
        serialNumber,
    };

    //! Print n characters from null terminated string c
    //! if there are not enough characters, prints ' ' for remaining n.
    //!
    //! @param c null terminated string to print
    //! @param n number of characters to print or clear
    //! ignored for terminator Terminator::serialNumber - prints always 18 characters
    //! @param terminator additional symbol to be printed
    //!  * Terminator::none none
    //!  * Terminator::back back arrow
    //!  * Terminator::right right arrow
    //!  * Terminator::serialNumber none
    void printClear_P(const char *c, uint_least8_t n, Terminator terminator)
    {
        if (terminator == Terminator::serialNumber)
        {
            print('S');
            print('N');
            print(':');
            n = 15;
        }
        else if (terminator != Terminator::none) --n;

        for (uint_least8_t i = 0; i < n; ++i)
        {
            if ((char)pgm_read_byte(c))
            {
                print((char)pgm_read_byte(c));
                ++c;
            }
            else
            {
                print(' ');
            }
        }
        if (terminator == Terminator::back) print(char(0));
        if (terminator == Terminator::right) print(char(1));
    }

};


#endif /* PRUSALCD_H */
