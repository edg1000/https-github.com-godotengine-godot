/*************************************************************************/
/*  bidi-joining-type.tab.i                                              */
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
  assumed sizeof(unsigned char): 1
  required memory: 19968
  lookups: 2
  partition shape: Joi[4096][256]
  different table entries: 1 46
*/

#define PACKTAB_UINT8 uint8_t
#define PACKTAB_UINT16 uint16_t
#define PACKTAB_UINT32 uint32_t

static const unsigned char JoiLev1[256*46] = {
#define JoiLev1_00000 0x0
  G,G,G,G,G,G,G,G,G,U,U,U,U,U,G,G,G,G,G,G,G,G,G,G,G,G,G,G,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,G,
  G,G,G,G,G,U,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,
  U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_00100 0x100
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_00300 0x200
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_00400 0x300
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_00500 0x400
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,U,T,
  U,T,T,U,T,T,U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_00600 0x500
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,T,T,T,T,T,T,T,T,U,U,U,U,U,
  D,U,R,R,R,R,D,R,D,R,D,D,D,D,D,R,R,R,R,D,D,D,D,D,D,D,D,D,D,D,D,D,
  C,D,D,D,D,D,D,D,R,D,D,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,D,D,T,R,R,R,U,R,R,R,D,D,D,D,D,D,D,D,
  D,D,D,D,D,D,D,D,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,R,D,D,D,D,D,D,
  D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,
  R,D,D,R,R,R,R,R,R,R,R,R,D,R,D,R,D,D,R,R,U,R,T,T,T,T,T,T,T,U,U,T,
  T,T,T,T,T,U,U,T,T,U,T,T,T,T,R,R,U,U,U,U,U,U,U,U,U,U,D,D,D,U,U,D,
#define JoiLev1_00700 0x600
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,R,T,D,D,D,R,R,R,R,R,D,D,D,D,R,D,
  D,D,D,D,D,D,D,D,R,D,R,D,R,D,D,R,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
  T,T,T,T,T,T,T,T,T,T,T,U,U,R,D,D,D,D,D,D,D,D,D,D,D,R,R,R,D,D,D,D,
  D,D,D,D,D,D,D,D,D,D,D,R,R,D,D,D,D,R,D,R,R,D,D,D,R,R,D,D,D,D,D,D,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,T,T,T,T,T,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,D,
  D,D,D,D,D,D,D,D,D,D,D,T,T,T,T,T,T,T,T,T,U,U,U,U,U,U,C,U,U,U,U,U,
#define JoiLev1_00800 0x700
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,T,U,T,T,T,T,T,
  T,T,T,T,U,T,T,T,U,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  R,D,D,D,D,D,R,D,D,R,D,D,D,D,D,R,D,D,D,D,R,D,U,U,U,T,T,T,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  D,U,D,D,D,D,D,D,D,D,R,R,R,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,U,
#define JoiLev1_00900 0x800
  T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,T,U,U,U,
  U,T,T,T,T,T,T,T,T,U,U,U,U,T,U,U,U,T,T,T,T,T,T,T,U,U,U,U,U,U,U,U,
  U,U,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,U,U,
  U,T,T,T,T,U,U,U,U,U,U,U,U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_00A00 0x900
  U,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,U,U,
  U,T,T,U,U,U,U,T,T,U,U,T,T,T,U,U,U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,U,U,U,T,U,U,U,U,U,U,U,U,U,U,
  U,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,U,U,
  U,T,T,T,T,T,U,T,T,U,U,U,U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_00B00 0xA00
  U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,U,T,
  U,T,T,T,T,U,U,U,U,U,U,U,U,T,U,U,U,U,U,U,U,U,T,U,U,U,U,U,U,U,U,U,
  U,U,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  T,U,U,U,U,U,U,U,U,U,U,U,U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_00C00 0xB00
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,
  T,U,U,U,U,U,T,T,T,U,T,T,T,T,U,U,U,U,U,U,U,T,T,U,U,U,U,U,U,U,U,U,
  U,U,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,U,T,
  U,U,U,U,U,U,T,U,U,U,U,U,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_00D00 0xC00
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,T,T,T,T,U,U,U,U,U,U,U,U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,T,U,U,U,U,U,U,U,T,T,T,U,T,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_00E00 0xD00
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,U,T,T,T,T,T,T,T,U,U,U,U,U,
  U,U,U,U,U,U,U,T,T,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,U,T,T,T,T,T,T,U,T,T,U,U,U,
  U,U,U,U,U,U,U,U,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_00F00 0xE00
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,T,U,T,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,T,T,T,T,T,T,T,T,T,T,T,U,
  T,T,T,T,T,U,T,T,U,U,U,U,U,T,T,T,T,T,T,T,T,T,T,T,U,T,T,T,T,T,T,T,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,U,U,U,
  U,U,U,U,U,U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_01000 0xF00
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,T,U,T,T,T,T,T,T,U,T,T,U,U,T,T,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,U,U,U,U,T,T,
  T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,
  U,U,T,U,U,T,T,U,U,U,U,U,U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_01300 0x1000
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_01700 0x1100
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,U,T,T,T,T,T,T,T,U,U,
  U,U,U,U,U,U,T,U,U,T,T,T,T,T,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,T,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_01800 0x1200
  U,U,U,U,U,U,U,U,U,U,U,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_01900 0x1300
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  T,T,T,U,U,U,U,T,T,U,U,U,U,U,U,U,U,U,T,U,U,U,U,U,U,T,T,T,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_01A00 0x1400
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,T,T,T,T,T,T,T,U,
  T,U,T,U,U,T,T,T,T,T,T,T,T,U,U,U,U,U,U,T,T,T,T,T,T,T,T,T,T,U,U,T,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_01B00 0x1500
  T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,T,T,T,T,T,U,T,U,U,U,
  U,U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,T,T,T,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,
  T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,T,T,T,T,U,U,T,T,U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,T,U,T,T,U,U,U,T,U,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_01C00 0x1600
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,T,T,T,T,T,U,U,T,T,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,U,T,T,T,T,T,T,T,T,T,T,T,T,
  T,U,T,T,T,T,T,T,T,U,U,U,U,T,U,U,U,U,U,U,T,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_01D00 0x1700
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
  T,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,T,
#define JoiLev1_02000 0x1800
  U,U,U,U,U,U,U,U,U,U,U,T,U,C,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  T,T,T,T,T,U,U,U,U,U,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,T,T,T,T,T,T,T,T,T,T,U,U,U,
  U,T,U,U,U,T,T,T,T,T,T,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_02C00 0x1900
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_02D00 0x1A00
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
#define JoiLev1_03000 0x1B00
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_0A600 0x1C00
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,U,U,U,T,T,T,T,T,T,T,T,T,T,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_0A800 0x1D00
  U,U,T,U,U,U,T,U,U,U,U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_0A900 0x1E00
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,T,T,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,T,T,T,T,T,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,U,T,T,T,T,U,U,T,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_0AA00 0x1F00
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,T,T,T,T,T,T,U,U,T,T,U,U,T,T,U,U,U,U,U,U,U,U,U,
  U,U,U,T,U,U,U,U,U,U,U,U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,T,T,T,U,U,T,T,U,U,U,U,U,T,T,
  U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,T,T,U,U,U,U,U,U,U,U,T,U,U,U,U,U,U,U,U,U,
#define JoiLev1_0AB00 0x2000
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,T,U,U,T,U,U,U,U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_0FB00 0x2100
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_0FE00 0x2200
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  T,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,
#define JoiLev1_0FF00 0x2300
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,U,U,U,U,
#define JoiLev1_10100 0x2400
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,U,U,
#define JoiLev1_10A00 0x2500
  U,T,T,T,U,T,T,U,U,U,U,U,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,U,U,U,U,T,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_11000 0x2600
  U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,T,T,T,T,T,
  T,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,T,U,U,T,T,U,U,T,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_11100 0x2700
  T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,T,T,T,T,T,U,T,T,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,T,T,T,T,T,T,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_11600 0x2800
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,T,U,T,U,U,T,T,T,T,T,T,U,T,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_16F00 0x2900
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_1D100 0x2A00
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,T,T,T,U,U,U,U,U,U,U,U,U,T,T,T,T,T,T,T,T,T,T,T,T,T,
  T,T,T,U,U,T,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_1D200 0x2B00
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_E0000 0x2C00
  U,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
  U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
#define JoiLev1_E0100 0x2D00
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,
  T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,T,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,U,
};

static const PACKTAB_UINT16 JoiLev0[4096*1] = {
#define JoiLev0_00000 0x0
  JoiLev1_00000,  /* 00000..000FF */
  JoiLev1_00100,  /* 00100..001FF */
  JoiLev1_00100,  /* 00200..002FF */
  JoiLev1_00300,  /* 00300..003FF */
  JoiLev1_00400,  /* 00400..004FF */
  JoiLev1_00500,  /* 00500..005FF */
  JoiLev1_00600,  /* 00600..006FF */
  JoiLev1_00700,  /* 00700..007FF */
  JoiLev1_00800,  /* 00800..008FF */
  JoiLev1_00900,  /* 00900..009FF */
  JoiLev1_00A00,  /* 00A00..00AFF */
  JoiLev1_00B00,  /* 00B00..00BFF */
  JoiLev1_00C00,  /* 00C00..00CFF */
  JoiLev1_00D00,  /* 00D00..00DFF */
  JoiLev1_00E00,  /* 00E00..00EFF */
  JoiLev1_00F00,  /* 00F00..00FFF */
  JoiLev1_01000,  /* 01000..010FF */
  JoiLev1_00100,  /* 01100..011FF */
  JoiLev1_00100,  /* 01200..012FF */
  JoiLev1_01300,  /* 01300..013FF */
  JoiLev1_00100,  /* 01400..014FF */
  JoiLev1_00100,  /* 01500..015FF */
  JoiLev1_00100,  /* 01600..016FF */
  JoiLev1_01700,  /* 01700..017FF */
  JoiLev1_01800,  /* 01800..018FF */
  JoiLev1_01900,  /* 01900..019FF */
  JoiLev1_01A00,  /* 01A00..01AFF */
  JoiLev1_01B00,  /* 01B00..01BFF */
  JoiLev1_01C00,  /* 01C00..01CFF */
  JoiLev1_01D00,  /* 01D00..01DFF */
  JoiLev1_00100,  /* 01E00..01EFF */
  JoiLev1_00100,  /* 01F00..01FFF */
  JoiLev1_02000,  /* 02000..020FF */
  JoiLev1_00100,  /* 02100..021FF */
  JoiLev1_00100,  /* 02200..022FF */
  JoiLev1_00100,  /* 02300..023FF */
  JoiLev1_00100,  /* 02400..024FF */
  JoiLev1_00100,  /* 02500..025FF */
  JoiLev1_00100,  /* 02600..026FF */
  JoiLev1_00100,  /* 02700..027FF */
  JoiLev1_00100,  /* 02800..028FF */
  JoiLev1_00100,  /* 02900..029FF */
  JoiLev1_00100,  /* 02A00..02AFF */
  JoiLev1_00100,  /* 02B00..02BFF */
  JoiLev1_02C00,  /* 02C00..02CFF */
  JoiLev1_02D00,  /* 02D00..02DFF */
  JoiLev1_00100,  /* 02E00..02EFF */
  JoiLev1_00100,  /* 02F00..02FFF */
  JoiLev1_03000,  /* 03000..030FF */
  JoiLev1_00100,  /* 03100..031FF */
  JoiLev1_00100,  /* 03200..032FF */
  JoiLev1_00100,  /* 03300..033FF */
  JoiLev1_00100,  /* 03400..034FF */
  JoiLev1_00100,  /* 03500..035FF */
  JoiLev1_00100,  /* 03600..036FF */
  JoiLev1_00100,  /* 03700..037FF */
  JoiLev1_00100,  /* 03800..038FF */
  JoiLev1_00100,  /* 03900..039FF */
  JoiLev1_00100,  /* 03A00..03AFF */
  JoiLev1_00100,  /* 03B00..03BFF */
  JoiLev1_00100,  /* 03C00..03CFF */
  JoiLev1_00100,  /* 03D00..03DFF */
  JoiLev1_00100,  /* 03E00..03EFF */
  JoiLev1_00100,  /* 03F00..03FFF */
  JoiLev1_00100,  /* 04000..040FF */
  JoiLev1_00100,  /* 04100..041FF */
  JoiLev1_00100,  /* 04200..042FF */
  JoiLev1_00100,  /* 04300..043FF */
  JoiLev1_00100,  /* 04400..044FF */
  JoiLev1_00100,  /* 04500..045FF */
  JoiLev1_00100,  /* 04600..046FF */
  JoiLev1_00100,  /* 04700..047FF */
  JoiLev1_00100,  /* 04800..048FF */
  JoiLev1_00100,  /* 04900..049FF */
  JoiLev1_00100,  /* 04A00..04AFF */
  JoiLev1_00100,  /* 04B00..04BFF */
  JoiLev1_00100,  /* 04C00..04CFF */
  JoiLev1_00100,  /* 04D00..04DFF */
  JoiLev1_00100,  /* 04E00..04EFF */
  JoiLev1_00100,  /* 04F00..04FFF */
  JoiLev1_00100,  /* 05000..050FF */
  JoiLev1_00100,  /* 05100..051FF */
  JoiLev1_00100,  /* 05200..052FF */
  JoiLev1_00100,  /* 05300..053FF */
  JoiLev1_00100,  /* 05400..054FF */
  JoiLev1_00100,  /* 05500..055FF */
  JoiLev1_00100,  /* 05600..056FF */
  JoiLev1_00100,  /* 05700..057FF */
  JoiLev1_00100,  /* 05800..058FF */
  JoiLev1_00100,  /* 05900..059FF */
  JoiLev1_00100,  /* 05A00..05AFF */
  JoiLev1_00100,  /* 05B00..05BFF */
  JoiLev1_00100,  /* 05C00..05CFF */
  JoiLev1_00100,  /* 05D00..05DFF */
  JoiLev1_00100,  /* 05E00..05EFF */
  JoiLev1_00100,  /* 05F00..05FFF */
  JoiLev1_00100,  /* 06000..060FF */
  JoiLev1_00100,  /* 06100..061FF */
  JoiLev1_00100,  /* 06200..062FF */
  JoiLev1_00100,  /* 06300..063FF */
  JoiLev1_00100,  /* 06400..064FF */
  JoiLev1_00100,  /* 06500..065FF */
  JoiLev1_00100,  /* 06600..066FF */
  JoiLev1_00100,  /* 06700..067FF */
  JoiLev1_00100,  /* 06800..068FF */
  JoiLev1_00100,  /* 06900..069FF */
  JoiLev1_00100,  /* 06A00..06AFF */
  JoiLev1_00100,  /* 06B00..06BFF */
  JoiLev1_00100,  /* 06C00..06CFF */
  JoiLev1_00100,  /* 06D00..06DFF */
  JoiLev1_00100,  /* 06E00..06EFF */
  JoiLev1_00100,  /* 06F00..06FFF */
  JoiLev1_00100,  /* 07000..070FF */
  JoiLev1_00100,  /* 07100..071FF */
  JoiLev1_00100,  /* 07200..072FF */
  JoiLev1_00100,  /* 07300..073FF */
  JoiLev1_00100,  /* 07400..074FF */
  JoiLev1_00100,  /* 07500..075FF */
  JoiLev1_00100,  /* 07600..076FF */
  JoiLev1_00100,  /* 07700..077FF */
  JoiLev1_00100,  /* 07800..078FF */
  JoiLev1_00100,  /* 07900..079FF */
  JoiLev1_00100,  /* 07A00..07AFF */
  JoiLev1_00100,  /* 07B00..07BFF */
  JoiLev1_00100,  /* 07C00..07CFF */
  JoiLev1_00100,  /* 07D00..07DFF */
  JoiLev1_00100,  /* 07E00..07EFF */
  JoiLev1_00100,  /* 07F00..07FFF */
  JoiLev1_00100,  /* 08000..080FF */
  JoiLev1_00100,  /* 08100..081FF */
  JoiLev1_00100,  /* 08200..082FF */
  JoiLev1_00100,  /* 08300..083FF */
  JoiLev1_00100,  /* 08400..084FF */
  JoiLev1_00100,  /* 08500..085FF */
  JoiLev1_00100,  /* 08600..086FF */
  JoiLev1_00100,  /* 08700..087FF */
  JoiLev1_00100,  /* 08800..088FF */
  JoiLev1_00100,  /* 08900..089FF */
  JoiLev1_00100,  /* 08A00..08AFF */
  JoiLev1_00100,  /* 08B00..08BFF */
  JoiLev1_00100,  /* 08C00..08CFF */
  JoiLev1_00100,  /* 08D00..08DFF */
  JoiLev1_00100,  /* 08E00..08EFF */
  JoiLev1_00100,  /* 08F00..08FFF */
  JoiLev1_00100,  /* 09000..090FF */
  JoiLev1_00100,  /* 09100..091FF */
  JoiLev1_00100,  /* 09200..092FF */
  JoiLev1_00100,  /* 09300..093FF */
  JoiLev1_00100,  /* 09400..094FF */
  JoiLev1_00100,  /* 09500..095FF */
  JoiLev1_00100,  /* 09600..096FF */
  JoiLev1_00100,  /* 09700..097FF */
  JoiLev1_00100,  /* 09800..098FF */
  JoiLev1_00100,  /* 09900..099FF */
  JoiLev1_00100,  /* 09A00..09AFF */
  JoiLev1_00100,  /* 09B00..09BFF */
  JoiLev1_00100,  /* 09C00..09CFF */
  JoiLev1_00100,  /* 09D00..09DFF */
  JoiLev1_00100,  /* 09E00..09EFF */
  JoiLev1_00100,  /* 09F00..09FFF */
  JoiLev1_00100,  /* 0A000..0A0FF */
  JoiLev1_00100,  /* 0A100..0A1FF */
  JoiLev1_00100,  /* 0A200..0A2FF */
  JoiLev1_00100,  /* 0A300..0A3FF */
  JoiLev1_00100,  /* 0A400..0A4FF */
  JoiLev1_00100,  /* 0A500..0A5FF */
  JoiLev1_0A600,  /* 0A600..0A6FF */
  JoiLev1_00100,  /* 0A700..0A7FF */
  JoiLev1_0A800,  /* 0A800..0A8FF */
  JoiLev1_0A900,  /* 0A900..0A9FF */
  JoiLev1_0AA00,  /* 0AA00..0AAFF */
  JoiLev1_0AB00,  /* 0AB00..0ABFF */
  JoiLev1_00100,  /* 0AC00..0ACFF */
  JoiLev1_00100,  /* 0AD00..0ADFF */
  JoiLev1_00100,  /* 0AE00..0AEFF */
  JoiLev1_00100,  /* 0AF00..0AFFF */
  JoiLev1_00100,  /* 0B000..0B0FF */
  JoiLev1_00100,  /* 0B100..0B1FF */
  JoiLev1_00100,  /* 0B200..0B2FF */
  JoiLev1_00100,  /* 0B300..0B3FF */
  JoiLev1_00100,  /* 0B400..0B4FF */
  JoiLev1_00100,  /* 0B500..0B5FF */
  JoiLev1_00100,  /* 0B600..0B6FF */
  JoiLev1_00100,  /* 0B700..0B7FF */
  JoiLev1_00100,  /* 0B800..0B8FF */
  JoiLev1_00100,  /* 0B900..0B9FF */
  JoiLev1_00100,  /* 0BA00..0BAFF */
  JoiLev1_00100,  /* 0BB00..0BBFF */
  JoiLev1_00100,  /* 0BC00..0BCFF */
  JoiLev1_00100,  /* 0BD00..0BDFF */
  JoiLev1_00100,  /* 0BE00..0BEFF */
  JoiLev1_00100,  /* 0BF00..0BFFF */
  JoiLev1_00100,  /* 0C000..0C0FF */
  JoiLev1_00100,  /* 0C100..0C1FF */
  JoiLev1_00100,  /* 0C200..0C2FF */
  JoiLev1_00100,  /* 0C300..0C3FF */
  JoiLev1_00100,  /* 0C400..0C4FF */
  JoiLev1_00100,  /* 0C500..0C5FF */
  JoiLev1_00100,  /* 0C600..0C6FF */
  JoiLev1_00100,  /* 0C700..0C7FF */
  JoiLev1_00100,  /* 0C800..0C8FF */
  JoiLev1_00100,  /* 0C900..0C9FF */
  JoiLev1_00100,  /* 0CA00..0CAFF */
  JoiLev1_00100,  /* 0CB00..0CBFF */
  JoiLev1_00100,  /* 0CC00..0CCFF */
  JoiLev1_00100,  /* 0CD00..0CDFF */
  JoiLev1_00100,  /* 0CE00..0CEFF */
  JoiLev1_00100,  /* 0CF00..0CFFF */
  JoiLev1_00100,  /* 0D000..0D0FF */
  JoiLev1_00100,  /* 0D100..0D1FF */
  JoiLev1_00100,  /* 0D200..0D2FF */
  JoiLev1_00100,  /* 0D300..0D3FF */
  JoiLev1_00100,  /* 0D400..0D4FF */
  JoiLev1_00100,  /* 0D500..0D5FF */
  JoiLev1_00100,  /* 0D600..0D6FF */
  JoiLev1_00100,  /* 0D700..0D7FF */
  JoiLev1_00100,  /* 0D800..0D8FF */
  JoiLev1_00100,  /* 0D900..0D9FF */
  JoiLev1_00100,  /* 0DA00..0DAFF */
  JoiLev1_00100,  /* 0DB00..0DBFF */
  JoiLev1_00100,  /* 0DC00..0DCFF */
  JoiLev1_00100,  /* 0DD00..0DDFF */
  JoiLev1_00100,  /* 0DE00..0DEFF */
  JoiLev1_00100,  /* 0DF00..0DFFF */
  JoiLev1_00100,  /* 0E000..0E0FF */
  JoiLev1_00100,  /* 0E100..0E1FF */
  JoiLev1_00100,  /* 0E200..0E2FF */
  JoiLev1_00100,  /* 0E300..0E3FF */
  JoiLev1_00100,  /* 0E400..0E4FF */
  JoiLev1_00100,  /* 0E500..0E5FF */
  JoiLev1_00100,  /* 0E600..0E6FF */
  JoiLev1_00100,  /* 0E700..0E7FF */
  JoiLev1_00100,  /* 0E800..0E8FF */
  JoiLev1_00100,  /* 0E900..0E9FF */
  JoiLev1_00100,  /* 0EA00..0EAFF */
  JoiLev1_00100,  /* 0EB00..0EBFF */
  JoiLev1_00100,  /* 0EC00..0ECFF */
  JoiLev1_00100,  /* 0ED00..0EDFF */
  JoiLev1_00100,  /* 0EE00..0EEFF */
  JoiLev1_00100,  /* 0EF00..0EFFF */
  JoiLev1_00100,  /* 0F000..0F0FF */
  JoiLev1_00100,  /* 0F100..0F1FF */
  JoiLev1_00100,  /* 0F200..0F2FF */
  JoiLev1_00100,  /* 0F300..0F3FF */
  JoiLev1_00100,  /* 0F400..0F4FF */
  JoiLev1_00100,  /* 0F500..0F5FF */
  JoiLev1_00100,  /* 0F600..0F6FF */
  JoiLev1_00100,  /* 0F700..0F7FF */
  JoiLev1_00100,  /* 0F800..0F8FF */
  JoiLev1_00100,  /* 0F900..0F9FF */
  JoiLev1_00100,  /* 0FA00..0FAFF */
  JoiLev1_0FB00,  /* 0FB00..0FBFF */
  JoiLev1_00100,  /* 0FC00..0FCFF */
  JoiLev1_00100,  /* 0FD00..0FDFF */
  JoiLev1_0FE00,  /* 0FE00..0FEFF */
  JoiLev1_0FF00,  /* 0FF00..0FFFF */
  JoiLev1_00100,  /* 10000..100FF */
  JoiLev1_10100,  /* 10100..101FF */
  JoiLev1_00100,  /* 10200..102FF */
  JoiLev1_00100,  /* 10300..103FF */
  JoiLev1_00100,  /* 10400..104FF */
  JoiLev1_00100,  /* 10500..105FF */
  JoiLev1_00100,  /* 10600..106FF */
  JoiLev1_00100,  /* 10700..107FF */
  JoiLev1_00100,  /* 10800..108FF */
  JoiLev1_00100,  /* 10900..109FF */
  JoiLev1_10A00,  /* 10A00..10AFF */
  JoiLev1_00100,  /* 10B00..10BFF */
  JoiLev1_00100,  /* 10C00..10CFF */
  JoiLev1_00100,  /* 10D00..10DFF */
  JoiLev1_00100,  /* 10E00..10EFF */
  JoiLev1_00100,  /* 10F00..10FFF */
  JoiLev1_11000,  /* 11000..110FF */
  JoiLev1_11100,  /* 11100..111FF */
  JoiLev1_00100,  /* 11200..112FF */
  JoiLev1_00100,  /* 11300..113FF */
  JoiLev1_00100,  /* 11400..114FF */
  JoiLev1_00100,  /* 11500..115FF */
  JoiLev1_11600,  /* 11600..116FF */
  JoiLev1_00100,  /* 11700..117FF */
  JoiLev1_00100,  /* 11800..118FF */
  JoiLev1_00100,  /* 11900..119FF */
  JoiLev1_00100,  /* 11A00..11AFF */
  JoiLev1_00100,  /* 11B00..11BFF */
  JoiLev1_00100,  /* 11C00..11CFF */
  JoiLev1_00100,  /* 11D00..11DFF */
  JoiLev1_00100,  /* 11E00..11EFF */
  JoiLev1_00100,  /* 11F00..11FFF */
  JoiLev1_00100,  /* 12000..120FF */
  JoiLev1_00100,  /* 12100..121FF */
  JoiLev1_00100,  /* 12200..122FF */
  JoiLev1_00100,  /* 12300..123FF */
  JoiLev1_00100,  /* 12400..124FF */
  JoiLev1_00100,  /* 12500..125FF */
  JoiLev1_00100,  /* 12600..126FF */
  JoiLev1_00100,  /* 12700..127FF */
  JoiLev1_00100,  /* 12800..128FF */
  JoiLev1_00100,  /* 12900..129FF */
  JoiLev1_00100,  /* 12A00..12AFF */
  JoiLev1_00100,  /* 12B00..12BFF */
  JoiLev1_00100,  /* 12C00..12CFF */
  JoiLev1_00100,  /* 12D00..12DFF */
  JoiLev1_00100,  /* 12E00..12EFF */
  JoiLev1_00100,  /* 12F00..12FFF */
  JoiLev1_00100,  /* 13000..130FF */
  JoiLev1_00100,  /* 13100..131FF */
  JoiLev1_00100,  /* 13200..132FF */
  JoiLev1_00100,  /* 13300..133FF */
  JoiLev1_00100,  /* 13400..134FF */
  JoiLev1_00100,  /* 13500..135FF */
  JoiLev1_00100,  /* 13600..136FF */
  JoiLev1_00100,  /* 13700..137FF */
  JoiLev1_00100,  /* 13800..138FF */
  JoiLev1_00100,  /* 13900..139FF */
  JoiLev1_00100,  /* 13A00..13AFF */
  JoiLev1_00100,  /* 13B00..13BFF */
  JoiLev1_00100,  /* 13C00..13CFF */
  JoiLev1_00100,  /* 13D00..13DFF */
  JoiLev1_00100,  /* 13E00..13EFF */
  JoiLev1_00100,  /* 13F00..13FFF */
  JoiLev1_00100,  /* 14000..140FF */
  JoiLev1_00100,  /* 14100..141FF */
  JoiLev1_00100,  /* 14200..142FF */
  JoiLev1_00100,  /* 14300..143FF */
  JoiLev1_00100,  /* 14400..144FF */
  JoiLev1_00100,  /* 14500..145FF */
  JoiLev1_00100,  /* 14600..146FF */
  JoiLev1_00100,  /* 14700..147FF */
  JoiLev1_00100,  /* 14800..148FF */
  JoiLev1_00100,  /* 14900..149FF */
  JoiLev1_00100,  /* 14A00..14AFF */
  JoiLev1_00100,  /* 14B00..14BFF */
  JoiLev1_00100,  /* 14C00..14CFF */
  JoiLev1_00100,  /* 14D00..14DFF */
  JoiLev1_00100,  /* 14E00..14EFF */
  JoiLev1_00100,  /* 14F00..14FFF */
  JoiLev1_00100,  /* 15000..150FF */
  JoiLev1_00100,  /* 15100..151FF */
  JoiLev1_00100,  /* 15200..152FF */
  JoiLev1_00100,  /* 15300..153FF */
  JoiLev1_00100,  /* 15400..154FF */
  JoiLev1_00100,  /* 15500..155FF */
  JoiLev1_00100,  /* 15600..156FF */
  JoiLev1_00100,  /* 15700..157FF */
  JoiLev1_00100,  /* 15800..158FF */
  JoiLev1_00100,  /* 15900..159FF */
  JoiLev1_00100,  /* 15A00..15AFF */
  JoiLev1_00100,  /* 15B00..15BFF */
  JoiLev1_00100,  /* 15C00..15CFF */
  JoiLev1_00100,  /* 15D00..15DFF */
  JoiLev1_00100,  /* 15E00..15EFF */
  JoiLev1_00100,  /* 15F00..15FFF */
  JoiLev1_00100,  /* 16000..160FF */
  JoiLev1_00100,  /* 16100..161FF */
  JoiLev1_00100,  /* 16200..162FF */
  JoiLev1_00100,  /* 16300..163FF */
  JoiLev1_00100,  /* 16400..164FF */
  JoiLev1_00100,  /* 16500..165FF */
  JoiLev1_00100,  /* 16600..166FF */
  JoiLev1_00100,  /* 16700..167FF */
  JoiLev1_00100,  /* 16800..168FF */
  JoiLev1_00100,  /* 16900..169FF */
  JoiLev1_00100,  /* 16A00..16AFF */
  JoiLev1_00100,  /* 16B00..16BFF */
  JoiLev1_00100,  /* 16C00..16CFF */
  JoiLev1_00100,  /* 16D00..16DFF */
  JoiLev1_00100,  /* 16E00..16EFF */
  JoiLev1_16F00,  /* 16F00..16FFF */
  JoiLev1_00100,  /* 17000..170FF */
  JoiLev1_00100,  /* 17100..171FF */
  JoiLev1_00100,  /* 17200..172FF */
  JoiLev1_00100,  /* 17300..173FF */
  JoiLev1_00100,  /* 17400..174FF */
  JoiLev1_00100,  /* 17500..175FF */
  JoiLev1_00100,  /* 17600..176FF */
  JoiLev1_00100,  /* 17700..177FF */
  JoiLev1_00100,  /* 17800..178FF */
  JoiLev1_00100,  /* 17900..179FF */
  JoiLev1_00100,  /* 17A00..17AFF */
  JoiLev1_00100,  /* 17B00..17BFF */
  JoiLev1_00100,  /* 17C00..17CFF */
  JoiLev1_00100,  /* 17D00..17DFF */
  JoiLev1_00100,  /* 17E00..17EFF */
  JoiLev1_00100,  /* 17F00..17FFF */
  JoiLev1_00100,  /* 18000..180FF */
  JoiLev1_00100,  /* 18100..181FF */
  JoiLev1_00100,  /* 18200..182FF */
  JoiLev1_00100,  /* 18300..183FF */
  JoiLev1_00100,  /* 18400..184FF */
  JoiLev1_00100,  /* 18500..185FF */
  JoiLev1_00100,  /* 18600..186FF */
  JoiLev1_00100,  /* 18700..187FF */
  JoiLev1_00100,  /* 18800..188FF */
  JoiLev1_00100,  /* 18900..189FF */
  JoiLev1_00100,  /* 18A00..18AFF */
  JoiLev1_00100,  /* 18B00..18BFF */
  JoiLev1_00100,  /* 18C00..18CFF */
  JoiLev1_00100,  /* 18D00..18DFF */
  JoiLev1_00100,  /* 18E00..18EFF */
  JoiLev1_00100,  /* 18F00..18FFF */
  JoiLev1_00100,  /* 19000..190FF */
  JoiLev1_00100,  /* 19100..191FF */
  JoiLev1_00100,  /* 19200..192FF */
  JoiLev1_00100,  /* 19300..193FF */
  JoiLev1_00100,  /* 19400..194FF */
  JoiLev1_00100,  /* 19500..195FF */
  JoiLev1_00100,  /* 19600..196FF */
  JoiLev1_00100,  /* 19700..197FF */
  JoiLev1_00100,  /* 19800..198FF */
  JoiLev1_00100,  /* 19900..199FF */
  JoiLev1_00100,  /* 19A00..19AFF */
  JoiLev1_00100,  /* 19B00..19BFF */
  JoiLev1_00100,  /* 19C00..19CFF */
  JoiLev1_00100,  /* 19D00..19DFF */
  JoiLev1_00100,  /* 19E00..19EFF */
  JoiLev1_00100,  /* 19F00..19FFF */
  JoiLev1_00100,  /* 1A000..1A0FF */
  JoiLev1_00100,  /* 1A100..1A1FF */
  JoiLev1_00100,  /* 1A200..1A2FF */
  JoiLev1_00100,  /* 1A300..1A3FF */
  JoiLev1_00100,  /* 1A400..1A4FF */
  JoiLev1_00100,  /* 1A500..1A5FF */
  JoiLev1_00100,  /* 1A600..1A6FF */
  JoiLev1_00100,  /* 1A700..1A7FF */
  JoiLev1_00100,  /* 1A800..1A8FF */
  JoiLev1_00100,  /* 1A900..1A9FF */
  JoiLev1_00100,  /* 1AA00..1AAFF */
  JoiLev1_00100,  /* 1AB00..1ABFF */
  JoiLev1_00100,  /* 1AC00..1ACFF */
  JoiLev1_00100,  /* 1AD00..1ADFF */
  JoiLev1_00100,  /* 1AE00..1AEFF */
  JoiLev1_00100,  /* 1AF00..1AFFF */
  JoiLev1_00100,  /* 1B000..1B0FF */
  JoiLev1_00100,  /* 1B100..1B1FF */
  JoiLev1_00100,  /* 1B200..1B2FF */
  JoiLev1_00100,  /* 1B300..1B3FF */
  JoiLev1_00100,  /* 1B400..1B4FF */
  JoiLev1_00100,  /* 1B500..1B5FF */
  JoiLev1_00100,  /* 1B600..1B6FF */
  JoiLev1_00100,  /* 1B700..1B7FF */
  JoiLev1_00100,  /* 1B800..1B8FF */
  JoiLev1_00100,  /* 1B900..1B9FF */
  JoiLev1_00100,  /* 1BA00..1BAFF */
  JoiLev1_00100,  /* 1BB00..1BBFF */
  JoiLev1_00100,  /* 1BC00..1BCFF */
  JoiLev1_00100,  /* 1BD00..1BDFF */
  JoiLev1_00100,  /* 1BE00..1BEFF */
  JoiLev1_00100,  /* 1BF00..1BFFF */
  JoiLev1_00100,  /* 1C000..1C0FF */
  JoiLev1_00100,  /* 1C100..1C1FF */
  JoiLev1_00100,  /* 1C200..1C2FF */
  JoiLev1_00100,  /* 1C300..1C3FF */
  JoiLev1_00100,  /* 1C400..1C4FF */
  JoiLev1_00100,  /* 1C500..1C5FF */
  JoiLev1_00100,  /* 1C600..1C6FF */
  JoiLev1_00100,  /* 1C700..1C7FF */
  JoiLev1_00100,  /* 1C800..1C8FF */
  JoiLev1_00100,  /* 1C900..1C9FF */
  JoiLev1_00100,  /* 1CA00..1CAFF */
  JoiLev1_00100,  /* 1CB00..1CBFF */
  JoiLev1_00100,  /* 1CC00..1CCFF */
  JoiLev1_00100,  /* 1CD00..1CDFF */
  JoiLev1_00100,  /* 1CE00..1CEFF */
  JoiLev1_00100,  /* 1CF00..1CFFF */
  JoiLev1_00100,  /* 1D000..1D0FF */
  JoiLev1_1D100,  /* 1D100..1D1FF */
  JoiLev1_1D200,  /* 1D200..1D2FF */
  JoiLev1_00100,  /* 1D300..1D3FF */
  JoiLev1_00100,  /* 1D400..1D4FF */
  JoiLev1_00100,  /* 1D500..1D5FF */
  JoiLev1_00100,  /* 1D600..1D6FF */
  JoiLev1_00100,  /* 1D700..1D7FF */
  JoiLev1_00100,  /* 1D800..1D8FF */
  JoiLev1_00100,  /* 1D900..1D9FF */
  JoiLev1_00100,  /* 1DA00..1DAFF */
  JoiLev1_00100,  /* 1DB00..1DBFF */
  JoiLev1_00100,  /* 1DC00..1DCFF */
  JoiLev1_00100,  /* 1DD00..1DDFF */
  JoiLev1_00100,  /* 1DE00..1DEFF */
  JoiLev1_00100,  /* 1DF00..1DFFF */
  JoiLev1_00100,  /* 1E000..1E0FF */
  JoiLev1_00100,  /* 1E100..1E1FF */
  JoiLev1_00100,  /* 1E200..1E2FF */
  JoiLev1_00100,  /* 1E300..1E3FF */
  JoiLev1_00100,  /* 1E400..1E4FF */
  JoiLev1_00100,  /* 1E500..1E5FF */
  JoiLev1_00100,  /* 1E600..1E6FF */
  JoiLev1_00100,  /* 1E700..1E7FF */
  JoiLev1_00100,  /* 1E800..1E8FF */
  JoiLev1_00100,  /* 1E900..1E9FF */
  JoiLev1_00100,  /* 1EA00..1EAFF */
  JoiLev1_00100,  /* 1EB00..1EBFF */
  JoiLev1_00100,  /* 1EC00..1ECFF */
  JoiLev1_00100,  /* 1ED00..1EDFF */
  JoiLev1_00100,  /* 1EE00..1EEFF */
  JoiLev1_00100,  /* 1EF00..1EFFF */
  JoiLev1_00100,  /* 1F000..1F0FF */
  JoiLev1_00100,  /* 1F100..1F1FF */
  JoiLev1_00100,  /* 1F200..1F2FF */
  JoiLev1_00100,  /* 1F300..1F3FF */
  JoiLev1_00100,  /* 1F400..1F4FF */
  JoiLev1_00100,  /* 1F500..1F5FF */
  JoiLev1_00100,  /* 1F600..1F6FF */
  JoiLev1_00100,  /* 1F700..1F7FF */
  JoiLev1_00100,  /* 1F800..1F8FF */
  JoiLev1_00100,  /* 1F900..1F9FF */
  JoiLev1_00100,  /* 1FA00..1FAFF */
  JoiLev1_00100,  /* 1FB00..1FBFF */
  JoiLev1_00100,  /* 1FC00..1FCFF */
  JoiLev1_00100,  /* 1FD00..1FDFF */
  JoiLev1_00100,  /* 1FE00..1FEFF */
  JoiLev1_00100,  /* 1FF00..1FFFF */
  JoiLev1_00100,  /* 20000..200FF */
  JoiLev1_00100,  /* 20100..201FF */
  JoiLev1_00100,  /* 20200..202FF */
  JoiLev1_00100,  /* 20300..203FF */
  JoiLev1_00100,  /* 20400..204FF */
  JoiLev1_00100,  /* 20500..205FF */
  JoiLev1_00100,  /* 20600..206FF */
  JoiLev1_00100,  /* 20700..207FF */
  JoiLev1_00100,  /* 20800..208FF */
  JoiLev1_00100,  /* 20900..209FF */
  JoiLev1_00100,  /* 20A00..20AFF */
  JoiLev1_00100,  /* 20B00..20BFF */
  JoiLev1_00100,  /* 20C00..20CFF */
  JoiLev1_00100,  /* 20D00..20DFF */
  JoiLev1_00100,  /* 20E00..20EFF */
  JoiLev1_00100,  /* 20F00..20FFF */
  JoiLev1_00100,  /* 21000..210FF */
  JoiLev1_00100,  /* 21100..211FF */
  JoiLev1_00100,  /* 21200..212FF */
  JoiLev1_00100,  /* 21300..213FF */
  JoiLev1_00100,  /* 21400..214FF */
  JoiLev1_00100,  /* 21500..215FF */
  JoiLev1_00100,  /* 21600..216FF */
  JoiLev1_00100,  /* 21700..217FF */
  JoiLev1_00100,  /* 21800..218FF */
  JoiLev1_00100,  /* 21900..219FF */
  JoiLev1_00100,  /* 21A00..21AFF */
  JoiLev1_00100,  /* 21B00..21BFF */
  JoiLev1_00100,  /* 21C00..21CFF */
  JoiLev1_00100,  /* 21D00..21DFF */
  JoiLev1_00100,  /* 21E00..21EFF */
  JoiLev1_00100,  /* 21F00..21FFF */
  JoiLev1_00100,  /* 22000..220FF */
  JoiLev1_00100,  /* 22100..221FF */
  JoiLev1_00100,  /* 22200..222FF */
  JoiLev1_00100,  /* 22300..223FF */
  JoiLev1_00100,  /* 22400..224FF */
  JoiLev1_00100,  /* 22500..225FF */
  JoiLev1_00100,  /* 22600..226FF */
  JoiLev1_00100,  /* 22700..227FF */
  JoiLev1_00100,  /* 22800..228FF */
  JoiLev1_00100,  /* 22900..229FF */
  JoiLev1_00100,  /* 22A00..22AFF */
  JoiLev1_00100,  /* 22B00..22BFF */
  JoiLev1_00100,  /* 22C00..22CFF */
  JoiLev1_00100,  /* 22D00..22DFF */
  JoiLev1_00100,  /* 22E00..22EFF */
  JoiLev1_00100,  /* 22F00..22FFF */
  JoiLev1_00100,  /* 23000..230FF */
  JoiLev1_00100,  /* 23100..231FF */
  JoiLev1_00100,  /* 23200..232FF */
  JoiLev1_00100,  /* 23300..233FF */
  JoiLev1_00100,  /* 23400..234FF */
  JoiLev1_00100,  /* 23500..235FF */
  JoiLev1_00100,  /* 23600..236FF */
  JoiLev1_00100,  /* 23700..237FF */
  JoiLev1_00100,  /* 23800..238FF */
  JoiLev1_00100,  /* 23900..239FF */
  JoiLev1_00100,  /* 23A00..23AFF */
  JoiLev1_00100,  /* 23B00..23BFF */
  JoiLev1_00100,  /* 23C00..23CFF */
  JoiLev1_00100,  /* 23D00..23DFF */
  JoiLev1_00100,  /* 23E00..23EFF */
  JoiLev1_00100,  /* 23F00..23FFF */
  JoiLev1_00100,  /* 24000..240FF */
  JoiLev1_00100,  /* 24100..241FF */
  JoiLev1_00100,  /* 24200..242FF */
  JoiLev1_00100,  /* 24300..243FF */
  JoiLev1_00100,  /* 24400..244FF */
  JoiLev1_00100,  /* 24500..245FF */
  JoiLev1_00100,  /* 24600..246FF */
  JoiLev1_00100,  /* 24700..247FF */
  JoiLev1_00100,  /* 24800..248FF */
  JoiLev1_00100,  /* 24900..249FF */
  JoiLev1_00100,  /* 24A00..24AFF */
  JoiLev1_00100,  /* 24B00..24BFF */
  JoiLev1_00100,  /* 24C00..24CFF */
  JoiLev1_00100,  /* 24D00..24DFF */
  JoiLev1_00100,  /* 24E00..24EFF */
  JoiLev1_00100,  /* 24F00..24FFF */
  JoiLev1_00100,  /* 25000..250FF */
  JoiLev1_00100,  /* 25100..251FF */
  JoiLev1_00100,  /* 25200..252FF */
  JoiLev1_00100,  /* 25300..253FF */
  JoiLev1_00100,  /* 25400..254FF */
  JoiLev1_00100,  /* 25500..255FF */
  JoiLev1_00100,  /* 25600..256FF */
  JoiLev1_00100,  /* 25700..257FF */
  JoiLev1_00100,  /* 25800..258FF */
  JoiLev1_00100,  /* 25900..259FF */
  JoiLev1_00100,  /* 25A00..25AFF */
  JoiLev1_00100,  /* 25B00..25BFF */
  JoiLev1_00100,  /* 25C00..25CFF */
  JoiLev1_00100,  /* 25D00..25DFF */
  JoiLev1_00100,  /* 25E00..25EFF */
  JoiLev1_00100,  /* 25F00..25FFF */
  JoiLev1_00100,  /* 26000..260FF */
  JoiLev1_00100,  /* 26100..261FF */
  JoiLev1_00100,  /* 26200..262FF */
  JoiLev1_00100,  /* 26300..263FF */
  JoiLev1_00100,  /* 26400..264FF */
  JoiLev1_00100,  /* 26500..265FF */
  JoiLev1_00100,  /* 26600..266FF */
  JoiLev1_00100,  /* 26700..267FF */
  JoiLev1_00100,  /* 26800..268FF */
  JoiLev1_00100,  /* 26900..269FF */
  JoiLev1_00100,  /* 26A00..26AFF */
  JoiLev1_00100,  /* 26B00..26BFF */
  JoiLev1_00100,  /* 26C00..26CFF */
  JoiLev1_00100,  /* 26D00..26DFF */
  JoiLev1_00100,  /* 26E00..26EFF */
  JoiLev1_00100,  /* 26F00..26FFF */
  JoiLev1_00100,  /* 27000..270FF */
  JoiLev1_00100,  /* 27100..271FF */
  JoiLev1_00100,  /* 27200..272FF */
  JoiLev1_00100,  /* 27300..273FF */
  JoiLev1_00100,  /* 27400..274FF */
  JoiLev1_00100,  /* 27500..275FF */
  JoiLev1_00100,  /* 27600..276FF */
  JoiLev1_00100,  /* 27700..277FF */
  JoiLev1_00100,  /* 27800..278FF */
  JoiLev1_00100,  /* 27900..279FF */
  JoiLev1_00100,  /* 27A00..27AFF */
  JoiLev1_00100,  /* 27B00..27BFF */
  JoiLev1_00100,  /* 27C00..27CFF */
  JoiLev1_00100,  /* 27D00..27DFF */
  JoiLev1_00100,  /* 27E00..27EFF */
  JoiLev1_00100,  /* 27F00..27FFF */
  JoiLev1_00100,  /* 28000..280FF */
  JoiLev1_00100,  /* 28100..281FF */
  JoiLev1_00100,  /* 28200..282FF */
  JoiLev1_00100,  /* 28300..283FF */
  JoiLev1_00100,  /* 28400..284FF */
  JoiLev1_00100,  /* 28500..285FF */
  JoiLev1_00100,  /* 28600..286FF */
  JoiLev1_00100,  /* 28700..287FF */
  JoiLev1_00100,  /* 28800..288FF */
  JoiLev1_00100,  /* 28900..289FF */
  JoiLev1_00100,  /* 28A00..28AFF */
  JoiLev1_00100,  /* 28B00..28BFF */
  JoiLev1_00100,  /* 28C00..28CFF */
  JoiLev1_00100,  /* 28D00..28DFF */
  JoiLev1_00100,  /* 28E00..28EFF */
  JoiLev1_00100,  /* 28F00..28FFF */
  JoiLev1_00100,  /* 29000..290FF */
  JoiLev1_00100,  /* 29100..291FF */
  JoiLev1_00100,  /* 29200..292FF */
  JoiLev1_00100,  /* 29300..293FF */
  JoiLev1_00100,  /* 29400..294FF */
  JoiLev1_00100,  /* 29500..295FF */
  JoiLev1_00100,  /* 29600..296FF */
  JoiLev1_00100,  /* 29700..297FF */
  JoiLev1_00100,  /* 29800..298FF */
  JoiLev1_00100,  /* 29900..299FF */
  JoiLev1_00100,  /* 29A00..29AFF */
  JoiLev1_00100,  /* 29B00..29BFF */
  JoiLev1_00100,  /* 29C00..29CFF */
  JoiLev1_00100,  /* 29D00..29DFF */
  JoiLev1_00100,  /* 29E00..29EFF */
  JoiLev1_00100,  /* 29F00..29FFF */
  JoiLev1_00100,  /* 2A000..2A0FF */
  JoiLev1_00100,  /* 2A100..2A1FF */
  JoiLev1_00100,  /* 2A200..2A2FF */
  JoiLev1_00100,  /* 2A300..2A3FF */
  JoiLev1_00100,  /* 2A400..2A4FF */
  JoiLev1_00100,  /* 2A500..2A5FF */
  JoiLev1_00100,  /* 2A600..2A6FF */
  JoiLev1_00100,  /* 2A700..2A7FF */
  JoiLev1_00100,  /* 2A800..2A8FF */
  JoiLev1_00100,  /* 2A900..2A9FF */
  JoiLev1_00100,  /* 2AA00..2AAFF */
  JoiLev1_00100,  /* 2AB00..2ABFF */
  JoiLev1_00100,  /* 2AC00..2ACFF */
  JoiLev1_00100,  /* 2AD00..2ADFF */
  JoiLev1_00100,  /* 2AE00..2AEFF */
  JoiLev1_00100,  /* 2AF00..2AFFF */
  JoiLev1_00100,  /* 2B000..2B0FF */
  JoiLev1_00100,  /* 2B100..2B1FF */
  JoiLev1_00100,  /* 2B200..2B2FF */
  JoiLev1_00100,  /* 2B300..2B3FF */
  JoiLev1_00100,  /* 2B400..2B4FF */
  JoiLev1_00100,  /* 2B500..2B5FF */
  JoiLev1_00100,  /* 2B600..2B6FF */
  JoiLev1_00100,  /* 2B700..2B7FF */
  JoiLev1_00100,  /* 2B800..2B8FF */
  JoiLev1_00100,  /* 2B900..2B9FF */
  JoiLev1_00100,  /* 2BA00..2BAFF */
  JoiLev1_00100,  /* 2BB00..2BBFF */
  JoiLev1_00100,  /* 2BC00..2BCFF */
  JoiLev1_00100,  /* 2BD00..2BDFF */
  JoiLev1_00100,  /* 2BE00..2BEFF */
  JoiLev1_00100,  /* 2BF00..2BFFF */
  JoiLev1_00100,  /* 2C000..2C0FF */
  JoiLev1_00100,  /* 2C100..2C1FF */
  JoiLev1_00100,  /* 2C200..2C2FF */
  JoiLev1_00100,  /* 2C300..2C3FF */
  JoiLev1_00100,  /* 2C400..2C4FF */
  JoiLev1_00100,  /* 2C500..2C5FF */
  JoiLev1_00100,  /* 2C600..2C6FF */
  JoiLev1_00100,  /* 2C700..2C7FF */
  JoiLev1_00100,  /* 2C800..2C8FF */
  JoiLev1_00100,  /* 2C900..2C9FF */
  JoiLev1_00100,  /* 2CA00..2CAFF */
  JoiLev1_00100,  /* 2CB00..2CBFF */
  JoiLev1_00100,  /* 2CC00..2CCFF */
  JoiLev1_00100,  /* 2CD00..2CDFF */
  JoiLev1_00100,  /* 2CE00..2CEFF */
  JoiLev1_00100,  /* 2CF00..2CFFF */
  JoiLev1_00100,  /* 2D000..2D0FF */
  JoiLev1_00100,  /* 2D100..2D1FF */
  JoiLev1_00100,  /* 2D200..2D2FF */
  JoiLev1_00100,  /* 2D300..2D3FF */
  JoiLev1_00100,  /* 2D400..2D4FF */
  JoiLev1_00100,  /* 2D500..2D5FF */
  JoiLev1_00100,  /* 2D600..2D6FF */
  JoiLev1_00100,  /* 2D700..2D7FF */
  JoiLev1_00100,  /* 2D800..2D8FF */
  JoiLev1_00100,  /* 2D900..2D9FF */
  JoiLev1_00100,  /* 2DA00..2DAFF */
  JoiLev1_00100,  /* 2DB00..2DBFF */
  JoiLev1_00100,  /* 2DC00..2DCFF */
  JoiLev1_00100,  /* 2DD00..2DDFF */
  JoiLev1_00100,  /* 2DE00..2DEFF */
  JoiLev1_00100,  /* 2DF00..2DFFF */
  JoiLev1_00100,  /* 2E000..2E0FF */
  JoiLev1_00100,  /* 2E100..2E1FF */
  JoiLev1_00100,  /* 2E200..2E2FF */
  JoiLev1_00100,  /* 2E300..2E3FF */
  JoiLev1_00100,  /* 2E400..2E4FF */
  JoiLev1_00100,  /* 2E500..2E5FF */
  JoiLev1_00100,  /* 2E600..2E6FF */
  JoiLev1_00100,  /* 2E700..2E7FF */
  JoiLev1_00100,  /* 2E800..2E8FF */
  JoiLev1_00100,  /* 2E900..2E9FF */
  JoiLev1_00100,  /* 2EA00..2EAFF */
  JoiLev1_00100,  /* 2EB00..2EBFF */
  JoiLev1_00100,  /* 2EC00..2ECFF */
  JoiLev1_00100,  /* 2ED00..2EDFF */
  JoiLev1_00100,  /* 2EE00..2EEFF */
  JoiLev1_00100,  /* 2EF00..2EFFF */
  JoiLev1_00100,  /* 2F000..2F0FF */
  JoiLev1_00100,  /* 2F100..2F1FF */
  JoiLev1_00100,  /* 2F200..2F2FF */
  JoiLev1_00100,  /* 2F300..2F3FF */
  JoiLev1_00100,  /* 2F400..2F4FF */
  JoiLev1_00100,  /* 2F500..2F5FF */
  JoiLev1_00100,  /* 2F600..2F6FF */
  JoiLev1_00100,  /* 2F700..2F7FF */
  JoiLev1_00100,  /* 2F800..2F8FF */
  JoiLev1_00100,  /* 2F900..2F9FF */
  JoiLev1_00100,  /* 2FA00..2FAFF */
  JoiLev1_00100,  /* 2FB00..2FBFF */
  JoiLev1_00100,  /* 2FC00..2FCFF */
  JoiLev1_00100,  /* 2FD00..2FDFF */
  JoiLev1_00100,  /* 2FE00..2FEFF */
  JoiLev1_00100,  /* 2FF00..2FFFF */
  JoiLev1_00100,  /* 30000..300FF */
  JoiLev1_00100,  /* 30100..301FF */
  JoiLev1_00100,  /* 30200..302FF */
  JoiLev1_00100,  /* 30300..303FF */
  JoiLev1_00100,  /* 30400..304FF */
  JoiLev1_00100,  /* 30500..305FF */
  JoiLev1_00100,  /* 30600..306FF */
  JoiLev1_00100,  /* 30700..307FF */
  JoiLev1_00100,  /* 30800..308FF */
  JoiLev1_00100,  /* 30900..309FF */
  JoiLev1_00100,  /* 30A00..30AFF */
  JoiLev1_00100,  /* 30B00..30BFF */
  JoiLev1_00100,  /* 30C00..30CFF */
  JoiLev1_00100,  /* 30D00..30DFF */
  JoiLev1_00100,  /* 30E00..30EFF */
  JoiLev1_00100,  /* 30F00..30FFF */
  JoiLev1_00100,  /* 31000..310FF */
  JoiLev1_00100,  /* 31100..311FF */
  JoiLev1_00100,  /* 31200..312FF */
  JoiLev1_00100,  /* 31300..313FF */
  JoiLev1_00100,  /* 31400..314FF */
  JoiLev1_00100,  /* 31500..315FF */
  JoiLev1_00100,  /* 31600..316FF */
  JoiLev1_00100,  /* 31700..317FF */
  JoiLev1_00100,  /* 31800..318FF */
  JoiLev1_00100,  /* 31900..319FF */
  JoiLev1_00100,  /* 31A00..31AFF */
  JoiLev1_00100,  /* 31B00..31BFF */
  JoiLev1_00100,  /* 31C00..31CFF */
  JoiLev1_00100,  /* 31D00..31DFF */
  JoiLev1_00100,  /* 31E00..31EFF */
  JoiLev1_00100,  /* 31F00..31FFF */
  JoiLev1_00100,  /* 32000..320FF */
  JoiLev1_00100,  /* 32100..321FF */
  JoiLev1_00100,  /* 32200..322FF */
  JoiLev1_00100,  /* 32300..323FF */
  JoiLev1_00100,  /* 32400..324FF */
  JoiLev1_00100,  /* 32500..325FF */
  JoiLev1_00100,  /* 32600..326FF */
  JoiLev1_00100,  /* 32700..327FF */
  JoiLev1_00100,  /* 32800..328FF */
  JoiLev1_00100,  /* 32900..329FF */
  JoiLev1_00100,  /* 32A00..32AFF */
  JoiLev1_00100,  /* 32B00..32BFF */
  JoiLev1_00100,  /* 32C00..32CFF */
  JoiLev1_00100,  /* 32D00..32DFF */
  JoiLev1_00100,  /* 32E00..32EFF */
  JoiLev1_00100,  /* 32F00..32FFF */
  JoiLev1_00100,  /* 33000..330FF */
  JoiLev1_00100,  /* 33100..331FF */
  JoiLev1_00100,  /* 33200..332FF */
  JoiLev1_00100,  /* 33300..333FF */
  JoiLev1_00100,  /* 33400..334FF */
  JoiLev1_00100,  /* 33500..335FF */
  JoiLev1_00100,  /* 33600..336FF */
  JoiLev1_00100,  /* 33700..337FF */
  JoiLev1_00100,  /* 33800..338FF */
  JoiLev1_00100,  /* 33900..339FF */
  JoiLev1_00100,  /* 33A00..33AFF */
  JoiLev1_00100,  /* 33B00..33BFF */
  JoiLev1_00100,  /* 33C00..33CFF */
  JoiLev1_00100,  /* 33D00..33DFF */
  JoiLev1_00100,  /* 33E00..33EFF */
  JoiLev1_00100,  /* 33F00..33FFF */
  JoiLev1_00100,  /* 34000..340FF */
  JoiLev1_00100,  /* 34100..341FF */
  JoiLev1_00100,  /* 34200..342FF */
  JoiLev1_00100,  /* 34300..343FF */
  JoiLev1_00100,  /* 34400..344FF */
  JoiLev1_00100,  /* 34500..345FF */
  JoiLev1_00100,  /* 34600..346FF */
  JoiLev1_00100,  /* 34700..347FF */
  JoiLev1_00100,  /* 34800..348FF */
  JoiLev1_00100,  /* 34900..349FF */
  JoiLev1_00100,  /* 34A00..34AFF */
  JoiLev1_00100,  /* 34B00..34BFF */
  JoiLev1_00100,  /* 34C00..34CFF */
  JoiLev1_00100,  /* 34D00..34DFF */
  JoiLev1_00100,  /* 34E00..34EFF */
  JoiLev1_00100,  /* 34F00..34FFF */
  JoiLev1_00100,  /* 35000..350FF */
  JoiLev1_00100,  /* 35100..351FF */
  JoiLev1_00100,  /* 35200..352FF */
  JoiLev1_00100,  /* 35300..353FF */
  JoiLev1_00100,  /* 35400..354FF */
  JoiLev1_00100,  /* 35500..355FF */
  JoiLev1_00100,  /* 35600..356FF */
  JoiLev1_00100,  /* 35700..357FF */
  JoiLev1_00100,  /* 35800..358FF */
  JoiLev1_00100,  /* 35900..359FF */
  JoiLev1_00100,  /* 35A00..35AFF */
  JoiLev1_00100,  /* 35B00..35BFF */
  JoiLev1_00100,  /* 35C00..35CFF */
  JoiLev1_00100,  /* 35D00..35DFF */
  JoiLev1_00100,  /* 35E00..35EFF */
  JoiLev1_00100,  /* 35F00..35FFF */
  JoiLev1_00100,  /* 36000..360FF */
  JoiLev1_00100,  /* 36100..361FF */
  JoiLev1_00100,  /* 36200..362FF */
  JoiLev1_00100,  /* 36300..363FF */
  JoiLev1_00100,  /* 36400..364FF */
  JoiLev1_00100,  /* 36500..365FF */
  JoiLev1_00100,  /* 36600..366FF */
  JoiLev1_00100,  /* 36700..367FF */
  JoiLev1_00100,  /* 36800..368FF */
  JoiLev1_00100,  /* 36900..369FF */
  JoiLev1_00100,  /* 36A00..36AFF */
  JoiLev1_00100,  /* 36B00..36BFF */
  JoiLev1_00100,  /* 36C00..36CFF */
  JoiLev1_00100,  /* 36D00..36DFF */
  JoiLev1_00100,  /* 36E00..36EFF */
  JoiLev1_00100,  /* 36F00..36FFF */
  JoiLev1_00100,  /* 37000..370FF */
  JoiLev1_00100,  /* 37100..371FF */
  JoiLev1_00100,  /* 37200..372FF */
  JoiLev1_00100,  /* 37300..373FF */
  JoiLev1_00100,  /* 37400..374FF */
  JoiLev1_00100,  /* 37500..375FF */
  JoiLev1_00100,  /* 37600..376FF */
  JoiLev1_00100,  /* 37700..377FF */
  JoiLev1_00100,  /* 37800..378FF */
  JoiLev1_00100,  /* 37900..379FF */
  JoiLev1_00100,  /* 37A00..37AFF */
  JoiLev1_00100,  /* 37B00..37BFF */
  JoiLev1_00100,  /* 37C00..37CFF */
  JoiLev1_00100,  /* 37D00..37DFF */
  JoiLev1_00100,  /* 37E00..37EFF */
  JoiLev1_00100,  /* 37F00..37FFF */
  JoiLev1_00100,  /* 38000..380FF */
  JoiLev1_00100,  /* 38100..381FF */
  JoiLev1_00100,  /* 38200..382FF */
  JoiLev1_00100,  /* 38300..383FF */
  JoiLev1_00100,  /* 38400..384FF */
  JoiLev1_00100,  /* 38500..385FF */
  JoiLev1_00100,  /* 38600..386FF */
  JoiLev1_00100,  /* 38700..387FF */
  JoiLev1_00100,  /* 38800..388FF */
  JoiLev1_00100,  /* 38900..389FF */
  JoiLev1_00100,  /* 38A00..38AFF */
  JoiLev1_00100,  /* 38B00..38BFF */
  JoiLev1_00100,  /* 38C00..38CFF */
  JoiLev1_00100,  /* 38D00..38DFF */
  JoiLev1_00100,  /* 38E00..38EFF */
  JoiLev1_00100,  /* 38F00..38FFF */
  JoiLev1_00100,  /* 39000..390FF */
  JoiLev1_00100,  /* 39100..391FF */
  JoiLev1_00100,  /* 39200..392FF */
  JoiLev1_00100,  /* 39300..393FF */
  JoiLev1_00100,  /* 39400..394FF */
  JoiLev1_00100,  /* 39500..395FF */
  JoiLev1_00100,  /* 39600..396FF */
  JoiLev1_00100,  /* 39700..397FF */
  JoiLev1_00100,  /* 39800..398FF */
  JoiLev1_00100,  /* 39900..399FF */
  JoiLev1_00100,  /* 39A00..39AFF */
  JoiLev1_00100,  /* 39B00..39BFF */
  JoiLev1_00100,  /* 39C00..39CFF */
  JoiLev1_00100,  /* 39D00..39DFF */
  JoiLev1_00100,  /* 39E00..39EFF */
  JoiLev1_00100,  /* 39F00..39FFF */
  JoiLev1_00100,  /* 3A000..3A0FF */
  JoiLev1_00100,  /* 3A100..3A1FF */
  JoiLev1_00100,  /* 3A200..3A2FF */
  JoiLev1_00100,  /* 3A300..3A3FF */
  JoiLev1_00100,  /* 3A400..3A4FF */
  JoiLev1_00100,  /* 3A500..3A5FF */
  JoiLev1_00100,  /* 3A600..3A6FF */
  JoiLev1_00100,  /* 3A700..3A7FF */
  JoiLev1_00100,  /* 3A800..3A8FF */
  JoiLev1_00100,  /* 3A900..3A9FF */
  JoiLev1_00100,  /* 3AA00..3AAFF */
  JoiLev1_00100,  /* 3AB00..3ABFF */
  JoiLev1_00100,  /* 3AC00..3ACFF */
  JoiLev1_00100,  /* 3AD00..3ADFF */
  JoiLev1_00100,  /* 3AE00..3AEFF */
  JoiLev1_00100,  /* 3AF00..3AFFF */
  JoiLev1_00100,  /* 3B000..3B0FF */
  JoiLev1_00100,  /* 3B100..3B1FF */
  JoiLev1_00100,  /* 3B200..3B2FF */
  JoiLev1_00100,  /* 3B300..3B3FF */
  JoiLev1_00100,  /* 3B400..3B4FF */
  JoiLev1_00100,  /* 3B500..3B5FF */
  JoiLev1_00100,  /* 3B600..3B6FF */
  JoiLev1_00100,  /* 3B700..3B7FF */
  JoiLev1_00100,  /* 3B800..3B8FF */
  JoiLev1_00100,  /* 3B900..3B9FF */
  JoiLev1_00100,  /* 3BA00..3BAFF */
  JoiLev1_00100,  /* 3BB00..3BBFF */
  JoiLev1_00100,  /* 3BC00..3BCFF */
  JoiLev1_00100,  /* 3BD00..3BDFF */
  JoiLev1_00100,  /* 3BE00..3BEFF */
  JoiLev1_00100,  /* 3BF00..3BFFF */
  JoiLev1_00100,  /* 3C000..3C0FF */
  JoiLev1_00100,  /* 3C100..3C1FF */
  JoiLev1_00100,  /* 3C200..3C2FF */
  JoiLev1_00100,  /* 3C300..3C3FF */
  JoiLev1_00100,  /* 3C400..3C4FF */
  JoiLev1_00100,  /* 3C500..3C5FF */
  JoiLev1_00100,  /* 3C600..3C6FF */
  JoiLev1_00100,  /* 3C700..3C7FF */
  JoiLev1_00100,  /* 3C800..3C8FF */
  JoiLev1_00100,  /* 3C900..3C9FF */
  JoiLev1_00100,  /* 3CA00..3CAFF */
  JoiLev1_00100,  /* 3CB00..3CBFF */
  JoiLev1_00100,  /* 3CC00..3CCFF */
  JoiLev1_00100,  /* 3CD00..3CDFF */
  JoiLev1_00100,  /* 3CE00..3CEFF */
  JoiLev1_00100,  /* 3CF00..3CFFF */
  JoiLev1_00100,  /* 3D000..3D0FF */
  JoiLev1_00100,  /* 3D100..3D1FF */
  JoiLev1_00100,  /* 3D200..3D2FF */
  JoiLev1_00100,  /* 3D300..3D3FF */
  JoiLev1_00100,  /* 3D400..3D4FF */
  JoiLev1_00100,  /* 3D500..3D5FF */
  JoiLev1_00100,  /* 3D600..3D6FF */
  JoiLev1_00100,  /* 3D700..3D7FF */
  JoiLev1_00100,  /* 3D800..3D8FF */
  JoiLev1_00100,  /* 3D900..3D9FF */
  JoiLev1_00100,  /* 3DA00..3DAFF */
  JoiLev1_00100,  /* 3DB00..3DBFF */
  JoiLev1_00100,  /* 3DC00..3DCFF */
  JoiLev1_00100,  /* 3DD00..3DDFF */
  JoiLev1_00100,  /* 3DE00..3DEFF */
  JoiLev1_00100,  /* 3DF00..3DFFF */
  JoiLev1_00100,  /* 3E000..3E0FF */
  JoiLev1_00100,  /* 3E100..3E1FF */
  JoiLev1_00100,  /* 3E200..3E2FF */
  JoiLev1_00100,  /* 3E300..3E3FF */
  JoiLev1_00100,  /* 3E400..3E4FF */
  JoiLev1_00100,  /* 3E500..3E5FF */
  JoiLev1_00100,  /* 3E600..3E6FF */
  JoiLev1_00100,  /* 3E700..3E7FF */
  JoiLev1_00100,  /* 3E800..3E8FF */
  JoiLev1_00100,  /* 3E900..3E9FF */
  JoiLev1_00100,  /* 3EA00..3EAFF */
  JoiLev1_00100,  /* 3EB00..3EBFF */
  JoiLev1_00100,  /* 3EC00..3ECFF */
  JoiLev1_00100,  /* 3ED00..3EDFF */
  JoiLev1_00100,  /* 3EE00..3EEFF */
  JoiLev1_00100,  /* 3EF00..3EFFF */
  JoiLev1_00100,  /* 3F000..3F0FF */
  JoiLev1_00100,  /* 3F100..3F1FF */
  JoiLev1_00100,  /* 3F200..3F2FF */
  JoiLev1_00100,  /* 3F300..3F3FF */
  JoiLev1_00100,  /* 3F400..3F4FF */
  JoiLev1_00100,  /* 3F500..3F5FF */
  JoiLev1_00100,  /* 3F600..3F6FF */
  JoiLev1_00100,  /* 3F700..3F7FF */
  JoiLev1_00100,  /* 3F800..3F8FF */
  JoiLev1_00100,  /* 3F900..3F9FF */
  JoiLev1_00100,  /* 3FA00..3FAFF */
  JoiLev1_00100,  /* 3FB00..3FBFF */
  JoiLev1_00100,  /* 3FC00..3FCFF */
  JoiLev1_00100,  /* 3FD00..3FDFF */
  JoiLev1_00100,  /* 3FE00..3FEFF */
  JoiLev1_00100,  /* 3FF00..3FFFF */
  JoiLev1_00100,  /* 40000..400FF */
  JoiLev1_00100,  /* 40100..401FF */
  JoiLev1_00100,  /* 40200..402FF */
  JoiLev1_00100,  /* 40300..403FF */
  JoiLev1_00100,  /* 40400..404FF */
  JoiLev1_00100,  /* 40500..405FF */
  JoiLev1_00100,  /* 40600..406FF */
  JoiLev1_00100,  /* 40700..407FF */
  JoiLev1_00100,  /* 40800..408FF */
  JoiLev1_00100,  /* 40900..409FF */
  JoiLev1_00100,  /* 40A00..40AFF */
  JoiLev1_00100,  /* 40B00..40BFF */
  JoiLev1_00100,  /* 40C00..40CFF */
  JoiLev1_00100,  /* 40D00..40DFF */
  JoiLev1_00100,  /* 40E00..40EFF */
  JoiLev1_00100,  /* 40F00..40FFF */
  JoiLev1_00100,  /* 41000..410FF */
  JoiLev1_00100,  /* 41100..411FF */
  JoiLev1_00100,  /* 41200..412FF */
  JoiLev1_00100,  /* 41300..413FF */
  JoiLev1_00100,  /* 41400..414FF */
  JoiLev1_00100,  /* 41500..415FF */
  JoiLev1_00100,  /* 41600..416FF */
  JoiLev1_00100,  /* 41700..417FF */
  JoiLev1_00100,  /* 41800..418FF */
  JoiLev1_00100,  /* 41900..419FF */
  JoiLev1_00100,  /* 41A00..41AFF */
  JoiLev1_00100,  /* 41B00..41BFF */
  JoiLev1_00100,  /* 41C00..41CFF */
  JoiLev1_00100,  /* 41D00..41DFF */
  JoiLev1_00100,  /* 41E00..41EFF */
  JoiLev1_00100,  /* 41F00..41FFF */
  JoiLev1_00100,  /* 42000..420FF */
  JoiLev1_00100,  /* 42100..421FF */
  JoiLev1_00100,  /* 42200..422FF */
  JoiLev1_00100,  /* 42300..423FF */
  JoiLev1_00100,  /* 42400..424FF */
  JoiLev1_00100,  /* 42500..425FF */
  JoiLev1_00100,  /* 42600..426FF */
  JoiLev1_00100,  /* 42700..427FF */
  JoiLev1_00100,  /* 42800..428FF */
  JoiLev1_00100,  /* 42900..429FF */
  JoiLev1_00100,  /* 42A00..42AFF */
  JoiLev1_00100,  /* 42B00..42BFF */
  JoiLev1_00100,  /* 42C00..42CFF */
  JoiLev1_00100,  /* 42D00..42DFF */
  JoiLev1_00100,  /* 42E00..42EFF */
  JoiLev1_00100,  /* 42F00..42FFF */
  JoiLev1_00100,  /* 43000..430FF */
  JoiLev1_00100,  /* 43100..431FF */
  JoiLev1_00100,  /* 43200..432FF */
  JoiLev1_00100,  /* 43300..433FF */
  JoiLev1_00100,  /* 43400..434FF */
  JoiLev1_00100,  /* 43500..435FF */
  JoiLev1_00100,  /* 43600..436FF */
  JoiLev1_00100,  /* 43700..437FF */
  JoiLev1_00100,  /* 43800..438FF */
  JoiLev1_00100,  /* 43900..439FF */
  JoiLev1_00100,  /* 43A00..43AFF */
  JoiLev1_00100,  /* 43B00..43BFF */
  JoiLev1_00100,  /* 43C00..43CFF */
  JoiLev1_00100,  /* 43D00..43DFF */
  JoiLev1_00100,  /* 43E00..43EFF */
  JoiLev1_00100,  /* 43F00..43FFF */
  JoiLev1_00100,  /* 44000..440FF */
  JoiLev1_00100,  /* 44100..441FF */
  JoiLev1_00100,  /* 44200..442FF */
  JoiLev1_00100,  /* 44300..443FF */
  JoiLev1_00100,  /* 44400..444FF */
  JoiLev1_00100,  /* 44500..445FF */
  JoiLev1_00100,  /* 44600..446FF */
  JoiLev1_00100,  /* 44700..447FF */
  JoiLev1_00100,  /* 44800..448FF */
  JoiLev1_00100,  /* 44900..449FF */
  JoiLev1_00100,  /* 44A00..44AFF */
  JoiLev1_00100,  /* 44B00..44BFF */
  JoiLev1_00100,  /* 44C00..44CFF */
  JoiLev1_00100,  /* 44D00..44DFF */
  JoiLev1_00100,  /* 44E00..44EFF */
  JoiLev1_00100,  /* 44F00..44FFF */
  JoiLev1_00100,  /* 45000..450FF */
  JoiLev1_00100,  /* 45100..451FF */
  JoiLev1_00100,  /* 45200..452FF */
  JoiLev1_00100,  /* 45300..453FF */
  JoiLev1_00100,  /* 45400..454FF */
  JoiLev1_00100,  /* 45500..455FF */
  JoiLev1_00100,  /* 45600..456FF */
  JoiLev1_00100,  /* 45700..457FF */
  JoiLev1_00100,  /* 45800..458FF */
  JoiLev1_00100,  /* 45900..459FF */
  JoiLev1_00100,  /* 45A00..45AFF */
  JoiLev1_00100,  /* 45B00..45BFF */
  JoiLev1_00100,  /* 45C00..45CFF */
  JoiLev1_00100,  /* 45D00..45DFF */
  JoiLev1_00100,  /* 45E00..45EFF */
  JoiLev1_00100,  /* 45F00..45FFF */
  JoiLev1_00100,  /* 46000..460FF */
  JoiLev1_00100,  /* 46100..461FF */
  JoiLev1_00100,  /* 46200..462FF */
  JoiLev1_00100,  /* 46300..463FF */
  JoiLev1_00100,  /* 46400..464FF */
  JoiLev1_00100,  /* 46500..465FF */
  JoiLev1_00100,  /* 46600..466FF */
  JoiLev1_00100,  /* 46700..467FF */
  JoiLev1_00100,  /* 46800..468FF */
  JoiLev1_00100,  /* 46900..469FF */
  JoiLev1_00100,  /* 46A00..46AFF */
  JoiLev1_00100,  /* 46B00..46BFF */
  JoiLev1_00100,  /* 46C00..46CFF */
  JoiLev1_00100,  /* 46D00..46DFF */
  JoiLev1_00100,  /* 46E00..46EFF */
  JoiLev1_00100,  /* 46F00..46FFF */
  JoiLev1_00100,  /* 47000..470FF */
  JoiLev1_00100,  /* 47100..471FF */
  JoiLev1_00100,  /* 47200..472FF */
  JoiLev1_00100,  /* 47300..473FF */
  JoiLev1_00100,  /* 47400..474FF */
  JoiLev1_00100,  /* 47500..475FF */
  JoiLev1_00100,  /* 47600..476FF */
  JoiLev1_00100,  /* 47700..477FF */
  JoiLev1_00100,  /* 47800..478FF */
  JoiLev1_00100,  /* 47900..479FF */
  JoiLev1_00100,  /* 47A00..47AFF */
  JoiLev1_00100,  /* 47B00..47BFF */
  JoiLev1_00100,  /* 47C00..47CFF */
  JoiLev1_00100,  /* 47D00..47DFF */
  JoiLev1_00100,  /* 47E00..47EFF */
  JoiLev1_00100,  /* 47F00..47FFF */
  JoiLev1_00100,  /* 48000..480FF */
  JoiLev1_00100,  /* 48100..481FF */
  JoiLev1_00100,  /* 48200..482FF */
  JoiLev1_00100,  /* 48300..483FF */
  JoiLev1_00100,  /* 48400..484FF */
  JoiLev1_00100,  /* 48500..485FF */
  JoiLev1_00100,  /* 48600..486FF */
  JoiLev1_00100,  /* 48700..487FF */
  JoiLev1_00100,  /* 48800..488FF */
  JoiLev1_00100,  /* 48900..489FF */
  JoiLev1_00100,  /* 48A00..48AFF */
  JoiLev1_00100,  /* 48B00..48BFF */
  JoiLev1_00100,  /* 48C00..48CFF */
  JoiLev1_00100,  /* 48D00..48DFF */
  JoiLev1_00100,  /* 48E00..48EFF */
  JoiLev1_00100,  /* 48F00..48FFF */
  JoiLev1_00100,  /* 49000..490FF */
  JoiLev1_00100,  /* 49100..491FF */
  JoiLev1_00100,  /* 49200..492FF */
  JoiLev1_00100,  /* 49300..493FF */
  JoiLev1_00100,  /* 49400..494FF */
  JoiLev1_00100,  /* 49500..495FF */
  JoiLev1_00100,  /* 49600..496FF */
  JoiLev1_00100,  /* 49700..497FF */
  JoiLev1_00100,  /* 49800..498FF */
  JoiLev1_00100,  /* 49900..499FF */
  JoiLev1_00100,  /* 49A00..49AFF */
  JoiLev1_00100,  /* 49B00..49BFF */
  JoiLev1_00100,  /* 49C00..49CFF */
  JoiLev1_00100,  /* 49D00..49DFF */
  JoiLev1_00100,  /* 49E00..49EFF */
  JoiLev1_00100,  /* 49F00..49FFF */
  JoiLev1_00100,  /* 4A000..4A0FF */
  JoiLev1_00100,  /* 4A100..4A1FF */
  JoiLev1_00100,  /* 4A200..4A2FF */
  JoiLev1_00100,  /* 4A300..4A3FF */
  JoiLev1_00100,  /* 4A400..4A4FF */
  JoiLev1_00100,  /* 4A500..4A5FF */
  JoiLev1_00100,  /* 4A600..4A6FF */
  JoiLev1_00100,  /* 4A700..4A7FF */
  JoiLev1_00100,  /* 4A800..4A8FF */
  JoiLev1_00100,  /* 4A900..4A9FF */
  JoiLev1_00100,  /* 4AA00..4AAFF */
  JoiLev1_00100,  /* 4AB00..4ABFF */
  JoiLev1_00100,  /* 4AC00..4ACFF */
  JoiLev1_00100,  /* 4AD00..4ADFF */
  JoiLev1_00100,  /* 4AE00..4AEFF */
  JoiLev1_00100,  /* 4AF00..4AFFF */
  JoiLev1_00100,  /* 4B000..4B0FF */
  JoiLev1_00100,  /* 4B100..4B1FF */
  JoiLev1_00100,  /* 4B200..4B2FF */
  JoiLev1_00100,  /* 4B300..4B3FF */
  JoiLev1_00100,  /* 4B400..4B4FF */
  JoiLev1_00100,  /* 4B500..4B5FF */
  JoiLev1_00100,  /* 4B600..4B6FF */
  JoiLev1_00100,  /* 4B700..4B7FF */
  JoiLev1_00100,  /* 4B800..4B8FF */
  JoiLev1_00100,  /* 4B900..4B9FF */
  JoiLev1_00100,  /* 4BA00..4BAFF */
  JoiLev1_00100,  /* 4BB00..4BBFF */
  JoiLev1_00100,  /* 4BC00..4BCFF */
  JoiLev1_00100,  /* 4BD00..4BDFF */
  JoiLev1_00100,  /* 4BE00..4BEFF */
  JoiLev1_00100,  /* 4BF00..4BFFF */
  JoiLev1_00100,  /* 4C000..4C0FF */
  JoiLev1_00100,  /* 4C100..4C1FF */
  JoiLev1_00100,  /* 4C200..4C2FF */
  JoiLev1_00100,  /* 4C300..4C3FF */
  JoiLev1_00100,  /* 4C400..4C4FF */
  JoiLev1_00100,  /* 4C500..4C5FF */
  JoiLev1_00100,  /* 4C600..4C6FF */
  JoiLev1_00100,  /* 4C700..4C7FF */
  JoiLev1_00100,  /* 4C800..4C8FF */
  JoiLev1_00100,  /* 4C900..4C9FF */
  JoiLev1_00100,  /* 4CA00..4CAFF */
  JoiLev1_00100,  /* 4CB00..4CBFF */
  JoiLev1_00100,  /* 4CC00..4CCFF */
  JoiLev1_00100,  /* 4CD00..4CDFF */
  JoiLev1_00100,  /* 4CE00..4CEFF */
  JoiLev1_00100,  /* 4CF00..4CFFF */
  JoiLev1_00100,  /* 4D000..4D0FF */
  JoiLev1_00100,  /* 4D100..4D1FF */
  JoiLev1_00100,  /* 4D200..4D2FF */
  JoiLev1_00100,  /* 4D300..4D3FF */
  JoiLev1_00100,  /* 4D400..4D4FF */
  JoiLev1_00100,  /* 4D500..4D5FF */
  JoiLev1_00100,  /* 4D600..4D6FF */
  JoiLev1_00100,  /* 4D700..4D7FF */
  JoiLev1_00100,  /* 4D800..4D8FF */
  JoiLev1_00100,  /* 4D900..4D9FF */
  JoiLev1_00100,  /* 4DA00..4DAFF */
  JoiLev1_00100,  /* 4DB00..4DBFF */
  JoiLev1_00100,  /* 4DC00..4DCFF */
  JoiLev1_00100,  /* 4DD00..4DDFF */
  JoiLev1_00100,  /* 4DE00..4DEFF */
  JoiLev1_00100,  /* 4DF00..4DFFF */
  JoiLev1_00100,  /* 4E000..4E0FF */
  JoiLev1_00100,  /* 4E100..4E1FF */
  JoiLev1_00100,  /* 4E200..4E2FF */
  JoiLev1_00100,  /* 4E300..4E3FF */
  JoiLev1_00100,  /* 4E400..4E4FF */
  JoiLev1_00100,  /* 4E500..4E5FF */
  JoiLev1_00100,  /* 4E600..4E6FF */
  JoiLev1_00100,  /* 4E700..4E7FF */
  JoiLev1_00100,  /* 4E800..4E8FF */
  JoiLev1_00100,  /* 4E900..4E9FF */
  JoiLev1_00100,  /* 4EA00..4EAFF */
  JoiLev1_00100,  /* 4EB00..4EBFF */
  JoiLev1_00100,  /* 4EC00..4ECFF */
  JoiLev1_00100,  /* 4ED00..4EDFF */
  JoiLev1_00100,  /* 4EE00..4EEFF */
  JoiLev1_00100,  /* 4EF00..4EFFF */
  JoiLev1_00100,  /* 4F000..4F0FF */
  JoiLev1_00100,  /* 4F100..4F1FF */
  JoiLev1_00100,  /* 4F200..4F2FF */
  JoiLev1_00100,  /* 4F300..4F3FF */
  JoiLev1_00100,  /* 4F400..4F4FF */
  JoiLev1_00100,  /* 4F500..4F5FF */
  JoiLev1_00100,  /* 4F600..4F6FF */
  JoiLev1_00100,  /* 4F700..4F7FF */
  JoiLev1_00100,  /* 4F800..4F8FF */
  JoiLev1_00100,  /* 4F900..4F9FF */
  JoiLev1_00100,  /* 4FA00..4FAFF */
  JoiLev1_00100,  /* 4FB00..4FBFF */
  JoiLev1_00100,  /* 4FC00..4FCFF */
  JoiLev1_00100,  /* 4FD00..4FDFF */
  JoiLev1_00100,  /* 4FE00..4FEFF */
  JoiLev1_00100,  /* 4FF00..4FFFF */
  JoiLev1_00100,  /* 50000..500FF */
  JoiLev1_00100,  /* 50100..501FF */
  JoiLev1_00100,  /* 50200..502FF */
  JoiLev1_00100,  /* 50300..503FF */
  JoiLev1_00100,  /* 50400..504FF */
  JoiLev1_00100,  /* 50500..505FF */
  JoiLev1_00100,  /* 50600..506FF */
  JoiLev1_00100,  /* 50700..507FF */
  JoiLev1_00100,  /* 50800..508FF */
  JoiLev1_00100,  /* 50900..509FF */
  JoiLev1_00100,  /* 50A00..50AFF */
  JoiLev1_00100,  /* 50B00..50BFF */
  JoiLev1_00100,  /* 50C00..50CFF */
  JoiLev1_00100,  /* 50D00..50DFF */
  JoiLev1_00100,  /* 50E00..50EFF */
  JoiLev1_00100,  /* 50F00..50FFF */
  JoiLev1_00100,  /* 51000..510FF */
  JoiLev1_00100,  /* 51100..511FF */
  JoiLev1_00100,  /* 51200..512FF */
  JoiLev1_00100,  /* 51300..513FF */
  JoiLev1_00100,  /* 51400..514FF */
  JoiLev1_00100,  /* 51500..515FF */
  JoiLev1_00100,  /* 51600..516FF */
  JoiLev1_00100,  /* 51700..517FF */
  JoiLev1_00100,  /* 51800..518FF */
  JoiLev1_00100,  /* 51900..519FF */
  JoiLev1_00100,  /* 51A00..51AFF */
  JoiLev1_00100,  /* 51B00..51BFF */
  JoiLev1_00100,  /* 51C00..51CFF */
  JoiLev1_00100,  /* 51D00..51DFF */
  JoiLev1_00100,  /* 51E00..51EFF */
  JoiLev1_00100,  /* 51F00..51FFF */
  JoiLev1_00100,  /* 52000..520FF */
  JoiLev1_00100,  /* 52100..521FF */
  JoiLev1_00100,  /* 52200..522FF */
  JoiLev1_00100,  /* 52300..523FF */
  JoiLev1_00100,  /* 52400..524FF */
  JoiLev1_00100,  /* 52500..525FF */
  JoiLev1_00100,  /* 52600..526FF */
  JoiLev1_00100,  /* 52700..527FF */
  JoiLev1_00100,  /* 52800..528FF */
  JoiLev1_00100,  /* 52900..529FF */
  JoiLev1_00100,  /* 52A00..52AFF */
  JoiLev1_00100,  /* 52B00..52BFF */
  JoiLev1_00100,  /* 52C00..52CFF */
  JoiLev1_00100,  /* 52D00..52DFF */
  JoiLev1_00100,  /* 52E00..52EFF */
  JoiLev1_00100,  /* 52F00..52FFF */
  JoiLev1_00100,  /* 53000..530FF */
  JoiLev1_00100,  /* 53100..531FF */
  JoiLev1_00100,  /* 53200..532FF */
  JoiLev1_00100,  /* 53300..533FF */
  JoiLev1_00100,  /* 53400..534FF */
  JoiLev1_00100,  /* 53500..535FF */
  JoiLev1_00100,  /* 53600..536FF */
  JoiLev1_00100,  /* 53700..537FF */
  JoiLev1_00100,  /* 53800..538FF */
  JoiLev1_00100,  /* 53900..539FF */
  JoiLev1_00100,  /* 53A00..53AFF */
  JoiLev1_00100,  /* 53B00..53BFF */
  JoiLev1_00100,  /* 53C00..53CFF */
  JoiLev1_00100,  /* 53D00..53DFF */
  JoiLev1_00100,  /* 53E00..53EFF */
  JoiLev1_00100,  /* 53F00..53FFF */
  JoiLev1_00100,  /* 54000..540FF */
  JoiLev1_00100,  /* 54100..541FF */
  JoiLev1_00100,  /* 54200..542FF */
  JoiLev1_00100,  /* 54300..543FF */
  JoiLev1_00100,  /* 54400..544FF */
  JoiLev1_00100,  /* 54500..545FF */
  JoiLev1_00100,  /* 54600..546FF */
  JoiLev1_00100,  /* 54700..547FF */
  JoiLev1_00100,  /* 54800..548FF */
  JoiLev1_00100,  /* 54900..549FF */
  JoiLev1_00100,  /* 54A00..54AFF */
  JoiLev1_00100,  /* 54B00..54BFF */
  JoiLev1_00100,  /* 54C00..54CFF */
  JoiLev1_00100,  /* 54D00..54DFF */
  JoiLev1_00100,  /* 54E00..54EFF */
  JoiLev1_00100,  /* 54F00..54FFF */
  JoiLev1_00100,  /* 55000..550FF */
  JoiLev1_00100,  /* 55100..551FF */
  JoiLev1_00100,  /* 55200..552FF */
  JoiLev1_00100,  /* 55300..553FF */
  JoiLev1_00100,  /* 55400..554FF */
  JoiLev1_00100,  /* 55500..555FF */
  JoiLev1_00100,  /* 55600..556FF */
  JoiLev1_00100,  /* 55700..557FF */
  JoiLev1_00100,  /* 55800..558FF */
  JoiLev1_00100,  /* 55900..559FF */
  JoiLev1_00100,  /* 55A00..55AFF */
  JoiLev1_00100,  /* 55B00..55BFF */
  JoiLev1_00100,  /* 55C00..55CFF */
  JoiLev1_00100,  /* 55D00..55DFF */
  JoiLev1_00100,  /* 55E00..55EFF */
  JoiLev1_00100,  /* 55F00..55FFF */
  JoiLev1_00100,  /* 56000..560FF */
  JoiLev1_00100,  /* 56100..561FF */
  JoiLev1_00100,  /* 56200..562FF */
  JoiLev1_00100,  /* 56300..563FF */
  JoiLev1_00100,  /* 56400..564FF */
  JoiLev1_00100,  /* 56500..565FF */
  JoiLev1_00100,  /* 56600..566FF */
  JoiLev1_00100,  /* 56700..567FF */
  JoiLev1_00100,  /* 56800..568FF */
  JoiLev1_00100,  /* 56900..569FF */
  JoiLev1_00100,  /* 56A00..56AFF */
  JoiLev1_00100,  /* 56B00..56BFF */
  JoiLev1_00100,  /* 56C00..56CFF */
  JoiLev1_00100,  /* 56D00..56DFF */
  JoiLev1_00100,  /* 56E00..56EFF */
  JoiLev1_00100,  /* 56F00..56FFF */
  JoiLev1_00100,  /* 57000..570FF */
  JoiLev1_00100,  /* 57100..571FF */
  JoiLev1_00100,  /* 57200..572FF */
  JoiLev1_00100,  /* 57300..573FF */
  JoiLev1_00100,  /* 57400..574FF */
  JoiLev1_00100,  /* 57500..575FF */
  JoiLev1_00100,  /* 57600..576FF */
  JoiLev1_00100,  /* 57700..577FF */
  JoiLev1_00100,  /* 57800..578FF */
  JoiLev1_00100,  /* 57900..579FF */
  JoiLev1_00100,  /* 57A00..57AFF */
  JoiLev1_00100,  /* 57B00..57BFF */
  JoiLev1_00100,  /* 57C00..57CFF */
  JoiLev1_00100,  /* 57D00..57DFF */
  JoiLev1_00100,  /* 57E00..57EFF */
  JoiLev1_00100,  /* 57F00..57FFF */
  JoiLev1_00100,  /* 58000..580FF */
  JoiLev1_00100,  /* 58100..581FF */
  JoiLev1_00100,  /* 58200..582FF */
  JoiLev1_00100,  /* 58300..583FF */
  JoiLev1_00100,  /* 58400..584FF */
  JoiLev1_00100,  /* 58500..585FF */
  JoiLev1_00100,  /* 58600..586FF */
  JoiLev1_00100,  /* 58700..587FF */
  JoiLev1_00100,  /* 58800..588FF */
  JoiLev1_00100,  /* 58900..589FF */
  JoiLev1_00100,  /* 58A00..58AFF */
  JoiLev1_00100,  /* 58B00..58BFF */
  JoiLev1_00100,  /* 58C00..58CFF */
  JoiLev1_00100,  /* 58D00..58DFF */
  JoiLev1_00100,  /* 58E00..58EFF */
  JoiLev1_00100,  /* 58F00..58FFF */
  JoiLev1_00100,  /* 59000..590FF */
  JoiLev1_00100,  /* 59100..591FF */
  JoiLev1_00100,  /* 59200..592FF */
  JoiLev1_00100,  /* 59300..593FF */
  JoiLev1_00100,  /* 59400..594FF */
  JoiLev1_00100,  /* 59500..595FF */
  JoiLev1_00100,  /* 59600..596FF */
  JoiLev1_00100,  /* 59700..597FF */
  JoiLev1_00100,  /* 59800..598FF */
  JoiLev1_00100,  /* 59900..599FF */
  JoiLev1_00100,  /* 59A00..59AFF */
  JoiLev1_00100,  /* 59B00..59BFF */
  JoiLev1_00100,  /* 59C00..59CFF */
  JoiLev1_00100,  /* 59D00..59DFF */
  JoiLev1_00100,  /* 59E00..59EFF */
  JoiLev1_00100,  /* 59F00..59FFF */
  JoiLev1_00100,  /* 5A000..5A0FF */
  JoiLev1_00100,  /* 5A100..5A1FF */
  JoiLev1_00100,  /* 5A200..5A2FF */
  JoiLev1_00100,  /* 5A300..5A3FF */
  JoiLev1_00100,  /* 5A400..5A4FF */
  JoiLev1_00100,  /* 5A500..5A5FF */
  JoiLev1_00100,  /* 5A600..5A6FF */
  JoiLev1_00100,  /* 5A700..5A7FF */
  JoiLev1_00100,  /* 5A800..5A8FF */
  JoiLev1_00100,  /* 5A900..5A9FF */
  JoiLev1_00100,  /* 5AA00..5AAFF */
  JoiLev1_00100,  /* 5AB00..5ABFF */
  JoiLev1_00100,  /* 5AC00..5ACFF */
  JoiLev1_00100,  /* 5AD00..5ADFF */
  JoiLev1_00100,  /* 5AE00..5AEFF */
  JoiLev1_00100,  /* 5AF00..5AFFF */
  JoiLev1_00100,  /* 5B000..5B0FF */
  JoiLev1_00100,  /* 5B100..5B1FF */
  JoiLev1_00100,  /* 5B200..5B2FF */
  JoiLev1_00100,  /* 5B300..5B3FF */
  JoiLev1_00100,  /* 5B400..5B4FF */
  JoiLev1_00100,  /* 5B500..5B5FF */
  JoiLev1_00100,  /* 5B600..5B6FF */
  JoiLev1_00100,  /* 5B700..5B7FF */
  JoiLev1_00100,  /* 5B800..5B8FF */
  JoiLev1_00100,  /* 5B900..5B9FF */
  JoiLev1_00100,  /* 5BA00..5BAFF */
  JoiLev1_00100,  /* 5BB00..5BBFF */
  JoiLev1_00100,  /* 5BC00..5BCFF */
  JoiLev1_00100,  /* 5BD00..5BDFF */
  JoiLev1_00100,  /* 5BE00..5BEFF */
  JoiLev1_00100,  /* 5BF00..5BFFF */
  JoiLev1_00100,  /* 5C000..5C0FF */
  JoiLev1_00100,  /* 5C100..5C1FF */
  JoiLev1_00100,  /* 5C200..5C2FF */
  JoiLev1_00100,  /* 5C300..5C3FF */
  JoiLev1_00100,  /* 5C400..5C4FF */
  JoiLev1_00100,  /* 5C500..5C5FF */
  JoiLev1_00100,  /* 5C600..5C6FF */
  JoiLev1_00100,  /* 5C700..5C7FF */
  JoiLev1_00100,  /* 5C800..5C8FF */
  JoiLev1_00100,  /* 5C900..5C9FF */
  JoiLev1_00100,  /* 5CA00..5CAFF */
  JoiLev1_00100,  /* 5CB00..5CBFF */
  JoiLev1_00100,  /* 5CC00..5CCFF */
  JoiLev1_00100,  /* 5CD00..5CDFF */
  JoiLev1_00100,  /* 5CE00..5CEFF */
  JoiLev1_00100,  /* 5CF00..5CFFF */
  JoiLev1_00100,  /* 5D000..5D0FF */
  JoiLev1_00100,  /* 5D100..5D1FF */
  JoiLev1_00100,  /* 5D200..5D2FF */
  JoiLev1_00100,  /* 5D300..5D3FF */
  JoiLev1_00100,  /* 5D400..5D4FF */
  JoiLev1_00100,  /* 5D500..5D5FF */
  JoiLev1_00100,  /* 5D600..5D6FF */
  JoiLev1_00100,  /* 5D700..5D7FF */
  JoiLev1_00100,  /* 5D800..5D8FF */
  JoiLev1_00100,  /* 5D900..5D9FF */
  JoiLev1_00100,  /* 5DA00..5DAFF */
  JoiLev1_00100,  /* 5DB00..5DBFF */
  JoiLev1_00100,  /* 5DC00..5DCFF */
  JoiLev1_00100,  /* 5DD00..5DDFF */
  JoiLev1_00100,  /* 5DE00..5DEFF */
  JoiLev1_00100,  /* 5DF00..5DFFF */
  JoiLev1_00100,  /* 5E000..5E0FF */
  JoiLev1_00100,  /* 5E100..5E1FF */
  JoiLev1_00100,  /* 5E200..5E2FF */
  JoiLev1_00100,  /* 5E300..5E3FF */
  JoiLev1_00100,  /* 5E400..5E4FF */
  JoiLev1_00100,  /* 5E500..5E5FF */
  JoiLev1_00100,  /* 5E600..5E6FF */
  JoiLev1_00100,  /* 5E700..5E7FF */
  JoiLev1_00100,  /* 5E800..5E8FF */
  JoiLev1_00100,  /* 5E900..5E9FF */
  JoiLev1_00100,  /* 5EA00..5EAFF */
  JoiLev1_00100,  /* 5EB00..5EBFF */
  JoiLev1_00100,  /* 5EC00..5ECFF */
  JoiLev1_00100,  /* 5ED00..5EDFF */
  JoiLev1_00100,  /* 5EE00..5EEFF */
  JoiLev1_00100,  /* 5EF00..5EFFF */
  JoiLev1_00100,  /* 5F000..5F0FF */
  JoiLev1_00100,  /* 5F100..5F1FF */
  JoiLev1_00100,  /* 5F200..5F2FF */
  JoiLev1_00100,  /* 5F300..5F3FF */
  JoiLev1_00100,  /* 5F400..5F4FF */
  JoiLev1_00100,  /* 5F500..5F5FF */
  JoiLev1_00100,  /* 5F600..5F6FF */
  JoiLev1_00100,  /* 5F700..5F7FF */
  JoiLev1_00100,  /* 5F800..5F8FF */
  JoiLev1_00100,  /* 5F900..5F9FF */
  JoiLev1_00100,  /* 5FA00..5FAFF */
  JoiLev1_00100,  /* 5FB00..5FBFF */
  JoiLev1_00100,  /* 5FC00..5FCFF */
  JoiLev1_00100,  /* 5FD00..5FDFF */
  JoiLev1_00100,  /* 5FE00..5FEFF */
  JoiLev1_00100,  /* 5FF00..5FFFF */
  JoiLev1_00100,  /* 60000..600FF */
  JoiLev1_00100,  /* 60100..601FF */
  JoiLev1_00100,  /* 60200..602FF */
  JoiLev1_00100,  /* 60300..603FF */
  JoiLev1_00100,  /* 60400..604FF */
  JoiLev1_00100,  /* 60500..605FF */
  JoiLev1_00100,  /* 60600..606FF */
  JoiLev1_00100,  /* 60700..607FF */
  JoiLev1_00100,  /* 60800..608FF */
  JoiLev1_00100,  /* 60900..609FF */
  JoiLev1_00100,  /* 60A00..60AFF */
  JoiLev1_00100,  /* 60B00..60BFF */
  JoiLev1_00100,  /* 60C00..60CFF */
  JoiLev1_00100,  /* 60D00..60DFF */
  JoiLev1_00100,  /* 60E00..60EFF */
  JoiLev1_00100,  /* 60F00..60FFF */
  JoiLev1_00100,  /* 61000..610FF */
  JoiLev1_00100,  /* 61100..611FF */
  JoiLev1_00100,  /* 61200..612FF */
  JoiLev1_00100,  /* 61300..613FF */
  JoiLev1_00100,  /* 61400..614FF */
  JoiLev1_00100,  /* 61500..615FF */
  JoiLev1_00100,  /* 61600..616FF */
  JoiLev1_00100,  /* 61700..617FF */
  JoiLev1_00100,  /* 61800..618FF */
  JoiLev1_00100,  /* 61900..619FF */
  JoiLev1_00100,  /* 61A00..61AFF */
  JoiLev1_00100,  /* 61B00..61BFF */
  JoiLev1_00100,  /* 61C00..61CFF */
  JoiLev1_00100,  /* 61D00..61DFF */
  JoiLev1_00100,  /* 61E00..61EFF */
  JoiLev1_00100,  /* 61F00..61FFF */
  JoiLev1_00100,  /* 62000..620FF */
  JoiLev1_00100,  /* 62100..621FF */
  JoiLev1_00100,  /* 62200..622FF */
  JoiLev1_00100,  /* 62300..623FF */
  JoiLev1_00100,  /* 62400..624FF */
  JoiLev1_00100,  /* 62500..625FF */
  JoiLev1_00100,  /* 62600..626FF */
  JoiLev1_00100,  /* 62700..627FF */
  JoiLev1_00100,  /* 62800..628FF */
  JoiLev1_00100,  /* 62900..629FF */
  JoiLev1_00100,  /* 62A00..62AFF */
  JoiLev1_00100,  /* 62B00..62BFF */
  JoiLev1_00100,  /* 62C00..62CFF */
  JoiLev1_00100,  /* 62D00..62DFF */
  JoiLev1_00100,  /* 62E00..62EFF */
  JoiLev1_00100,  /* 62F00..62FFF */
  JoiLev1_00100,  /* 63000..630FF */
  JoiLev1_00100,  /* 63100..631FF */
  JoiLev1_00100,  /* 63200..632FF */
  JoiLev1_00100,  /* 63300..633FF */
  JoiLev1_00100,  /* 63400..634FF */
  JoiLev1_00100,  /* 63500..635FF */
  JoiLev1_00100,  /* 63600..636FF */
  JoiLev1_00100,  /* 63700..637FF */
  JoiLev1_00100,  /* 63800..638FF */
  JoiLev1_00100,  /* 63900..639FF */
  JoiLev1_00100,  /* 63A00..63AFF */
  JoiLev1_00100,  /* 63B00..63BFF */
  JoiLev1_00100,  /* 63C00..63CFF */
  JoiLev1_00100,  /* 63D00..63DFF */
  JoiLev1_00100,  /* 63E00..63EFF */
  JoiLev1_00100,  /* 63F00..63FFF */
  JoiLev1_00100,  /* 64000..640FF */
  JoiLev1_00100,  /* 64100..641FF */
  JoiLev1_00100,  /* 64200..642FF */
  JoiLev1_00100,  /* 64300..643FF */
  JoiLev1_00100,  /* 64400..644FF */
  JoiLev1_00100,  /* 64500..645FF */
  JoiLev1_00100,  /* 64600..646FF */
  JoiLev1_00100,  /* 64700..647FF */
  JoiLev1_00100,  /* 64800..648FF */
  JoiLev1_00100,  /* 64900..649FF */
  JoiLev1_00100,  /* 64A00..64AFF */
  JoiLev1_00100,  /* 64B00..64BFF */
  JoiLev1_00100,  /* 64C00..64CFF */
  JoiLev1_00100,  /* 64D00..64DFF */
  JoiLev1_00100,  /* 64E00..64EFF */
  JoiLev1_00100,  /* 64F00..64FFF */
  JoiLev1_00100,  /* 65000..650FF */
  JoiLev1_00100,  /* 65100..651FF */
  JoiLev1_00100,  /* 65200..652FF */
  JoiLev1_00100,  /* 65300..653FF */
  JoiLev1_00100,  /* 65400..654FF */
  JoiLev1_00100,  /* 65500..655FF */
  JoiLev1_00100,  /* 65600..656FF */
  JoiLev1_00100,  /* 65700..657FF */
  JoiLev1_00100,  /* 65800..658FF */
  JoiLev1_00100,  /* 65900..659FF */
  JoiLev1_00100,  /* 65A00..65AFF */
  JoiLev1_00100,  /* 65B00..65BFF */
  JoiLev1_00100,  /* 65C00..65CFF */
  JoiLev1_00100,  /* 65D00..65DFF */
  JoiLev1_00100,  /* 65E00..65EFF */
  JoiLev1_00100,  /* 65F00..65FFF */
  JoiLev1_00100,  /* 66000..660FF */
  JoiLev1_00100,  /* 66100..661FF */
  JoiLev1_00100,  /* 66200..662FF */
  JoiLev1_00100,  /* 66300..663FF */
  JoiLev1_00100,  /* 66400..664FF */
  JoiLev1_00100,  /* 66500..665FF */
  JoiLev1_00100,  /* 66600..666FF */
  JoiLev1_00100,  /* 66700..667FF */
  JoiLev1_00100,  /* 66800..668FF */
  JoiLev1_00100,  /* 66900..669FF */
  JoiLev1_00100,  /* 66A00..66AFF */
  JoiLev1_00100,  /* 66B00..66BFF */
  JoiLev1_00100,  /* 66C00..66CFF */
  JoiLev1_00100,  /* 66D00..66DFF */
  JoiLev1_00100,  /* 66E00..66EFF */
  JoiLev1_00100,  /* 66F00..66FFF */
  JoiLev1_00100,  /* 67000..670FF */
  JoiLev1_00100,  /* 67100..671FF */
  JoiLev1_00100,  /* 67200..672FF */
  JoiLev1_00100,  /* 67300..673FF */
  JoiLev1_00100,  /* 67400..674FF */
  JoiLev1_00100,  /* 67500..675FF */
  JoiLev1_00100,  /* 67600..676FF */
  JoiLev1_00100,  /* 67700..677FF */
  JoiLev1_00100,  /* 67800..678FF */
  JoiLev1_00100,  /* 67900..679FF */
  JoiLev1_00100,  /* 67A00..67AFF */
  JoiLev1_00100,  /* 67B00..67BFF */
  JoiLev1_00100,  /* 67C00..67CFF */
  JoiLev1_00100,  /* 67D00..67DFF */
  JoiLev1_00100,  /* 67E00..67EFF */
  JoiLev1_00100,  /* 67F00..67FFF */
  JoiLev1_00100,  /* 68000..680FF */
  JoiLev1_00100,  /* 68100..681FF */
  JoiLev1_00100,  /* 68200..682FF */
  JoiLev1_00100,  /* 68300..683FF */
  JoiLev1_00100,  /* 68400..684FF */
  JoiLev1_00100,  /* 68500..685FF */
  JoiLev1_00100,  /* 68600..686FF */
  JoiLev1_00100,  /* 68700..687FF */
  JoiLev1_00100,  /* 68800..688FF */
  JoiLev1_00100,  /* 68900..689FF */
  JoiLev1_00100,  /* 68A00..68AFF */
  JoiLev1_00100,  /* 68B00..68BFF */
  JoiLev1_00100,  /* 68C00..68CFF */
  JoiLev1_00100,  /* 68D00..68DFF */
  JoiLev1_00100,  /* 68E00..68EFF */
  JoiLev1_00100,  /* 68F00..68FFF */
  JoiLev1_00100,  /* 69000..690FF */
  JoiLev1_00100,  /* 69100..691FF */
  JoiLev1_00100,  /* 69200..692FF */
  JoiLev1_00100,  /* 69300..693FF */
  JoiLev1_00100,  /* 69400..694FF */
  JoiLev1_00100,  /* 69500..695FF */
  JoiLev1_00100,  /* 69600..696FF */
  JoiLev1_00100,  /* 69700..697FF */
  JoiLev1_00100,  /* 69800..698FF */
  JoiLev1_00100,  /* 69900..699FF */
  JoiLev1_00100,  /* 69A00..69AFF */
  JoiLev1_00100,  /* 69B00..69BFF */
  JoiLev1_00100,  /* 69C00..69CFF */
  JoiLev1_00100,  /* 69D00..69DFF */
  JoiLev1_00100,  /* 69E00..69EFF */
  JoiLev1_00100,  /* 69F00..69FFF */
  JoiLev1_00100,  /* 6A000..6A0FF */
  JoiLev1_00100,  /* 6A100..6A1FF */
  JoiLev1_00100,  /* 6A200..6A2FF */
  JoiLev1_00100,  /* 6A300..6A3FF */
  JoiLev1_00100,  /* 6A400..6A4FF */
  JoiLev1_00100,  /* 6A500..6A5FF */
  JoiLev1_00100,  /* 6A600..6A6FF */
  JoiLev1_00100,  /* 6A700..6A7FF */
  JoiLev1_00100,  /* 6A800..6A8FF */
  JoiLev1_00100,  /* 6A900..6A9FF */
  JoiLev1_00100,  /* 6AA00..6AAFF */
  JoiLev1_00100,  /* 6AB00..6ABFF */
  JoiLev1_00100,  /* 6AC00..6ACFF */
  JoiLev1_00100,  /* 6AD00..6ADFF */
  JoiLev1_00100,  /* 6AE00..6AEFF */
  JoiLev1_00100,  /* 6AF00..6AFFF */
  JoiLev1_00100,  /* 6B000..6B0FF */
  JoiLev1_00100,  /* 6B100..6B1FF */
  JoiLev1_00100,  /* 6B200..6B2FF */
  JoiLev1_00100,  /* 6B300..6B3FF */
  JoiLev1_00100,  /* 6B400..6B4FF */
  JoiLev1_00100,  /* 6B500..6B5FF */
  JoiLev1_00100,  /* 6B600..6B6FF */
  JoiLev1_00100,  /* 6B700..6B7FF */
  JoiLev1_00100,  /* 6B800..6B8FF */
  JoiLev1_00100,  /* 6B900..6B9FF */
  JoiLev1_00100,  /* 6BA00..6BAFF */
  JoiLev1_00100,  /* 6BB00..6BBFF */
  JoiLev1_00100,  /* 6BC00..6BCFF */
  JoiLev1_00100,  /* 6BD00..6BDFF */
  JoiLev1_00100,  /* 6BE00..6BEFF */
  JoiLev1_00100,  /* 6BF00..6BFFF */
  JoiLev1_00100,  /* 6C000..6C0FF */
  JoiLev1_00100,  /* 6C100..6C1FF */
  JoiLev1_00100,  /* 6C200..6C2FF */
  JoiLev1_00100,  /* 6C300..6C3FF */
  JoiLev1_00100,  /* 6C400..6C4FF */
  JoiLev1_00100,  /* 6C500..6C5FF */
  JoiLev1_00100,  /* 6C600..6C6FF */
  JoiLev1_00100,  /* 6C700..6C7FF */
  JoiLev1_00100,  /* 6C800..6C8FF */
  JoiLev1_00100,  /* 6C900..6C9FF */
  JoiLev1_00100,  /* 6CA00..6CAFF */
  JoiLev1_00100,  /* 6CB00..6CBFF */
  JoiLev1_00100,  /* 6CC00..6CCFF */
  JoiLev1_00100,  /* 6CD00..6CDFF */
  JoiLev1_00100,  /* 6CE00..6CEFF */
  JoiLev1_00100,  /* 6CF00..6CFFF */
  JoiLev1_00100,  /* 6D000..6D0FF */
  JoiLev1_00100,  /* 6D100..6D1FF */
  JoiLev1_00100,  /* 6D200..6D2FF */
  JoiLev1_00100,  /* 6D300..6D3FF */
  JoiLev1_00100,  /* 6D400..6D4FF */
  JoiLev1_00100,  /* 6D500..6D5FF */
  JoiLev1_00100,  /* 6D600..6D6FF */
  JoiLev1_00100,  /* 6D700..6D7FF */
  JoiLev1_00100,  /* 6D800..6D8FF */
  JoiLev1_00100,  /* 6D900..6D9FF */
  JoiLev1_00100,  /* 6DA00..6DAFF */
  JoiLev1_00100,  /* 6DB00..6DBFF */
  JoiLev1_00100,  /* 6DC00..6DCFF */
  JoiLev1_00100,  /* 6DD00..6DDFF */
  JoiLev1_00100,  /* 6DE00..6DEFF */
  JoiLev1_00100,  /* 6DF00..6DFFF */
  JoiLev1_00100,  /* 6E000..6E0FF */
  JoiLev1_00100,  /* 6E100..6E1FF */
  JoiLev1_00100,  /* 6E200..6E2FF */
  JoiLev1_00100,  /* 6E300..6E3FF */
  JoiLev1_00100,  /* 6E400..6E4FF */
  JoiLev1_00100,  /* 6E500..6E5FF */
  JoiLev1_00100,  /* 6E600..6E6FF */
  JoiLev1_00100,  /* 6E700..6E7FF */
  JoiLev1_00100,  /* 6E800..6E8FF */
  JoiLev1_00100,  /* 6E900..6E9FF */
  JoiLev1_00100,  /* 6EA00..6EAFF */
  JoiLev1_00100,  /* 6EB00..6EBFF */
  JoiLev1_00100,  /* 6EC00..6ECFF */
  JoiLev1_00100,  /* 6ED00..6EDFF */
  JoiLev1_00100,  /* 6EE00..6EEFF */
  JoiLev1_00100,  /* 6EF00..6EFFF */
  JoiLev1_00100,  /* 6F000..6F0FF */
  JoiLev1_00100,  /* 6F100..6F1FF */
  JoiLev1_00100,  /* 6F200..6F2FF */
  JoiLev1_00100,  /* 6F300..6F3FF */
  JoiLev1_00100,  /* 6F400..6F4FF */
  JoiLev1_00100,  /* 6F500..6F5FF */
  JoiLev1_00100,  /* 6F600..6F6FF */
  JoiLev1_00100,  /* 6F700..6F7FF */
  JoiLev1_00100,  /* 6F800..6F8FF */
  JoiLev1_00100,  /* 6F900..6F9FF */
  JoiLev1_00100,  /* 6FA00..6FAFF */
  JoiLev1_00100,  /* 6FB00..6FBFF */
  JoiLev1_00100,  /* 6FC00..6FCFF */
  JoiLev1_00100,  /* 6FD00..6FDFF */
  JoiLev1_00100,  /* 6FE00..6FEFF */
  JoiLev1_00100,  /* 6FF00..6FFFF */
  JoiLev1_00100,  /* 70000..700FF */
  JoiLev1_00100,  /* 70100..701FF */
  JoiLev1_00100,  /* 70200..702FF */
  JoiLev1_00100,  /* 70300..703FF */
  JoiLev1_00100,  /* 70400..704FF */
  JoiLev1_00100,  /* 70500..705FF */
  JoiLev1_00100,  /* 70600..706FF */
  JoiLev1_00100,  /* 70700..707FF */
  JoiLev1_00100,  /* 70800..708FF */
  JoiLev1_00100,  /* 70900..709FF */
  JoiLev1_00100,  /* 70A00..70AFF */
  JoiLev1_00100,  /* 70B00..70BFF */
  JoiLev1_00100,  /* 70C00..70CFF */
  JoiLev1_00100,  /* 70D00..70DFF */
  JoiLev1_00100,  /* 70E00..70EFF */
  JoiLev1_00100,  /* 70F00..70FFF */
  JoiLev1_00100,  /* 71000..710FF */
  JoiLev1_00100,  /* 71100..711FF */
  JoiLev1_00100,  /* 71200..712FF */
  JoiLev1_00100,  /* 71300..713FF */
  JoiLev1_00100,  /* 71400..714FF */
  JoiLev1_00100,  /* 71500..715FF */
  JoiLev1_00100,  /* 71600..716FF */
  JoiLev1_00100,  /* 71700..717FF */
  JoiLev1_00100,  /* 71800..718FF */
  JoiLev1_00100,  /* 71900..719FF */
  JoiLev1_00100,  /* 71A00..71AFF */
  JoiLev1_00100,  /* 71B00..71BFF */
  JoiLev1_00100,  /* 71C00..71CFF */
  JoiLev1_00100,  /* 71D00..71DFF */
  JoiLev1_00100,  /* 71E00..71EFF */
  JoiLev1_00100,  /* 71F00..71FFF */
  JoiLev1_00100,  /* 72000..720FF */
  JoiLev1_00100,  /* 72100..721FF */
  JoiLev1_00100,  /* 72200..722FF */
  JoiLev1_00100,  /* 72300..723FF */
  JoiLev1_00100,  /* 72400..724FF */
  JoiLev1_00100,  /* 72500..725FF */
  JoiLev1_00100,  /* 72600..726FF */
  JoiLev1_00100,  /* 72700..727FF */
  JoiLev1_00100,  /* 72800..728FF */
  JoiLev1_00100,  /* 72900..729FF */
  JoiLev1_00100,  /* 72A00..72AFF */
  JoiLev1_00100,  /* 72B00..72BFF */
  JoiLev1_00100,  /* 72C00..72CFF */
  JoiLev1_00100,  /* 72D00..72DFF */
  JoiLev1_00100,  /* 72E00..72EFF */
  JoiLev1_00100,  /* 72F00..72FFF */
  JoiLev1_00100,  /* 73000..730FF */
  JoiLev1_00100,  /* 73100..731FF */
  JoiLev1_00100,  /* 73200..732FF */
  JoiLev1_00100,  /* 73300..733FF */
  JoiLev1_00100,  /* 73400..734FF */
  JoiLev1_00100,  /* 73500..735FF */
  JoiLev1_00100,  /* 73600..736FF */
  JoiLev1_00100,  /* 73700..737FF */
  JoiLev1_00100,  /* 73800..738FF */
  JoiLev1_00100,  /* 73900..739FF */
  JoiLev1_00100,  /* 73A00..73AFF */
  JoiLev1_00100,  /* 73B00..73BFF */
  JoiLev1_00100,  /* 73C00..73CFF */
  JoiLev1_00100,  /* 73D00..73DFF */
  JoiLev1_00100,  /* 73E00..73EFF */
  JoiLev1_00100,  /* 73F00..73FFF */
  JoiLev1_00100,  /* 74000..740FF */
  JoiLev1_00100,  /* 74100..741FF */
  JoiLev1_00100,  /* 74200..742FF */
  JoiLev1_00100,  /* 74300..743FF */
  JoiLev1_00100,  /* 74400..744FF */
  JoiLev1_00100,  /* 74500..745FF */
  JoiLev1_00100,  /* 74600..746FF */
  JoiLev1_00100,  /* 74700..747FF */
  JoiLev1_00100,  /* 74800..748FF */
  JoiLev1_00100,  /* 74900..749FF */
  JoiLev1_00100,  /* 74A00..74AFF */
  JoiLev1_00100,  /* 74B00..74BFF */
  JoiLev1_00100,  /* 74C00..74CFF */
  JoiLev1_00100,  /* 74D00..74DFF */
  JoiLev1_00100,  /* 74E00..74EFF */
  JoiLev1_00100,  /* 74F00..74FFF */
  JoiLev1_00100,  /* 75000..750FF */
  JoiLev1_00100,  /* 75100..751FF */
  JoiLev1_00100,  /* 75200..752FF */
  JoiLev1_00100,  /* 75300..753FF */
  JoiLev1_00100,  /* 75400..754FF */
  JoiLev1_00100,  /* 75500..755FF */
  JoiLev1_00100,  /* 75600..756FF */
  JoiLev1_00100,  /* 75700..757FF */
  JoiLev1_00100,  /* 75800..758FF */
  JoiLev1_00100,  /* 75900..759FF */
  JoiLev1_00100,  /* 75A00..75AFF */
  JoiLev1_00100,  /* 75B00..75BFF */
  JoiLev1_00100,  /* 75C00..75CFF */
  JoiLev1_00100,  /* 75D00..75DFF */
  JoiLev1_00100,  /* 75E00..75EFF */
  JoiLev1_00100,  /* 75F00..75FFF */
  JoiLev1_00100,  /* 76000..760FF */
  JoiLev1_00100,  /* 76100..761FF */
  JoiLev1_00100,  /* 76200..762FF */
  JoiLev1_00100,  /* 76300..763FF */
  JoiLev1_00100,  /* 76400..764FF */
  JoiLev1_00100,  /* 76500..765FF */
  JoiLev1_00100,  /* 76600..766FF */
  JoiLev1_00100,  /* 76700..767FF */
  JoiLev1_00100,  /* 76800..768FF */
  JoiLev1_00100,  /* 76900..769FF */
  JoiLev1_00100,  /* 76A00..76AFF */
  JoiLev1_00100,  /* 76B00..76BFF */
  JoiLev1_00100,  /* 76C00..76CFF */
  JoiLev1_00100,  /* 76D00..76DFF */
  JoiLev1_00100,  /* 76E00..76EFF */
  JoiLev1_00100,  /* 76F00..76FFF */
  JoiLev1_00100,  /* 77000..770FF */
  JoiLev1_00100,  /* 77100..771FF */
  JoiLev1_00100,  /* 77200..772FF */
  JoiLev1_00100,  /* 77300..773FF */
  JoiLev1_00100,  /* 77400..774FF */
  JoiLev1_00100,  /* 77500..775FF */
  JoiLev1_00100,  /* 77600..776FF */
  JoiLev1_00100,  /* 77700..777FF */
  JoiLev1_00100,  /* 77800..778FF */
  JoiLev1_00100,  /* 77900..779FF */
  JoiLev1_00100,  /* 77A00..77AFF */
  JoiLev1_00100,  /* 77B00..77BFF */
  JoiLev1_00100,  /* 77C00..77CFF */
  JoiLev1_00100,  /* 77D00..77DFF */
  JoiLev1_00100,  /* 77E00..77EFF */
  JoiLev1_00100,  /* 77F00..77FFF */
  JoiLev1_00100,  /* 78000..780FF */
  JoiLev1_00100,  /* 78100..781FF */
  JoiLev1_00100,  /* 78200..782FF */
  JoiLev1_00100,  /* 78300..783FF */
  JoiLev1_00100,  /* 78400..784FF */
  JoiLev1_00100,  /* 78500..785FF */
  JoiLev1_00100,  /* 78600..786FF */
  JoiLev1_00100,  /* 78700..787FF */
  JoiLev1_00100,  /* 78800..788FF */
  JoiLev1_00100,  /* 78900..789FF */
  JoiLev1_00100,  /* 78A00..78AFF */
  JoiLev1_00100,  /* 78B00..78BFF */
  JoiLev1_00100,  /* 78C00..78CFF */
  JoiLev1_00100,  /* 78D00..78DFF */
  JoiLev1_00100,  /* 78E00..78EFF */
  JoiLev1_00100,  /* 78F00..78FFF */
  JoiLev1_00100,  /* 79000..790FF */
  JoiLev1_00100,  /* 79100..791FF */
  JoiLev1_00100,  /* 79200..792FF */
  JoiLev1_00100,  /* 79300..793FF */
  JoiLev1_00100,  /* 79400..794FF */
  JoiLev1_00100,  /* 79500..795FF */
  JoiLev1_00100,  /* 79600..796FF */
  JoiLev1_00100,  /* 79700..797FF */
  JoiLev1_00100,  /* 79800..798FF */
  JoiLev1_00100,  /* 79900..799FF */
  JoiLev1_00100,  /* 79A00..79AFF */
  JoiLev1_00100,  /* 79B00..79BFF */
  JoiLev1_00100,  /* 79C00..79CFF */
  JoiLev1_00100,  /* 79D00..79DFF */
  JoiLev1_00100,  /* 79E00..79EFF */
  JoiLev1_00100,  /* 79F00..79FFF */
  JoiLev1_00100,  /* 7A000..7A0FF */
  JoiLev1_00100,  /* 7A100..7A1FF */
  JoiLev1_00100,  /* 7A200..7A2FF */
  JoiLev1_00100,  /* 7A300..7A3FF */
  JoiLev1_00100,  /* 7A400..7A4FF */
  JoiLev1_00100,  /* 7A500..7A5FF */
  JoiLev1_00100,  /* 7A600..7A6FF */
  JoiLev1_00100,  /* 7A700..7A7FF */
  JoiLev1_00100,  /* 7A800..7A8FF */
  JoiLev1_00100,  /* 7A900..7A9FF */
  JoiLev1_00100,  /* 7AA00..7AAFF */
  JoiLev1_00100,  /* 7AB00..7ABFF */
  JoiLev1_00100,  /* 7AC00..7ACFF */
  JoiLev1_00100,  /* 7AD00..7ADFF */
  JoiLev1_00100,  /* 7AE00..7AEFF */
  JoiLev1_00100,  /* 7AF00..7AFFF */
  JoiLev1_00100,  /* 7B000..7B0FF */
  JoiLev1_00100,  /* 7B100..7B1FF */
  JoiLev1_00100,  /* 7B200..7B2FF */
  JoiLev1_00100,  /* 7B300..7B3FF */
  JoiLev1_00100,  /* 7B400..7B4FF */
  JoiLev1_00100,  /* 7B500..7B5FF */
  JoiLev1_00100,  /* 7B600..7B6FF */
  JoiLev1_00100,  /* 7B700..7B7FF */
  JoiLev1_00100,  /* 7B800..7B8FF */
  JoiLev1_00100,  /* 7B900..7B9FF */
  JoiLev1_00100,  /* 7BA00..7BAFF */
  JoiLev1_00100,  /* 7BB00..7BBFF */
  JoiLev1_00100,  /* 7BC00..7BCFF */
  JoiLev1_00100,  /* 7BD00..7BDFF */
  JoiLev1_00100,  /* 7BE00..7BEFF */
  JoiLev1_00100,  /* 7BF00..7BFFF */
  JoiLev1_00100,  /* 7C000..7C0FF */
  JoiLev1_00100,  /* 7C100..7C1FF */
  JoiLev1_00100,  /* 7C200..7C2FF */
  JoiLev1_00100,  /* 7C300..7C3FF */
  JoiLev1_00100,  /* 7C400..7C4FF */
  JoiLev1_00100,  /* 7C500..7C5FF */
  JoiLev1_00100,  /* 7C600..7C6FF */
  JoiLev1_00100,  /* 7C700..7C7FF */
  JoiLev1_00100,  /* 7C800..7C8FF */
  JoiLev1_00100,  /* 7C900..7C9FF */
  JoiLev1_00100,  /* 7CA00..7CAFF */
  JoiLev1_00100,  /* 7CB00..7CBFF */
  JoiLev1_00100,  /* 7CC00..7CCFF */
  JoiLev1_00100,  /* 7CD00..7CDFF */
  JoiLev1_00100,  /* 7CE00..7CEFF */
  JoiLev1_00100,  /* 7CF00..7CFFF */
  JoiLev1_00100,  /* 7D000..7D0FF */
  JoiLev1_00100,  /* 7D100..7D1FF */
  JoiLev1_00100,  /* 7D200..7D2FF */
  JoiLev1_00100,  /* 7D300..7D3FF */
  JoiLev1_00100,  /* 7D400..7D4FF */
  JoiLev1_00100,  /* 7D500..7D5FF */
  JoiLev1_00100,  /* 7D600..7D6FF */
  JoiLev1_00100,  /* 7D700..7D7FF */
  JoiLev1_00100,  /* 7D800..7D8FF */
  JoiLev1_00100,  /* 7D900..7D9FF */
  JoiLev1_00100,  /* 7DA00..7DAFF */
  JoiLev1_00100,  /* 7DB00..7DBFF */
  JoiLev1_00100,  /* 7DC00..7DCFF */
  JoiLev1_00100,  /* 7DD00..7DDFF */
  JoiLev1_00100,  /* 7DE00..7DEFF */
  JoiLev1_00100,  /* 7DF00..7DFFF */
  JoiLev1_00100,  /* 7E000..7E0FF */
  JoiLev1_00100,  /* 7E100..7E1FF */
  JoiLev1_00100,  /* 7E200..7E2FF */
  JoiLev1_00100,  /* 7E300..7E3FF */
  JoiLev1_00100,  /* 7E400..7E4FF */
  JoiLev1_00100,  /* 7E500..7E5FF */
  JoiLev1_00100,  /* 7E600..7E6FF */
  JoiLev1_00100,  /* 7E700..7E7FF */
  JoiLev1_00100,  /* 7E800..7E8FF */
  JoiLev1_00100,  /* 7E900..7E9FF */
  JoiLev1_00100,  /* 7EA00..7EAFF */
  JoiLev1_00100,  /* 7EB00..7EBFF */
  JoiLev1_00100,  /* 7EC00..7ECFF */
  JoiLev1_00100,  /* 7ED00..7EDFF */
  JoiLev1_00100,  /* 7EE00..7EEFF */
  JoiLev1_00100,  /* 7EF00..7EFFF */
  JoiLev1_00100,  /* 7F000..7F0FF */
  JoiLev1_00100,  /* 7F100..7F1FF */
  JoiLev1_00100,  /* 7F200..7F2FF */
  JoiLev1_00100,  /* 7F300..7F3FF */
  JoiLev1_00100,  /* 7F400..7F4FF */
  JoiLev1_00100,  /* 7F500..7F5FF */
  JoiLev1_00100,  /* 7F600..7F6FF */
  JoiLev1_00100,  /* 7F700..7F7FF */
  JoiLev1_00100,  /* 7F800..7F8FF */
  JoiLev1_00100,  /* 7F900..7F9FF */
  JoiLev1_00100,  /* 7FA00..7FAFF */
  JoiLev1_00100,  /* 7FB00..7FBFF */
  JoiLev1_00100,  /* 7FC00..7FCFF */
  JoiLev1_00100,  /* 7FD00..7FDFF */
  JoiLev1_00100,  /* 7FE00..7FEFF */
  JoiLev1_00100,  /* 7FF00..7FFFF */
  JoiLev1_00100,  /* 80000..800FF */
  JoiLev1_00100,  /* 80100..801FF */
  JoiLev1_00100,  /* 80200..802FF */
  JoiLev1_00100,  /* 80300..803FF */
  JoiLev1_00100,  /* 80400..804FF */
  JoiLev1_00100,  /* 80500..805FF */
  JoiLev1_00100,  /* 80600..806FF */
  JoiLev1_00100,  /* 80700..807FF */
  JoiLev1_00100,  /* 80800..808FF */
  JoiLev1_00100,  /* 80900..809FF */
  JoiLev1_00100,  /* 80A00..80AFF */
  JoiLev1_00100,  /* 80B00..80BFF */
  JoiLev1_00100,  /* 80C00..80CFF */
  JoiLev1_00100,  /* 80D00..80DFF */
  JoiLev1_00100,  /* 80E00..80EFF */
  JoiLev1_00100,  /* 80F00..80FFF */
  JoiLev1_00100,  /* 81000..810FF */
  JoiLev1_00100,  /* 81100..811FF */
  JoiLev1_00100,  /* 81200..812FF */
  JoiLev1_00100,  /* 81300..813FF */
  JoiLev1_00100,  /* 81400..814FF */
  JoiLev1_00100,  /* 81500..815FF */
  JoiLev1_00100,  /* 81600..816FF */
  JoiLev1_00100,  /* 81700..817FF */
  JoiLev1_00100,  /* 81800..818FF */
  JoiLev1_00100,  /* 81900..819FF */
  JoiLev1_00100,  /* 81A00..81AFF */
  JoiLev1_00100,  /* 81B00..81BFF */
  JoiLev1_00100,  /* 81C00..81CFF */
  JoiLev1_00100,  /* 81D00..81DFF */
  JoiLev1_00100,  /* 81E00..81EFF */
  JoiLev1_00100,  /* 81F00..81FFF */
  JoiLev1_00100,  /* 82000..820FF */
  JoiLev1_00100,  /* 82100..821FF */
  JoiLev1_00100,  /* 82200..822FF */
  JoiLev1_00100,  /* 82300..823FF */
  JoiLev1_00100,  /* 82400..824FF */
  JoiLev1_00100,  /* 82500..825FF */
  JoiLev1_00100,  /* 82600..826FF */
  JoiLev1_00100,  /* 82700..827FF */
  JoiLev1_00100,  /* 82800..828FF */
  JoiLev1_00100,  /* 82900..829FF */
  JoiLev1_00100,  /* 82A00..82AFF */
  JoiLev1_00100,  /* 82B00..82BFF */
  JoiLev1_00100,  /* 82C00..82CFF */
  JoiLev1_00100,  /* 82D00..82DFF */
  JoiLev1_00100,  /* 82E00..82EFF */
  JoiLev1_00100,  /* 82F00..82FFF */
  JoiLev1_00100,  /* 83000..830FF */
  JoiLev1_00100,  /* 83100..831FF */
  JoiLev1_00100,  /* 83200..832FF */
  JoiLev1_00100,  /* 83300..833FF */
  JoiLev1_00100,  /* 83400..834FF */
  JoiLev1_00100,  /* 83500..835FF */
  JoiLev1_00100,  /* 83600..836FF */
  JoiLev1_00100,  /* 83700..837FF */
  JoiLev1_00100,  /* 83800..838FF */
  JoiLev1_00100,  /* 83900..839FF */
  JoiLev1_00100,  /* 83A00..83AFF */
  JoiLev1_00100,  /* 83B00..83BFF */
  JoiLev1_00100,  /* 83C00..83CFF */
  JoiLev1_00100,  /* 83D00..83DFF */
  JoiLev1_00100,  /* 83E00..83EFF */
  JoiLev1_00100,  /* 83F00..83FFF */
  JoiLev1_00100,  /* 84000..840FF */
  JoiLev1_00100,  /* 84100..841FF */
  JoiLev1_00100,  /* 84200..842FF */
  JoiLev1_00100,  /* 84300..843FF */
  JoiLev1_00100,  /* 84400..844FF */
  JoiLev1_00100,  /* 84500..845FF */
  JoiLev1_00100,  /* 84600..846FF */
  JoiLev1_00100,  /* 84700..847FF */
  JoiLev1_00100,  /* 84800..848FF */
  JoiLev1_00100,  /* 84900..849FF */
  JoiLev1_00100,  /* 84A00..84AFF */
  JoiLev1_00100,  /* 84B00..84BFF */
  JoiLev1_00100,  /* 84C00..84CFF */
  JoiLev1_00100,  /* 84D00..84DFF */
  JoiLev1_00100,  /* 84E00..84EFF */
  JoiLev1_00100,  /* 84F00..84FFF */
  JoiLev1_00100,  /* 85000..850FF */
  JoiLev1_00100,  /* 85100..851FF */
  JoiLev1_00100,  /* 85200..852FF */
  JoiLev1_00100,  /* 85300..853FF */
  JoiLev1_00100,  /* 85400..854FF */
  JoiLev1_00100,  /* 85500..855FF */
  JoiLev1_00100,  /* 85600..856FF */
  JoiLev1_00100,  /* 85700..857FF */
  JoiLev1_00100,  /* 85800..858FF */
  JoiLev1_00100,  /* 85900..859FF */
  JoiLev1_00100,  /* 85A00..85AFF */
  JoiLev1_00100,  /* 85B00..85BFF */
  JoiLev1_00100,  /* 85C00..85CFF */
  JoiLev1_00100,  /* 85D00..85DFF */
  JoiLev1_00100,  /* 85E00..85EFF */
  JoiLev1_00100,  /* 85F00..85FFF */
  JoiLev1_00100,  /* 86000..860FF */
  JoiLev1_00100,  /* 86100..861FF */
  JoiLev1_00100,  /* 86200..862FF */
  JoiLev1_00100,  /* 86300..863FF */
  JoiLev1_00100,  /* 86400..864FF */
  JoiLev1_00100,  /* 86500..865FF */
  JoiLev1_00100,  /* 86600..866FF */
  JoiLev1_00100,  /* 86700..867FF */
  JoiLev1_00100,  /* 86800..868FF */
  JoiLev1_00100,  /* 86900..869FF */
  JoiLev1_00100,  /* 86A00..86AFF */
  JoiLev1_00100,  /* 86B00..86BFF */
  JoiLev1_00100,  /* 86C00..86CFF */
  JoiLev1_00100,  /* 86D00..86DFF */
  JoiLev1_00100,  /* 86E00..86EFF */
  JoiLev1_00100,  /* 86F00..86FFF */
  JoiLev1_00100,  /* 87000..870FF */
  JoiLev1_00100,  /* 87100..871FF */
  JoiLev1_00100,  /* 87200..872FF */
  JoiLev1_00100,  /* 87300..873FF */
  JoiLev1_00100,  /* 87400..874FF */
  JoiLev1_00100,  /* 87500..875FF */
  JoiLev1_00100,  /* 87600..876FF */
  JoiLev1_00100,  /* 87700..877FF */
  JoiLev1_00100,  /* 87800..878FF */
  JoiLev1_00100,  /* 87900..879FF */
  JoiLev1_00100,  /* 87A00..87AFF */
  JoiLev1_00100,  /* 87B00..87BFF */
  JoiLev1_00100,  /* 87C00..87CFF */
  JoiLev1_00100,  /* 87D00..87DFF */
  JoiLev1_00100,  /* 87E00..87EFF */
  JoiLev1_00100,  /* 87F00..87FFF */
  JoiLev1_00100,  /* 88000..880FF */
  JoiLev1_00100,  /* 88100..881FF */
  JoiLev1_00100,  /* 88200..882FF */
  JoiLev1_00100,  /* 88300..883FF */
  JoiLev1_00100,  /* 88400..884FF */
  JoiLev1_00100,  /* 88500..885FF */
  JoiLev1_00100,  /* 88600..886FF */
  JoiLev1_00100,  /* 88700..887FF */
  JoiLev1_00100,  /* 88800..888FF */
  JoiLev1_00100,  /* 88900..889FF */
  JoiLev1_00100,  /* 88A00..88AFF */
  JoiLev1_00100,  /* 88B00..88BFF */
  JoiLev1_00100,  /* 88C00..88CFF */
  JoiLev1_00100,  /* 88D00..88DFF */
  JoiLev1_00100,  /* 88E00..88EFF */
  JoiLev1_00100,  /* 88F00..88FFF */
  JoiLev1_00100,  /* 89000..890FF */
  JoiLev1_00100,  /* 89100..891FF */
  JoiLev1_00100,  /* 89200..892FF */
  JoiLev1_00100,  /* 89300..893FF */
  JoiLev1_00100,  /* 89400..894FF */
  JoiLev1_00100,  /* 89500..895FF */
  JoiLev1_00100,  /* 89600..896FF */
  JoiLev1_00100,  /* 89700..897FF */
  JoiLev1_00100,  /* 89800..898FF */
  JoiLev1_00100,  /* 89900..899FF */
  JoiLev1_00100,  /* 89A00..89AFF */
  JoiLev1_00100,  /* 89B00..89BFF */
  JoiLev1_00100,  /* 89C00..89CFF */
  JoiLev1_00100,  /* 89D00..89DFF */
  JoiLev1_00100,  /* 89E00..89EFF */
  JoiLev1_00100,  /* 89F00..89FFF */
  JoiLev1_00100,  /* 8A000..8A0FF */
  JoiLev1_00100,  /* 8A100..8A1FF */
  JoiLev1_00100,  /* 8A200..8A2FF */
  JoiLev1_00100,  /* 8A300..8A3FF */
  JoiLev1_00100,  /* 8A400..8A4FF */
  JoiLev1_00100,  /* 8A500..8A5FF */
  JoiLev1_00100,  /* 8A600..8A6FF */
  JoiLev1_00100,  /* 8A700..8A7FF */
  JoiLev1_00100,  /* 8A800..8A8FF */
  JoiLev1_00100,  /* 8A900..8A9FF */
  JoiLev1_00100,  /* 8AA00..8AAFF */
  JoiLev1_00100,  /* 8AB00..8ABFF */
  JoiLev1_00100,  /* 8AC00..8ACFF */
  JoiLev1_00100,  /* 8AD00..8ADFF */
  JoiLev1_00100,  /* 8AE00..8AEFF */
  JoiLev1_00100,  /* 8AF00..8AFFF */
  JoiLev1_00100,  /* 8B000..8B0FF */
  JoiLev1_00100,  /* 8B100..8B1FF */
  JoiLev1_00100,  /* 8B200..8B2FF */
  JoiLev1_00100,  /* 8B300..8B3FF */
  JoiLev1_00100,  /* 8B400..8B4FF */
  JoiLev1_00100,  /* 8B500..8B5FF */
  JoiLev1_00100,  /* 8B600..8B6FF */
  JoiLev1_00100,  /* 8B700..8B7FF */
  JoiLev1_00100,  /* 8B800..8B8FF */
  JoiLev1_00100,  /* 8B900..8B9FF */
  JoiLev1_00100,  /* 8BA00..8BAFF */
  JoiLev1_00100,  /* 8BB00..8BBFF */
  JoiLev1_00100,  /* 8BC00..8BCFF */
  JoiLev1_00100,  /* 8BD00..8BDFF */
  JoiLev1_00100,  /* 8BE00..8BEFF */
  JoiLev1_00100,  /* 8BF00..8BFFF */
  JoiLev1_00100,  /* 8C000..8C0FF */
  JoiLev1_00100,  /* 8C100..8C1FF */
  JoiLev1_00100,  /* 8C200..8C2FF */
  JoiLev1_00100,  /* 8C300..8C3FF */
  JoiLev1_00100,  /* 8C400..8C4FF */
  JoiLev1_00100,  /* 8C500..8C5FF */
  JoiLev1_00100,  /* 8C600..8C6FF */
  JoiLev1_00100,  /* 8C700..8C7FF */
  JoiLev1_00100,  /* 8C800..8C8FF */
  JoiLev1_00100,  /* 8C900..8C9FF */
  JoiLev1_00100,  /* 8CA00..8CAFF */
  JoiLev1_00100,  /* 8CB00..8CBFF */
  JoiLev1_00100,  /* 8CC00..8CCFF */
  JoiLev1_00100,  /* 8CD00..8CDFF */
  JoiLev1_00100,  /* 8CE00..8CEFF */
  JoiLev1_00100,  /* 8CF00..8CFFF */
  JoiLev1_00100,  /* 8D000..8D0FF */
  JoiLev1_00100,  /* 8D100..8D1FF */
  JoiLev1_00100,  /* 8D200..8D2FF */
  JoiLev1_00100,  /* 8D300..8D3FF */
  JoiLev1_00100,  /* 8D400..8D4FF */
  JoiLev1_00100,  /* 8D500..8D5FF */
  JoiLev1_00100,  /* 8D600..8D6FF */
  JoiLev1_00100,  /* 8D700..8D7FF */
  JoiLev1_00100,  /* 8D800..8D8FF */
  JoiLev1_00100,  /* 8D900..8D9FF */
  JoiLev1_00100,  /* 8DA00..8DAFF */
  JoiLev1_00100,  /* 8DB00..8DBFF */
  JoiLev1_00100,  /* 8DC00..8DCFF */
  JoiLev1_00100,  /* 8DD00..8DDFF */
  JoiLev1_00100,  /* 8DE00..8DEFF */
  JoiLev1_00100,  /* 8DF00..8DFFF */
  JoiLev1_00100,  /* 8E000..8E0FF */
  JoiLev1_00100,  /* 8E100..8E1FF */
  JoiLev1_00100,  /* 8E200..8E2FF */
  JoiLev1_00100,  /* 8E300..8E3FF */
  JoiLev1_00100,  /* 8E400..8E4FF */
  JoiLev1_00100,  /* 8E500..8E5FF */
  JoiLev1_00100,  /* 8E600..8E6FF */
  JoiLev1_00100,  /* 8E700..8E7FF */
  JoiLev1_00100,  /* 8E800..8E8FF */
  JoiLev1_00100,  /* 8E900..8E9FF */
  JoiLev1_00100,  /* 8EA00..8EAFF */
  JoiLev1_00100,  /* 8EB00..8EBFF */
  JoiLev1_00100,  /* 8EC00..8ECFF */
  JoiLev1_00100,  /* 8ED00..8EDFF */
  JoiLev1_00100,  /* 8EE00..8EEFF */
  JoiLev1_00100,  /* 8EF00..8EFFF */
  JoiLev1_00100,  /* 8F000..8F0FF */
  JoiLev1_00100,  /* 8F100..8F1FF */
  JoiLev1_00100,  /* 8F200..8F2FF */
  JoiLev1_00100,  /* 8F300..8F3FF */
  JoiLev1_00100,  /* 8F400..8F4FF */
  JoiLev1_00100,  /* 8F500..8F5FF */
  JoiLev1_00100,  /* 8F600..8F6FF */
  JoiLev1_00100,  /* 8F700..8F7FF */
  JoiLev1_00100,  /* 8F800..8F8FF */
  JoiLev1_00100,  /* 8F900..8F9FF */
  JoiLev1_00100,  /* 8FA00..8FAFF */
  JoiLev1_00100,  /* 8FB00..8FBFF */
  JoiLev1_00100,  /* 8FC00..8FCFF */
  JoiLev1_00100,  /* 8FD00..8FDFF */
  JoiLev1_00100,  /* 8FE00..8FEFF */
  JoiLev1_00100,  /* 8FF00..8FFFF */
  JoiLev1_00100,  /* 90000..900FF */
  JoiLev1_00100,  /* 90100..901FF */
  JoiLev1_00100,  /* 90200..902FF */
  JoiLev1_00100,  /* 90300..903FF */
  JoiLev1_00100,  /* 90400..904FF */
  JoiLev1_00100,  /* 90500..905FF */
  JoiLev1_00100,  /* 90600..906FF */
  JoiLev1_00100,  /* 90700..907FF */
  JoiLev1_00100,  /* 90800..908FF */
  JoiLev1_00100,  /* 90900..909FF */
  JoiLev1_00100,  /* 90A00..90AFF */
  JoiLev1_00100,  /* 90B00..90BFF */
  JoiLev1_00100,  /* 90C00..90CFF */
  JoiLev1_00100,  /* 90D00..90DFF */
  JoiLev1_00100,  /* 90E00..90EFF */
  JoiLev1_00100,  /* 90F00..90FFF */
  JoiLev1_00100,  /* 91000..910FF */
  JoiLev1_00100,  /* 91100..911FF */
  JoiLev1_00100,  /* 91200..912FF */
  JoiLev1_00100,  /* 91300..913FF */
  JoiLev1_00100,  /* 91400..914FF */
  JoiLev1_00100,  /* 91500..915FF */
  JoiLev1_00100,  /* 91600..916FF */
  JoiLev1_00100,  /* 91700..917FF */
  JoiLev1_00100,  /* 91800..918FF */
  JoiLev1_00100,  /* 91900..919FF */
  JoiLev1_00100,  /* 91A00..91AFF */
  JoiLev1_00100,  /* 91B00..91BFF */
  JoiLev1_00100,  /* 91C00..91CFF */
  JoiLev1_00100,  /* 91D00..91DFF */
  JoiLev1_00100,  /* 91E00..91EFF */
  JoiLev1_00100,  /* 91F00..91FFF */
  JoiLev1_00100,  /* 92000..920FF */
  JoiLev1_00100,  /* 92100..921FF */
  JoiLev1_00100,  /* 92200..922FF */
  JoiLev1_00100,  /* 92300..923FF */
  JoiLev1_00100,  /* 92400..924FF */
  JoiLev1_00100,  /* 92500..925FF */
  JoiLev1_00100,  /* 92600..926FF */
  JoiLev1_00100,  /* 92700..927FF */
  JoiLev1_00100,  /* 92800..928FF */
  JoiLev1_00100,  /* 92900..929FF */
  JoiLev1_00100,  /* 92A00..92AFF */
  JoiLev1_00100,  /* 92B00..92BFF */
  JoiLev1_00100,  /* 92C00..92CFF */
  JoiLev1_00100,  /* 92D00..92DFF */
  JoiLev1_00100,  /* 92E00..92EFF */
  JoiLev1_00100,  /* 92F00..92FFF */
  JoiLev1_00100,  /* 93000..930FF */
  JoiLev1_00100,  /* 93100..931FF */
  JoiLev1_00100,  /* 93200..932FF */
  JoiLev1_00100,  /* 93300..933FF */
  JoiLev1_00100,  /* 93400..934FF */
  JoiLev1_00100,  /* 93500..935FF */
  JoiLev1_00100,  /* 93600..936FF */
  JoiLev1_00100,  /* 93700..937FF */
  JoiLev1_00100,  /* 93800..938FF */
  JoiLev1_00100,  /* 93900..939FF */
  JoiLev1_00100,  /* 93A00..93AFF */
  JoiLev1_00100,  /* 93B00..93BFF */
  JoiLev1_00100,  /* 93C00..93CFF */
  JoiLev1_00100,  /* 93D00..93DFF */
  JoiLev1_00100,  /* 93E00..93EFF */
  JoiLev1_00100,  /* 93F00..93FFF */
  JoiLev1_00100,  /* 94000..940FF */
  JoiLev1_00100,  /* 94100..941FF */
  JoiLev1_00100,  /* 94200..942FF */
  JoiLev1_00100,  /* 94300..943FF */
  JoiLev1_00100,  /* 94400..944FF */
  JoiLev1_00100,  /* 94500..945FF */
  JoiLev1_00100,  /* 94600..946FF */
  JoiLev1_00100,  /* 94700..947FF */
  JoiLev1_00100,  /* 94800..948FF */
  JoiLev1_00100,  /* 94900..949FF */
  JoiLev1_00100,  /* 94A00..94AFF */
  JoiLev1_00100,  /* 94B00..94BFF */
  JoiLev1_00100,  /* 94C00..94CFF */
  JoiLev1_00100,  /* 94D00..94DFF */
  JoiLev1_00100,  /* 94E00..94EFF */
  JoiLev1_00100,  /* 94F00..94FFF */
  JoiLev1_00100,  /* 95000..950FF */
  JoiLev1_00100,  /* 95100..951FF */
  JoiLev1_00100,  /* 95200..952FF */
  JoiLev1_00100,  /* 95300..953FF */
  JoiLev1_00100,  /* 95400..954FF */
  JoiLev1_00100,  /* 95500..955FF */
  JoiLev1_00100,  /* 95600..956FF */
  JoiLev1_00100,  /* 95700..957FF */
  JoiLev1_00100,  /* 95800..958FF */
  JoiLev1_00100,  /* 95900..959FF */
  JoiLev1_00100,  /* 95A00..95AFF */
  JoiLev1_00100,  /* 95B00..95BFF */
  JoiLev1_00100,  /* 95C00..95CFF */
  JoiLev1_00100,  /* 95D00..95DFF */
  JoiLev1_00100,  /* 95E00..95EFF */
  JoiLev1_00100,  /* 95F00..95FFF */
  JoiLev1_00100,  /* 96000..960FF */
  JoiLev1_00100,  /* 96100..961FF */
  JoiLev1_00100,  /* 96200..962FF */
  JoiLev1_00100,  /* 96300..963FF */
  JoiLev1_00100,  /* 96400..964FF */
  JoiLev1_00100,  /* 96500..965FF */
  JoiLev1_00100,  /* 96600..966FF */
  JoiLev1_00100,  /* 96700..967FF */
  JoiLev1_00100,  /* 96800..968FF */
  JoiLev1_00100,  /* 96900..969FF */
  JoiLev1_00100,  /* 96A00..96AFF */
  JoiLev1_00100,  /* 96B00..96BFF */
  JoiLev1_00100,  /* 96C00..96CFF */
  JoiLev1_00100,  /* 96D00..96DFF */
  JoiLev1_00100,  /* 96E00..96EFF */
  JoiLev1_00100,  /* 96F00..96FFF */
  JoiLev1_00100,  /* 97000..970FF */
  JoiLev1_00100,  /* 97100..971FF */
  JoiLev1_00100,  /* 97200..972FF */
  JoiLev1_00100,  /* 97300..973FF */
  JoiLev1_00100,  /* 97400..974FF */
  JoiLev1_00100,  /* 97500..975FF */
  JoiLev1_00100,  /* 97600..976FF */
  JoiLev1_00100,  /* 97700..977FF */
  JoiLev1_00100,  /* 97800..978FF */
  JoiLev1_00100,  /* 97900..979FF */
  JoiLev1_00100,  /* 97A00..97AFF */
  JoiLev1_00100,  /* 97B00..97BFF */
  JoiLev1_00100,  /* 97C00..97CFF */
  JoiLev1_00100,  /* 97D00..97DFF */
  JoiLev1_00100,  /* 97E00..97EFF */
  JoiLev1_00100,  /* 97F00..97FFF */
  JoiLev1_00100,  /* 98000..980FF */
  JoiLev1_00100,  /* 98100..981FF */
  JoiLev1_00100,  /* 98200..982FF */
  JoiLev1_00100,  /* 98300..983FF */
  JoiLev1_00100,  /* 98400..984FF */
  JoiLev1_00100,  /* 98500..985FF */
  JoiLev1_00100,  /* 98600..986FF */
  JoiLev1_00100,  /* 98700..987FF */
  JoiLev1_00100,  /* 98800..988FF */
  JoiLev1_00100,  /* 98900..989FF */
  JoiLev1_00100,  /* 98A00..98AFF */
  JoiLev1_00100,  /* 98B00..98BFF */
  JoiLev1_00100,  /* 98C00..98CFF */
  JoiLev1_00100,  /* 98D00..98DFF */
  JoiLev1_00100,  /* 98E00..98EFF */
  JoiLev1_00100,  /* 98F00..98FFF */
  JoiLev1_00100,  /* 99000..990FF */
  JoiLev1_00100,  /* 99100..991FF */
  JoiLev1_00100,  /* 99200..992FF */
  JoiLev1_00100,  /* 99300..993FF */
  JoiLev1_00100,  /* 99400..994FF */
  JoiLev1_00100,  /* 99500..995FF */
  JoiLev1_00100,  /* 99600..996FF */
  JoiLev1_00100,  /* 99700..997FF */
  JoiLev1_00100,  /* 99800..998FF */
  JoiLev1_00100,  /* 99900..999FF */
  JoiLev1_00100,  /* 99A00..99AFF */
  JoiLev1_00100,  /* 99B00..99BFF */
  JoiLev1_00100,  /* 99C00..99CFF */
  JoiLev1_00100,  /* 99D00..99DFF */
  JoiLev1_00100,  /* 99E00..99EFF */
  JoiLev1_00100,  /* 99F00..99FFF */
  JoiLev1_00100,  /* 9A000..9A0FF */
  JoiLev1_00100,  /* 9A100..9A1FF */
  JoiLev1_00100,  /* 9A200..9A2FF */
  JoiLev1_00100,  /* 9A300..9A3FF */
  JoiLev1_00100,  /* 9A400..9A4FF */
  JoiLev1_00100,  /* 9A500..9A5FF */
  JoiLev1_00100,  /* 9A600..9A6FF */
  JoiLev1_00100,  /* 9A700..9A7FF */
  JoiLev1_00100,  /* 9A800..9A8FF */
  JoiLev1_00100,  /* 9A900..9A9FF */
  JoiLev1_00100,  /* 9AA00..9AAFF */
  JoiLev1_00100,  /* 9AB00..9ABFF */
  JoiLev1_00100,  /* 9AC00..9ACFF */
  JoiLev1_00100,  /* 9AD00..9ADFF */
  JoiLev1_00100,  /* 9AE00..9AEFF */
  JoiLev1_00100,  /* 9AF00..9AFFF */
  JoiLev1_00100,  /* 9B000..9B0FF */
  JoiLev1_00100,  /* 9B100..9B1FF */
  JoiLev1_00100,  /* 9B200..9B2FF */
  JoiLev1_00100,  /* 9B300..9B3FF */
  JoiLev1_00100,  /* 9B400..9B4FF */
  JoiLev1_00100,  /* 9B500..9B5FF */
  JoiLev1_00100,  /* 9B600..9B6FF */
  JoiLev1_00100,  /* 9B700..9B7FF */
  JoiLev1_00100,  /* 9B800..9B8FF */
  JoiLev1_00100,  /* 9B900..9B9FF */
  JoiLev1_00100,  /* 9BA00..9BAFF */
  JoiLev1_00100,  /* 9BB00..9BBFF */
  JoiLev1_00100,  /* 9BC00..9BCFF */
  JoiLev1_00100,  /* 9BD00..9BDFF */
  JoiLev1_00100,  /* 9BE00..9BEFF */
  JoiLev1_00100,  /* 9BF00..9BFFF */
  JoiLev1_00100,  /* 9C000..9C0FF */
  JoiLev1_00100,  /* 9C100..9C1FF */
  JoiLev1_00100,  /* 9C200..9C2FF */
  JoiLev1_00100,  /* 9C300..9C3FF */
  JoiLev1_00100,  /* 9C400..9C4FF */
  JoiLev1_00100,  /* 9C500..9C5FF */
  JoiLev1_00100,  /* 9C600..9C6FF */
  JoiLev1_00100,  /* 9C700..9C7FF */
  JoiLev1_00100,  /* 9C800..9C8FF */
  JoiLev1_00100,  /* 9C900..9C9FF */
  JoiLev1_00100,  /* 9CA00..9CAFF */
  JoiLev1_00100,  /* 9CB00..9CBFF */
  JoiLev1_00100,  /* 9CC00..9CCFF */
  JoiLev1_00100,  /* 9CD00..9CDFF */
  JoiLev1_00100,  /* 9CE00..9CEFF */
  JoiLev1_00100,  /* 9CF00..9CFFF */
  JoiLev1_00100,  /* 9D000..9D0FF */
  JoiLev1_00100,  /* 9D100..9D1FF */
  JoiLev1_00100,  /* 9D200..9D2FF */
  JoiLev1_00100,  /* 9D300..9D3FF */
  JoiLev1_00100,  /* 9D400..9D4FF */
  JoiLev1_00100,  /* 9D500..9D5FF */
  JoiLev1_00100,  /* 9D600..9D6FF */
  JoiLev1_00100,  /* 9D700..9D7FF */
  JoiLev1_00100,  /* 9D800..9D8FF */
  JoiLev1_00100,  /* 9D900..9D9FF */
  JoiLev1_00100,  /* 9DA00..9DAFF */
  JoiLev1_00100,  /* 9DB00..9DBFF */
  JoiLev1_00100,  /* 9DC00..9DCFF */
  JoiLev1_00100,  /* 9DD00..9DDFF */
  JoiLev1_00100,  /* 9DE00..9DEFF */
  JoiLev1_00100,  /* 9DF00..9DFFF */
  JoiLev1_00100,  /* 9E000..9E0FF */
  JoiLev1_00100,  /* 9E100..9E1FF */
  JoiLev1_00100,  /* 9E200..9E2FF */
  JoiLev1_00100,  /* 9E300..9E3FF */
  JoiLev1_00100,  /* 9E400..9E4FF */
  JoiLev1_00100,  /* 9E500..9E5FF */
  JoiLev1_00100,  /* 9E600..9E6FF */
  JoiLev1_00100,  /* 9E700..9E7FF */
  JoiLev1_00100,  /* 9E800..9E8FF */
  JoiLev1_00100,  /* 9E900..9E9FF */
  JoiLev1_00100,  /* 9EA00..9EAFF */
  JoiLev1_00100,  /* 9EB00..9EBFF */
  JoiLev1_00100,  /* 9EC00..9ECFF */
  JoiLev1_00100,  /* 9ED00..9EDFF */
  JoiLev1_00100,  /* 9EE00..9EEFF */
  JoiLev1_00100,  /* 9EF00..9EFFF */
  JoiLev1_00100,  /* 9F000..9F0FF */
  JoiLev1_00100,  /* 9F100..9F1FF */
  JoiLev1_00100,  /* 9F200..9F2FF */
  JoiLev1_00100,  /* 9F300..9F3FF */
  JoiLev1_00100,  /* 9F400..9F4FF */
  JoiLev1_00100,  /* 9F500..9F5FF */
  JoiLev1_00100,  /* 9F600..9F6FF */
  JoiLev1_00100,  /* 9F700..9F7FF */
  JoiLev1_00100,  /* 9F800..9F8FF */
  JoiLev1_00100,  /* 9F900..9F9FF */
  JoiLev1_00100,  /* 9FA00..9FAFF */
  JoiLev1_00100,  /* 9FB00..9FBFF */
  JoiLev1_00100,  /* 9FC00..9FCFF */
  JoiLev1_00100,  /* 9FD00..9FDFF */
  JoiLev1_00100,  /* 9FE00..9FEFF */
  JoiLev1_00100,  /* 9FF00..9FFFF */
  JoiLev1_00100,  /* A0000..A00FF */
  JoiLev1_00100,  /* A0100..A01FF */
  JoiLev1_00100,  /* A0200..A02FF */
  JoiLev1_00100,  /* A0300..A03FF */
  JoiLev1_00100,  /* A0400..A04FF */
  JoiLev1_00100,  /* A0500..A05FF */
  JoiLev1_00100,  /* A0600..A06FF */
  JoiLev1_00100,  /* A0700..A07FF */
  JoiLev1_00100,  /* A0800..A08FF */
  JoiLev1_00100,  /* A0900..A09FF */
  JoiLev1_00100,  /* A0A00..A0AFF */
  JoiLev1_00100,  /* A0B00..A0BFF */
  JoiLev1_00100,  /* A0C00..A0CFF */
  JoiLev1_00100,  /* A0D00..A0DFF */
  JoiLev1_00100,  /* A0E00..A0EFF */
  JoiLev1_00100,  /* A0F00..A0FFF */
  JoiLev1_00100,  /* A1000..A10FF */
  JoiLev1_00100,  /* A1100..A11FF */
  JoiLev1_00100,  /* A1200..A12FF */
  JoiLev1_00100,  /* A1300..A13FF */
  JoiLev1_00100,  /* A1400..A14FF */
  JoiLev1_00100,  /* A1500..A15FF */
  JoiLev1_00100,  /* A1600..A16FF */
  JoiLev1_00100,  /* A1700..A17FF */
  JoiLev1_00100,  /* A1800..A18FF */
  JoiLev1_00100,  /* A1900..A19FF */
  JoiLev1_00100,  /* A1A00..A1AFF */
  JoiLev1_00100,  /* A1B00..A1BFF */
  JoiLev1_00100,  /* A1C00..A1CFF */
  JoiLev1_00100,  /* A1D00..A1DFF */
  JoiLev1_00100,  /* A1E00..A1EFF */
  JoiLev1_00100,  /* A1F00..A1FFF */
  JoiLev1_00100,  /* A2000..A20FF */
  JoiLev1_00100,  /* A2100..A21FF */
  JoiLev1_00100,  /* A2200..A22FF */
  JoiLev1_00100,  /* A2300..A23FF */
  JoiLev1_00100,  /* A2400..A24FF */
  JoiLev1_00100,  /* A2500..A25FF */
  JoiLev1_00100,  /* A2600..A26FF */
  JoiLev1_00100,  /* A2700..A27FF */
  JoiLev1_00100,  /* A2800..A28FF */
  JoiLev1_00100,  /* A2900..A29FF */
  JoiLev1_00100,  /* A2A00..A2AFF */
  JoiLev1_00100,  /* A2B00..A2BFF */
  JoiLev1_00100,  /* A2C00..A2CFF */
  JoiLev1_00100,  /* A2D00..A2DFF */
  JoiLev1_00100,  /* A2E00..A2EFF */
  JoiLev1_00100,  /* A2F00..A2FFF */
  JoiLev1_00100,  /* A3000..A30FF */
  JoiLev1_00100,  /* A3100..A31FF */
  JoiLev1_00100,  /* A3200..A32FF */
  JoiLev1_00100,  /* A3300..A33FF */
  JoiLev1_00100,  /* A3400..A34FF */
  JoiLev1_00100,  /* A3500..A35FF */
  JoiLev1_00100,  /* A3600..A36FF */
  JoiLev1_00100,  /* A3700..A37FF */
  JoiLev1_00100,  /* A3800..A38FF */
  JoiLev1_00100,  /* A3900..A39FF */
  JoiLev1_00100,  /* A3A00..A3AFF */
  JoiLev1_00100,  /* A3B00..A3BFF */
  JoiLev1_00100,  /* A3C00..A3CFF */
  JoiLev1_00100,  /* A3D00..A3DFF */
  JoiLev1_00100,  /* A3E00..A3EFF */
  JoiLev1_00100,  /* A3F00..A3FFF */
  JoiLev1_00100,  /* A4000..A40FF */
  JoiLev1_00100,  /* A4100..A41FF */
  JoiLev1_00100,  /* A4200..A42FF */
  JoiLev1_00100,  /* A4300..A43FF */
  JoiLev1_00100,  /* A4400..A44FF */
  JoiLev1_00100,  /* A4500..A45FF */
  JoiLev1_00100,  /* A4600..A46FF */
  JoiLev1_00100,  /* A4700..A47FF */
  JoiLev1_00100,  /* A4800..A48FF */
  JoiLev1_00100,  /* A4900..A49FF */
  JoiLev1_00100,  /* A4A00..A4AFF */
  JoiLev1_00100,  /* A4B00..A4BFF */
  JoiLev1_00100,  /* A4C00..A4CFF */
  JoiLev1_00100,  /* A4D00..A4DFF */
  JoiLev1_00100,  /* A4E00..A4EFF */
  JoiLev1_00100,  /* A4F00..A4FFF */
  JoiLev1_00100,  /* A5000..A50FF */
  JoiLev1_00100,  /* A5100..A51FF */
  JoiLev1_00100,  /* A5200..A52FF */
  JoiLev1_00100,  /* A5300..A53FF */
  JoiLev1_00100,  /* A5400..A54FF */
  JoiLev1_00100,  /* A5500..A55FF */
  JoiLev1_00100,  /* A5600..A56FF */
  JoiLev1_00100,  /* A5700..A57FF */
  JoiLev1_00100,  /* A5800..A58FF */
  JoiLev1_00100,  /* A5900..A59FF */
  JoiLev1_00100,  /* A5A00..A5AFF */
  JoiLev1_00100,  /* A5B00..A5BFF */
  JoiLev1_00100,  /* A5C00..A5CFF */
  JoiLev1_00100,  /* A5D00..A5DFF */
  JoiLev1_00100,  /* A5E00..A5EFF */
  JoiLev1_00100,  /* A5F00..A5FFF */
  JoiLev1_00100,  /* A6000..A60FF */
  JoiLev1_00100,  /* A6100..A61FF */
  JoiLev1_00100,  /* A6200..A62FF */
  JoiLev1_00100,  /* A6300..A63FF */
  JoiLev1_00100,  /* A6400..A64FF */
  JoiLev1_00100,  /* A6500..A65FF */
  JoiLev1_00100,  /* A6600..A66FF */
  JoiLev1_00100,  /* A6700..A67FF */
  JoiLev1_00100,  /* A6800..A68FF */
  JoiLev1_00100,  /* A6900..A69FF */
  JoiLev1_00100,  /* A6A00..A6AFF */
  JoiLev1_00100,  /* A6B00..A6BFF */
  JoiLev1_00100,  /* A6C00..A6CFF */
  JoiLev1_00100,  /* A6D00..A6DFF */
  JoiLev1_00100,  /* A6E00..A6EFF */
  JoiLev1_00100,  /* A6F00..A6FFF */
  JoiLev1_00100,  /* A7000..A70FF */
  JoiLev1_00100,  /* A7100..A71FF */
  JoiLev1_00100,  /* A7200..A72FF */
  JoiLev1_00100,  /* A7300..A73FF */
  JoiLev1_00100,  /* A7400..A74FF */
  JoiLev1_00100,  /* A7500..A75FF */
  JoiLev1_00100,  /* A7600..A76FF */
  JoiLev1_00100,  /* A7700..A77FF */
  JoiLev1_00100,  /* A7800..A78FF */
  JoiLev1_00100,  /* A7900..A79FF */
  JoiLev1_00100,  /* A7A00..A7AFF */
  JoiLev1_00100,  /* A7B00..A7BFF */
  JoiLev1_00100,  /* A7C00..A7CFF */
  JoiLev1_00100,  /* A7D00..A7DFF */
  JoiLev1_00100,  /* A7E00..A7EFF */
  JoiLev1_00100,  /* A7F00..A7FFF */
  JoiLev1_00100,  /* A8000..A80FF */
  JoiLev1_00100,  /* A8100..A81FF */
  JoiLev1_00100,  /* A8200..A82FF */
  JoiLev1_00100,  /* A8300..A83FF */
  JoiLev1_00100,  /* A8400..A84FF */
  JoiLev1_00100,  /* A8500..A85FF */
  JoiLev1_00100,  /* A8600..A86FF */
  JoiLev1_00100,  /* A8700..A87FF */
  JoiLev1_00100,  /* A8800..A88FF */
  JoiLev1_00100,  /* A8900..A89FF */
  JoiLev1_00100,  /* A8A00..A8AFF */
  JoiLev1_00100,  /* A8B00..A8BFF */
  JoiLev1_00100,  /* A8C00..A8CFF */
  JoiLev1_00100,  /* A8D00..A8DFF */
  JoiLev1_00100,  /* A8E00..A8EFF */
  JoiLev1_00100,  /* A8F00..A8FFF */
  JoiLev1_00100,  /* A9000..A90FF */
  JoiLev1_00100,  /* A9100..A91FF */
  JoiLev1_00100,  /* A9200..A92FF */
  JoiLev1_00100,  /* A9300..A93FF */
  JoiLev1_00100,  /* A9400..A94FF */
  JoiLev1_00100,  /* A9500..A95FF */
  JoiLev1_00100,  /* A9600..A96FF */
  JoiLev1_00100,  /* A9700..A97FF */
  JoiLev1_00100,  /* A9800..A98FF */
  JoiLev1_00100,  /* A9900..A99FF */
  JoiLev1_00100,  /* A9A00..A9AFF */
  JoiLev1_00100,  /* A9B00..A9BFF */
  JoiLev1_00100,  /* A9C00..A9CFF */
  JoiLev1_00100,  /* A9D00..A9DFF */
  JoiLev1_00100,  /* A9E00..A9EFF */
  JoiLev1_00100,  /* A9F00..A9FFF */
  JoiLev1_00100,  /* AA000..AA0FF */
  JoiLev1_00100,  /* AA100..AA1FF */
  JoiLev1_00100,  /* AA200..AA2FF */
  JoiLev1_00100,  /* AA300..AA3FF */
  JoiLev1_00100,  /* AA400..AA4FF */
  JoiLev1_00100,  /* AA500..AA5FF */
  JoiLev1_00100,  /* AA600..AA6FF */
  JoiLev1_00100,  /* AA700..AA7FF */
  JoiLev1_00100,  /* AA800..AA8FF */
  JoiLev1_00100,  /* AA900..AA9FF */
  JoiLev1_00100,  /* AAA00..AAAFF */
  JoiLev1_00100,  /* AAB00..AABFF */
  JoiLev1_00100,  /* AAC00..AACFF */
  JoiLev1_00100,  /* AAD00..AADFF */
  JoiLev1_00100,  /* AAE00..AAEFF */
  JoiLev1_00100,  /* AAF00..AAFFF */
  JoiLev1_00100,  /* AB000..AB0FF */
  JoiLev1_00100,  /* AB100..AB1FF */
  JoiLev1_00100,  /* AB200..AB2FF */
  JoiLev1_00100,  /* AB300..AB3FF */
  JoiLev1_00100,  /* AB400..AB4FF */
  JoiLev1_00100,  /* AB500..AB5FF */
  JoiLev1_00100,  /* AB600..AB6FF */
  JoiLev1_00100,  /* AB700..AB7FF */
  JoiLev1_00100,  /* AB800..AB8FF */
  JoiLev1_00100,  /* AB900..AB9FF */
  JoiLev1_00100,  /* ABA00..ABAFF */
  JoiLev1_00100,  /* ABB00..ABBFF */
  JoiLev1_00100,  /* ABC00..ABCFF */
  JoiLev1_00100,  /* ABD00..ABDFF */
  JoiLev1_00100,  /* ABE00..ABEFF */
  JoiLev1_00100,  /* ABF00..ABFFF */
  JoiLev1_00100,  /* AC000..AC0FF */
  JoiLev1_00100,  /* AC100..AC1FF */
  JoiLev1_00100,  /* AC200..AC2FF */
  JoiLev1_00100,  /* AC300..AC3FF */
  JoiLev1_00100,  /* AC400..AC4FF */
  JoiLev1_00100,  /* AC500..AC5FF */
  JoiLev1_00100,  /* AC600..AC6FF */
  JoiLev1_00100,  /* AC700..AC7FF */
  JoiLev1_00100,  /* AC800..AC8FF */
  JoiLev1_00100,  /* AC900..AC9FF */
  JoiLev1_00100,  /* ACA00..ACAFF */
  JoiLev1_00100,  /* ACB00..ACBFF */
  JoiLev1_00100,  /* ACC00..ACCFF */
  JoiLev1_00100,  /* ACD00..ACDFF */
  JoiLev1_00100,  /* ACE00..ACEFF */
  JoiLev1_00100,  /* ACF00..ACFFF */
  JoiLev1_00100,  /* AD000..AD0FF */
  JoiLev1_00100,  /* AD100..AD1FF */
  JoiLev1_00100,  /* AD200..AD2FF */
  JoiLev1_00100,  /* AD300..AD3FF */
  JoiLev1_00100,  /* AD400..AD4FF */
  JoiLev1_00100,  /* AD500..AD5FF */
  JoiLev1_00100,  /* AD600..AD6FF */
  JoiLev1_00100,  /* AD700..AD7FF */
  JoiLev1_00100,  /* AD800..AD8FF */
  JoiLev1_00100,  /* AD900..AD9FF */
  JoiLev1_00100,  /* ADA00..ADAFF */
  JoiLev1_00100,  /* ADB00..ADBFF */
  JoiLev1_00100,  /* ADC00..ADCFF */
  JoiLev1_00100,  /* ADD00..ADDFF */
  JoiLev1_00100,  /* ADE00..ADEFF */
  JoiLev1_00100,  /* ADF00..ADFFF */
  JoiLev1_00100,  /* AE000..AE0FF */
  JoiLev1_00100,  /* AE100..AE1FF */
  JoiLev1_00100,  /* AE200..AE2FF */
  JoiLev1_00100,  /* AE300..AE3FF */
  JoiLev1_00100,  /* AE400..AE4FF */
  JoiLev1_00100,  /* AE500..AE5FF */
  JoiLev1_00100,  /* AE600..AE6FF */
  JoiLev1_00100,  /* AE700..AE7FF */
  JoiLev1_00100,  /* AE800..AE8FF */
  JoiLev1_00100,  /* AE900..AE9FF */
  JoiLev1_00100,  /* AEA00..AEAFF */
  JoiLev1_00100,  /* AEB00..AEBFF */
  JoiLev1_00100,  /* AEC00..AECFF */
  JoiLev1_00100,  /* AED00..AEDFF */
  JoiLev1_00100,  /* AEE00..AEEFF */
  JoiLev1_00100,  /* AEF00..AEFFF */
  JoiLev1_00100,  /* AF000..AF0FF */
  JoiLev1_00100,  /* AF100..AF1FF */
  JoiLev1_00100,  /* AF200..AF2FF */
  JoiLev1_00100,  /* AF300..AF3FF */
  JoiLev1_00100,  /* AF400..AF4FF */
  JoiLev1_00100,  /* AF500..AF5FF */
  JoiLev1_00100,  /* AF600..AF6FF */
  JoiLev1_00100,  /* AF700..AF7FF */
  JoiLev1_00100,  /* AF800..AF8FF */
  JoiLev1_00100,  /* AF900..AF9FF */
  JoiLev1_00100,  /* AFA00..AFAFF */
  JoiLev1_00100,  /* AFB00..AFBFF */
  JoiLev1_00100,  /* AFC00..AFCFF */
  JoiLev1_00100,  /* AFD00..AFDFF */
  JoiLev1_00100,  /* AFE00..AFEFF */
  JoiLev1_00100,  /* AFF00..AFFFF */
  JoiLev1_00100,  /* B0000..B00FF */
  JoiLev1_00100,  /* B0100..B01FF */
  JoiLev1_00100,  /* B0200..B02FF */
  JoiLev1_00100,  /* B0300..B03FF */
  JoiLev1_00100,  /* B0400..B04FF */
  JoiLev1_00100,  /* B0500..B05FF */
  JoiLev1_00100,  /* B0600..B06FF */
  JoiLev1_00100,  /* B0700..B07FF */
  JoiLev1_00100,  /* B0800..B08FF */
  JoiLev1_00100,  /* B0900..B09FF */
  JoiLev1_00100,  /* B0A00..B0AFF */
  JoiLev1_00100,  /* B0B00..B0BFF */
  JoiLev1_00100,  /* B0C00..B0CFF */
  JoiLev1_00100,  /* B0D00..B0DFF */
  JoiLev1_00100,  /* B0E00..B0EFF */
  JoiLev1_00100,  /* B0F00..B0FFF */
  JoiLev1_00100,  /* B1000..B10FF */
  JoiLev1_00100,  /* B1100..B11FF */
  JoiLev1_00100,  /* B1200..B12FF */
  JoiLev1_00100,  /* B1300..B13FF */
  JoiLev1_00100,  /* B1400..B14FF */
  JoiLev1_00100,  /* B1500..B15FF */
  JoiLev1_00100,  /* B1600..B16FF */
  JoiLev1_00100,  /* B1700..B17FF */
  JoiLev1_00100,  /* B1800..B18FF */
  JoiLev1_00100,  /* B1900..B19FF */
  JoiLev1_00100,  /* B1A00..B1AFF */
  JoiLev1_00100,  /* B1B00..B1BFF */
  JoiLev1_00100,  /* B1C00..B1CFF */
  JoiLev1_00100,  /* B1D00..B1DFF */
  JoiLev1_00100,  /* B1E00..B1EFF */
  JoiLev1_00100,  /* B1F00..B1FFF */
  JoiLev1_00100,  /* B2000..B20FF */
  JoiLev1_00100,  /* B2100..B21FF */
  JoiLev1_00100,  /* B2200..B22FF */
  JoiLev1_00100,  /* B2300..B23FF */
  JoiLev1_00100,  /* B2400..B24FF */
  JoiLev1_00100,  /* B2500..B25FF */
  JoiLev1_00100,  /* B2600..B26FF */
  JoiLev1_00100,  /* B2700..B27FF */
  JoiLev1_00100,  /* B2800..B28FF */
  JoiLev1_00100,  /* B2900..B29FF */
  JoiLev1_00100,  /* B2A00..B2AFF */
  JoiLev1_00100,  /* B2B00..B2BFF */
  JoiLev1_00100,  /* B2C00..B2CFF */
  JoiLev1_00100,  /* B2D00..B2DFF */
  JoiLev1_00100,  /* B2E00..B2EFF */
  JoiLev1_00100,  /* B2F00..B2FFF */
  JoiLev1_00100,  /* B3000..B30FF */
  JoiLev1_00100,  /* B3100..B31FF */
  JoiLev1_00100,  /* B3200..B32FF */
  JoiLev1_00100,  /* B3300..B33FF */
  JoiLev1_00100,  /* B3400..B34FF */
  JoiLev1_00100,  /* B3500..B35FF */
  JoiLev1_00100,  /* B3600..B36FF */
  JoiLev1_00100,  /* B3700..B37FF */
  JoiLev1_00100,  /* B3800..B38FF */
  JoiLev1_00100,  /* B3900..B39FF */
  JoiLev1_00100,  /* B3A00..B3AFF */
  JoiLev1_00100,  /* B3B00..B3BFF */
  JoiLev1_00100,  /* B3C00..B3CFF */
  JoiLev1_00100,  /* B3D00..B3DFF */
  JoiLev1_00100,  /* B3E00..B3EFF */
  JoiLev1_00100,  /* B3F00..B3FFF */
  JoiLev1_00100,  /* B4000..B40FF */
  JoiLev1_00100,  /* B4100..B41FF */
  JoiLev1_00100,  /* B4200..B42FF */
  JoiLev1_00100,  /* B4300..B43FF */
  JoiLev1_00100,  /* B4400..B44FF */
  JoiLev1_00100,  /* B4500..B45FF */
  JoiLev1_00100,  /* B4600..B46FF */
  JoiLev1_00100,  /* B4700..B47FF */
  JoiLev1_00100,  /* B4800..B48FF */
  JoiLev1_00100,  /* B4900..B49FF */
  JoiLev1_00100,  /* B4A00..B4AFF */
  JoiLev1_00100,  /* B4B00..B4BFF */
  JoiLev1_00100,  /* B4C00..B4CFF */
  JoiLev1_00100,  /* B4D00..B4DFF */
  JoiLev1_00100,  /* B4E00..B4EFF */
  JoiLev1_00100,  /* B4F00..B4FFF */
  JoiLev1_00100,  /* B5000..B50FF */
  JoiLev1_00100,  /* B5100..B51FF */
  JoiLev1_00100,  /* B5200..B52FF */
  JoiLev1_00100,  /* B5300..B53FF */
  JoiLev1_00100,  /* B5400..B54FF */
  JoiLev1_00100,  /* B5500..B55FF */
  JoiLev1_00100,  /* B5600..B56FF */
  JoiLev1_00100,  /* B5700..B57FF */
  JoiLev1_00100,  /* B5800..B58FF */
  JoiLev1_00100,  /* B5900..B59FF */
  JoiLev1_00100,  /* B5A00..B5AFF */
  JoiLev1_00100,  /* B5B00..B5BFF */
  JoiLev1_00100,  /* B5C00..B5CFF */
  JoiLev1_00100,  /* B5D00..B5DFF */
  JoiLev1_00100,  /* B5E00..B5EFF */
  JoiLev1_00100,  /* B5F00..B5FFF */
  JoiLev1_00100,  /* B6000..B60FF */
  JoiLev1_00100,  /* B6100..B61FF */
  JoiLev1_00100,  /* B6200..B62FF */
  JoiLev1_00100,  /* B6300..B63FF */
  JoiLev1_00100,  /* B6400..B64FF */
  JoiLev1_00100,  /* B6500..B65FF */
  JoiLev1_00100,  /* B6600..B66FF */
  JoiLev1_00100,  /* B6700..B67FF */
  JoiLev1_00100,  /* B6800..B68FF */
  JoiLev1_00100,  /* B6900..B69FF */
  JoiLev1_00100,  /* B6A00..B6AFF */
  JoiLev1_00100,  /* B6B00..B6BFF */
  JoiLev1_00100,  /* B6C00..B6CFF */
  JoiLev1_00100,  /* B6D00..B6DFF */
  JoiLev1_00100,  /* B6E00..B6EFF */
  JoiLev1_00100,  /* B6F00..B6FFF */
  JoiLev1_00100,  /* B7000..B70FF */
  JoiLev1_00100,  /* B7100..B71FF */
  JoiLev1_00100,  /* B7200..B72FF */
  JoiLev1_00100,  /* B7300..B73FF */
  JoiLev1_00100,  /* B7400..B74FF */
  JoiLev1_00100,  /* B7500..B75FF */
  JoiLev1_00100,  /* B7600..B76FF */
  JoiLev1_00100,  /* B7700..B77FF */
  JoiLev1_00100,  /* B7800..B78FF */
  JoiLev1_00100,  /* B7900..B79FF */
  JoiLev1_00100,  /* B7A00..B7AFF */
  JoiLev1_00100,  /* B7B00..B7BFF */
  JoiLev1_00100,  /* B7C00..B7CFF */
  JoiLev1_00100,  /* B7D00..B7DFF */
  JoiLev1_00100,  /* B7E00..B7EFF */
  JoiLev1_00100,  /* B7F00..B7FFF */
  JoiLev1_00100,  /* B8000..B80FF */
  JoiLev1_00100,  /* B8100..B81FF */
  JoiLev1_00100,  /* B8200..B82FF */
  JoiLev1_00100,  /* B8300..B83FF */
  JoiLev1_00100,  /* B8400..B84FF */
  JoiLev1_00100,  /* B8500..B85FF */
  JoiLev1_00100,  /* B8600..B86FF */
  JoiLev1_00100,  /* B8700..B87FF */
  JoiLev1_00100,  /* B8800..B88FF */
  JoiLev1_00100,  /* B8900..B89FF */
  JoiLev1_00100,  /* B8A00..B8AFF */
  JoiLev1_00100,  /* B8B00..B8BFF */
  JoiLev1_00100,  /* B8C00..B8CFF */
  JoiLev1_00100,  /* B8D00..B8DFF */
  JoiLev1_00100,  /* B8E00..B8EFF */
  JoiLev1_00100,  /* B8F00..B8FFF */
  JoiLev1_00100,  /* B9000..B90FF */
  JoiLev1_00100,  /* B9100..B91FF */
  JoiLev1_00100,  /* B9200..B92FF */
  JoiLev1_00100,  /* B9300..B93FF */
  JoiLev1_00100,  /* B9400..B94FF */
  JoiLev1_00100,  /* B9500..B95FF */
  JoiLev1_00100,  /* B9600..B96FF */
  JoiLev1_00100,  /* B9700..B97FF */
  JoiLev1_00100,  /* B9800..B98FF */
  JoiLev1_00100,  /* B9900..B99FF */
  JoiLev1_00100,  /* B9A00..B9AFF */
  JoiLev1_00100,  /* B9B00..B9BFF */
  JoiLev1_00100,  /* B9C00..B9CFF */
  JoiLev1_00100,  /* B9D00..B9DFF */
  JoiLev1_00100,  /* B9E00..B9EFF */
  JoiLev1_00100,  /* B9F00..B9FFF */
  JoiLev1_00100,  /* BA000..BA0FF */
  JoiLev1_00100,  /* BA100..BA1FF */
  JoiLev1_00100,  /* BA200..BA2FF */
  JoiLev1_00100,  /* BA300..BA3FF */
  JoiLev1_00100,  /* BA400..BA4FF */
  JoiLev1_00100,  /* BA500..BA5FF */
  JoiLev1_00100,  /* BA600..BA6FF */
  JoiLev1_00100,  /* BA700..BA7FF */
  JoiLev1_00100,  /* BA800..BA8FF */
  JoiLev1_00100,  /* BA900..BA9FF */
  JoiLev1_00100,  /* BAA00..BAAFF */
  JoiLev1_00100,  /* BAB00..BABFF */
  JoiLev1_00100,  /* BAC00..BACFF */
  JoiLev1_00100,  /* BAD00..BADFF */
  JoiLev1_00100,  /* BAE00..BAEFF */
  JoiLev1_00100,  /* BAF00..BAFFF */
  JoiLev1_00100,  /* BB000..BB0FF */
  JoiLev1_00100,  /* BB100..BB1FF */
  JoiLev1_00100,  /* BB200..BB2FF */
  JoiLev1_00100,  /* BB300..BB3FF */
  JoiLev1_00100,  /* BB400..BB4FF */
  JoiLev1_00100,  /* BB500..BB5FF */
  JoiLev1_00100,  /* BB600..BB6FF */
  JoiLev1_00100,  /* BB700..BB7FF */
  JoiLev1_00100,  /* BB800..BB8FF */
  JoiLev1_00100,  /* BB900..BB9FF */
  JoiLev1_00100,  /* BBA00..BBAFF */
  JoiLev1_00100,  /* BBB00..BBBFF */
  JoiLev1_00100,  /* BBC00..BBCFF */
  JoiLev1_00100,  /* BBD00..BBDFF */
  JoiLev1_00100,  /* BBE00..BBEFF */
  JoiLev1_00100,  /* BBF00..BBFFF */
  JoiLev1_00100,  /* BC000..BC0FF */
  JoiLev1_00100,  /* BC100..BC1FF */
  JoiLev1_00100,  /* BC200..BC2FF */
  JoiLev1_00100,  /* BC300..BC3FF */
  JoiLev1_00100,  /* BC400..BC4FF */
  JoiLev1_00100,  /* BC500..BC5FF */
  JoiLev1_00100,  /* BC600..BC6FF */
  JoiLev1_00100,  /* BC700..BC7FF */
  JoiLev1_00100,  /* BC800..BC8FF */
  JoiLev1_00100,  /* BC900..BC9FF */
  JoiLev1_00100,  /* BCA00..BCAFF */
  JoiLev1_00100,  /* BCB00..BCBFF */
  JoiLev1_00100,  /* BCC00..BCCFF */
  JoiLev1_00100,  /* BCD00..BCDFF */
  JoiLev1_00100,  /* BCE00..BCEFF */
  JoiLev1_00100,  /* BCF00..BCFFF */
  JoiLev1_00100,  /* BD000..BD0FF */
  JoiLev1_00100,  /* BD100..BD1FF */
  JoiLev1_00100,  /* BD200..BD2FF */
  JoiLev1_00100,  /* BD300..BD3FF */
  JoiLev1_00100,  /* BD400..BD4FF */
  JoiLev1_00100,  /* BD500..BD5FF */
  JoiLev1_00100,  /* BD600..BD6FF */
  JoiLev1_00100,  /* BD700..BD7FF */
  JoiLev1_00100,  /* BD800..BD8FF */
  JoiLev1_00100,  /* BD900..BD9FF */
  JoiLev1_00100,  /* BDA00..BDAFF */
  JoiLev1_00100,  /* BDB00..BDBFF */
  JoiLev1_00100,  /* BDC00..BDCFF */
  JoiLev1_00100,  /* BDD00..BDDFF */
  JoiLev1_00100,  /* BDE00..BDEFF */
  JoiLev1_00100,  /* BDF00..BDFFF */
  JoiLev1_00100,  /* BE000..BE0FF */
  JoiLev1_00100,  /* BE100..BE1FF */
  JoiLev1_00100,  /* BE200..BE2FF */
  JoiLev1_00100,  /* BE300..BE3FF */
  JoiLev1_00100,  /* BE400..BE4FF */
  JoiLev1_00100,  /* BE500..BE5FF */
  JoiLev1_00100,  /* BE600..BE6FF */
  JoiLev1_00100,  /* BE700..BE7FF */
  JoiLev1_00100,  /* BE800..BE8FF */
  JoiLev1_00100,  /* BE900..BE9FF */
  JoiLev1_00100,  /* BEA00..BEAFF */
  JoiLev1_00100,  /* BEB00..BEBFF */
  JoiLev1_00100,  /* BEC00..BECFF */
  JoiLev1_00100,  /* BED00..BEDFF */
  JoiLev1_00100,  /* BEE00..BEEFF */
  JoiLev1_00100,  /* BEF00..BEFFF */
  JoiLev1_00100,  /* BF000..BF0FF */
  JoiLev1_00100,  /* BF100..BF1FF */
  JoiLev1_00100,  /* BF200..BF2FF */
  JoiLev1_00100,  /* BF300..BF3FF */
  JoiLev1_00100,  /* BF400..BF4FF */
  JoiLev1_00100,  /* BF500..BF5FF */
  JoiLev1_00100,  /* BF600..BF6FF */
  JoiLev1_00100,  /* BF700..BF7FF */
  JoiLev1_00100,  /* BF800..BF8FF */
  JoiLev1_00100,  /* BF900..BF9FF */
  JoiLev1_00100,  /* BFA00..BFAFF */
  JoiLev1_00100,  /* BFB00..BFBFF */
  JoiLev1_00100,  /* BFC00..BFCFF */
  JoiLev1_00100,  /* BFD00..BFDFF */
  JoiLev1_00100,  /* BFE00..BFEFF */
  JoiLev1_00100,  /* BFF00..BFFFF */
  JoiLev1_00100,  /* C0000..C00FF */
  JoiLev1_00100,  /* C0100..C01FF */
  JoiLev1_00100,  /* C0200..C02FF */
  JoiLev1_00100,  /* C0300..C03FF */
  JoiLev1_00100,  /* C0400..C04FF */
  JoiLev1_00100,  /* C0500..C05FF */
  JoiLev1_00100,  /* C0600..C06FF */
  JoiLev1_00100,  /* C0700..C07FF */
  JoiLev1_00100,  /* C0800..C08FF */
  JoiLev1_00100,  /* C0900..C09FF */
  JoiLev1_00100,  /* C0A00..C0AFF */
  JoiLev1_00100,  /* C0B00..C0BFF */
  JoiLev1_00100,  /* C0C00..C0CFF */
  JoiLev1_00100,  /* C0D00..C0DFF */
  JoiLev1_00100,  /* C0E00..C0EFF */
  JoiLev1_00100,  /* C0F00..C0FFF */
  JoiLev1_00100,  /* C1000..C10FF */
  JoiLev1_00100,  /* C1100..C11FF */
  JoiLev1_00100,  /* C1200..C12FF */
  JoiLev1_00100,  /* C1300..C13FF */
  JoiLev1_00100,  /* C1400..C14FF */
  JoiLev1_00100,  /* C1500..C15FF */
  JoiLev1_00100,  /* C1600..C16FF */
  JoiLev1_00100,  /* C1700..C17FF */
  JoiLev1_00100,  /* C1800..C18FF */
  JoiLev1_00100,  /* C1900..C19FF */
  JoiLev1_00100,  /* C1A00..C1AFF */
  JoiLev1_00100,  /* C1B00..C1BFF */
  JoiLev1_00100,  /* C1C00..C1CFF */
  JoiLev1_00100,  /* C1D00..C1DFF */
  JoiLev1_00100,  /* C1E00..C1EFF */
  JoiLev1_00100,  /* C1F00..C1FFF */
  JoiLev1_00100,  /* C2000..C20FF */
  JoiLev1_00100,  /* C2100..C21FF */
  JoiLev1_00100,  /* C2200..C22FF */
  JoiLev1_00100,  /* C2300..C23FF */
  JoiLev1_00100,  /* C2400..C24FF */
  JoiLev1_00100,  /* C2500..C25FF */
  JoiLev1_00100,  /* C2600..C26FF */
  JoiLev1_00100,  /* C2700..C27FF */
  JoiLev1_00100,  /* C2800..C28FF */
  JoiLev1_00100,  /* C2900..C29FF */
  JoiLev1_00100,  /* C2A00..C2AFF */
  JoiLev1_00100,  /* C2B00..C2BFF */
  JoiLev1_00100,  /* C2C00..C2CFF */
  JoiLev1_00100,  /* C2D00..C2DFF */
  JoiLev1_00100,  /* C2E00..C2EFF */
  JoiLev1_00100,  /* C2F00..C2FFF */
  JoiLev1_00100,  /* C3000..C30FF */
  JoiLev1_00100,  /* C3100..C31FF */
  JoiLev1_00100,  /* C3200..C32FF */
  JoiLev1_00100,  /* C3300..C33FF */
  JoiLev1_00100,  /* C3400..C34FF */
  JoiLev1_00100,  /* C3500..C35FF */
  JoiLev1_00100,  /* C3600..C36FF */
  JoiLev1_00100,  /* C3700..C37FF */
  JoiLev1_00100,  /* C3800..C38FF */
  JoiLev1_00100,  /* C3900..C39FF */
  JoiLev1_00100,  /* C3A00..C3AFF */
  JoiLev1_00100,  /* C3B00..C3BFF */
  JoiLev1_00100,  /* C3C00..C3CFF */
  JoiLev1_00100,  /* C3D00..C3DFF */
  JoiLev1_00100,  /* C3E00..C3EFF */
  JoiLev1_00100,  /* C3F00..C3FFF */
  JoiLev1_00100,  /* C4000..C40FF */
  JoiLev1_00100,  /* C4100..C41FF */
  JoiLev1_00100,  /* C4200..C42FF */
  JoiLev1_00100,  /* C4300..C43FF */
  JoiLev1_00100,  /* C4400..C44FF */
  JoiLev1_00100,  /* C4500..C45FF */
  JoiLev1_00100,  /* C4600..C46FF */
  JoiLev1_00100,  /* C4700..C47FF */
  JoiLev1_00100,  /* C4800..C48FF */
  JoiLev1_00100,  /* C4900..C49FF */
  JoiLev1_00100,  /* C4A00..C4AFF */
  JoiLev1_00100,  /* C4B00..C4BFF */
  JoiLev1_00100,  /* C4C00..C4CFF */
  JoiLev1_00100,  /* C4D00..C4DFF */
  JoiLev1_00100,  /* C4E00..C4EFF */
  JoiLev1_00100,  /* C4F00..C4FFF */
  JoiLev1_00100,  /* C5000..C50FF */
  JoiLev1_00100,  /* C5100..C51FF */
  JoiLev1_00100,  /* C5200..C52FF */
  JoiLev1_00100,  /* C5300..C53FF */
  JoiLev1_00100,  /* C5400..C54FF */
  JoiLev1_00100,  /* C5500..C55FF */
  JoiLev1_00100,  /* C5600..C56FF */
  JoiLev1_00100,  /* C5700..C57FF */
  JoiLev1_00100,  /* C5800..C58FF */
  JoiLev1_00100,  /* C5900..C59FF */
  JoiLev1_00100,  /* C5A00..C5AFF */
  JoiLev1_00100,  /* C5B00..C5BFF */
  JoiLev1_00100,  /* C5C00..C5CFF */
  JoiLev1_00100,  /* C5D00..C5DFF */
  JoiLev1_00100,  /* C5E00..C5EFF */
  JoiLev1_00100,  /* C5F00..C5FFF */
  JoiLev1_00100,  /* C6000..C60FF */
  JoiLev1_00100,  /* C6100..C61FF */
  JoiLev1_00100,  /* C6200..C62FF */
  JoiLev1_00100,  /* C6300..C63FF */
  JoiLev1_00100,  /* C6400..C64FF */
  JoiLev1_00100,  /* C6500..C65FF */
  JoiLev1_00100,  /* C6600..C66FF */
  JoiLev1_00100,  /* C6700..C67FF */
  JoiLev1_00100,  /* C6800..C68FF */
  JoiLev1_00100,  /* C6900..C69FF */
  JoiLev1_00100,  /* C6A00..C6AFF */
  JoiLev1_00100,  /* C6B00..C6BFF */
  JoiLev1_00100,  /* C6C00..C6CFF */
  JoiLev1_00100,  /* C6D00..C6DFF */
  JoiLev1_00100,  /* C6E00..C6EFF */
  JoiLev1_00100,  /* C6F00..C6FFF */
  JoiLev1_00100,  /* C7000..C70FF */
  JoiLev1_00100,  /* C7100..C71FF */
  JoiLev1_00100,  /* C7200..C72FF */
  JoiLev1_00100,  /* C7300..C73FF */
  JoiLev1_00100,  /* C7400..C74FF */
  JoiLev1_00100,  /* C7500..C75FF */
  JoiLev1_00100,  /* C7600..C76FF */
  JoiLev1_00100,  /* C7700..C77FF */
  JoiLev1_00100,  /* C7800..C78FF */
  JoiLev1_00100,  /* C7900..C79FF */
  JoiLev1_00100,  /* C7A00..C7AFF */
  JoiLev1_00100,  /* C7B00..C7BFF */
  JoiLev1_00100,  /* C7C00..C7CFF */
  JoiLev1_00100,  /* C7D00..C7DFF */
  JoiLev1_00100,  /* C7E00..C7EFF */
  JoiLev1_00100,  /* C7F00..C7FFF */
  JoiLev1_00100,  /* C8000..C80FF */
  JoiLev1_00100,  /* C8100..C81FF */
  JoiLev1_00100,  /* C8200..C82FF */
  JoiLev1_00100,  /* C8300..C83FF */
  JoiLev1_00100,  /* C8400..C84FF */
  JoiLev1_00100,  /* C8500..C85FF */
  JoiLev1_00100,  /* C8600..C86FF */
  JoiLev1_00100,  /* C8700..C87FF */
  JoiLev1_00100,  /* C8800..C88FF */
  JoiLev1_00100,  /* C8900..C89FF */
  JoiLev1_00100,  /* C8A00..C8AFF */
  JoiLev1_00100,  /* C8B00..C8BFF */
  JoiLev1_00100,  /* C8C00..C8CFF */
  JoiLev1_00100,  /* C8D00..C8DFF */
  JoiLev1_00100,  /* C8E00..C8EFF */
  JoiLev1_00100,  /* C8F00..C8FFF */
  JoiLev1_00100,  /* C9000..C90FF */
  JoiLev1_00100,  /* C9100..C91FF */
  JoiLev1_00100,  /* C9200..C92FF */
  JoiLev1_00100,  /* C9300..C93FF */
  JoiLev1_00100,  /* C9400..C94FF */
  JoiLev1_00100,  /* C9500..C95FF */
  JoiLev1_00100,  /* C9600..C96FF */
  JoiLev1_00100,  /* C9700..C97FF */
  JoiLev1_00100,  /* C9800..C98FF */
  JoiLev1_00100,  /* C9900..C99FF */
  JoiLev1_00100,  /* C9A00..C9AFF */
  JoiLev1_00100,  /* C9B00..C9BFF */
  JoiLev1_00100,  /* C9C00..C9CFF */
  JoiLev1_00100,  /* C9D00..C9DFF */
  JoiLev1_00100,  /* C9E00..C9EFF */
  JoiLev1_00100,  /* C9F00..C9FFF */
  JoiLev1_00100,  /* CA000..CA0FF */
  JoiLev1_00100,  /* CA100..CA1FF */
  JoiLev1_00100,  /* CA200..CA2FF */
  JoiLev1_00100,  /* CA300..CA3FF */
  JoiLev1_00100,  /* CA400..CA4FF */
  JoiLev1_00100,  /* CA500..CA5FF */
  JoiLev1_00100,  /* CA600..CA6FF */
  JoiLev1_00100,  /* CA700..CA7FF */
  JoiLev1_00100,  /* CA800..CA8FF */
  JoiLev1_00100,  /* CA900..CA9FF */
  JoiLev1_00100,  /* CAA00..CAAFF */
  JoiLev1_00100,  /* CAB00..CABFF */
  JoiLev1_00100,  /* CAC00..CACFF */
  JoiLev1_00100,  /* CAD00..CADFF */
  JoiLev1_00100,  /* CAE00..CAEFF */
  JoiLev1_00100,  /* CAF00..CAFFF */
  JoiLev1_00100,  /* CB000..CB0FF */
  JoiLev1_00100,  /* CB100..CB1FF */
  JoiLev1_00100,  /* CB200..CB2FF */
  JoiLev1_00100,  /* CB300..CB3FF */
  JoiLev1_00100,  /* CB400..CB4FF */
  JoiLev1_00100,  /* CB500..CB5FF */
  JoiLev1_00100,  /* CB600..CB6FF */
  JoiLev1_00100,  /* CB700..CB7FF */
  JoiLev1_00100,  /* CB800..CB8FF */
  JoiLev1_00100,  /* CB900..CB9FF */
  JoiLev1_00100,  /* CBA00..CBAFF */
  JoiLev1_00100,  /* CBB00..CBBFF */
  JoiLev1_00100,  /* CBC00..CBCFF */
  JoiLev1_00100,  /* CBD00..CBDFF */
  JoiLev1_00100,  /* CBE00..CBEFF */
  JoiLev1_00100,  /* CBF00..CBFFF */
  JoiLev1_00100,  /* CC000..CC0FF */
  JoiLev1_00100,  /* CC100..CC1FF */
  JoiLev1_00100,  /* CC200..CC2FF */
  JoiLev1_00100,  /* CC300..CC3FF */
  JoiLev1_00100,  /* CC400..CC4FF */
  JoiLev1_00100,  /* CC500..CC5FF */
  JoiLev1_00100,  /* CC600..CC6FF */
  JoiLev1_00100,  /* CC700..CC7FF */
  JoiLev1_00100,  /* CC800..CC8FF */
  JoiLev1_00100,  /* CC900..CC9FF */
  JoiLev1_00100,  /* CCA00..CCAFF */
  JoiLev1_00100,  /* CCB00..CCBFF */
  JoiLev1_00100,  /* CCC00..CCCFF */
  JoiLev1_00100,  /* CCD00..CCDFF */
  JoiLev1_00100,  /* CCE00..CCEFF */
  JoiLev1_00100,  /* CCF00..CCFFF */
  JoiLev1_00100,  /* CD000..CD0FF */
  JoiLev1_00100,  /* CD100..CD1FF */
  JoiLev1_00100,  /* CD200..CD2FF */
  JoiLev1_00100,  /* CD300..CD3FF */
  JoiLev1_00100,  /* CD400..CD4FF */
  JoiLev1_00100,  /* CD500..CD5FF */
  JoiLev1_00100,  /* CD600..CD6FF */
  JoiLev1_00100,  /* CD700..CD7FF */
  JoiLev1_00100,  /* CD800..CD8FF */
  JoiLev1_00100,  /* CD900..CD9FF */
  JoiLev1_00100,  /* CDA00..CDAFF */
  JoiLev1_00100,  /* CDB00..CDBFF */
  JoiLev1_00100,  /* CDC00..CDCFF */
  JoiLev1_00100,  /* CDD00..CDDFF */
  JoiLev1_00100,  /* CDE00..CDEFF */
  JoiLev1_00100,  /* CDF00..CDFFF */
  JoiLev1_00100,  /* CE000..CE0FF */
  JoiLev1_00100,  /* CE100..CE1FF */
  JoiLev1_00100,  /* CE200..CE2FF */
  JoiLev1_00100,  /* CE300..CE3FF */
  JoiLev1_00100,  /* CE400..CE4FF */
  JoiLev1_00100,  /* CE500..CE5FF */
  JoiLev1_00100,  /* CE600..CE6FF */
  JoiLev1_00100,  /* CE700..CE7FF */
  JoiLev1_00100,  /* CE800..CE8FF */
  JoiLev1_00100,  /* CE900..CE9FF */
  JoiLev1_00100,  /* CEA00..CEAFF */
  JoiLev1_00100,  /* CEB00..CEBFF */
  JoiLev1_00100,  /* CEC00..CECFF */
  JoiLev1_00100,  /* CED00..CEDFF */
  JoiLev1_00100,  /* CEE00..CEEFF */
  JoiLev1_00100,  /* CEF00..CEFFF */
  JoiLev1_00100,  /* CF000..CF0FF */
  JoiLev1_00100,  /* CF100..CF1FF */
  JoiLev1_00100,  /* CF200..CF2FF */
  JoiLev1_00100,  /* CF300..CF3FF */
  JoiLev1_00100,  /* CF400..CF4FF */
  JoiLev1_00100,  /* CF500..CF5FF */
  JoiLev1_00100,  /* CF600..CF6FF */
  JoiLev1_00100,  /* CF700..CF7FF */
  JoiLev1_00100,  /* CF800..CF8FF */
  JoiLev1_00100,  /* CF900..CF9FF */
  JoiLev1_00100,  /* CFA00..CFAFF */
  JoiLev1_00100,  /* CFB00..CFBFF */
  JoiLev1_00100,  /* CFC00..CFCFF */
  JoiLev1_00100,  /* CFD00..CFDFF */
  JoiLev1_00100,  /* CFE00..CFEFF */
  JoiLev1_00100,  /* CFF00..CFFFF */
  JoiLev1_00100,  /* D0000..D00FF */
  JoiLev1_00100,  /* D0100..D01FF */
  JoiLev1_00100,  /* D0200..D02FF */
  JoiLev1_00100,  /* D0300..D03FF */
  JoiLev1_00100,  /* D0400..D04FF */
  JoiLev1_00100,  /* D0500..D05FF */
  JoiLev1_00100,  /* D0600..D06FF */
  JoiLev1_00100,  /* D0700..D07FF */
  JoiLev1_00100,  /* D0800..D08FF */
  JoiLev1_00100,  /* D0900..D09FF */
  JoiLev1_00100,  /* D0A00..D0AFF */
  JoiLev1_00100,  /* D0B00..D0BFF */
  JoiLev1_00100,  /* D0C00..D0CFF */
  JoiLev1_00100,  /* D0D00..D0DFF */
  JoiLev1_00100,  /* D0E00..D0EFF */
  JoiLev1_00100,  /* D0F00..D0FFF */
  JoiLev1_00100,  /* D1000..D10FF */
  JoiLev1_00100,  /* D1100..D11FF */
  JoiLev1_00100,  /* D1200..D12FF */
  JoiLev1_00100,  /* D1300..D13FF */
  JoiLev1_00100,  /* D1400..D14FF */
  JoiLev1_00100,  /* D1500..D15FF */
  JoiLev1_00100,  /* D1600..D16FF */
  JoiLev1_00100,  /* D1700..D17FF */
  JoiLev1_00100,  /* D1800..D18FF */
  JoiLev1_00100,  /* D1900..D19FF */
  JoiLev1_00100,  /* D1A00..D1AFF */
  JoiLev1_00100,  /* D1B00..D1BFF */
  JoiLev1_00100,  /* D1C00..D1CFF */
  JoiLev1_00100,  /* D1D00..D1DFF */
  JoiLev1_00100,  /* D1E00..D1EFF */
  JoiLev1_00100,  /* D1F00..D1FFF */
  JoiLev1_00100,  /* D2000..D20FF */
  JoiLev1_00100,  /* D2100..D21FF */
  JoiLev1_00100,  /* D2200..D22FF */
  JoiLev1_00100,  /* D2300..D23FF */
  JoiLev1_00100,  /* D2400..D24FF */
  JoiLev1_00100,  /* D2500..D25FF */
  JoiLev1_00100,  /* D2600..D26FF */
  JoiLev1_00100,  /* D2700..D27FF */
  JoiLev1_00100,  /* D2800..D28FF */
  JoiLev1_00100,  /* D2900..D29FF */
  JoiLev1_00100,  /* D2A00..D2AFF */
  JoiLev1_00100,  /* D2B00..D2BFF */
  JoiLev1_00100,  /* D2C00..D2CFF */
  JoiLev1_00100,  /* D2D00..D2DFF */
  JoiLev1_00100,  /* D2E00..D2EFF */
  JoiLev1_00100,  /* D2F00..D2FFF */
  JoiLev1_00100,  /* D3000..D30FF */
  JoiLev1_00100,  /* D3100..D31FF */
  JoiLev1_00100,  /* D3200..D32FF */
  JoiLev1_00100,  /* D3300..D33FF */
  JoiLev1_00100,  /* D3400..D34FF */
  JoiLev1_00100,  /* D3500..D35FF */
  JoiLev1_00100,  /* D3600..D36FF */
  JoiLev1_00100,  /* D3700..D37FF */
  JoiLev1_00100,  /* D3800..D38FF */
  JoiLev1_00100,  /* D3900..D39FF */
  JoiLev1_00100,  /* D3A00..D3AFF */
  JoiLev1_00100,  /* D3B00..D3BFF */
  JoiLev1_00100,  /* D3C00..D3CFF */
  JoiLev1_00100,  /* D3D00..D3DFF */
  JoiLev1_00100,  /* D3E00..D3EFF */
  JoiLev1_00100,  /* D3F00..D3FFF */
  JoiLev1_00100,  /* D4000..D40FF */
  JoiLev1_00100,  /* D4100..D41FF */
  JoiLev1_00100,  /* D4200..D42FF */
  JoiLev1_00100,  /* D4300..D43FF */
  JoiLev1_00100,  /* D4400..D44FF */
  JoiLev1_00100,  /* D4500..D45FF */
  JoiLev1_00100,  /* D4600..D46FF */
  JoiLev1_00100,  /* D4700..D47FF */
  JoiLev1_00100,  /* D4800..D48FF */
  JoiLev1_00100,  /* D4900..D49FF */
  JoiLev1_00100,  /* D4A00..D4AFF */
  JoiLev1_00100,  /* D4B00..D4BFF */
  JoiLev1_00100,  /* D4C00..D4CFF */
  JoiLev1_00100,  /* D4D00..D4DFF */
  JoiLev1_00100,  /* D4E00..D4EFF */
  JoiLev1_00100,  /* D4F00..D4FFF */
  JoiLev1_00100,  /* D5000..D50FF */
  JoiLev1_00100,  /* D5100..D51FF */
  JoiLev1_00100,  /* D5200..D52FF */
  JoiLev1_00100,  /* D5300..D53FF */
  JoiLev1_00100,  /* D5400..D54FF */
  JoiLev1_00100,  /* D5500..D55FF */
  JoiLev1_00100,  /* D5600..D56FF */
  JoiLev1_00100,  /* D5700..D57FF */
  JoiLev1_00100,  /* D5800..D58FF */
  JoiLev1_00100,  /* D5900..D59FF */
  JoiLev1_00100,  /* D5A00..D5AFF */
  JoiLev1_00100,  /* D5B00..D5BFF */
  JoiLev1_00100,  /* D5C00..D5CFF */
  JoiLev1_00100,  /* D5D00..D5DFF */
  JoiLev1_00100,  /* D5E00..D5EFF */
  JoiLev1_00100,  /* D5F00..D5FFF */
  JoiLev1_00100,  /* D6000..D60FF */
  JoiLev1_00100,  /* D6100..D61FF */
  JoiLev1_00100,  /* D6200..D62FF */
  JoiLev1_00100,  /* D6300..D63FF */
  JoiLev1_00100,  /* D6400..D64FF */
  JoiLev1_00100,  /* D6500..D65FF */
  JoiLev1_00100,  /* D6600..D66FF */
  JoiLev1_00100,  /* D6700..D67FF */
  JoiLev1_00100,  /* D6800..D68FF */
  JoiLev1_00100,  /* D6900..D69FF */
  JoiLev1_00100,  /* D6A00..D6AFF */
  JoiLev1_00100,  /* D6B00..D6BFF */
  JoiLev1_00100,  /* D6C00..D6CFF */
  JoiLev1_00100,  /* D6D00..D6DFF */
  JoiLev1_00100,  /* D6E00..D6EFF */
  JoiLev1_00100,  /* D6F00..D6FFF */
  JoiLev1_00100,  /* D7000..D70FF */
  JoiLev1_00100,  /* D7100..D71FF */
  JoiLev1_00100,  /* D7200..D72FF */
  JoiLev1_00100,  /* D7300..D73FF */
  JoiLev1_00100,  /* D7400..D74FF */
  JoiLev1_00100,  /* D7500..D75FF */
  JoiLev1_00100,  /* D7600..D76FF */
  JoiLev1_00100,  /* D7700..D77FF */
  JoiLev1_00100,  /* D7800..D78FF */
  JoiLev1_00100,  /* D7900..D79FF */
  JoiLev1_00100,  /* D7A00..D7AFF */
  JoiLev1_00100,  /* D7B00..D7BFF */
  JoiLev1_00100,  /* D7C00..D7CFF */
  JoiLev1_00100,  /* D7D00..D7DFF */
  JoiLev1_00100,  /* D7E00..D7EFF */
  JoiLev1_00100,  /* D7F00..D7FFF */
  JoiLev1_00100,  /* D8000..D80FF */
  JoiLev1_00100,  /* D8100..D81FF */
  JoiLev1_00100,  /* D8200..D82FF */
  JoiLev1_00100,  /* D8300..D83FF */
  JoiLev1_00100,  /* D8400..D84FF */
  JoiLev1_00100,  /* D8500..D85FF */
  JoiLev1_00100,  /* D8600..D86FF */
  JoiLev1_00100,  /* D8700..D87FF */
  JoiLev1_00100,  /* D8800..D88FF */
  JoiLev1_00100,  /* D8900..D89FF */
  JoiLev1_00100,  /* D8A00..D8AFF */
  JoiLev1_00100,  /* D8B00..D8BFF */
  JoiLev1_00100,  /* D8C00..D8CFF */
  JoiLev1_00100,  /* D8D00..D8DFF */
  JoiLev1_00100,  /* D8E00..D8EFF */
  JoiLev1_00100,  /* D8F00..D8FFF */
  JoiLev1_00100,  /* D9000..D90FF */
  JoiLev1_00100,  /* D9100..D91FF */
  JoiLev1_00100,  /* D9200..D92FF */
  JoiLev1_00100,  /* D9300..D93FF */
  JoiLev1_00100,  /* D9400..D94FF */
  JoiLev1_00100,  /* D9500..D95FF */
  JoiLev1_00100,  /* D9600..D96FF */
  JoiLev1_00100,  /* D9700..D97FF */
  JoiLev1_00100,  /* D9800..D98FF */
  JoiLev1_00100,  /* D9900..D99FF */
  JoiLev1_00100,  /* D9A00..D9AFF */
  JoiLev1_00100,  /* D9B00..D9BFF */
  JoiLev1_00100,  /* D9C00..D9CFF */
  JoiLev1_00100,  /* D9D00..D9DFF */
  JoiLev1_00100,  /* D9E00..D9EFF */
  JoiLev1_00100,  /* D9F00..D9FFF */
  JoiLev1_00100,  /* DA000..DA0FF */
  JoiLev1_00100,  /* DA100..DA1FF */
  JoiLev1_00100,  /* DA200..DA2FF */
  JoiLev1_00100,  /* DA300..DA3FF */
  JoiLev1_00100,  /* DA400..DA4FF */
  JoiLev1_00100,  /* DA500..DA5FF */
  JoiLev1_00100,  /* DA600..DA6FF */
  JoiLev1_00100,  /* DA700..DA7FF */
  JoiLev1_00100,  /* DA800..DA8FF */
  JoiLev1_00100,  /* DA900..DA9FF */
  JoiLev1_00100,  /* DAA00..DAAFF */
  JoiLev1_00100,  /* DAB00..DABFF */
  JoiLev1_00100,  /* DAC00..DACFF */
  JoiLev1_00100,  /* DAD00..DADFF */
  JoiLev1_00100,  /* DAE00..DAEFF */
  JoiLev1_00100,  /* DAF00..DAFFF */
  JoiLev1_00100,  /* DB000..DB0FF */
  JoiLev1_00100,  /* DB100..DB1FF */
  JoiLev1_00100,  /* DB200..DB2FF */
  JoiLev1_00100,  /* DB300..DB3FF */
  JoiLev1_00100,  /* DB400..DB4FF */
  JoiLev1_00100,  /* DB500..DB5FF */
  JoiLev1_00100,  /* DB600..DB6FF */
  JoiLev1_00100,  /* DB700..DB7FF */
  JoiLev1_00100,  /* DB800..DB8FF */
  JoiLev1_00100,  /* DB900..DB9FF */
  JoiLev1_00100,  /* DBA00..DBAFF */
  JoiLev1_00100,  /* DBB00..DBBFF */
  JoiLev1_00100,  /* DBC00..DBCFF */
  JoiLev1_00100,  /* DBD00..DBDFF */
  JoiLev1_00100,  /* DBE00..DBEFF */
  JoiLev1_00100,  /* DBF00..DBFFF */
  JoiLev1_00100,  /* DC000..DC0FF */
  JoiLev1_00100,  /* DC100..DC1FF */
  JoiLev1_00100,  /* DC200..DC2FF */
  JoiLev1_00100,  /* DC300..DC3FF */
  JoiLev1_00100,  /* DC400..DC4FF */
  JoiLev1_00100,  /* DC500..DC5FF */
  JoiLev1_00100,  /* DC600..DC6FF */
  JoiLev1_00100,  /* DC700..DC7FF */
  JoiLev1_00100,  /* DC800..DC8FF */
  JoiLev1_00100,  /* DC900..DC9FF */
  JoiLev1_00100,  /* DCA00..DCAFF */
  JoiLev1_00100,  /* DCB00..DCBFF */
  JoiLev1_00100,  /* DCC00..DCCFF */
  JoiLev1_00100,  /* DCD00..DCDFF */
  JoiLev1_00100,  /* DCE00..DCEFF */
  JoiLev1_00100,  /* DCF00..DCFFF */
  JoiLev1_00100,  /* DD000..DD0FF */
  JoiLev1_00100,  /* DD100..DD1FF */
  JoiLev1_00100,  /* DD200..DD2FF */
  JoiLev1_00100,  /* DD300..DD3FF */
  JoiLev1_00100,  /* DD400..DD4FF */
  JoiLev1_00100,  /* DD500..DD5FF */
  JoiLev1_00100,  /* DD600..DD6FF */
  JoiLev1_00100,  /* DD700..DD7FF */
  JoiLev1_00100,  /* DD800..DD8FF */
  JoiLev1_00100,  /* DD900..DD9FF */
  JoiLev1_00100,  /* DDA00..DDAFF */
  JoiLev1_00100,  /* DDB00..DDBFF */
  JoiLev1_00100,  /* DDC00..DDCFF */
  JoiLev1_00100,  /* DDD00..DDDFF */
  JoiLev1_00100,  /* DDE00..DDEFF */
  JoiLev1_00100,  /* DDF00..DDFFF */
  JoiLev1_00100,  /* DE000..DE0FF */
  JoiLev1_00100,  /* DE100..DE1FF */
  JoiLev1_00100,  /* DE200..DE2FF */
  JoiLev1_00100,  /* DE300..DE3FF */
  JoiLev1_00100,  /* DE400..DE4FF */
  JoiLev1_00100,  /* DE500..DE5FF */
  JoiLev1_00100,  /* DE600..DE6FF */
  JoiLev1_00100,  /* DE700..DE7FF */
  JoiLev1_00100,  /* DE800..DE8FF */
  JoiLev1_00100,  /* DE900..DE9FF */
  JoiLev1_00100,  /* DEA00..DEAFF */
  JoiLev1_00100,  /* DEB00..DEBFF */
  JoiLev1_00100,  /* DEC00..DECFF */
  JoiLev1_00100,  /* DED00..DEDFF */
  JoiLev1_00100,  /* DEE00..DEEFF */
  JoiLev1_00100,  /* DEF00..DEFFF */
  JoiLev1_00100,  /* DF000..DF0FF */
  JoiLev1_00100,  /* DF100..DF1FF */
  JoiLev1_00100,  /* DF200..DF2FF */
  JoiLev1_00100,  /* DF300..DF3FF */
  JoiLev1_00100,  /* DF400..DF4FF */
  JoiLev1_00100,  /* DF500..DF5FF */
  JoiLev1_00100,  /* DF600..DF6FF */
  JoiLev1_00100,  /* DF700..DF7FF */
  JoiLev1_00100,  /* DF800..DF8FF */
  JoiLev1_00100,  /* DF900..DF9FF */
  JoiLev1_00100,  /* DFA00..DFAFF */
  JoiLev1_00100,  /* DFB00..DFBFF */
  JoiLev1_00100,  /* DFC00..DFCFF */
  JoiLev1_00100,  /* DFD00..DFDFF */
  JoiLev1_00100,  /* DFE00..DFEFF */
  JoiLev1_00100,  /* DFF00..DFFFF */
  JoiLev1_E0000,  /* E0000..E00FF */
  JoiLev1_E0100,  /* E0100..E01FF */
  JoiLev1_00100,  /* E0200..E02FF */
  JoiLev1_00100,  /* E0300..E03FF */
  JoiLev1_00100,  /* E0400..E04FF */
  JoiLev1_00100,  /* E0500..E05FF */
  JoiLev1_00100,  /* E0600..E06FF */
  JoiLev1_00100,  /* E0700..E07FF */
  JoiLev1_00100,  /* E0800..E08FF */
  JoiLev1_00100,  /* E0900..E09FF */
  JoiLev1_00100,  /* E0A00..E0AFF */
  JoiLev1_00100,  /* E0B00..E0BFF */
  JoiLev1_00100,  /* E0C00..E0CFF */
  JoiLev1_00100,  /* E0D00..E0DFF */
  JoiLev1_00100,  /* E0E00..E0EFF */
  JoiLev1_00100,  /* E0F00..E0FFF */
  JoiLev1_00100,  /* E1000..E10FF */
  JoiLev1_00100,  /* E1100..E11FF */
  JoiLev1_00100,  /* E1200..E12FF */
  JoiLev1_00100,  /* E1300..E13FF */
  JoiLev1_00100,  /* E1400..E14FF */
  JoiLev1_00100,  /* E1500..E15FF */
  JoiLev1_00100,  /* E1600..E16FF */
  JoiLev1_00100,  /* E1700..E17FF */
  JoiLev1_00100,  /* E1800..E18FF */
  JoiLev1_00100,  /* E1900..E19FF */
  JoiLev1_00100,  /* E1A00..E1AFF */
  JoiLev1_00100,  /* E1B00..E1BFF */
  JoiLev1_00100,  /* E1C00..E1CFF */
  JoiLev1_00100,  /* E1D00..E1DFF */
  JoiLev1_00100,  /* E1E00..E1EFF */
  JoiLev1_00100,  /* E1F00..E1FFF */
  JoiLev1_00100,  /* E2000..E20FF */
  JoiLev1_00100,  /* E2100..E21FF */
  JoiLev1_00100,  /* E2200..E22FF */
  JoiLev1_00100,  /* E2300..E23FF */
  JoiLev1_00100,  /* E2400..E24FF */
  JoiLev1_00100,  /* E2500..E25FF */
  JoiLev1_00100,  /* E2600..E26FF */
  JoiLev1_00100,  /* E2700..E27FF */
  JoiLev1_00100,  /* E2800..E28FF */
  JoiLev1_00100,  /* E2900..E29FF */
  JoiLev1_00100,  /* E2A00..E2AFF */
  JoiLev1_00100,  /* E2B00..E2BFF */
  JoiLev1_00100,  /* E2C00..E2CFF */
  JoiLev1_00100,  /* E2D00..E2DFF */
  JoiLev1_00100,  /* E2E00..E2EFF */
  JoiLev1_00100,  /* E2F00..E2FFF */
  JoiLev1_00100,  /* E3000..E30FF */
  JoiLev1_00100,  /* E3100..E31FF */
  JoiLev1_00100,  /* E3200..E32FF */
  JoiLev1_00100,  /* E3300..E33FF */
  JoiLev1_00100,  /* E3400..E34FF */
  JoiLev1_00100,  /* E3500..E35FF */
  JoiLev1_00100,  /* E3600..E36FF */
  JoiLev1_00100,  /* E3700..E37FF */
  JoiLev1_00100,  /* E3800..E38FF */
  JoiLev1_00100,  /* E3900..E39FF */
  JoiLev1_00100,  /* E3A00..E3AFF */
  JoiLev1_00100,  /* E3B00..E3BFF */
  JoiLev1_00100,  /* E3C00..E3CFF */
  JoiLev1_00100,  /* E3D00..E3DFF */
  JoiLev1_00100,  /* E3E00..E3EFF */
  JoiLev1_00100,  /* E3F00..E3FFF */
  JoiLev1_00100,  /* E4000..E40FF */
  JoiLev1_00100,  /* E4100..E41FF */
  JoiLev1_00100,  /* E4200..E42FF */
  JoiLev1_00100,  /* E4300..E43FF */
  JoiLev1_00100,  /* E4400..E44FF */
  JoiLev1_00100,  /* E4500..E45FF */
  JoiLev1_00100,  /* E4600..E46FF */
  JoiLev1_00100,  /* E4700..E47FF */
  JoiLev1_00100,  /* E4800..E48FF */
  JoiLev1_00100,  /* E4900..E49FF */
  JoiLev1_00100,  /* E4A00..E4AFF */
  JoiLev1_00100,  /* E4B00..E4BFF */
  JoiLev1_00100,  /* E4C00..E4CFF */
  JoiLev1_00100,  /* E4D00..E4DFF */
  JoiLev1_00100,  /* E4E00..E4EFF */
  JoiLev1_00100,  /* E4F00..E4FFF */
  JoiLev1_00100,  /* E5000..E50FF */
  JoiLev1_00100,  /* E5100..E51FF */
  JoiLev1_00100,  /* E5200..E52FF */
  JoiLev1_00100,  /* E5300..E53FF */
  JoiLev1_00100,  /* E5400..E54FF */
  JoiLev1_00100,  /* E5500..E55FF */
  JoiLev1_00100,  /* E5600..E56FF */
  JoiLev1_00100,  /* E5700..E57FF */
  JoiLev1_00100,  /* E5800..E58FF */
  JoiLev1_00100,  /* E5900..E59FF */
  JoiLev1_00100,  /* E5A00..E5AFF */
  JoiLev1_00100,  /* E5B00..E5BFF */
  JoiLev1_00100,  /* E5C00..E5CFF */
  JoiLev1_00100,  /* E5D00..E5DFF */
  JoiLev1_00100,  /* E5E00..E5EFF */
  JoiLev1_00100,  /* E5F00..E5FFF */
  JoiLev1_00100,  /* E6000..E60FF */
  JoiLev1_00100,  /* E6100..E61FF */
  JoiLev1_00100,  /* E6200..E62FF */
  JoiLev1_00100,  /* E6300..E63FF */
  JoiLev1_00100,  /* E6400..E64FF */
  JoiLev1_00100,  /* E6500..E65FF */
  JoiLev1_00100,  /* E6600..E66FF */
  JoiLev1_00100,  /* E6700..E67FF */
  JoiLev1_00100,  /* E6800..E68FF */
  JoiLev1_00100,  /* E6900..E69FF */
  JoiLev1_00100,  /* E6A00..E6AFF */
  JoiLev1_00100,  /* E6B00..E6BFF */
  JoiLev1_00100,  /* E6C00..E6CFF */
  JoiLev1_00100,  /* E6D00..E6DFF */
  JoiLev1_00100,  /* E6E00..E6EFF */
  JoiLev1_00100,  /* E6F00..E6FFF */
  JoiLev1_00100,  /* E7000..E70FF */
  JoiLev1_00100,  /* E7100..E71FF */
  JoiLev1_00100,  /* E7200..E72FF */
  JoiLev1_00100,  /* E7300..E73FF */
  JoiLev1_00100,  /* E7400..E74FF */
  JoiLev1_00100,  /* E7500..E75FF */
  JoiLev1_00100,  /* E7600..E76FF */
  JoiLev1_00100,  /* E7700..E77FF */
  JoiLev1_00100,  /* E7800..E78FF */
  JoiLev1_00100,  /* E7900..E79FF */
  JoiLev1_00100,  /* E7A00..E7AFF */
  JoiLev1_00100,  /* E7B00..E7BFF */
  JoiLev1_00100,  /* E7C00..E7CFF */
  JoiLev1_00100,  /* E7D00..E7DFF */
  JoiLev1_00100,  /* E7E00..E7EFF */
  JoiLev1_00100,  /* E7F00..E7FFF */
  JoiLev1_00100,  /* E8000..E80FF */
  JoiLev1_00100,  /* E8100..E81FF */
  JoiLev1_00100,  /* E8200..E82FF */
  JoiLev1_00100,  /* E8300..E83FF */
  JoiLev1_00100,  /* E8400..E84FF */
  JoiLev1_00100,  /* E8500..E85FF */
  JoiLev1_00100,  /* E8600..E86FF */
  JoiLev1_00100,  /* E8700..E87FF */
  JoiLev1_00100,  /* E8800..E88FF */
  JoiLev1_00100,  /* E8900..E89FF */
  JoiLev1_00100,  /* E8A00..E8AFF */
  JoiLev1_00100,  /* E8B00..E8BFF */
  JoiLev1_00100,  /* E8C00..E8CFF */
  JoiLev1_00100,  /* E8D00..E8DFF */
  JoiLev1_00100,  /* E8E00..E8EFF */
  JoiLev1_00100,  /* E8F00..E8FFF */
  JoiLev1_00100,  /* E9000..E90FF */
  JoiLev1_00100,  /* E9100..E91FF */
  JoiLev1_00100,  /* E9200..E92FF */
  JoiLev1_00100,  /* E9300..E93FF */
  JoiLev1_00100,  /* E9400..E94FF */
  JoiLev1_00100,  /* E9500..E95FF */
  JoiLev1_00100,  /* E9600..E96FF */
  JoiLev1_00100,  /* E9700..E97FF */
  JoiLev1_00100,  /* E9800..E98FF */
  JoiLev1_00100,  /* E9900..E99FF */
  JoiLev1_00100,  /* E9A00..E9AFF */
  JoiLev1_00100,  /* E9B00..E9BFF */
  JoiLev1_00100,  /* E9C00..E9CFF */
  JoiLev1_00100,  /* E9D00..E9DFF */
  JoiLev1_00100,  /* E9E00..E9EFF */
  JoiLev1_00100,  /* E9F00..E9FFF */
  JoiLev1_00100,  /* EA000..EA0FF */
  JoiLev1_00100,  /* EA100..EA1FF */
  JoiLev1_00100,  /* EA200..EA2FF */
  JoiLev1_00100,  /* EA300..EA3FF */
  JoiLev1_00100,  /* EA400..EA4FF */
  JoiLev1_00100,  /* EA500..EA5FF */
  JoiLev1_00100,  /* EA600..EA6FF */
  JoiLev1_00100,  /* EA700..EA7FF */
  JoiLev1_00100,  /* EA800..EA8FF */
  JoiLev1_00100,  /* EA900..EA9FF */
  JoiLev1_00100,  /* EAA00..EAAFF */
  JoiLev1_00100,  /* EAB00..EABFF */
  JoiLev1_00100,  /* EAC00..EACFF */
  JoiLev1_00100,  /* EAD00..EADFF */
  JoiLev1_00100,  /* EAE00..EAEFF */
  JoiLev1_00100,  /* EAF00..EAFFF */
  JoiLev1_00100,  /* EB000..EB0FF */
  JoiLev1_00100,  /* EB100..EB1FF */
  JoiLev1_00100,  /* EB200..EB2FF */
  JoiLev1_00100,  /* EB300..EB3FF */
  JoiLev1_00100,  /* EB400..EB4FF */
  JoiLev1_00100,  /* EB500..EB5FF */
  JoiLev1_00100,  /* EB600..EB6FF */
  JoiLev1_00100,  /* EB700..EB7FF */
  JoiLev1_00100,  /* EB800..EB8FF */
  JoiLev1_00100,  /* EB900..EB9FF */
  JoiLev1_00100,  /* EBA00..EBAFF */
  JoiLev1_00100,  /* EBB00..EBBFF */
  JoiLev1_00100,  /* EBC00..EBCFF */
  JoiLev1_00100,  /* EBD00..EBDFF */
  JoiLev1_00100,  /* EBE00..EBEFF */
  JoiLev1_00100,  /* EBF00..EBFFF */
  JoiLev1_00100,  /* EC000..EC0FF */
  JoiLev1_00100,  /* EC100..EC1FF */
  JoiLev1_00100,  /* EC200..EC2FF */
  JoiLev1_00100,  /* EC300..EC3FF */
  JoiLev1_00100,  /* EC400..EC4FF */
  JoiLev1_00100,  /* EC500..EC5FF */
  JoiLev1_00100,  /* EC600..EC6FF */
  JoiLev1_00100,  /* EC700..EC7FF */
  JoiLev1_00100,  /* EC800..EC8FF */
  JoiLev1_00100,  /* EC900..EC9FF */
  JoiLev1_00100,  /* ECA00..ECAFF */
  JoiLev1_00100,  /* ECB00..ECBFF */
  JoiLev1_00100,  /* ECC00..ECCFF */
  JoiLev1_00100,  /* ECD00..ECDFF */
  JoiLev1_00100,  /* ECE00..ECEFF */
  JoiLev1_00100,  /* ECF00..ECFFF */
  JoiLev1_00100,  /* ED000..ED0FF */
  JoiLev1_00100,  /* ED100..ED1FF */
  JoiLev1_00100,  /* ED200..ED2FF */
  JoiLev1_00100,  /* ED300..ED3FF */
  JoiLev1_00100,  /* ED400..ED4FF */
  JoiLev1_00100,  /* ED500..ED5FF */
  JoiLev1_00100,  /* ED600..ED6FF */
  JoiLev1_00100,  /* ED700..ED7FF */
  JoiLev1_00100,  /* ED800..ED8FF */
  JoiLev1_00100,  /* ED900..ED9FF */
  JoiLev1_00100,  /* EDA00..EDAFF */
  JoiLev1_00100,  /* EDB00..EDBFF */
  JoiLev1_00100,  /* EDC00..EDCFF */
  JoiLev1_00100,  /* EDD00..EDDFF */
  JoiLev1_00100,  /* EDE00..EDEFF */
  JoiLev1_00100,  /* EDF00..EDFFF */
  JoiLev1_00100,  /* EE000..EE0FF */
  JoiLev1_00100,  /* EE100..EE1FF */
  JoiLev1_00100,  /* EE200..EE2FF */
  JoiLev1_00100,  /* EE300..EE3FF */
  JoiLev1_00100,  /* EE400..EE4FF */
  JoiLev1_00100,  /* EE500..EE5FF */
  JoiLev1_00100,  /* EE600..EE6FF */
  JoiLev1_00100,  /* EE700..EE7FF */
  JoiLev1_00100,  /* EE800..EE8FF */
  JoiLev1_00100,  /* EE900..EE9FF */
  JoiLev1_00100,  /* EEA00..EEAFF */
  JoiLev1_00100,  /* EEB00..EEBFF */
  JoiLev1_00100,  /* EEC00..EECFF */
  JoiLev1_00100,  /* EED00..EEDFF */
  JoiLev1_00100,  /* EEE00..EEEFF */
  JoiLev1_00100,  /* EEF00..EEFFF */
  JoiLev1_00100,  /* EF000..EF0FF */
  JoiLev1_00100,  /* EF100..EF1FF */
  JoiLev1_00100,  /* EF200..EF2FF */
  JoiLev1_00100,  /* EF300..EF3FF */
  JoiLev1_00100,  /* EF400..EF4FF */
  JoiLev1_00100,  /* EF500..EF5FF */
  JoiLev1_00100,  /* EF600..EF6FF */
  JoiLev1_00100,  /* EF700..EF7FF */
  JoiLev1_00100,  /* EF800..EF8FF */
  JoiLev1_00100,  /* EF900..EF9FF */
  JoiLev1_00100,  /* EFA00..EFAFF */
  JoiLev1_00100,  /* EFB00..EFBFF */
  JoiLev1_00100,  /* EFC00..EFCFF */
  JoiLev1_00100,  /* EFD00..EFDFF */
  JoiLev1_00100,  /* EFE00..EFEFF */
  JoiLev1_00100,  /* EFF00..EFFFF */
  JoiLev1_00100,  /* F0000..F00FF */
  JoiLev1_00100,  /* F0100..F01FF */
  JoiLev1_00100,  /* F0200..F02FF */
  JoiLev1_00100,  /* F0300..F03FF */
  JoiLev1_00100,  /* F0400..F04FF */
  JoiLev1_00100,  /* F0500..F05FF */
  JoiLev1_00100,  /* F0600..F06FF */
  JoiLev1_00100,  /* F0700..F07FF */
  JoiLev1_00100,  /* F0800..F08FF */
  JoiLev1_00100,  /* F0900..F09FF */
  JoiLev1_00100,  /* F0A00..F0AFF */
  JoiLev1_00100,  /* F0B00..F0BFF */
  JoiLev1_00100,  /* F0C00..F0CFF */
  JoiLev1_00100,  /* F0D00..F0DFF */
  JoiLev1_00100,  /* F0E00..F0EFF */
  JoiLev1_00100,  /* F0F00..F0FFF */
  JoiLev1_00100,  /* F1000..F10FF */
  JoiLev1_00100,  /* F1100..F11FF */
  JoiLev1_00100,  /* F1200..F12FF */
  JoiLev1_00100,  /* F1300..F13FF */
  JoiLev1_00100,  /* F1400..F14FF */
  JoiLev1_00100,  /* F1500..F15FF */
  JoiLev1_00100,  /* F1600..F16FF */
  JoiLev1_00100,  /* F1700..F17FF */
  JoiLev1_00100,  /* F1800..F18FF */
  JoiLev1_00100,  /* F1900..F19FF */
  JoiLev1_00100,  /* F1A00..F1AFF */
  JoiLev1_00100,  /* F1B00..F1BFF */
  JoiLev1_00100,  /* F1C00..F1CFF */
  JoiLev1_00100,  /* F1D00..F1DFF */
  JoiLev1_00100,  /* F1E00..F1EFF */
  JoiLev1_00100,  /* F1F00..F1FFF */
  JoiLev1_00100,  /* F2000..F20FF */
  JoiLev1_00100,  /* F2100..F21FF */
  JoiLev1_00100,  /* F2200..F22FF */
  JoiLev1_00100,  /* F2300..F23FF */
  JoiLev1_00100,  /* F2400..F24FF */
  JoiLev1_00100,  /* F2500..F25FF */
  JoiLev1_00100,  /* F2600..F26FF */
  JoiLev1_00100,  /* F2700..F27FF */
  JoiLev1_00100,  /* F2800..F28FF */
  JoiLev1_00100,  /* F2900..F29FF */
  JoiLev1_00100,  /* F2A00..F2AFF */
  JoiLev1_00100,  /* F2B00..F2BFF */
  JoiLev1_00100,  /* F2C00..F2CFF */
  JoiLev1_00100,  /* F2D00..F2DFF */
  JoiLev1_00100,  /* F2E00..F2EFF */
  JoiLev1_00100,  /* F2F00..F2FFF */
  JoiLev1_00100,  /* F3000..F30FF */
  JoiLev1_00100,  /* F3100..F31FF */
  JoiLev1_00100,  /* F3200..F32FF */
  JoiLev1_00100,  /* F3300..F33FF */
  JoiLev1_00100,  /* F3400..F34FF */
  JoiLev1_00100,  /* F3500..F35FF */
  JoiLev1_00100,  /* F3600..F36FF */
  JoiLev1_00100,  /* F3700..F37FF */
  JoiLev1_00100,  /* F3800..F38FF */
  JoiLev1_00100,  /* F3900..F39FF */
  JoiLev1_00100,  /* F3A00..F3AFF */
  JoiLev1_00100,  /* F3B00..F3BFF */
  JoiLev1_00100,  /* F3C00..F3CFF */
  JoiLev1_00100,  /* F3D00..F3DFF */
  JoiLev1_00100,  /* F3E00..F3EFF */
  JoiLev1_00100,  /* F3F00..F3FFF */
  JoiLev1_00100,  /* F4000..F40FF */
  JoiLev1_00100,  /* F4100..F41FF */
  JoiLev1_00100,  /* F4200..F42FF */
  JoiLev1_00100,  /* F4300..F43FF */
  JoiLev1_00100,  /* F4400..F44FF */
  JoiLev1_00100,  /* F4500..F45FF */
  JoiLev1_00100,  /* F4600..F46FF */
  JoiLev1_00100,  /* F4700..F47FF */
  JoiLev1_00100,  /* F4800..F48FF */
  JoiLev1_00100,  /* F4900..F49FF */
  JoiLev1_00100,  /* F4A00..F4AFF */
  JoiLev1_00100,  /* F4B00..F4BFF */
  JoiLev1_00100,  /* F4C00..F4CFF */
  JoiLev1_00100,  /* F4D00..F4DFF */
  JoiLev1_00100,  /* F4E00..F4EFF */
  JoiLev1_00100,  /* F4F00..F4FFF */
  JoiLev1_00100,  /* F5000..F50FF */
  JoiLev1_00100,  /* F5100..F51FF */
  JoiLev1_00100,  /* F5200..F52FF */
  JoiLev1_00100,  /* F5300..F53FF */
  JoiLev1_00100,  /* F5400..F54FF */
  JoiLev1_00100,  /* F5500..F55FF */
  JoiLev1_00100,  /* F5600..F56FF */
  JoiLev1_00100,  /* F5700..F57FF */
  JoiLev1_00100,  /* F5800..F58FF */
  JoiLev1_00100,  /* F5900..F59FF */
  JoiLev1_00100,  /* F5A00..F5AFF */
  JoiLev1_00100,  /* F5B00..F5BFF */
  JoiLev1_00100,  /* F5C00..F5CFF */
  JoiLev1_00100,  /* F5D00..F5DFF */
  JoiLev1_00100,  /* F5E00..F5EFF */
  JoiLev1_00100,  /* F5F00..F5FFF */
  JoiLev1_00100,  /* F6000..F60FF */
  JoiLev1_00100,  /* F6100..F61FF */
  JoiLev1_00100,  /* F6200..F62FF */
  JoiLev1_00100,  /* F6300..F63FF */
  JoiLev1_00100,  /* F6400..F64FF */
  JoiLev1_00100,  /* F6500..F65FF */
  JoiLev1_00100,  /* F6600..F66FF */
  JoiLev1_00100,  /* F6700..F67FF */
  JoiLev1_00100,  /* F6800..F68FF */
  JoiLev1_00100,  /* F6900..F69FF */
  JoiLev1_00100,  /* F6A00..F6AFF */
  JoiLev1_00100,  /* F6B00..F6BFF */
  JoiLev1_00100,  /* F6C00..F6CFF */
  JoiLev1_00100,  /* F6D00..F6DFF */
  JoiLev1_00100,  /* F6E00..F6EFF */
  JoiLev1_00100,  /* F6F00..F6FFF */
  JoiLev1_00100,  /* F7000..F70FF */
  JoiLev1_00100,  /* F7100..F71FF */
  JoiLev1_00100,  /* F7200..F72FF */
  JoiLev1_00100,  /* F7300..F73FF */
  JoiLev1_00100,  /* F7400..F74FF */
  JoiLev1_00100,  /* F7500..F75FF */
  JoiLev1_00100,  /* F7600..F76FF */
  JoiLev1_00100,  /* F7700..F77FF */
  JoiLev1_00100,  /* F7800..F78FF */
  JoiLev1_00100,  /* F7900..F79FF */
  JoiLev1_00100,  /* F7A00..F7AFF */
  JoiLev1_00100,  /* F7B00..F7BFF */
  JoiLev1_00100,  /* F7C00..F7CFF */
  JoiLev1_00100,  /* F7D00..F7DFF */
  JoiLev1_00100,  /* F7E00..F7EFF */
  JoiLev1_00100,  /* F7F00..F7FFF */
  JoiLev1_00100,  /* F8000..F80FF */
  JoiLev1_00100,  /* F8100..F81FF */
  JoiLev1_00100,  /* F8200..F82FF */
  JoiLev1_00100,  /* F8300..F83FF */
  JoiLev1_00100,  /* F8400..F84FF */
  JoiLev1_00100,  /* F8500..F85FF */
  JoiLev1_00100,  /* F8600..F86FF */
  JoiLev1_00100,  /* F8700..F87FF */
  JoiLev1_00100,  /* F8800..F88FF */
  JoiLev1_00100,  /* F8900..F89FF */
  JoiLev1_00100,  /* F8A00..F8AFF */
  JoiLev1_00100,  /* F8B00..F8BFF */
  JoiLev1_00100,  /* F8C00..F8CFF */
  JoiLev1_00100,  /* F8D00..F8DFF */
  JoiLev1_00100,  /* F8E00..F8EFF */
  JoiLev1_00100,  /* F8F00..F8FFF */
  JoiLev1_00100,  /* F9000..F90FF */
  JoiLev1_00100,  /* F9100..F91FF */
  JoiLev1_00100,  /* F9200..F92FF */
  JoiLev1_00100,  /* F9300..F93FF */
  JoiLev1_00100,  /* F9400..F94FF */
  JoiLev1_00100,  /* F9500..F95FF */
  JoiLev1_00100,  /* F9600..F96FF */
  JoiLev1_00100,  /* F9700..F97FF */
  JoiLev1_00100,  /* F9800..F98FF */
  JoiLev1_00100,  /* F9900..F99FF */
  JoiLev1_00100,  /* F9A00..F9AFF */
  JoiLev1_00100,  /* F9B00..F9BFF */
  JoiLev1_00100,  /* F9C00..F9CFF */
  JoiLev1_00100,  /* F9D00..F9DFF */
  JoiLev1_00100,  /* F9E00..F9EFF */
  JoiLev1_00100,  /* F9F00..F9FFF */
  JoiLev1_00100,  /* FA000..FA0FF */
  JoiLev1_00100,  /* FA100..FA1FF */
  JoiLev1_00100,  /* FA200..FA2FF */
  JoiLev1_00100,  /* FA300..FA3FF */
  JoiLev1_00100,  /* FA400..FA4FF */
  JoiLev1_00100,  /* FA500..FA5FF */
  JoiLev1_00100,  /* FA600..FA6FF */
  JoiLev1_00100,  /* FA700..FA7FF */
  JoiLev1_00100,  /* FA800..FA8FF */
  JoiLev1_00100,  /* FA900..FA9FF */
  JoiLev1_00100,  /* FAA00..FAAFF */
  JoiLev1_00100,  /* FAB00..FABFF */
  JoiLev1_00100,  /* FAC00..FACFF */
  JoiLev1_00100,  /* FAD00..FADFF */
  JoiLev1_00100,  /* FAE00..FAEFF */
  JoiLev1_00100,  /* FAF00..FAFFF */
  JoiLev1_00100,  /* FB000..FB0FF */
  JoiLev1_00100,  /* FB100..FB1FF */
  JoiLev1_00100,  /* FB200..FB2FF */
  JoiLev1_00100,  /* FB300..FB3FF */
  JoiLev1_00100,  /* FB400..FB4FF */
  JoiLev1_00100,  /* FB500..FB5FF */
  JoiLev1_00100,  /* FB600..FB6FF */
  JoiLev1_00100,  /* FB700..FB7FF */
  JoiLev1_00100,  /* FB800..FB8FF */
  JoiLev1_00100,  /* FB900..FB9FF */
  JoiLev1_00100,  /* FBA00..FBAFF */
  JoiLev1_00100,  /* FBB00..FBBFF */
  JoiLev1_00100,  /* FBC00..FBCFF */
  JoiLev1_00100,  /* FBD00..FBDFF */
  JoiLev1_00100,  /* FBE00..FBEFF */
  JoiLev1_00100,  /* FBF00..FBFFF */
  JoiLev1_00100,  /* FC000..FC0FF */
  JoiLev1_00100,  /* FC100..FC1FF */
  JoiLev1_00100,  /* FC200..FC2FF */
  JoiLev1_00100,  /* FC300..FC3FF */
  JoiLev1_00100,  /* FC400..FC4FF */
  JoiLev1_00100,  /* FC500..FC5FF */
  JoiLev1_00100,  /* FC600..FC6FF */
  JoiLev1_00100,  /* FC700..FC7FF */
  JoiLev1_00100,  /* FC800..FC8FF */
  JoiLev1_00100,  /* FC900..FC9FF */
  JoiLev1_00100,  /* FCA00..FCAFF */
  JoiLev1_00100,  /* FCB00..FCBFF */
  JoiLev1_00100,  /* FCC00..FCCFF */
  JoiLev1_00100,  /* FCD00..FCDFF */
  JoiLev1_00100,  /* FCE00..FCEFF */
  JoiLev1_00100,  /* FCF00..FCFFF */
  JoiLev1_00100,  /* FD000..FD0FF */
  JoiLev1_00100,  /* FD100..FD1FF */
  JoiLev1_00100,  /* FD200..FD2FF */
  JoiLev1_00100,  /* FD300..FD3FF */
  JoiLev1_00100,  /* FD400..FD4FF */
  JoiLev1_00100,  /* FD500..FD5FF */
  JoiLev1_00100,  /* FD600..FD6FF */
  JoiLev1_00100,  /* FD700..FD7FF */
  JoiLev1_00100,  /* FD800..FD8FF */
  JoiLev1_00100,  /* FD900..FD9FF */
  JoiLev1_00100,  /* FDA00..FDAFF */
  JoiLev1_00100,  /* FDB00..FDBFF */
  JoiLev1_00100,  /* FDC00..FDCFF */
  JoiLev1_00100,  /* FDD00..FDDFF */
  JoiLev1_00100,  /* FDE00..FDEFF */
  JoiLev1_00100,  /* FDF00..FDFFF */
  JoiLev1_00100,  /* FE000..FE0FF */
  JoiLev1_00100,  /* FE100..FE1FF */
  JoiLev1_00100,  /* FE200..FE2FF */
  JoiLev1_00100,  /* FE300..FE3FF */
  JoiLev1_00100,  /* FE400..FE4FF */
  JoiLev1_00100,  /* FE500..FE5FF */
  JoiLev1_00100,  /* FE600..FE6FF */
  JoiLev1_00100,  /* FE700..FE7FF */
  JoiLev1_00100,  /* FE800..FE8FF */
  JoiLev1_00100,  /* FE900..FE9FF */
  JoiLev1_00100,  /* FEA00..FEAFF */
  JoiLev1_00100,  /* FEB00..FEBFF */
  JoiLev1_00100,  /* FEC00..FECFF */
  JoiLev1_00100,  /* FED00..FEDFF */
  JoiLev1_00100,  /* FEE00..FEEFF */
  JoiLev1_00100,  /* FEF00..FEFFF */
  JoiLev1_00100,  /* FF000..FF0FF */
  JoiLev1_00100,  /* FF100..FF1FF */
  JoiLev1_00100,  /* FF200..FF2FF */
  JoiLev1_00100,  /* FF300..FF3FF */
  JoiLev1_00100,  /* FF400..FF4FF */
  JoiLev1_00100,  /* FF500..FF5FF */
  JoiLev1_00100,  /* FF600..FF6FF */
  JoiLev1_00100,  /* FF700..FF7FF */
  JoiLev1_00100,  /* FF800..FF8FF */
  JoiLev1_00100,  /* FF900..FF9FF */
  JoiLev1_00100,  /* FFA00..FFAFF */
  JoiLev1_00100,  /* FFB00..FFBFF */
  JoiLev1_00100,  /* FFC00..FFCFF */
  JoiLev1_00100,  /* FFD00..FFDFF */
  JoiLev1_00100,  /* FFE00..FFEFF */
  JoiLev1_00100,  /* FFF00..FFFFF */
};

#define BIDI_GET_JOINING_TYPE(x) \
	((x) >= 0x100000 ? U :  \
	JoiLev1[((x) & 0xff) + \
	JoiLev0[((x) >> 8)]])

#undef PACKTAB_UINT8
#undef PACKTAB_UINT16
#undef PACKTAB_UINT32
