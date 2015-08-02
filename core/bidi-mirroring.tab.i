/*************************************************************************/
/*  bidi-mirroring.tab.i                                                 */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2015 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

/**
 * This class derived from fribidi library code with some bug fixes.
 * https://github.com/behdad/fribidi
 * You can't compile this class statically in commerical games
 * because fribidi licensed under LGPL . 
 * https://github.com/behdad/fribidi/blob/master/COPYING
 * this class for demo. soon we impelement bidi algorithm from 
 * unicode site:  http://unicode.org/reports/tr9/ 
 * and change this class from base. 
 * this class is only a interface.
 * 
 * @author Masoud BaniHashemian <masoudbh3@gmail.com>
*/

/*
  assumed sizeof(signed short): 2
  required memory: 5504
  lookups: 2
  partition shape: Mir[1024][64]
  different table entries: 1 27
*/

#define PACKTAB_UINT8 uint8_t
#define PACKTAB_UINT16 uint16_t
#define PACKTAB_UINT32 uint32_t


static const signed short MirLev1[64*27] = {
#define MirLev1_0000 0x0
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,-2,0,
#define MirLev1_0040 0x40
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,-2,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,-2,0,0,
#define MirLev1_0080 0x80
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-16,0,0,0,0,
#define MirLev1_00C0 0xC0
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
#define MirLev1_0F00 0x100
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,-1,1,-1,0,0,
#define MirLev1_1680 0x140
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,-1,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
#define MirLev1_2000 0x180
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,-1,0,0,0,0,0,
#define MirLev1_2040 0x1C0
  0,0,0,0,0,1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,-1,0,
#define MirLev1_2080 0x200
  0,0,0,0,0,0,0,0,0,0,0,0,0,1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
#define MirLev1_2200 0x240
  0,0,0,0,0,0,0,0,3,3,3,-3,-3,-3,0,0,0,0,0,0,0,2016,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,-1,0,0,
#define MirLev1_2240 0x280
  0,0,0,138,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,-1,1,-1,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,1,-1,1,-1,1,-1,1,-1,0,0,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,
#define MirLev1_2280 0x2C0
  1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,0,0,0,1,-1,1,-1,0,0,0,0,0,1824,0,0,0,0,0,0,0,
  0,0,1,-1,0,0,2104,0,2108,2106,0,2106,0,0,0,0,1,-1,1,-1,1,-1,1,-1,0,0,0,0,0,0,0,0,
#define MirLev1_22C0 0x300
  0,0,0,0,0,0,0,0,0,1,-1,1,-1,-138,0,0,1,-1,0,0,0,0,1,-1,1,-1,1,-1,1,-1,1,-1,
  1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,0,0,1,-1,8,8,8,0,7,7,0,0,-8,-8,-8,-7,-7,0,
#define MirLev1_2300 0x340
  0,0,0,0,0,0,0,0,1,-1,1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
#define MirLev1_2740 0x380
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,0,0,0,0,0,0,0,0,0,0,
#define MirLev1_27C0 0x3C0
  0,0,0,1,-1,1,-1,0,1,-1,0,2,0,-2,0,0,0,0,0,0,0,1,-1,0,0,0,0,0,0,1,-1,0,
  0,0,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
#define MirLev1_2980 0x400
  0,0,0,1,-1,1,-1,1,-1,1,-1,1,-1,3,1,-1,-3,1,-1,1,-1,1,-1,1,-1,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1824,0,0,0,0,0,0,0,
#define MirLev1_29C0 0x440
  1,-1,0,0,1,-1,0,0,0,0,0,0,0,0,0,1,-1,1,-1,0,1,-1,0,0,1,-1,1,-1,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-2016,0,0,1,-1,0,0,1,-1,0,0,
#define MirLev1_2A00 0x480
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,1,-1,1,-1,0,0,0,0,0,1,-1,0,0,0,0,0,0,1,-1,0,0,
#define MirLev1_2A40 0x4C0
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,-1,0,0,1,-1,1,
#define MirLev1_2A80 0x500
  -1,1,-1,1,-1,0,0,0,0,0,0,1,-1,0,0,0,0,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,0,0,0,
  0,1,-1,0,0,0,1,-1,1,-1,1,-1,1,-1,0,1,-1,0,0,1,-1,0,0,0,0,0,0,1,-1,1,-1,1,
#define MirLev1_2AC0 0x540
  -1,1,-1,1,-1,1,-1,0,0,0,0,0,0,1,-1,1,-1,1,-1,1,-1,1,-1,0,0,0,0,0,0,0,-2104,0,
  0,0,0,-2106,-2108,-2106,0,0,0,0,0,0,1,-1,0,0,0,0,0,0,0,0,0,1,-1,1,-1,0,0,0,0,0,
#define MirLev1_2E00 0x580
  0,0,1,-1,1,-1,0,0,0,1,-1,0,1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,-1,0,0,
  1,-1,1,-1,1,-1,1,-1,1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
#define MirLev1_3000 0x5C0
  0,0,0,0,0,0,0,0,1,-1,1,-1,1,-1,1,-1,1,-1,0,0,1,-1,1,-1,1,-1,1,-1,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
#define MirLev1_FE40 0x600
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,-1,1,-1,1,-1,0,
  0,0,0,0,1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
#define MirLev1_FF00 0x640
  0,0,0,0,0,0,0,0,1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,-2,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,-2,0,0,
#define MirLev1_FF40 0x680
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,-2,0,1,
  -1,0,1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

static const PACKTAB_UINT16 MirLev0[1024*1] = {
#define MirLev0_0000 0x0
  MirLev1_0000,  /* 0000..003F */
  MirLev1_0040,  /* 0040..007F */
  MirLev1_0080,  /* 0080..00BF */
  MirLev1_00C0,  /* 00C0..00FF */
  MirLev1_00C0,  /* 0100..013F */
  MirLev1_00C0,  /* 0140..017F */
  MirLev1_00C0,  /* 0180..01BF */
  MirLev1_00C0,  /* 01C0..01FF */
  MirLev1_00C0,  /* 0200..023F */
  MirLev1_00C0,  /* 0240..027F */
  MirLev1_00C0,  /* 0280..02BF */
  MirLev1_00C0,  /* 02C0..02FF */
  MirLev1_00C0,  /* 0300..033F */
  MirLev1_00C0,  /* 0340..037F */
  MirLev1_00C0,  /* 0380..03BF */
  MirLev1_00C0,  /* 03C0..03FF */
  MirLev1_00C0,  /* 0400..043F */
  MirLev1_00C0,  /* 0440..047F */
  MirLev1_00C0,  /* 0480..04BF */
  MirLev1_00C0,  /* 04C0..04FF */
  MirLev1_00C0,  /* 0500..053F */
  MirLev1_00C0,  /* 0540..057F */
  MirLev1_00C0,  /* 0580..05BF */
  MirLev1_00C0,  /* 05C0..05FF */
  MirLev1_00C0,  /* 0600..063F */
  MirLev1_00C0,  /* 0640..067F */
  MirLev1_00C0,  /* 0680..06BF */
  MirLev1_00C0,  /* 06C0..06FF */
  MirLev1_00C0,  /* 0700..073F */
  MirLev1_00C0,  /* 0740..077F */
  MirLev1_00C0,  /* 0780..07BF */
  MirLev1_00C0,  /* 07C0..07FF */
  MirLev1_00C0,  /* 0800..083F */
  MirLev1_00C0,  /* 0840..087F */
  MirLev1_00C0,  /* 0880..08BF */
  MirLev1_00C0,  /* 08C0..08FF */
  MirLev1_00C0,  /* 0900..093F */
  MirLev1_00C0,  /* 0940..097F */
  MirLev1_00C0,  /* 0980..09BF */
  MirLev1_00C0,  /* 09C0..09FF */
  MirLev1_00C0,  /* 0A00..0A3F */
  MirLev1_00C0,  /* 0A40..0A7F */
  MirLev1_00C0,  /* 0A80..0ABF */
  MirLev1_00C0,  /* 0AC0..0AFF */
  MirLev1_00C0,  /* 0B00..0B3F */
  MirLev1_00C0,  /* 0B40..0B7F */
  MirLev1_00C0,  /* 0B80..0BBF */
  MirLev1_00C0,  /* 0BC0..0BFF */
  MirLev1_00C0,  /* 0C00..0C3F */
  MirLev1_00C0,  /* 0C40..0C7F */
  MirLev1_00C0,  /* 0C80..0CBF */
  MirLev1_00C0,  /* 0CC0..0CFF */
  MirLev1_00C0,  /* 0D00..0D3F */
  MirLev1_00C0,  /* 0D40..0D7F */
  MirLev1_00C0,  /* 0D80..0DBF */
  MirLev1_00C0,  /* 0DC0..0DFF */
  MirLev1_00C0,  /* 0E00..0E3F */
  MirLev1_00C0,  /* 0E40..0E7F */
  MirLev1_00C0,  /* 0E80..0EBF */
  MirLev1_00C0,  /* 0EC0..0EFF */
  MirLev1_0F00,  /* 0F00..0F3F */
  MirLev1_00C0,  /* 0F40..0F7F */
  MirLev1_00C0,  /* 0F80..0FBF */
  MirLev1_00C0,  /* 0FC0..0FFF */
  MirLev1_00C0,  /* 1000..103F */
  MirLev1_00C0,  /* 1040..107F */
  MirLev1_00C0,  /* 1080..10BF */
  MirLev1_00C0,  /* 10C0..10FF */
  MirLev1_00C0,  /* 1100..113F */
  MirLev1_00C0,  /* 1140..117F */
  MirLev1_00C0,  /* 1180..11BF */
  MirLev1_00C0,  /* 11C0..11FF */
  MirLev1_00C0,  /* 1200..123F */
  MirLev1_00C0,  /* 1240..127F */
  MirLev1_00C0,  /* 1280..12BF */
  MirLev1_00C0,  /* 12C0..12FF */
  MirLev1_00C0,  /* 1300..133F */
  MirLev1_00C0,  /* 1340..137F */
  MirLev1_00C0,  /* 1380..13BF */
  MirLev1_00C0,  /* 13C0..13FF */
  MirLev1_00C0,  /* 1400..143F */
  MirLev1_00C0,  /* 1440..147F */
  MirLev1_00C0,  /* 1480..14BF */
  MirLev1_00C0,  /* 14C0..14FF */
  MirLev1_00C0,  /* 1500..153F */
  MirLev1_00C0,  /* 1540..157F */
  MirLev1_00C0,  /* 1580..15BF */
  MirLev1_00C0,  /* 15C0..15FF */
  MirLev1_00C0,  /* 1600..163F */
  MirLev1_00C0,  /* 1640..167F */
  MirLev1_1680,  /* 1680..16BF */
  MirLev1_00C0,  /* 16C0..16FF */
  MirLev1_00C0,  /* 1700..173F */
  MirLev1_00C0,  /* 1740..177F */
  MirLev1_00C0,  /* 1780..17BF */
  MirLev1_00C0,  /* 17C0..17FF */
  MirLev1_00C0,  /* 1800..183F */
  MirLev1_00C0,  /* 1840..187F */
  MirLev1_00C0,  /* 1880..18BF */
  MirLev1_00C0,  /* 18C0..18FF */
  MirLev1_00C0,  /* 1900..193F */
  MirLev1_00C0,  /* 1940..197F */
  MirLev1_00C0,  /* 1980..19BF */
  MirLev1_00C0,  /* 19C0..19FF */
  MirLev1_00C0,  /* 1A00..1A3F */
  MirLev1_00C0,  /* 1A40..1A7F */
  MirLev1_00C0,  /* 1A80..1ABF */
  MirLev1_00C0,  /* 1AC0..1AFF */
  MirLev1_00C0,  /* 1B00..1B3F */
  MirLev1_00C0,  /* 1B40..1B7F */
  MirLev1_00C0,  /* 1B80..1BBF */
  MirLev1_00C0,  /* 1BC0..1BFF */
  MirLev1_00C0,  /* 1C00..1C3F */
  MirLev1_00C0,  /* 1C40..1C7F */
  MirLev1_00C0,  /* 1C80..1CBF */
  MirLev1_00C0,  /* 1CC0..1CFF */
  MirLev1_00C0,  /* 1D00..1D3F */
  MirLev1_00C0,  /* 1D40..1D7F */
  MirLev1_00C0,  /* 1D80..1DBF */
  MirLev1_00C0,  /* 1DC0..1DFF */
  MirLev1_00C0,  /* 1E00..1E3F */
  MirLev1_00C0,  /* 1E40..1E7F */
  MirLev1_00C0,  /* 1E80..1EBF */
  MirLev1_00C0,  /* 1EC0..1EFF */
  MirLev1_00C0,  /* 1F00..1F3F */
  MirLev1_00C0,  /* 1F40..1F7F */
  MirLev1_00C0,  /* 1F80..1FBF */
  MirLev1_00C0,  /* 1FC0..1FFF */
  MirLev1_2000,  /* 2000..203F */
  MirLev1_2040,  /* 2040..207F */
  MirLev1_2080,  /* 2080..20BF */
  MirLev1_00C0,  /* 20C0..20FF */
  MirLev1_00C0,  /* 2100..213F */
  MirLev1_00C0,  /* 2140..217F */
  MirLev1_00C0,  /* 2180..21BF */
  MirLev1_00C0,  /* 21C0..21FF */
  MirLev1_2200,  /* 2200..223F */
  MirLev1_2240,  /* 2240..227F */
  MirLev1_2280,  /* 2280..22BF */
  MirLev1_22C0,  /* 22C0..22FF */
  MirLev1_2300,  /* 2300..233F */
  MirLev1_00C0,  /* 2340..237F */
  MirLev1_00C0,  /* 2380..23BF */
  MirLev1_00C0,  /* 23C0..23FF */
  MirLev1_00C0,  /* 2400..243F */
  MirLev1_00C0,  /* 2440..247F */
  MirLev1_00C0,  /* 2480..24BF */
  MirLev1_00C0,  /* 24C0..24FF */
  MirLev1_00C0,  /* 2500..253F */
  MirLev1_00C0,  /* 2540..257F */
  MirLev1_00C0,  /* 2580..25BF */
  MirLev1_00C0,  /* 25C0..25FF */
  MirLev1_00C0,  /* 2600..263F */
  MirLev1_00C0,  /* 2640..267F */
  MirLev1_00C0,  /* 2680..26BF */
  MirLev1_00C0,  /* 26C0..26FF */
  MirLev1_00C0,  /* 2700..273F */
  MirLev1_2740,  /* 2740..277F */
  MirLev1_00C0,  /* 2780..27BF */
  MirLev1_27C0,  /* 27C0..27FF */
  MirLev1_00C0,  /* 2800..283F */
  MirLev1_00C0,  /* 2840..287F */
  MirLev1_00C0,  /* 2880..28BF */
  MirLev1_00C0,  /* 28C0..28FF */
  MirLev1_00C0,  /* 2900..293F */
  MirLev1_00C0,  /* 2940..297F */
  MirLev1_2980,  /* 2980..29BF */
  MirLev1_29C0,  /* 29C0..29FF */
  MirLev1_2A00,  /* 2A00..2A3F */
  MirLev1_2A40,  /* 2A40..2A7F */
  MirLev1_2A80,  /* 2A80..2ABF */
  MirLev1_2AC0,  /* 2AC0..2AFF */
  MirLev1_00C0,  /* 2B00..2B3F */
  MirLev1_00C0,  /* 2B40..2B7F */
  MirLev1_00C0,  /* 2B80..2BBF */
  MirLev1_00C0,  /* 2BC0..2BFF */
  MirLev1_00C0,  /* 2C00..2C3F */
  MirLev1_00C0,  /* 2C40..2C7F */
  MirLev1_00C0,  /* 2C80..2CBF */
  MirLev1_00C0,  /* 2CC0..2CFF */
  MirLev1_00C0,  /* 2D00..2D3F */
  MirLev1_00C0,  /* 2D40..2D7F */
  MirLev1_00C0,  /* 2D80..2DBF */
  MirLev1_00C0,  /* 2DC0..2DFF */
  MirLev1_2E00,  /* 2E00..2E3F */
  MirLev1_00C0,  /* 2E40..2E7F */
  MirLev1_00C0,  /* 2E80..2EBF */
  MirLev1_00C0,  /* 2EC0..2EFF */
  MirLev1_00C0,  /* 2F00..2F3F */
  MirLev1_00C0,  /* 2F40..2F7F */
  MirLev1_00C0,  /* 2F80..2FBF */
  MirLev1_00C0,  /* 2FC0..2FFF */
  MirLev1_3000,  /* 3000..303F */
  MirLev1_00C0,  /* 3040..307F */
  MirLev1_00C0,  /* 3080..30BF */
  MirLev1_00C0,  /* 30C0..30FF */
  MirLev1_00C0,  /* 3100..313F */
  MirLev1_00C0,  /* 3140..317F */
  MirLev1_00C0,  /* 3180..31BF */
  MirLev1_00C0,  /* 31C0..31FF */
  MirLev1_00C0,  /* 3200..323F */
  MirLev1_00C0,  /* 3240..327F */
  MirLev1_00C0,  /* 3280..32BF */
  MirLev1_00C0,  /* 32C0..32FF */
  MirLev1_00C0,  /* 3300..333F */
  MirLev1_00C0,  /* 3340..337F */
  MirLev1_00C0,  /* 3380..33BF */
  MirLev1_00C0,  /* 33C0..33FF */
  MirLev1_00C0,  /* 3400..343F */
  MirLev1_00C0,  /* 3440..347F */
  MirLev1_00C0,  /* 3480..34BF */
  MirLev1_00C0,  /* 34C0..34FF */
  MirLev1_00C0,  /* 3500..353F */
  MirLev1_00C0,  /* 3540..357F */
  MirLev1_00C0,  /* 3580..35BF */
  MirLev1_00C0,  /* 35C0..35FF */
  MirLev1_00C0,  /* 3600..363F */
  MirLev1_00C0,  /* 3640..367F */
  MirLev1_00C0,  /* 3680..36BF */
  MirLev1_00C0,  /* 36C0..36FF */
  MirLev1_00C0,  /* 3700..373F */
  MirLev1_00C0,  /* 3740..377F */
  MirLev1_00C0,  /* 3780..37BF */
  MirLev1_00C0,  /* 37C0..37FF */
  MirLev1_00C0,  /* 3800..383F */
  MirLev1_00C0,  /* 3840..387F */
  MirLev1_00C0,  /* 3880..38BF */
  MirLev1_00C0,  /* 38C0..38FF */
  MirLev1_00C0,  /* 3900..393F */
  MirLev1_00C0,  /* 3940..397F */
  MirLev1_00C0,  /* 3980..39BF */
  MirLev1_00C0,  /* 39C0..39FF */
  MirLev1_00C0,  /* 3A00..3A3F */
  MirLev1_00C0,  /* 3A40..3A7F */
  MirLev1_00C0,  /* 3A80..3ABF */
  MirLev1_00C0,  /* 3AC0..3AFF */
  MirLev1_00C0,  /* 3B00..3B3F */
  MirLev1_00C0,  /* 3B40..3B7F */
  MirLev1_00C0,  /* 3B80..3BBF */
  MirLev1_00C0,  /* 3BC0..3BFF */
  MirLev1_00C0,  /* 3C00..3C3F */
  MirLev1_00C0,  /* 3C40..3C7F */
  MirLev1_00C0,  /* 3C80..3CBF */
  MirLev1_00C0,  /* 3CC0..3CFF */
  MirLev1_00C0,  /* 3D00..3D3F */
  MirLev1_00C0,  /* 3D40..3D7F */
  MirLev1_00C0,  /* 3D80..3DBF */
  MirLev1_00C0,  /* 3DC0..3DFF */
  MirLev1_00C0,  /* 3E00..3E3F */
  MirLev1_00C0,  /* 3E40..3E7F */
  MirLev1_00C0,  /* 3E80..3EBF */
  MirLev1_00C0,  /* 3EC0..3EFF */
  MirLev1_00C0,  /* 3F00..3F3F */
  MirLev1_00C0,  /* 3F40..3F7F */
  MirLev1_00C0,  /* 3F80..3FBF */
  MirLev1_00C0,  /* 3FC0..3FFF */
  MirLev1_00C0,  /* 4000..403F */
  MirLev1_00C0,  /* 4040..407F */
  MirLev1_00C0,  /* 4080..40BF */
  MirLev1_00C0,  /* 40C0..40FF */
  MirLev1_00C0,  /* 4100..413F */
  MirLev1_00C0,  /* 4140..417F */
  MirLev1_00C0,  /* 4180..41BF */
  MirLev1_00C0,  /* 41C0..41FF */
  MirLev1_00C0,  /* 4200..423F */
  MirLev1_00C0,  /* 4240..427F */
  MirLev1_00C0,  /* 4280..42BF */
  MirLev1_00C0,  /* 42C0..42FF */
  MirLev1_00C0,  /* 4300..433F */
  MirLev1_00C0,  /* 4340..437F */
  MirLev1_00C0,  /* 4380..43BF */
  MirLev1_00C0,  /* 43C0..43FF */
  MirLev1_00C0,  /* 4400..443F */
  MirLev1_00C0,  /* 4440..447F */
  MirLev1_00C0,  /* 4480..44BF */
  MirLev1_00C0,  /* 44C0..44FF */
  MirLev1_00C0,  /* 4500..453F */
  MirLev1_00C0,  /* 4540..457F */
  MirLev1_00C0,  /* 4580..45BF */
  MirLev1_00C0,  /* 45C0..45FF */
  MirLev1_00C0,  /* 4600..463F */
  MirLev1_00C0,  /* 4640..467F */
  MirLev1_00C0,  /* 4680..46BF */
  MirLev1_00C0,  /* 46C0..46FF */
  MirLev1_00C0,  /* 4700..473F */
  MirLev1_00C0,  /* 4740..477F */
  MirLev1_00C0,  /* 4780..47BF */
  MirLev1_00C0,  /* 47C0..47FF */
  MirLev1_00C0,  /* 4800..483F */
  MirLev1_00C0,  /* 4840..487F */
  MirLev1_00C0,  /* 4880..48BF */
  MirLev1_00C0,  /* 48C0..48FF */
  MirLev1_00C0,  /* 4900..493F */
  MirLev1_00C0,  /* 4940..497F */
  MirLev1_00C0,  /* 4980..49BF */
  MirLev1_00C0,  /* 49C0..49FF */
  MirLev1_00C0,  /* 4A00..4A3F */
  MirLev1_00C0,  /* 4A40..4A7F */
  MirLev1_00C0,  /* 4A80..4ABF */
  MirLev1_00C0,  /* 4AC0..4AFF */
  MirLev1_00C0,  /* 4B00..4B3F */
  MirLev1_00C0,  /* 4B40..4B7F */
  MirLev1_00C0,  /* 4B80..4BBF */
  MirLev1_00C0,  /* 4BC0..4BFF */
  MirLev1_00C0,  /* 4C00..4C3F */
  MirLev1_00C0,  /* 4C40..4C7F */
  MirLev1_00C0,  /* 4C80..4CBF */
  MirLev1_00C0,  /* 4CC0..4CFF */
  MirLev1_00C0,  /* 4D00..4D3F */
  MirLev1_00C0,  /* 4D40..4D7F */
  MirLev1_00C0,  /* 4D80..4DBF */
  MirLev1_00C0,  /* 4DC0..4DFF */
  MirLev1_00C0,  /* 4E00..4E3F */
  MirLev1_00C0,  /* 4E40..4E7F */
  MirLev1_00C0,  /* 4E80..4EBF */
  MirLev1_00C0,  /* 4EC0..4EFF */
  MirLev1_00C0,  /* 4F00..4F3F */
  MirLev1_00C0,  /* 4F40..4F7F */
  MirLev1_00C0,  /* 4F80..4FBF */
  MirLev1_00C0,  /* 4FC0..4FFF */
  MirLev1_00C0,  /* 5000..503F */
  MirLev1_00C0,  /* 5040..507F */
  MirLev1_00C0,  /* 5080..50BF */
  MirLev1_00C0,  /* 50C0..50FF */
  MirLev1_00C0,  /* 5100..513F */
  MirLev1_00C0,  /* 5140..517F */
  MirLev1_00C0,  /* 5180..51BF */
  MirLev1_00C0,  /* 51C0..51FF */
  MirLev1_00C0,  /* 5200..523F */
  MirLev1_00C0,  /* 5240..527F */
  MirLev1_00C0,  /* 5280..52BF */
  MirLev1_00C0,  /* 52C0..52FF */
  MirLev1_00C0,  /* 5300..533F */
  MirLev1_00C0,  /* 5340..537F */
  MirLev1_00C0,  /* 5380..53BF */
  MirLev1_00C0,  /* 53C0..53FF */
  MirLev1_00C0,  /* 5400..543F */
  MirLev1_00C0,  /* 5440..547F */
  MirLev1_00C0,  /* 5480..54BF */
  MirLev1_00C0,  /* 54C0..54FF */
  MirLev1_00C0,  /* 5500..553F */
  MirLev1_00C0,  /* 5540..557F */
  MirLev1_00C0,  /* 5580..55BF */
  MirLev1_00C0,  /* 55C0..55FF */
  MirLev1_00C0,  /* 5600..563F */
  MirLev1_00C0,  /* 5640..567F */
  MirLev1_00C0,  /* 5680..56BF */
  MirLev1_00C0,  /* 56C0..56FF */
  MirLev1_00C0,  /* 5700..573F */
  MirLev1_00C0,  /* 5740..577F */
  MirLev1_00C0,  /* 5780..57BF */
  MirLev1_00C0,  /* 57C0..57FF */
  MirLev1_00C0,  /* 5800..583F */
  MirLev1_00C0,  /* 5840..587F */
  MirLev1_00C0,  /* 5880..58BF */
  MirLev1_00C0,  /* 58C0..58FF */
  MirLev1_00C0,  /* 5900..593F */
  MirLev1_00C0,  /* 5940..597F */
  MirLev1_00C0,  /* 5980..59BF */
  MirLev1_00C0,  /* 59C0..59FF */
  MirLev1_00C0,  /* 5A00..5A3F */
  MirLev1_00C0,  /* 5A40..5A7F */
  MirLev1_00C0,  /* 5A80..5ABF */
  MirLev1_00C0,  /* 5AC0..5AFF */
  MirLev1_00C0,  /* 5B00..5B3F */
  MirLev1_00C0,  /* 5B40..5B7F */
  MirLev1_00C0,  /* 5B80..5BBF */
  MirLev1_00C0,  /* 5BC0..5BFF */
  MirLev1_00C0,  /* 5C00..5C3F */
  MirLev1_00C0,  /* 5C40..5C7F */
  MirLev1_00C0,  /* 5C80..5CBF */
  MirLev1_00C0,  /* 5CC0..5CFF */
  MirLev1_00C0,  /* 5D00..5D3F */
  MirLev1_00C0,  /* 5D40..5D7F */
  MirLev1_00C0,  /* 5D80..5DBF */
  MirLev1_00C0,  /* 5DC0..5DFF */
  MirLev1_00C0,  /* 5E00..5E3F */
  MirLev1_00C0,  /* 5E40..5E7F */
  MirLev1_00C0,  /* 5E80..5EBF */
  MirLev1_00C0,  /* 5EC0..5EFF */
  MirLev1_00C0,  /* 5F00..5F3F */
  MirLev1_00C0,  /* 5F40..5F7F */
  MirLev1_00C0,  /* 5F80..5FBF */
  MirLev1_00C0,  /* 5FC0..5FFF */
  MirLev1_00C0,  /* 6000..603F */
  MirLev1_00C0,  /* 6040..607F */
  MirLev1_00C0,  /* 6080..60BF */
  MirLev1_00C0,  /* 60C0..60FF */
  MirLev1_00C0,  /* 6100..613F */
  MirLev1_00C0,  /* 6140..617F */
  MirLev1_00C0,  /* 6180..61BF */
  MirLev1_00C0,  /* 61C0..61FF */
  MirLev1_00C0,  /* 6200..623F */
  MirLev1_00C0,  /* 6240..627F */
  MirLev1_00C0,  /* 6280..62BF */
  MirLev1_00C0,  /* 62C0..62FF */
  MirLev1_00C0,  /* 6300..633F */
  MirLev1_00C0,  /* 6340..637F */
  MirLev1_00C0,  /* 6380..63BF */
  MirLev1_00C0,  /* 63C0..63FF */
  MirLev1_00C0,  /* 6400..643F */
  MirLev1_00C0,  /* 6440..647F */
  MirLev1_00C0,  /* 6480..64BF */
  MirLev1_00C0,  /* 64C0..64FF */
  MirLev1_00C0,  /* 6500..653F */
  MirLev1_00C0,  /* 6540..657F */
  MirLev1_00C0,  /* 6580..65BF */
  MirLev1_00C0,  /* 65C0..65FF */
  MirLev1_00C0,  /* 6600..663F */
  MirLev1_00C0,  /* 6640..667F */
  MirLev1_00C0,  /* 6680..66BF */
  MirLev1_00C0,  /* 66C0..66FF */
  MirLev1_00C0,  /* 6700..673F */
  MirLev1_00C0,  /* 6740..677F */
  MirLev1_00C0,  /* 6780..67BF */
  MirLev1_00C0,  /* 67C0..67FF */
  MirLev1_00C0,  /* 6800..683F */
  MirLev1_00C0,  /* 6840..687F */
  MirLev1_00C0,  /* 6880..68BF */
  MirLev1_00C0,  /* 68C0..68FF */
  MirLev1_00C0,  /* 6900..693F */
  MirLev1_00C0,  /* 6940..697F */
  MirLev1_00C0,  /* 6980..69BF */
  MirLev1_00C0,  /* 69C0..69FF */
  MirLev1_00C0,  /* 6A00..6A3F */
  MirLev1_00C0,  /* 6A40..6A7F */
  MirLev1_00C0,  /* 6A80..6ABF */
  MirLev1_00C0,  /* 6AC0..6AFF */
  MirLev1_00C0,  /* 6B00..6B3F */
  MirLev1_00C0,  /* 6B40..6B7F */
  MirLev1_00C0,  /* 6B80..6BBF */
  MirLev1_00C0,  /* 6BC0..6BFF */
  MirLev1_00C0,  /* 6C00..6C3F */
  MirLev1_00C0,  /* 6C40..6C7F */
  MirLev1_00C0,  /* 6C80..6CBF */
  MirLev1_00C0,  /* 6CC0..6CFF */
  MirLev1_00C0,  /* 6D00..6D3F */
  MirLev1_00C0,  /* 6D40..6D7F */
  MirLev1_00C0,  /* 6D80..6DBF */
  MirLev1_00C0,  /* 6DC0..6DFF */
  MirLev1_00C0,  /* 6E00..6E3F */
  MirLev1_00C0,  /* 6E40..6E7F */
  MirLev1_00C0,  /* 6E80..6EBF */
  MirLev1_00C0,  /* 6EC0..6EFF */
  MirLev1_00C0,  /* 6F00..6F3F */
  MirLev1_00C0,  /* 6F40..6F7F */
  MirLev1_00C0,  /* 6F80..6FBF */
  MirLev1_00C0,  /* 6FC0..6FFF */
  MirLev1_00C0,  /* 7000..703F */
  MirLev1_00C0,  /* 7040..707F */
  MirLev1_00C0,  /* 7080..70BF */
  MirLev1_00C0,  /* 70C0..70FF */
  MirLev1_00C0,  /* 7100..713F */
  MirLev1_00C0,  /* 7140..717F */
  MirLev1_00C0,  /* 7180..71BF */
  MirLev1_00C0,  /* 71C0..71FF */
  MirLev1_00C0,  /* 7200..723F */
  MirLev1_00C0,  /* 7240..727F */
  MirLev1_00C0,  /* 7280..72BF */
  MirLev1_00C0,  /* 72C0..72FF */
  MirLev1_00C0,  /* 7300..733F */
  MirLev1_00C0,  /* 7340..737F */
  MirLev1_00C0,  /* 7380..73BF */
  MirLev1_00C0,  /* 73C0..73FF */
  MirLev1_00C0,  /* 7400..743F */
  MirLev1_00C0,  /* 7440..747F */
  MirLev1_00C0,  /* 7480..74BF */
  MirLev1_00C0,  /* 74C0..74FF */
  MirLev1_00C0,  /* 7500..753F */
  MirLev1_00C0,  /* 7540..757F */
  MirLev1_00C0,  /* 7580..75BF */
  MirLev1_00C0,  /* 75C0..75FF */
  MirLev1_00C0,  /* 7600..763F */
  MirLev1_00C0,  /* 7640..767F */
  MirLev1_00C0,  /* 7680..76BF */
  MirLev1_00C0,  /* 76C0..76FF */
  MirLev1_00C0,  /* 7700..773F */
  MirLev1_00C0,  /* 7740..777F */
  MirLev1_00C0,  /* 7780..77BF */
  MirLev1_00C0,  /* 77C0..77FF */
  MirLev1_00C0,  /* 7800..783F */
  MirLev1_00C0,  /* 7840..787F */
  MirLev1_00C0,  /* 7880..78BF */
  MirLev1_00C0,  /* 78C0..78FF */
  MirLev1_00C0,  /* 7900..793F */
  MirLev1_00C0,  /* 7940..797F */
  MirLev1_00C0,  /* 7980..79BF */
  MirLev1_00C0,  /* 79C0..79FF */
  MirLev1_00C0,  /* 7A00..7A3F */
  MirLev1_00C0,  /* 7A40..7A7F */
  MirLev1_00C0,  /* 7A80..7ABF */
  MirLev1_00C0,  /* 7AC0..7AFF */
  MirLev1_00C0,  /* 7B00..7B3F */
  MirLev1_00C0,  /* 7B40..7B7F */
  MirLev1_00C0,  /* 7B80..7BBF */
  MirLev1_00C0,  /* 7BC0..7BFF */
  MirLev1_00C0,  /* 7C00..7C3F */
  MirLev1_00C0,  /* 7C40..7C7F */
  MirLev1_00C0,  /* 7C80..7CBF */
  MirLev1_00C0,  /* 7CC0..7CFF */
  MirLev1_00C0,  /* 7D00..7D3F */
  MirLev1_00C0,  /* 7D40..7D7F */
  MirLev1_00C0,  /* 7D80..7DBF */
  MirLev1_00C0,  /* 7DC0..7DFF */
  MirLev1_00C0,  /* 7E00..7E3F */
  MirLev1_00C0,  /* 7E40..7E7F */
  MirLev1_00C0,  /* 7E80..7EBF */
  MirLev1_00C0,  /* 7EC0..7EFF */
  MirLev1_00C0,  /* 7F00..7F3F */
  MirLev1_00C0,  /* 7F40..7F7F */
  MirLev1_00C0,  /* 7F80..7FBF */
  MirLev1_00C0,  /* 7FC0..7FFF */
  MirLev1_00C0,  /* 8000..803F */
  MirLev1_00C0,  /* 8040..807F */
  MirLev1_00C0,  /* 8080..80BF */
  MirLev1_00C0,  /* 80C0..80FF */
  MirLev1_00C0,  /* 8100..813F */
  MirLev1_00C0,  /* 8140..817F */
  MirLev1_00C0,  /* 8180..81BF */
  MirLev1_00C0,  /* 81C0..81FF */
  MirLev1_00C0,  /* 8200..823F */
  MirLev1_00C0,  /* 8240..827F */
  MirLev1_00C0,  /* 8280..82BF */
  MirLev1_00C0,  /* 82C0..82FF */
  MirLev1_00C0,  /* 8300..833F */
  MirLev1_00C0,  /* 8340..837F */
  MirLev1_00C0,  /* 8380..83BF */
  MirLev1_00C0,  /* 83C0..83FF */
  MirLev1_00C0,  /* 8400..843F */
  MirLev1_00C0,  /* 8440..847F */
  MirLev1_00C0,  /* 8480..84BF */
  MirLev1_00C0,  /* 84C0..84FF */
  MirLev1_00C0,  /* 8500..853F */
  MirLev1_00C0,  /* 8540..857F */
  MirLev1_00C0,  /* 8580..85BF */
  MirLev1_00C0,  /* 85C0..85FF */
  MirLev1_00C0,  /* 8600..863F */
  MirLev1_00C0,  /* 8640..867F */
  MirLev1_00C0,  /* 8680..86BF */
  MirLev1_00C0,  /* 86C0..86FF */
  MirLev1_00C0,  /* 8700..873F */
  MirLev1_00C0,  /* 8740..877F */
  MirLev1_00C0,  /* 8780..87BF */
  MirLev1_00C0,  /* 87C0..87FF */
  MirLev1_00C0,  /* 8800..883F */
  MirLev1_00C0,  /* 8840..887F */
  MirLev1_00C0,  /* 8880..88BF */
  MirLev1_00C0,  /* 88C0..88FF */
  MirLev1_00C0,  /* 8900..893F */
  MirLev1_00C0,  /* 8940..897F */
  MirLev1_00C0,  /* 8980..89BF */
  MirLev1_00C0,  /* 89C0..89FF */
  MirLev1_00C0,  /* 8A00..8A3F */
  MirLev1_00C0,  /* 8A40..8A7F */
  MirLev1_00C0,  /* 8A80..8ABF */
  MirLev1_00C0,  /* 8AC0..8AFF */
  MirLev1_00C0,  /* 8B00..8B3F */
  MirLev1_00C0,  /* 8B40..8B7F */
  MirLev1_00C0,  /* 8B80..8BBF */
  MirLev1_00C0,  /* 8BC0..8BFF */
  MirLev1_00C0,  /* 8C00..8C3F */
  MirLev1_00C0,  /* 8C40..8C7F */
  MirLev1_00C0,  /* 8C80..8CBF */
  MirLev1_00C0,  /* 8CC0..8CFF */
  MirLev1_00C0,  /* 8D00..8D3F */
  MirLev1_00C0,  /* 8D40..8D7F */
  MirLev1_00C0,  /* 8D80..8DBF */
  MirLev1_00C0,  /* 8DC0..8DFF */
  MirLev1_00C0,  /* 8E00..8E3F */
  MirLev1_00C0,  /* 8E40..8E7F */
  MirLev1_00C0,  /* 8E80..8EBF */
  MirLev1_00C0,  /* 8EC0..8EFF */
  MirLev1_00C0,  /* 8F00..8F3F */
  MirLev1_00C0,  /* 8F40..8F7F */
  MirLev1_00C0,  /* 8F80..8FBF */
  MirLev1_00C0,  /* 8FC0..8FFF */
  MirLev1_00C0,  /* 9000..903F */
  MirLev1_00C0,  /* 9040..907F */
  MirLev1_00C0,  /* 9080..90BF */
  MirLev1_00C0,  /* 90C0..90FF */
  MirLev1_00C0,  /* 9100..913F */
  MirLev1_00C0,  /* 9140..917F */
  MirLev1_00C0,  /* 9180..91BF */
  MirLev1_00C0,  /* 91C0..91FF */
  MirLev1_00C0,  /* 9200..923F */
  MirLev1_00C0,  /* 9240..927F */
  MirLev1_00C0,  /* 9280..92BF */
  MirLev1_00C0,  /* 92C0..92FF */
  MirLev1_00C0,  /* 9300..933F */
  MirLev1_00C0,  /* 9340..937F */
  MirLev1_00C0,  /* 9380..93BF */
  MirLev1_00C0,  /* 93C0..93FF */
  MirLev1_00C0,  /* 9400..943F */
  MirLev1_00C0,  /* 9440..947F */
  MirLev1_00C0,  /* 9480..94BF */
  MirLev1_00C0,  /* 94C0..94FF */
  MirLev1_00C0,  /* 9500..953F */
  MirLev1_00C0,  /* 9540..957F */
  MirLev1_00C0,  /* 9580..95BF */
  MirLev1_00C0,  /* 95C0..95FF */
  MirLev1_00C0,  /* 9600..963F */
  MirLev1_00C0,  /* 9640..967F */
  MirLev1_00C0,  /* 9680..96BF */
  MirLev1_00C0,  /* 96C0..96FF */
  MirLev1_00C0,  /* 9700..973F */
  MirLev1_00C0,  /* 9740..977F */
  MirLev1_00C0,  /* 9780..97BF */
  MirLev1_00C0,  /* 97C0..97FF */
  MirLev1_00C0,  /* 9800..983F */
  MirLev1_00C0,  /* 9840..987F */
  MirLev1_00C0,  /* 9880..98BF */
  MirLev1_00C0,  /* 98C0..98FF */
  MirLev1_00C0,  /* 9900..993F */
  MirLev1_00C0,  /* 9940..997F */
  MirLev1_00C0,  /* 9980..99BF */
  MirLev1_00C0,  /* 99C0..99FF */
  MirLev1_00C0,  /* 9A00..9A3F */
  MirLev1_00C0,  /* 9A40..9A7F */
  MirLev1_00C0,  /* 9A80..9ABF */
  MirLev1_00C0,  /* 9AC0..9AFF */
  MirLev1_00C0,  /* 9B00..9B3F */
  MirLev1_00C0,  /* 9B40..9B7F */
  MirLev1_00C0,  /* 9B80..9BBF */
  MirLev1_00C0,  /* 9BC0..9BFF */
  MirLev1_00C0,  /* 9C00..9C3F */
  MirLev1_00C0,  /* 9C40..9C7F */
  MirLev1_00C0,  /* 9C80..9CBF */
  MirLev1_00C0,  /* 9CC0..9CFF */
  MirLev1_00C0,  /* 9D00..9D3F */
  MirLev1_00C0,  /* 9D40..9D7F */
  MirLev1_00C0,  /* 9D80..9DBF */
  MirLev1_00C0,  /* 9DC0..9DFF */
  MirLev1_00C0,  /* 9E00..9E3F */
  MirLev1_00C0,  /* 9E40..9E7F */
  MirLev1_00C0,  /* 9E80..9EBF */
  MirLev1_00C0,  /* 9EC0..9EFF */
  MirLev1_00C0,  /* 9F00..9F3F */
  MirLev1_00C0,  /* 9F40..9F7F */
  MirLev1_00C0,  /* 9F80..9FBF */
  MirLev1_00C0,  /* 9FC0..9FFF */
  MirLev1_00C0,  /* A000..A03F */
  MirLev1_00C0,  /* A040..A07F */
  MirLev1_00C0,  /* A080..A0BF */
  MirLev1_00C0,  /* A0C0..A0FF */
  MirLev1_00C0,  /* A100..A13F */
  MirLev1_00C0,  /* A140..A17F */
  MirLev1_00C0,  /* A180..A1BF */
  MirLev1_00C0,  /* A1C0..A1FF */
  MirLev1_00C0,  /* A200..A23F */
  MirLev1_00C0,  /* A240..A27F */
  MirLev1_00C0,  /* A280..A2BF */
  MirLev1_00C0,  /* A2C0..A2FF */
  MirLev1_00C0,  /* A300..A33F */
  MirLev1_00C0,  /* A340..A37F */
  MirLev1_00C0,  /* A380..A3BF */
  MirLev1_00C0,  /* A3C0..A3FF */
  MirLev1_00C0,  /* A400..A43F */
  MirLev1_00C0,  /* A440..A47F */
  MirLev1_00C0,  /* A480..A4BF */
  MirLev1_00C0,  /* A4C0..A4FF */
  MirLev1_00C0,  /* A500..A53F */
  MirLev1_00C0,  /* A540..A57F */
  MirLev1_00C0,  /* A580..A5BF */
  MirLev1_00C0,  /* A5C0..A5FF */
  MirLev1_00C0,  /* A600..A63F */
  MirLev1_00C0,  /* A640..A67F */
  MirLev1_00C0,  /* A680..A6BF */
  MirLev1_00C0,  /* A6C0..A6FF */
  MirLev1_00C0,  /* A700..A73F */
  MirLev1_00C0,  /* A740..A77F */
  MirLev1_00C0,  /* A780..A7BF */
  MirLev1_00C0,  /* A7C0..A7FF */
  MirLev1_00C0,  /* A800..A83F */
  MirLev1_00C0,  /* A840..A87F */
  MirLev1_00C0,  /* A880..A8BF */
  MirLev1_00C0,  /* A8C0..A8FF */
  MirLev1_00C0,  /* A900..A93F */
  MirLev1_00C0,  /* A940..A97F */
  MirLev1_00C0,  /* A980..A9BF */
  MirLev1_00C0,  /* A9C0..A9FF */
  MirLev1_00C0,  /* AA00..AA3F */
  MirLev1_00C0,  /* AA40..AA7F */
  MirLev1_00C0,  /* AA80..AABF */
  MirLev1_00C0,  /* AAC0..AAFF */
  MirLev1_00C0,  /* AB00..AB3F */
  MirLev1_00C0,  /* AB40..AB7F */
  MirLev1_00C0,  /* AB80..ABBF */
  MirLev1_00C0,  /* ABC0..ABFF */
  MirLev1_00C0,  /* AC00..AC3F */
  MirLev1_00C0,  /* AC40..AC7F */
  MirLev1_00C0,  /* AC80..ACBF */
  MirLev1_00C0,  /* ACC0..ACFF */
  MirLev1_00C0,  /* AD00..AD3F */
  MirLev1_00C0,  /* AD40..AD7F */
  MirLev1_00C0,  /* AD80..ADBF */
  MirLev1_00C0,  /* ADC0..ADFF */
  MirLev1_00C0,  /* AE00..AE3F */
  MirLev1_00C0,  /* AE40..AE7F */
  MirLev1_00C0,  /* AE80..AEBF */
  MirLev1_00C0,  /* AEC0..AEFF */
  MirLev1_00C0,  /* AF00..AF3F */
  MirLev1_00C0,  /* AF40..AF7F */
  MirLev1_00C0,  /* AF80..AFBF */
  MirLev1_00C0,  /* AFC0..AFFF */
  MirLev1_00C0,  /* B000..B03F */
  MirLev1_00C0,  /* B040..B07F */
  MirLev1_00C0,  /* B080..B0BF */
  MirLev1_00C0,  /* B0C0..B0FF */
  MirLev1_00C0,  /* B100..B13F */
  MirLev1_00C0,  /* B140..B17F */
  MirLev1_00C0,  /* B180..B1BF */
  MirLev1_00C0,  /* B1C0..B1FF */
  MirLev1_00C0,  /* B200..B23F */
  MirLev1_00C0,  /* B240..B27F */
  MirLev1_00C0,  /* B280..B2BF */
  MirLev1_00C0,  /* B2C0..B2FF */
  MirLev1_00C0,  /* B300..B33F */
  MirLev1_00C0,  /* B340..B37F */
  MirLev1_00C0,  /* B380..B3BF */
  MirLev1_00C0,  /* B3C0..B3FF */
  MirLev1_00C0,  /* B400..B43F */
  MirLev1_00C0,  /* B440..B47F */
  MirLev1_00C0,  /* B480..B4BF */
  MirLev1_00C0,  /* B4C0..B4FF */
  MirLev1_00C0,  /* B500..B53F */
  MirLev1_00C0,  /* B540..B57F */
  MirLev1_00C0,  /* B580..B5BF */
  MirLev1_00C0,  /* B5C0..B5FF */
  MirLev1_00C0,  /* B600..B63F */
  MirLev1_00C0,  /* B640..B67F */
  MirLev1_00C0,  /* B680..B6BF */
  MirLev1_00C0,  /* B6C0..B6FF */
  MirLev1_00C0,  /* B700..B73F */
  MirLev1_00C0,  /* B740..B77F */
  MirLev1_00C0,  /* B780..B7BF */
  MirLev1_00C0,  /* B7C0..B7FF */
  MirLev1_00C0,  /* B800..B83F */
  MirLev1_00C0,  /* B840..B87F */
  MirLev1_00C0,  /* B880..B8BF */
  MirLev1_00C0,  /* B8C0..B8FF */
  MirLev1_00C0,  /* B900..B93F */
  MirLev1_00C0,  /* B940..B97F */
  MirLev1_00C0,  /* B980..B9BF */
  MirLev1_00C0,  /* B9C0..B9FF */
  MirLev1_00C0,  /* BA00..BA3F */
  MirLev1_00C0,  /* BA40..BA7F */
  MirLev1_00C0,  /* BA80..BABF */
  MirLev1_00C0,  /* BAC0..BAFF */
  MirLev1_00C0,  /* BB00..BB3F */
  MirLev1_00C0,  /* BB40..BB7F */
  MirLev1_00C0,  /* BB80..BBBF */
  MirLev1_00C0,  /* BBC0..BBFF */
  MirLev1_00C0,  /* BC00..BC3F */
  MirLev1_00C0,  /* BC40..BC7F */
  MirLev1_00C0,  /* BC80..BCBF */
  MirLev1_00C0,  /* BCC0..BCFF */
  MirLev1_00C0,  /* BD00..BD3F */
  MirLev1_00C0,  /* BD40..BD7F */
  MirLev1_00C0,  /* BD80..BDBF */
  MirLev1_00C0,  /* BDC0..BDFF */
  MirLev1_00C0,  /* BE00..BE3F */
  MirLev1_00C0,  /* BE40..BE7F */
  MirLev1_00C0,  /* BE80..BEBF */
  MirLev1_00C0,  /* BEC0..BEFF */
  MirLev1_00C0,  /* BF00..BF3F */
  MirLev1_00C0,  /* BF40..BF7F */
  MirLev1_00C0,  /* BF80..BFBF */
  MirLev1_00C0,  /* BFC0..BFFF */
  MirLev1_00C0,  /* C000..C03F */
  MirLev1_00C0,  /* C040..C07F */
  MirLev1_00C0,  /* C080..C0BF */
  MirLev1_00C0,  /* C0C0..C0FF */
  MirLev1_00C0,  /* C100..C13F */
  MirLev1_00C0,  /* C140..C17F */
  MirLev1_00C0,  /* C180..C1BF */
  MirLev1_00C0,  /* C1C0..C1FF */
  MirLev1_00C0,  /* C200..C23F */
  MirLev1_00C0,  /* C240..C27F */
  MirLev1_00C0,  /* C280..C2BF */
  MirLev1_00C0,  /* C2C0..C2FF */
  MirLev1_00C0,  /* C300..C33F */
  MirLev1_00C0,  /* C340..C37F */
  MirLev1_00C0,  /* C380..C3BF */
  MirLev1_00C0,  /* C3C0..C3FF */
  MirLev1_00C0,  /* C400..C43F */
  MirLev1_00C0,  /* C440..C47F */
  MirLev1_00C0,  /* C480..C4BF */
  MirLev1_00C0,  /* C4C0..C4FF */
  MirLev1_00C0,  /* C500..C53F */
  MirLev1_00C0,  /* C540..C57F */
  MirLev1_00C0,  /* C580..C5BF */
  MirLev1_00C0,  /* C5C0..C5FF */
  MirLev1_00C0,  /* C600..C63F */
  MirLev1_00C0,  /* C640..C67F */
  MirLev1_00C0,  /* C680..C6BF */
  MirLev1_00C0,  /* C6C0..C6FF */
  MirLev1_00C0,  /* C700..C73F */
  MirLev1_00C0,  /* C740..C77F */
  MirLev1_00C0,  /* C780..C7BF */
  MirLev1_00C0,  /* C7C0..C7FF */
  MirLev1_00C0,  /* C800..C83F */
  MirLev1_00C0,  /* C840..C87F */
  MirLev1_00C0,  /* C880..C8BF */
  MirLev1_00C0,  /* C8C0..C8FF */
  MirLev1_00C0,  /* C900..C93F */
  MirLev1_00C0,  /* C940..C97F */
  MirLev1_00C0,  /* C980..C9BF */
  MirLev1_00C0,  /* C9C0..C9FF */
  MirLev1_00C0,  /* CA00..CA3F */
  MirLev1_00C0,  /* CA40..CA7F */
  MirLev1_00C0,  /* CA80..CABF */
  MirLev1_00C0,  /* CAC0..CAFF */
  MirLev1_00C0,  /* CB00..CB3F */
  MirLev1_00C0,  /* CB40..CB7F */
  MirLev1_00C0,  /* CB80..CBBF */
  MirLev1_00C0,  /* CBC0..CBFF */
  MirLev1_00C0,  /* CC00..CC3F */
  MirLev1_00C0,  /* CC40..CC7F */
  MirLev1_00C0,  /* CC80..CCBF */
  MirLev1_00C0,  /* CCC0..CCFF */
  MirLev1_00C0,  /* CD00..CD3F */
  MirLev1_00C0,  /* CD40..CD7F */
  MirLev1_00C0,  /* CD80..CDBF */
  MirLev1_00C0,  /* CDC0..CDFF */
  MirLev1_00C0,  /* CE00..CE3F */
  MirLev1_00C0,  /* CE40..CE7F */
  MirLev1_00C0,  /* CE80..CEBF */
  MirLev1_00C0,  /* CEC0..CEFF */
  MirLev1_00C0,  /* CF00..CF3F */
  MirLev1_00C0,  /* CF40..CF7F */
  MirLev1_00C0,  /* CF80..CFBF */
  MirLev1_00C0,  /* CFC0..CFFF */
  MirLev1_00C0,  /* D000..D03F */
  MirLev1_00C0,  /* D040..D07F */
  MirLev1_00C0,  /* D080..D0BF */
  MirLev1_00C0,  /* D0C0..D0FF */
  MirLev1_00C0,  /* D100..D13F */
  MirLev1_00C0,  /* D140..D17F */
  MirLev1_00C0,  /* D180..D1BF */
  MirLev1_00C0,  /* D1C0..D1FF */
  MirLev1_00C0,  /* D200..D23F */
  MirLev1_00C0,  /* D240..D27F */
  MirLev1_00C0,  /* D280..D2BF */
  MirLev1_00C0,  /* D2C0..D2FF */
  MirLev1_00C0,  /* D300..D33F */
  MirLev1_00C0,  /* D340..D37F */
  MirLev1_00C0,  /* D380..D3BF */
  MirLev1_00C0,  /* D3C0..D3FF */
  MirLev1_00C0,  /* D400..D43F */
  MirLev1_00C0,  /* D440..D47F */
  MirLev1_00C0,  /* D480..D4BF */
  MirLev1_00C0,  /* D4C0..D4FF */
  MirLev1_00C0,  /* D500..D53F */
  MirLev1_00C0,  /* D540..D57F */
  MirLev1_00C0,  /* D580..D5BF */
  MirLev1_00C0,  /* D5C0..D5FF */
  MirLev1_00C0,  /* D600..D63F */
  MirLev1_00C0,  /* D640..D67F */
  MirLev1_00C0,  /* D680..D6BF */
  MirLev1_00C0,  /* D6C0..D6FF */
  MirLev1_00C0,  /* D700..D73F */
  MirLev1_00C0,  /* D740..D77F */
  MirLev1_00C0,  /* D780..D7BF */
  MirLev1_00C0,  /* D7C0..D7FF */
  MirLev1_00C0,  /* D800..D83F */
  MirLev1_00C0,  /* D840..D87F */
  MirLev1_00C0,  /* D880..D8BF */
  MirLev1_00C0,  /* D8C0..D8FF */
  MirLev1_00C0,  /* D900..D93F */
  MirLev1_00C0,  /* D940..D97F */
  MirLev1_00C0,  /* D980..D9BF */
  MirLev1_00C0,  /* D9C0..D9FF */
  MirLev1_00C0,  /* DA00..DA3F */
  MirLev1_00C0,  /* DA40..DA7F */
  MirLev1_00C0,  /* DA80..DABF */
  MirLev1_00C0,  /* DAC0..DAFF */
  MirLev1_00C0,  /* DB00..DB3F */
  MirLev1_00C0,  /* DB40..DB7F */
  MirLev1_00C0,  /* DB80..DBBF */
  MirLev1_00C0,  /* DBC0..DBFF */
  MirLev1_00C0,  /* DC00..DC3F */
  MirLev1_00C0,  /* DC40..DC7F */
  MirLev1_00C0,  /* DC80..DCBF */
  MirLev1_00C0,  /* DCC0..DCFF */
  MirLev1_00C0,  /* DD00..DD3F */
  MirLev1_00C0,  /* DD40..DD7F */
  MirLev1_00C0,  /* DD80..DDBF */
  MirLev1_00C0,  /* DDC0..DDFF */
  MirLev1_00C0,  /* DE00..DE3F */
  MirLev1_00C0,  /* DE40..DE7F */
  MirLev1_00C0,  /* DE80..DEBF */
  MirLev1_00C0,  /* DEC0..DEFF */
  MirLev1_00C0,  /* DF00..DF3F */
  MirLev1_00C0,  /* DF40..DF7F */
  MirLev1_00C0,  /* DF80..DFBF */
  MirLev1_00C0,  /* DFC0..DFFF */
  MirLev1_00C0,  /* E000..E03F */
  MirLev1_00C0,  /* E040..E07F */
  MirLev1_00C0,  /* E080..E0BF */
  MirLev1_00C0,  /* E0C0..E0FF */
  MirLev1_00C0,  /* E100..E13F */
  MirLev1_00C0,  /* E140..E17F */
  MirLev1_00C0,  /* E180..E1BF */
  MirLev1_00C0,  /* E1C0..E1FF */
  MirLev1_00C0,  /* E200..E23F */
  MirLev1_00C0,  /* E240..E27F */
  MirLev1_00C0,  /* E280..E2BF */
  MirLev1_00C0,  /* E2C0..E2FF */
  MirLev1_00C0,  /* E300..E33F */
  MirLev1_00C0,  /* E340..E37F */
  MirLev1_00C0,  /* E380..E3BF */
  MirLev1_00C0,  /* E3C0..E3FF */
  MirLev1_00C0,  /* E400..E43F */
  MirLev1_00C0,  /* E440..E47F */
  MirLev1_00C0,  /* E480..E4BF */
  MirLev1_00C0,  /* E4C0..E4FF */
  MirLev1_00C0,  /* E500..E53F */
  MirLev1_00C0,  /* E540..E57F */
  MirLev1_00C0,  /* E580..E5BF */
  MirLev1_00C0,  /* E5C0..E5FF */
  MirLev1_00C0,  /* E600..E63F */
  MirLev1_00C0,  /* E640..E67F */
  MirLev1_00C0,  /* E680..E6BF */
  MirLev1_00C0,  /* E6C0..E6FF */
  MirLev1_00C0,  /* E700..E73F */
  MirLev1_00C0,  /* E740..E77F */
  MirLev1_00C0,  /* E780..E7BF */
  MirLev1_00C0,  /* E7C0..E7FF */
  MirLev1_00C0,  /* E800..E83F */
  MirLev1_00C0,  /* E840..E87F */
  MirLev1_00C0,  /* E880..E8BF */
  MirLev1_00C0,  /* E8C0..E8FF */
  MirLev1_00C0,  /* E900..E93F */
  MirLev1_00C0,  /* E940..E97F */
  MirLev1_00C0,  /* E980..E9BF */
  MirLev1_00C0,  /* E9C0..E9FF */
  MirLev1_00C0,  /* EA00..EA3F */
  MirLev1_00C0,  /* EA40..EA7F */
  MirLev1_00C0,  /* EA80..EABF */
  MirLev1_00C0,  /* EAC0..EAFF */
  MirLev1_00C0,  /* EB00..EB3F */
  MirLev1_00C0,  /* EB40..EB7F */
  MirLev1_00C0,  /* EB80..EBBF */
  MirLev1_00C0,  /* EBC0..EBFF */
  MirLev1_00C0,  /* EC00..EC3F */
  MirLev1_00C0,  /* EC40..EC7F */
  MirLev1_00C0,  /* EC80..ECBF */
  MirLev1_00C0,  /* ECC0..ECFF */
  MirLev1_00C0,  /* ED00..ED3F */
  MirLev1_00C0,  /* ED40..ED7F */
  MirLev1_00C0,  /* ED80..EDBF */
  MirLev1_00C0,  /* EDC0..EDFF */
  MirLev1_00C0,  /* EE00..EE3F */
  MirLev1_00C0,  /* EE40..EE7F */
  MirLev1_00C0,  /* EE80..EEBF */
  MirLev1_00C0,  /* EEC0..EEFF */
  MirLev1_00C0,  /* EF00..EF3F */
  MirLev1_00C0,  /* EF40..EF7F */
  MirLev1_00C0,  /* EF80..EFBF */
  MirLev1_00C0,  /* EFC0..EFFF */
  MirLev1_00C0,  /* F000..F03F */
  MirLev1_00C0,  /* F040..F07F */
  MirLev1_00C0,  /* F080..F0BF */
  MirLev1_00C0,  /* F0C0..F0FF */
  MirLev1_00C0,  /* F100..F13F */
  MirLev1_00C0,  /* F140..F17F */
  MirLev1_00C0,  /* F180..F1BF */
  MirLev1_00C0,  /* F1C0..F1FF */
  MirLev1_00C0,  /* F200..F23F */
  MirLev1_00C0,  /* F240..F27F */
  MirLev1_00C0,  /* F280..F2BF */
  MirLev1_00C0,  /* F2C0..F2FF */
  MirLev1_00C0,  /* F300..F33F */
  MirLev1_00C0,  /* F340..F37F */
  MirLev1_00C0,  /* F380..F3BF */
  MirLev1_00C0,  /* F3C0..F3FF */
  MirLev1_00C0,  /* F400..F43F */
  MirLev1_00C0,  /* F440..F47F */
  MirLev1_00C0,  /* F480..F4BF */
  MirLev1_00C0,  /* F4C0..F4FF */
  MirLev1_00C0,  /* F500..F53F */
  MirLev1_00C0,  /* F540..F57F */
  MirLev1_00C0,  /* F580..F5BF */
  MirLev1_00C0,  /* F5C0..F5FF */
  MirLev1_00C0,  /* F600..F63F */
  MirLev1_00C0,  /* F640..F67F */
  MirLev1_00C0,  /* F680..F6BF */
  MirLev1_00C0,  /* F6C0..F6FF */
  MirLev1_00C0,  /* F700..F73F */
  MirLev1_00C0,  /* F740..F77F */
  MirLev1_00C0,  /* F780..F7BF */
  MirLev1_00C0,  /* F7C0..F7FF */
  MirLev1_00C0,  /* F800..F83F */
  MirLev1_00C0,  /* F840..F87F */
  MirLev1_00C0,  /* F880..F8BF */
  MirLev1_00C0,  /* F8C0..F8FF */
  MirLev1_00C0,  /* F900..F93F */
  MirLev1_00C0,  /* F940..F97F */
  MirLev1_00C0,  /* F980..F9BF */
  MirLev1_00C0,  /* F9C0..F9FF */
  MirLev1_00C0,  /* FA00..FA3F */
  MirLev1_00C0,  /* FA40..FA7F */
  MirLev1_00C0,  /* FA80..FABF */
  MirLev1_00C0,  /* FAC0..FAFF */
  MirLev1_00C0,  /* FB00..FB3F */
  MirLev1_00C0,  /* FB40..FB7F */
  MirLev1_00C0,  /* FB80..FBBF */
  MirLev1_00C0,  /* FBC0..FBFF */
  MirLev1_00C0,  /* FC00..FC3F */
  MirLev1_00C0,  /* FC40..FC7F */
  MirLev1_00C0,  /* FC80..FCBF */
  MirLev1_00C0,  /* FCC0..FCFF */
  MirLev1_00C0,  /* FD00..FD3F */
  MirLev1_00C0,  /* FD40..FD7F */
  MirLev1_00C0,  /* FD80..FDBF */
  MirLev1_00C0,  /* FDC0..FDFF */
  MirLev1_00C0,  /* FE00..FE3F */
  MirLev1_FE40,  /* FE40..FE7F */
  MirLev1_00C0,  /* FE80..FEBF */
  MirLev1_00C0,  /* FEC0..FEFF */
  MirLev1_FF00,  /* FF00..FF3F */
  MirLev1_FF40,  /* FF40..FF7F */
  MirLev1_00C0,  /* FF80..FFBF */
  MirLev1_00C0,  /* FFC0..FFFF */
};

#define BIDI_GET_MIRRORING_DELTA(x) ((x) >= 0x10000 ? 0 :  \
	MirLev1[((x) & 0x3f) + \
	MirLev0[((x) >> 6)]])

#undef PACKTAB_UINT8
#undef PACKTAB_UINT16
#undef PACKTAB_UINT32

#define BIDI_GET_MIRRORING(x) ((x) + BIDI_GET_MIRRORING_DELTA(x))
