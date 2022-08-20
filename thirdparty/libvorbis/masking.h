/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2009             *
 * by the Xiph.Org Foundation https://xiph.org/                     *
 *                                                                  *
 ********************************************************************

 function: masking curve data for psychoacoustics

 ********************************************************************/

#ifndef _V_MASKING_H_
#define _V_MASKING_H_

/* more detailed ATH; the bass if flat to save stressing the floor
   overly for only a bin or two of savings. */

#define MAX_ATH 88
static const float ATH[]={
  /*15*/  -51, -52, -53, -54, -55, -56, -57, -58,
  /*31*/  -59, -60, -61, -62, -63, -64, -65, -66,
  /*63*/  -67, -68, -69, -70, -71, -72, -73, -74,
  /*125*/ -75, -76, -77, -78, -80, -81, -82, -83,
  /*250*/ -84, -85, -86, -87, -88, -88, -89, -89,
  /*500*/ -90, -91, -91, -92, -93, -94, -95, -96,
  /*1k*/  -96, -97, -98, -98, -99, -99,-100,-100,
  /*2k*/ -101,-102,-103,-104,-106,-107,-107,-107,
  /*4k*/ -107,-105,-103,-102,-101, -99, -98, -96,
  /*8k*/  -95, -95, -96, -97, -96, -95, -93, -90,
  /*16k*/ -80, -70, -50, -40, -30, -30, -30, -30
};

/* The tone masking curves from Ehmer's and Fielder's papers have been
   replaced by an empirically collected data set.  The previously
   published values were, far too often, simply on crack. */

#define EHMER_OFFSET 16
#define EHMER_MAX 56

/* masking tones from -50 to 0dB, 62.5 through 16kHz at half octaves
   test tones from -2 octaves to +5 octaves sampled at eighth octaves */
/* (Vorbis 0dB, the loudest possible tone, is assumed to be ~100dB SPL
   for collection of these curves) */

static const float tonemasks[P_BANDS][6][EHMER_MAX]={
  /* 62.5 Hz */
  {{ -60,  -60,  -60,  -60,  -60,  -60,  -60,  -60,
     -60,  -60,  -60,  -60,  -62,  -62,  -65,  -73,
     -69,  -68,  -68,  -67,  -70,  -70,  -72,  -74,
     -75,  -79,  -79,  -80,  -83,  -88,  -93, -100,
     -110, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -48,  -48,  -48,  -48,  -48,  -48,  -48,  -48,
     -48,  -48,  -48,  -48,  -48,  -53,  -61,  -66,
     -66,  -68,  -67,  -70,  -76,  -76,  -72,  -73,
     -75,  -76,  -78,  -79,  -83,  -88,  -93, -100,
     -110, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -37,  -37,  -37,  -37,  -37,  -37,  -37,  -37,
     -38,  -40,  -42,  -46,  -48,  -53,  -55,  -62,
     -65,  -58,  -56,  -56,  -61,  -60,  -65,  -67,
     -69,  -71,  -77,  -77,  -78,  -80,  -82,  -84,
     -88,  -93,  -98, -106, -112, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -25,  -25,  -25,  -25,  -25,  -25,  -25,  -25,
     -25,  -26,  -27,  -29,  -32,  -38,  -48,  -52,
     -52,  -50,  -48,  -48,  -51,  -52,  -54,  -60,
     -67,  -67,  -66,  -68,  -69,  -73,  -73,  -76,
     -80,  -81,  -81,  -85,  -85,  -86,  -88,  -93,
     -100, -110, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -16,  -16,  -16,  -16,  -16,  -16,  -16,  -16,
     -17,  -19,  -20,  -22,  -26,  -28,  -31,  -40,
     -47,  -39,  -39,  -40,  -42,  -43,  -47,  -51,
     -57,  -52,  -55,  -55,  -60,  -58,  -62,  -63,
     -70,  -67,  -69,  -72,  -73,  -77,  -80,  -82,
     -83,  -87,  -90,  -94,  -98, -104, -115, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   {  -8,   -8,   -8,   -8,   -8,   -8,   -8,   -8,
      -8,   -8,  -10,  -11,  -15,  -19,  -25,  -30,
      -34,  -31,  -30,  -31,  -29,  -32,  -35,  -42,
      -48,  -42,  -44,  -46,  -50,  -50,  -51,  -52,
      -59,  -54,  -55,  -55,  -58,  -62,  -63,  -66,
      -72,  -73,  -76,  -75,  -78,  -80,  -80,  -81,
      -84,  -88,  -90,  -94,  -98, -101, -106, -110}},
  /* 88Hz */
  {{ -66,  -66,  -66,  -66,  -66,  -66,  -66,  -66,
     -66,  -66,  -66,  -66,  -66,  -67,  -67,  -67,
     -76,  -72,  -71,  -74,  -76,  -76,  -75,  -78,
     -79,  -79,  -81,  -83,  -86,  -89,  -93,  -97,
     -100, -105, -110, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -47,  -47,  -47,  -47,  -47,  -47,  -47,  -47,
     -47,  -47,  -47,  -48,  -51,  -55,  -59,  -66,
     -66,  -66,  -67,  -66,  -68,  -69,  -70,  -74,
     -79,  -77,  -77,  -78,  -80,  -81,  -82,  -84,
     -86,  -88,  -91,  -95, -100, -108, -116, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -36,  -36,  -36,  -36,  -36,  -36,  -36,  -36,
     -36,  -37,  -37,  -41,  -44,  -48,  -51,  -58,
     -62,  -60,  -57,  -59,  -59,  -60,  -63,  -65,
     -72,  -71,  -70,  -72,  -74,  -77,  -76,  -78,
     -81,  -81,  -80,  -83,  -86,  -91,  -96, -100,
     -105, -110, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -28,  -28,  -28,  -28,  -28,  -28,  -28,  -28,
     -28,  -30,  -32,  -32,  -33,  -35,  -41,  -49,
     -50,  -49,  -47,  -48,  -48,  -52,  -51,  -57,
     -65,  -61,  -59,  -61,  -64,  -69,  -70,  -74,
     -77,  -77,  -78,  -81,  -84,  -85,  -87,  -90,
     -92,  -96, -100, -107, -112, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -19,  -19,  -19,  -19,  -19,  -19,  -19,  -19,
     -20,  -21,  -23,  -27,  -30,  -35,  -36,  -41,
     -46,  -44,  -42,  -40,  -41,  -41,  -43,  -48,
     -55,  -53,  -52,  -53,  -56,  -59,  -58,  -60,
     -67,  -66,  -69,  -71,  -72,  -75,  -79,  -81,
     -84,  -87,  -90,  -93,  -97, -101, -107, -114,
     -999, -999, -999, -999, -999, -999, -999, -999},
   {  -9,   -9,   -9,   -9,   -9,   -9,   -9,   -9,
      -11,  -12,  -12,  -15,  -16,  -20,  -23,  -30,
      -37,  -34,  -33,  -34,  -31,  -32,  -32,  -38,
      -47,  -44,  -41,  -40,  -47,  -49,  -46,  -46,
      -58,  -50,  -50,  -54,  -58,  -62,  -64,  -67,
      -67,  -70,  -72,  -76,  -79,  -83,  -87,  -91,
      -96, -100, -104, -110, -999, -999, -999, -999}},
  /* 125 Hz */
  {{ -62,  -62,  -62,  -62,  -62,  -62,  -62,  -62,
     -62,  -62,  -63,  -64,  -66,  -67,  -66,  -68,
     -75,  -72,  -76,  -75,  -76,  -78,  -79,  -82,
     -84,  -85,  -90,  -94, -101, -110, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -59,  -59,  -59,  -59,  -59,  -59,  -59,  -59,
     -59,  -59,  -59,  -60,  -60,  -61,  -63,  -66,
     -71,  -68,  -70,  -70,  -71,  -72,  -72,  -75,
     -81,  -78,  -79,  -82,  -83,  -86,  -90,  -97,
     -103, -113, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -53,  -53,  -53,  -53,  -53,  -53,  -53,  -53,
     -53,  -54,  -55,  -57,  -56,  -57,  -55,  -61,
     -65,  -60,  -60,  -62,  -63,  -63,  -66,  -68,
     -74,  -73,  -75,  -75,  -78,  -80,  -80,  -82,
     -85,  -90,  -96, -101, -108, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -46,  -46,  -46,  -46,  -46,  -46,  -46,  -46,
     -46,  -46,  -47,  -47,  -47,  -47,  -48,  -51,
     -57,  -51,  -49,  -50,  -51,  -53,  -54,  -59,
     -66,  -60,  -62,  -67,  -67,  -70,  -72,  -75,
     -76,  -78,  -81,  -85,  -88,  -94,  -97, -104,
     -112, -999, -999, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -36,  -36,  -36,  -36,  -36,  -36,  -36,  -36,
     -39,  -41,  -42,  -42,  -39,  -38,  -41,  -43,
     -52,  -44,  -40,  -39,  -37,  -37,  -40,  -47,
     -54,  -50,  -48,  -50,  -55,  -61,  -59,  -62,
     -66,  -66,  -66,  -69,  -69,  -73,  -74,  -74,
     -75,  -77,  -79,  -82,  -87,  -91,  -95, -100,
     -108, -115, -999, -999, -999, -999, -999, -999},
   { -28,  -26,  -24,  -22,  -20,  -20,  -23,  -29,
     -30,  -31,  -28,  -27,  -28,  -28,  -28,  -35,
     -40,  -33,  -32,  -29,  -30,  -30,  -30,  -37,
     -45,  -41,  -37,  -38,  -45,  -47,  -47,  -48,
     -53,  -49,  -48,  -50,  -49,  -49,  -51,  -52,
     -58,  -56,  -57,  -56,  -60,  -61,  -62,  -70,
     -72,  -74,  -78,  -83,  -88,  -93, -100, -106}},
  /* 177 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -110, -105, -100,  -95,  -91,  -87,  -83,
    -80,  -78,  -76,  -78,  -78,  -81,  -83,  -85,
    -86,  -85,  -86,  -87,  -90,  -97, -107, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -110, -105, -100,  -95,  -90,
    -85,  -81,  -77,  -73,  -70,  -67,  -67,  -68,
    -75,  -73,  -70,  -69,  -70,  -72,  -75,  -79,
    -84,  -83,  -84,  -86,  -88,  -89,  -89,  -93,
    -98, -105, -112, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-105, -100,  -95,  -90,  -85,  -80,  -76,  -71,
    -68,  -68,  -65,  -63,  -63,  -62,  -62,  -64,
    -65,  -64,  -61,  -62,  -63,  -64,  -66,  -68,
    -73,  -73,  -74,  -75,  -76,  -81,  -83,  -85,
    -88,  -89,  -92,  -95, -100, -108, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   { -80,  -75,  -71,  -68,  -65,  -63,  -62,  -61,
     -61,  -61,  -61,  -59,  -56,  -57,  -53,  -50,
     -58,  -52,  -50,  -50,  -52,  -53,  -54,  -58,
     -67,  -63,  -67,  -68,  -72,  -75,  -78,  -80,
     -81,  -81,  -82,  -85,  -89,  -90,  -93,  -97,
     -101, -107, -114, -999, -999, -999, -999, -999,
     -999, -999, -999, -999, -999, -999, -999, -999},
   { -65,  -61,  -59,  -57,  -56,  -55,  -55,  -56,
     -56,  -57,  -55,  -53,  -52,  -47,  -44,  -44,
     -50,  -44,  -41,  -39,  -39,  -42,  -40,  -46,
     -51,  -49,  -50,  -53,  -54,  -63,  -60,  -61,
     -62,  -66,  -66,  -66,  -70,  -73,  -74,  -75,
     -76,  -75,  -79,  -85,  -89,  -91,  -96, -102,
     -110, -999, -999, -999, -999, -999, -999, -999},
   { -52,  -50,  -49,  -49,  -48,  -48,  -48,  -49,
     -50,  -50,  -49,  -46,  -43,  -39,  -35,  -33,
     -38,  -36,  -32,  -29,  -32,  -32,  -32,  -35,
     -44,  -39,  -38,  -38,  -46,  -50,  -45,  -46,
     -53,  -50,  -50,  -50,  -54,  -54,  -53,  -53,
     -56,  -57,  -59,  -66,  -70,  -72,  -74,  -79,
     -83,  -85,  -90, -97, -114, -999, -999, -999}},
  /* 250 Hz */
  {{-999, -999, -999, -999, -999, -999, -110, -105,
    -100,  -95,  -90,  -86,  -80,  -75,  -75,  -79,
    -80,  -79,  -80,  -81,  -82,  -88,  -95, -103,
    -110, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -108, -103,  -98,  -93,
    -88,  -83,  -79,  -78,  -75,  -71,  -67,  -68,
    -73,  -73,  -72,  -73,  -75,  -77,  -80,  -82,
    -88,  -93, -100, -107, -114, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -110, -105, -101,  -96,  -90,
    -86,  -81,  -77,  -73,  -69,  -66,  -61,  -62,
    -66,  -64,  -62,  -65,  -66,  -70,  -72,  -76,
    -81,  -80,  -84,  -90,  -95, -102, -110, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -107, -103,  -97,  -92,  -88,
    -83,  -79,  -74,  -70,  -66,  -59,  -53,  -58,
    -62,  -55,  -54,  -54,  -54,  -58,  -61,  -62,
    -72,  -70,  -72,  -75,  -78,  -80,  -81,  -80,
    -83,  -83,  -88,  -93, -100, -107, -115, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -105, -100,  -95,  -90,  -85,
    -80,  -75,  -70,  -66,  -62,  -56,  -48,  -44,
    -48,  -46,  -46,  -43,  -46,  -48,  -48,  -51,
    -58,  -58,  -59,  -60,  -62,  -62,  -61,  -61,
    -65,  -64,  -65,  -68,  -70,  -74,  -75,  -78,
    -81,  -86,  -95, -110, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999,  -999, -105, -100,  -95,  -90,  -85,  -80,
    -75,  -70,  -65,  -61,  -55,  -49,  -39,  -33,
    -40,  -35,  -32,  -38,  -40,  -33,  -35,  -37,
    -46,  -41,  -45,  -44,  -46,  -42,  -45,  -46,
    -52,  -50,  -50,  -50,  -54,  -54,  -55,  -57,
    -62,  -64,  -66,  -68,  -70,  -76,  -81,  -90,
    -100, -110, -999, -999, -999, -999, -999, -999}},
  /* 354 hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -105,  -98,  -90,  -85,  -82,  -83,  -80,  -78,
    -84,  -79,  -80,  -83,  -87,  -89,  -91,  -93,
    -99, -106, -117, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -105,  -98,  -90,  -85,  -80,  -75,  -70,  -68,
    -74,  -72,  -74,  -77,  -80,  -82,  -85,  -87,
    -92,  -89,  -91,  -95, -100, -106, -112, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -105,  -98,  -90,  -83,  -75,  -71,  -63,  -64,
    -67,  -62,  -64,  -67,  -70,  -73,  -77,  -81,
    -84,  -83,  -85,  -89,  -90,  -93,  -98, -104,
    -109, -114, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -103,  -96,  -88,  -81,  -75,  -68,  -58,  -54,
    -56,  -54,  -56,  -56,  -58,  -60,  -63,  -66,
    -74,  -69,  -72,  -72,  -75,  -74,  -77,  -81,
    -81,  -82,  -84,  -87,  -93,  -96,  -99, -104,
    -110, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -108, -102,  -96,
    -91,  -85,  -80,  -74,  -68,  -60,  -51,  -46,
    -48,  -46,  -43,  -45,  -47,  -47,  -49,  -48,
    -56,  -53,  -55,  -58,  -57,  -63,  -58,  -60,
    -66,  -64,  -67,  -70,  -70,  -74,  -77,  -84,
    -86,  -89,  -91,  -93,  -94, -101, -109, -118,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -108, -103,  -98,  -93,  -88,
    -83,  -78,  -73,  -68,  -60,  -53,  -44,  -35,
    -38,  -38,  -34,  -34,  -36,  -40,  -41,  -44,
    -51,  -45,  -46,  -47,  -46,  -54,  -50,  -49,
    -50,  -50,  -50,  -51,  -54,  -57,  -58,  -60,
    -66,  -66,  -66,  -64,  -65,  -68,  -77,  -82,
    -87,  -95, -110, -999, -999, -999, -999, -999}},
  /* 500 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -107, -102,  -97,  -92,  -87,  -83,  -78,  -75,
    -82,  -79,  -83,  -85,  -89,  -92,  -95,  -98,
    -101, -105, -109, -113, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -106,
    -100,  -95,  -90,  -86,  -81,  -78,  -74,  -69,
    -74,  -74,  -76,  -79,  -83,  -84,  -86,  -89,
    -92,  -97,  -93, -100, -103, -107, -110, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -106, -100,
    -95, -90, -87, -83, -80, -75, -69, -60,
    -66, -66, -68, -70, -74, -78, -79, -81,
    -81, -83, -84, -87, -93, -96, -99, -103,
    -107, -110, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -108, -103, -98,
    -93, -89, -85, -82, -78, -71, -62, -55,
    -58, -58, -54, -54, -55, -59, -61, -62,
    -70, -66, -66, -67, -70, -72, -75, -78,
    -84, -84, -84, -88, -91, -90, -95, -98,
    -102, -103, -106, -110, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -108, -103,  -98,  -94,
    -90,  -87,  -82,  -79,  -73,  -67,  -58,  -47,
    -50,  -45,  -41,  -45,  -48,  -44,  -44,  -49,
    -54,  -51,  -48,  -47,  -49,  -50,  -51,  -57,
    -58,  -60,  -63,  -69,  -70,  -69,  -71,  -74,
    -78,  -82,  -90,  -95, -101, -105, -110, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -105, -101, -97, -93, -90,
    -85, -80, -77, -72, -65, -56, -48, -37,
    -40, -36, -34, -40, -50, -47, -38, -41,
    -47, -38, -35, -39, -38, -43, -40, -45,
    -50, -45, -44, -47, -50, -55, -48, -48,
    -52, -66, -70, -76, -82, -90, -97, -105,
    -110, -999, -999, -999, -999, -999, -999, -999}},
  /* 707 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -108, -103,  -98,  -93,  -86,  -79,  -76,
    -83,  -81,  -85,  -87,  -89,  -93,  -98, -102,
    -107, -112, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -108, -103,  -98,  -93,  -86,  -79,  -71,
    -77,  -74,  -77,  -79,  -81,  -84,  -85,  -90,
    -92,  -93,  -92,  -98, -101, -108, -112, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -108, -103,  -98,  -93,  -87,  -78,  -68,  -65,
    -66,  -62,  -65,  -67,  -70,  -73,  -75,  -78,
    -82,  -82,  -83,  -84,  -91,  -93,  -98, -102,
    -106, -110, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -105, -100, -95, -90, -82, -74, -62, -57,
    -58, -56, -51, -52, -52, -54, -54, -58,
    -66, -59, -60, -63, -66, -69, -73, -79,
    -83, -84, -80, -81, -81, -82, -88, -92,
    -98, -105, -113, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -107,
    -102,  -97,  -92,  -84,  -79,  -69,  -57,  -47,
    -52,  -47,  -44,  -45,  -50,  -52,  -42,  -42,
    -53,  -43,  -43,  -48,  -51,  -56,  -55,  -52,
    -57,  -59,  -61,  -62,  -67,  -71,  -78,  -83,
    -86,  -94,  -98, -103, -110, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -105, -100,
    -95,  -90,  -84,  -78,  -70,  -61,  -51,  -41,
    -40,  -38,  -40,  -46,  -52,  -51,  -41,  -40,
    -46,  -40,  -38,  -38,  -41,  -46,  -41,  -46,
    -47,  -43,  -43,  -45,  -41,  -45,  -56,  -67,
    -68,  -83,  -87,  -90,  -95, -102, -107, -113,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 1000 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -109, -105, -101,  -96,  -91,  -84,  -77,
    -82,  -82,  -85,  -89,  -94, -100, -106, -110,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -106, -103,  -98,  -92,  -85,  -80,  -71,
    -75,  -72,  -76,  -80,  -84,  -86,  -89,  -93,
    -100, -107, -113, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -107,
    -104, -101,  -97,  -92,  -88,  -84,  -80,  -64,
    -66,  -63,  -64,  -66,  -69,  -73,  -77,  -83,
    -83,  -86,  -91,  -98, -104, -111, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -107,
    -104, -101,  -97,  -92,  -90,  -84,  -74,  -57,
    -58,  -52,  -55,  -54,  -50,  -52,  -50,  -52,
    -63,  -62,  -69,  -76,  -77,  -78,  -78,  -79,
    -82,  -88,  -94, -100, -106, -111, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -106, -102,
    -98,  -95,  -90,  -85,  -83,  -78,  -70,  -50,
    -50,  -41,  -44,  -49,  -47,  -50,  -50,  -44,
    -55,  -46,  -47,  -48,  -48,  -54,  -49,  -49,
    -58,  -62,  -71,  -81,  -87,  -92,  -97, -102,
    -108, -114, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -106, -102,
    -98,  -95,  -90,  -85,  -83,  -78,  -70,  -45,
    -43,  -41,  -47,  -50,  -51,  -50,  -49,  -45,
    -47,  -41,  -44,  -41,  -39,  -43,  -38,  -37,
    -40,  -41,  -44,  -50,  -58,  -65,  -73,  -79,
    -85,  -92,  -97, -101, -105, -109, -113, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 1414 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -107, -100,  -95,  -87,  -81,
    -85,  -83,  -88,  -93, -100, -107, -114, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -107, -101,  -95,  -88,  -83,  -76,
    -73,  -72,  -79,  -84,  -90,  -95, -100, -105,
    -110, -115, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -104,  -98,  -92,  -87,  -81,  -70,
    -65,  -62,  -67,  -71,  -74,  -80,  -85,  -91,
    -95,  -99, -103, -108, -111, -114, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -103,  -97,  -90,  -85,  -76,  -60,
    -56,  -54,  -60,  -62,  -61,  -56,  -63,  -65,
    -73,  -74,  -77,  -75,  -78,  -81,  -86,  -87,
    -88,  -91,  -94,  -98, -103, -110, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -105,
    -100,  -97,  -92,  -86,  -81,  -79,  -70,  -57,
    -51,  -47,  -51,  -58,  -60,  -56,  -53,  -50,
    -58,  -52,  -50,  -50,  -53,  -55,  -64,  -69,
    -71,  -85,  -82,  -78,  -81,  -85,  -95, -102,
    -112, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -105,
    -100,  -97,  -92,  -85,  -83,  -79,  -72,  -49,
    -40,  -43,  -43,  -54,  -56,  -51,  -50,  -40,
    -43,  -38,  -36,  -35,  -37,  -38,  -37,  -44,
    -54,  -60,  -57,  -60,  -70,  -75,  -84,  -92,
    -103, -112, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 2000 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -110, -102,  -95,  -89,  -82,
    -83,  -84,  -90,  -92,  -99, -107, -113, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -107, -101,  -95,  -89,  -83,  -72,
    -74,  -78,  -85,  -88,  -88,  -90,  -92,  -98,
    -105, -111, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -109, -103, -97, -93, -87, -81, -70,
    -70, -67, -75, -73, -76, -79, -81, -83,
    -88, -89, -97, -103, -110, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -107, -100,  -94,  -88,  -83,  -75,  -63,
    -59,  -59,  -63,  -66,  -60,  -62,  -67,  -67,
    -77,  -76,  -81,  -88,  -86,  -92,  -96, -102,
    -109, -116, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -105,  -98,  -92,  -86,  -81,  -73,  -56,
    -52,  -47,  -55,  -60,  -58,  -52,  -51,  -45,
    -49,  -50,  -53,  -54,  -61,  -71,  -70,  -69,
    -78,  -79,  -87,  -90,  -96, -104, -112, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -103,  -96,  -90,  -86,  -78,  -70,  -51,
    -42,  -47,  -48,  -55,  -54,  -54,  -53,  -42,
    -35,  -28,  -33,  -38,  -37,  -44,  -47,  -49,
    -54,  -63,  -68,  -78,  -82,  -89,  -94,  -99,
    -104, -109, -114, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 2828 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -110, -100,  -90,  -79,
    -85,  -81,  -82,  -82,  -89,  -94,  -99, -103,
    -109, -115, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -105,  -97,  -85,  -72,
    -74,  -70,  -70,  -70,  -76,  -85,  -91,  -93,
    -97, -103, -109, -115, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -112,  -93,  -81,  -68,
    -62,  -60,  -60,  -57,  -63,  -70,  -77,  -82,
    -90,  -93,  -98, -104, -109, -113, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -113, -100,  -93,  -84,  -63,
    -58,  -48,  -53,  -54,  -52,  -52,  -57,  -64,
    -66,  -76,  -83,  -81,  -85,  -85,  -90,  -95,
    -98, -101, -103, -106, -108, -111, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -105,  -95,  -86,  -74,  -53,
    -50,  -38,  -43,  -49,  -43,  -42,  -39,  -39,
    -46,  -52,  -57,  -56,  -72,  -69,  -74,  -81,
    -87,  -92,  -94,  -97,  -99, -102, -105, -108,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -108,  -99,  -90,  -76,  -66,  -45,
    -43,  -41,  -44,  -47,  -43,  -47,  -40,  -30,
    -31,  -31,  -39,  -33,  -40,  -41,  -43,  -53,
    -59,  -70,  -73,  -77,  -79,  -82,  -84,  -87,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 4000 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -110,  -91,  -76,
    -75,  -85,  -93,  -98, -104, -110, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -110,  -91,  -70,
    -70,  -75,  -86,  -89,  -94,  -98, -101, -106,
    -110, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -110,  -95,  -80,  -60,
    -65,  -64,  -74,  -83,  -88,  -91,  -95,  -99,
    -103, -107, -110, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -110,  -95,  -80,  -58,
    -55,  -49,  -66,  -68,  -71,  -78,  -78,  -80,
    -88,  -85,  -89,  -97, -100, -105, -110, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -110,  -95,  -80,  -53,
    -52,  -41,  -59,  -59,  -49,  -58,  -56,  -63,
    -86,  -79,  -90,  -93,  -98, -103, -107, -112,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -110,  -97,  -91,  -73,  -45,
    -40,  -33,  -53,  -61,  -49,  -54,  -50,  -50,
    -60,  -52,  -67,  -74,  -81,  -92,  -96, -100,
    -105, -110, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 5657 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -113, -106,  -99,  -92,  -77,
    -80,  -88,  -97, -106, -115, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -116, -109, -102,  -95,  -89,  -74,
    -72,  -88,  -87,  -95, -102, -109, -116, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -116, -109, -102,  -95,  -89,  -75,
    -66,  -74,  -77,  -78,  -86,  -87,  -90,  -96,
    -105, -115, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -115, -108, -101,  -94,  -88,  -66,
    -56,  -61,  -70,  -65,  -78,  -72,  -83,  -84,
    -93,  -98, -105, -110, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -110, -105,  -95,  -89,  -82,  -57,
    -52,  -52,  -59,  -56,  -59,  -58,  -69,  -67,
    -88,  -82,  -82,  -89,  -94, -100, -108, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -110, -101,  -96,  -90,  -83,  -77,  -54,
    -43,  -38,  -50,  -48,  -52,  -48,  -42,  -42,
    -51,  -52,  -53,  -59,  -65,  -71,  -78,  -85,
    -95, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 8000 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -120, -105,  -86,  -68,
    -78,  -79,  -90, -100, -110, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -120, -105,  -86,  -66,
    -73,  -77,  -88,  -96, -105, -115, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -120, -105,  -92,  -80,  -61,
    -64,  -68,  -80,  -87,  -92, -100, -110, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -120, -104,  -91,  -79,  -52,
    -60,  -54,  -64,  -69,  -77,  -80,  -82,  -84,
    -85,  -87,  -88,  -90, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -118, -100,  -87,  -77,  -49,
    -50,  -44,  -58,  -61,  -61,  -67,  -65,  -62,
    -62,  -62,  -65,  -68, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -115,  -98,  -84,  -62,  -49,
    -44,  -38,  -46,  -49,  -49,  -46,  -39,  -37,
    -39,  -40,  -42,  -43, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 11314 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -110,  -88,  -74,
    -77,  -82,  -82,  -85,  -90,  -94,  -99, -104,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -110,  -88,  -66,
    -70,  -81,  -80,  -81,  -84,  -88,  -91,  -93,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -110,  -88,  -61,
    -63,  -70,  -71,  -74,  -77,  -80,  -83,  -85,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -110, -86, -62,
    -63,  -62,  -62,  -58,  -52,  -50,  -50,  -52,
    -54, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -118, -108,  -84,  -53,
    -50,  -50,  -50,  -55,  -47,  -45,  -40,  -40,
    -40, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -118, -100,  -73,  -43,
    -37,  -42,  -43,  -53,  -38,  -37,  -35,  -35,
    -38, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}},
  /* 16000 Hz */
  {{-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -110, -100,  -91,  -84,  -74,
    -80,  -80,  -80,  -80,  -80, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -110, -100,  -91,  -84,  -74,
    -68,  -68,  -68,  -68,  -68, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -110, -100,  -86,  -78,  -70,
    -60,  -45,  -30,  -21, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -110, -100,  -87,  -78,  -67,
    -48,  -38,  -29,  -21, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -110, -100,  -86,  -69,  -56,
    -45,  -35,  -33,  -29, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999},
   {-999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -110, -100,  -83,  -71,  -48,
    -27,  -38,  -37,  -34, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999,
    -999, -999, -999, -999, -999, -999, -999, -999}}
};

#endif
