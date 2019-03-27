/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// 100k ParCan thermistor (104GT-2)
// ATC Semitec 104GT-2 (Used in ParCan)
// Verified by linagee. Source: http://shop.arcol.hu/static/datasheets/thermistors.pdf
// Calculated using 4.7kohm pullup, voltage divider math, and manufacturer provided temp/resistance
const short temptable_5[][2] PROGMEM = {
  { OV(   25), 125 },
  { OV(  29), 120 }, // top rating 300C
  { OV(  34), 115 },
  { OV(  40), 110 },
  { OV(  46), 105 },
  { OV(  54), 100 },
  { OV(  64), 95 },
  { OV(  75), 90 },
  { OV(  88), 85 },
  { OV(  105), 80 },
  { OV(  124), 75 },
  { OV(  146), 70 },
  { OV( 173), 65 },
  { OV( 204), 60 },
  { OV( 241), 55 },
  { OV( 282), 50 },
  { OV( 330), 45 },
  { OV( 382), 40 },
  { OV( 439), 35 },
  { OV( 500), 30 },
  { OV( 563), 25 },
  { OV( 625), 20 },
  { OV( 687),  15 },
  { OV( 744),  10 },
  { OV( 796),  5 },
  { OV( 842),  0 },
  { OV( 882),  -5 },
  { OV( 915),  -10 },
  { OV( 941),  -15 },
  { OV( 963),  -20 },
  { OV(979),  -25 },
  { OV(992),  -30 },
  { OV(1001),  -35 },
  { OV(1008),  -40 },
  { OV(1023),   -50 }
};
/*
const short temptable_5[][2] PROGMEM = {
  { OV(   1), 713 },
  { OV(  17), 300 }, // top rating 300C
  { OV(  20), 290 },
  { OV(  23), 280 },
  { OV(  27), 270 },
  { OV(  31), 260 },
  { OV(  37), 250 },
  { OV(  43), 240 },
  { OV(  51), 230 },
  { OV(  61), 220 },
  { OV(  73), 210 },
  { OV(  87), 200 },
  { OV( 106), 190 },
  { OV( 128), 180 },
  { OV( 155), 170 },
  { OV( 189), 160 },
  { OV( 230), 150 },
  { OV( 278), 140 },
  { OV( 336), 130 },
  { OV( 402), 120 },
  { OV( 476), 110 },
  { OV( 554), 100 },
  { OV( 635),  90 },
  { OV( 713),  80 },
  { OV( 784),  70 },
  { OV( 846),  60 },
  { OV( 897),  50 },
  { OV( 937),  40 },
  { OV( 966),  30 },
  { OV( 986),  20 },
  { OV(1000),  10 },
  { OV(1010),   0 }
};
*/