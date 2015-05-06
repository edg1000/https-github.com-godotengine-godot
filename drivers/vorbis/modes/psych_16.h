/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2009             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: 16kHz settings
 last mod: $Id: psych_16.h 16227 2009-07-08 06:58:46Z xiphmont $

 ********************************************************************/

/* stereo mode by base quality level */
static const adj_stereo _psy_stereo_modes_16[4]={
  /*  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  */
  {{  4,  4,  4,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3},
   {  6,  5,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4},
   {  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  4,  4},
   { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}},
  {{  4,  4,  4,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3},
   {  6,  5,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4},
   {  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  4,  4,  4,  4,  4},
   { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}},
  {{  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3},
   {  5,  4,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3},
   {  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4},
   { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}},
  {{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
   {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
   {  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8},
   { 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99}},
};

static const double _psy_lowpass_16[4]={6.5,8,30.,99.};

static const att3 _psy_tone_masteratt_16[4]={
  {{ 30,  25,  12},  0,   0},  /* 0 */
  {{ 25,  22,  12},  0,   0},  /* 0 */
  {{ 20,  12,   0},  0,   0},  /* 0 */
  {{ 15,   0, -14},  0,   0}, /* 0 */
};

static const vp_adjblock _vp_tonemask_adj_16[4]={
  /* adjust for mode zero */
  /* 63     125     250     500       1     2     4     8    16 */
  {{-20,-20,-20,-20,-20,-16,-10,  0,  0, 0, 0,10, 0, 0, 0, 0, 0}}, /* 0 */
  {{-20,-20,-20,-20,-20,-16,-10,  0,  0, 0, 0,10, 0, 0, 0, 0, 0}}, /* 1 */
  {{-20,-20,-20,-20,-20,-16,-10,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0}}, /* 2 */
  {{-30,-30,-30,-30,-30,-26,-20,-10, -5, 0, 0, 0, 0, 0, 0, 0, 0}}, /* 2 */
};


static const noise3 _psy_noisebias_16_short[4]={
  /*  63     125     250     500      1k       2k      4k      8k     16k*/
  {{{-15,-15,-15,-15,-15,-10,-10,-5,   4, 10, 10, 10, 10, 12, 12, 14, 20},
    {-15,-15,-15,-15,-15,-10,-10, -5,  0,  0,  4,  5,  5,  6,  8,  8, 15},
    {-30,-30,-30,-30,-30,-24,-20,-14,-10, -6, -8, -8, -6, -6, -6, -6, -6}}},

  {{{-15,-15,-15,-15,-15,-10,-10,-5,   4,  6,  6,  6,  6,  8, 10, 12, 20},
    {-15,-15,-15,-15,-15,-15,-15,-10, -5, -5, -5,  4,  5,  6,  8,  8, 15},
    {-30,-30,-30,-30,-30,-24,-20,-14,-10,-10,-10,-10,-10,-10,-10,-10,-10}}},

  {{{-15,-15,-15,-15,-15,-12,-10, -8,  0,  2,  4,  4,  5,  5,  5,  8, 12},
    {-20,-20,-20,-20,-16,-12,-20,-14,-10,-10, -8,  0,  0,  0,  0,  2,  5},
    {-30,-30,-30,-30,-26,-26,-26,-26,-26,-26,-26,-26,-26,-24,-20,-20,-20}}},

  {{{-15,-15,-15,-15,-15,-12,-10, -8, -5, -5, -5, -5, -5,  0,  0,  0,  6},
    {-30,-30,-30,-30,-26,-22,-20,-14,-12,-12,-10,-10,-10,-10,-10,-10, -6},
    {-30,-30,-30,-30,-26,-26,-26,-26,-26,-26,-26,-26,-26,-24,-20,-20,-20}}},
};

static const noise3 _psy_noisebias_16_impulse[4]={
  /*  63     125     250     500      1k       2k      4k      8k     16k*/
  {{{-15,-15,-15,-15,-15,-10,-10,-5,   4, 10, 10, 10, 10, 12, 12, 14, 20},
    {-15,-15,-15,-15,-15,-10,-10, -5,  0,  0,  4,  5,  5,  6,  8,  8, 15},
    {-30,-30,-30,-30,-30,-24,-20,-14,-10, -6, -8, -8, -6, -6, -6, -6, -6}}},

  {{{-15,-15,-15,-15,-15,-10,-10,-5,   4,  4,  4,  4,  5,  5,  6,  8, 15},
    {-15,-15,-15,-15,-15,-15,-15,-10, -5, -5, -5,  0,  0,  0,  0,  4, 10},
    {-30,-30,-30,-30,-30,-24,-20,-14,-10,-10,-10,-10,-10,-10,-10,-10,-10}}},

  {{{-15,-15,-15,-15,-15,-12,-10, -8,  0,  0,  0,  0,  0,  0,  0,  4, 10},
    {-20,-20,-20,-20,-16,-12,-20,-14,-10,-10,-10,-10,-10,-10,-10, -7, -5},
    {-30,-30,-30,-30,-26,-26,-26,-26,-26,-26,-26,-26,-26,-24,-20,-20,-20}}},

  {{{-15,-15,-15,-15,-15,-12,-10, -8, -5, -5, -5, -5, -5,  0,  0,  0,  6},
    {-30,-30,-30,-30,-26,-22,-20,-18,-18,-18,-20,-20,-20,-20,-20,-20,-16},
    {-30,-30,-30,-30,-26,-26,-26,-26,-26,-26,-26,-26,-26,-24,-20,-20,-20}}},
};

static const noise3 _psy_noisebias_16[4]={
  /*  63     125     250     500      1k       2k      4k      8k     16k*/
  {{{-10,-10,-10,-10, -5, -5, -5,  0,  4,  6,  8,  8, 10, 10, 10, 14, 20},
    {-10,-10,-10,-10,-10, -5, -2, -2,  0,  0,  0,  4,  5,  6,  8,  8, 15},
    {-30,-30,-30,-30,-30,-24,-20,-14,-10, -6, -8, -8, -6, -6, -6, -6, -6}}},

  {{{-10,-10,-10,-10, -5, -5, -5,  0,  4,  6,  6,  6,  6,  8, 10, 12, 20},
    {-15,-15,-15,-15,-15,-10, -5, -5,  0,  0,  0,  4,  5,  6,  8,  8, 15},
    {-30,-30,-30,-30,-30,-24,-20,-14,-10, -6, -8, -8, -6, -6, -6, -6, -6}}},

  {{{-15,-15,-15,-15,-15,-12,-10, -8,  0,  2,  4,  4,  5,  5,  5,  8, 12},
    {-20,-20,-20,-20,-16,-12,-20,-10, -5, -5,  0,  0,  0,  0,  0,  2,  5},
    {-30,-30,-30,-30,-26,-26,-26,-26,-26,-26,-26,-26,-26,-24,-20,-20,-20}}},

  {{{-15,-15,-15,-15,-15,-12,-10, -8, -5, -5, -5, -5, -5,  0,  0,  0,  6},
    {-30,-30,-30,-30,-26,-22,-20,-14,-12,-12,-10,-10,-10,-10,-10,-10, -6},
    {-30,-30,-30,-30,-26,-26,-26,-26,-26,-26,-26,-26,-26,-24,-20,-20,-20}}},
};

static const noiseguard _psy_noiseguards_16[4]={
  {10,10,-1},
  {10,10,-1},
  {20,20,-1},
  {20,20,-1},
};

static const double _noise_thresh_16[4]={ .3,.5,.5,.5 };

static const int _noise_start_16[3]={ 256,256,9999 };
static const int _noise_part_16[4]={ 8,8,8,8 };

static const int _psy_ath_floater_16[4]={
  -100,-100,-100,-105,
};

static const int _psy_ath_abs_16[4]={
  -130,-130,-130,-140,
};
