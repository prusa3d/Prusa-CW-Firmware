// intpol.h - linear interpolation functions
#ifndef _INTPOL_H
#define _INTPOL_H
#include <inttypes.h>


#if defined(__cplusplus)
extern "C" {
#endif //defined(__cplusplus)

extern int16_t interpolate_i16_ylin_P(int16_t x, uint8_t c, const int16_t* px, int16_t y0, int16_t yd);

#if defined(__cplusplus)
}
#endif //defined(__cplusplus)

#endif //_INTPOL_H
