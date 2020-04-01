//intpol.c - linear interpolation functions
#include "intpol.h"
#include <avr/pgmspace.h>


int16_t interpolate_i16_ylin_P(int16_t x, uint8_t c, const int16_t* px, int16_t y0, int16_t yd)
{
	int16_t x0;
	int16_t x1;
	int16_t y;
	uint8_t i = c >> 1;
	uint8_t j = c;
	int16_t imin = 0;
	int16_t imax = c - 1;
	if (x >= (int16_t)pgm_read_word(px + imax)) return y0 + imax * yd;
	if (x <= (int16_t)pgm_read_word(px + imin)) return y0;
	while (j--)
	{
		x0 = pgm_read_word(px + i);
		x1 = pgm_read_word(px + i + 1);
		if (x > x1)
		{
			imin = i;
			i = ((i + imax) >> 1);
		}
		else if (x < x0)
		{
			imax = i;
			i = ((i + imin) >> 1);
		}
		else
		{
			y = y0 + i * yd + (x - x0) * yd / (x1 - x0);
			return y;
		}
	}
	return 0;
}
