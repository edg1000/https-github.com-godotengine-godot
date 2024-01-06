/* This file has been generated by the Perl script `afblue.pl', */
/* using data from file `afblue.dat'.                           */

/****************************************************************************
 *
 * afblue.h
 *
 *   Auto-fitter data for blue strings (specification).
 *
 * Copyright (C) 2013-2023 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef AFBLUE_H_
#define AFBLUE_H_


FT_BEGIN_HEADER


  /* an auxiliary macro to decode a UTF-8 character -- since we only use */
  /* hard-coded, self-converted data, no error checking is performed     */
#define GET_UTF8_CHAR( ch, p )                      \
          do                                        \
          {                                         \
            ch = (unsigned char)*p++;               \
            if ( ch >= 0x80 )                       \
            {                                       \
              FT_UInt  len_;                        \
                                                    \
                                                    \
              if ( ch < 0xE0 )                      \
              {                                     \
                len_ = 1;                           \
                ch  &= 0x1F;                        \
              }                                     \
              else if ( ch < 0xF0 )                 \
              {                                     \
                len_ = 2;                           \
                ch  &= 0x0F;                        \
              }                                     \
              else                                  \
              {                                     \
                len_ = 3;                           \
                ch  &= 0x07;                        \
              }                                     \
                                                    \
              for ( ; len_ > 0; len_-- )            \
                ch = ( ch << 6 ) | ( *p++ & 0x3F ); \
            }                                       \
          } while ( 0 )


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                    B L U E   S T R I N G S                    *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /* At the bottommost level, we define strings for finding blue zones. */


#define AF_BLUE_STRING_MAX_LEN  51

  /* The AF_Blue_String enumeration values are offsets into the */
  /* `af_blue_strings' array.                                   */

  typedef enum  AF_Blue_String_
  {
    AF_BLUE_STRING_ADLAM_CAPITAL_TOP = 0,
    AF_BLUE_STRING_ADLAM_CAPITAL_BOTTOM = 30,
    AF_BLUE_STRING_ADLAM_SMALL_TOP = 40,
    AF_BLUE_STRING_ADLAM_SMALL_BOTTOM = 65,
    AF_BLUE_STRING_ARABIC_TOP = 105,
    AF_BLUE_STRING_ARABIC_BOTTOM = 123,
    AF_BLUE_STRING_ARABIC_JOIN = 138,
    AF_BLUE_STRING_ARMENIAN_CAPITAL_TOP = 141,
    AF_BLUE_STRING_ARMENIAN_CAPITAL_BOTTOM = 165,
    AF_BLUE_STRING_ARMENIAN_SMALL_ASCENDER = 189,
    AF_BLUE_STRING_ARMENIAN_SMALL_TOP = 210,
    AF_BLUE_STRING_ARMENIAN_SMALL_BOTTOM = 234,
    AF_BLUE_STRING_ARMENIAN_SMALL_DESCENDER = 258,
    AF_BLUE_STRING_AVESTAN_TOP = 282,
    AF_BLUE_STRING_AVESTAN_BOTTOM = 302,
    AF_BLUE_STRING_BAMUM_TOP = 312,
    AF_BLUE_STRING_BAMUM_BOTTOM = 344,
    AF_BLUE_STRING_BENGALI_BASE = 376,
    AF_BLUE_STRING_BENGALI_TOP = 408,
    AF_BLUE_STRING_BENGALI_HEAD = 436,
    AF_BLUE_STRING_BUHID_TOP = 468,
    AF_BLUE_STRING_BUHID_LARGE = 476,
    AF_BLUE_STRING_BUHID_SMALL = 488,
    AF_BLUE_STRING_BUHID_BOTTOM = 504,
    AF_BLUE_STRING_CANADIAN_SYLLABICS_TOP = 532,
    AF_BLUE_STRING_CANADIAN_SYLLABICS_BOTTOM = 564,
    AF_BLUE_STRING_CANADIAN_SYLLABICS_SMALL_TOP = 596,
    AF_BLUE_STRING_CANADIAN_SYLLABICS_SMALL_BOTTOM = 628,
    AF_BLUE_STRING_CANADIAN_SYLLABICS_SUPS_TOP = 660,
    AF_BLUE_STRING_CANADIAN_SYLLABICS_SUPS_BOTTOM = 688,
    AF_BLUE_STRING_CARIAN_TOP = 720,
    AF_BLUE_STRING_CARIAN_BOTTOM = 760,
    AF_BLUE_STRING_CHAKMA_TOP = 795,
    AF_BLUE_STRING_CHAKMA_BOTTOM = 820,
    AF_BLUE_STRING_CHAKMA_DESCENDER = 845,
    AF_BLUE_STRING_CHEROKEE_CAPITAL = 910,
    AF_BLUE_STRING_CHEROKEE_SMALL_ASCENDER = 942,
    AF_BLUE_STRING_CHEROKEE_SMALL = 974,
    AF_BLUE_STRING_CHEROKEE_SMALL_DESCENDER = 1006,
    AF_BLUE_STRING_COPTIC_CAPITAL_TOP = 1022,
    AF_BLUE_STRING_COPTIC_CAPITAL_BOTTOM = 1054,
    AF_BLUE_STRING_COPTIC_SMALL_TOP = 1086,
    AF_BLUE_STRING_COPTIC_SMALL_BOTTOM = 1118,
    AF_BLUE_STRING_CYPRIOT_TOP = 1150,
    AF_BLUE_STRING_CYPRIOT_BOTTOM = 1190,
    AF_BLUE_STRING_CYPRIOT_SMALL = 1225,
    AF_BLUE_STRING_CYRILLIC_CAPITAL_TOP = 1240,
    AF_BLUE_STRING_CYRILLIC_CAPITAL_BOTTOM = 1264,
    AF_BLUE_STRING_CYRILLIC_SMALL = 1288,
    AF_BLUE_STRING_CYRILLIC_SMALL_DESCENDER = 1312,
    AF_BLUE_STRING_DESERET_CAPITAL_TOP = 1321,
    AF_BLUE_STRING_DESERET_CAPITAL_BOTTOM = 1346,
    AF_BLUE_STRING_DESERET_SMALL_TOP = 1371,
    AF_BLUE_STRING_DESERET_SMALL_BOTTOM = 1396,
    AF_BLUE_STRING_DEVANAGARI_BASE = 1421,
    AF_BLUE_STRING_DEVANAGARI_TOP = 1453,
    AF_BLUE_STRING_DEVANAGARI_HEAD = 1485,
    AF_BLUE_STRING_DEVANAGARI_BOTTOM = 1517,
    AF_BLUE_STRING_ETHIOPIC_TOP = 1525,
    AF_BLUE_STRING_ETHIOPIC_BOTTOM = 1557,
    AF_BLUE_STRING_GEORGIAN_MKHEDRULI_TOP = 1589,
    AF_BLUE_STRING_GEORGIAN_MKHEDRULI_BOTTOM = 1621,
    AF_BLUE_STRING_GEORGIAN_MKHEDRULI_ASCENDER = 1653,
    AF_BLUE_STRING_GEORGIAN_MKHEDRULI_DESCENDER = 1685,
    AF_BLUE_STRING_GEORGIAN_ASOMTAVRULI_TOP = 1717,
    AF_BLUE_STRING_GEORGIAN_ASOMTAVRULI_BOTTOM = 1749,
    AF_BLUE_STRING_GEORGIAN_NUSKHURI_TOP = 1781,
    AF_BLUE_STRING_GEORGIAN_NUSKHURI_BOTTOM = 1813,
    AF_BLUE_STRING_GEORGIAN_NUSKHURI_ASCENDER = 1845,
    AF_BLUE_STRING_GEORGIAN_NUSKHURI_DESCENDER = 1877,
    AF_BLUE_STRING_GEORGIAN_MTAVRULI_TOP = 1909,
    AF_BLUE_STRING_GEORGIAN_MTAVRULI_BOTTOM = 1941,
    AF_BLUE_STRING_GLAGOLITIC_CAPITAL_TOP = 1973,
    AF_BLUE_STRING_GLAGOLITIC_CAPITAL_BOTTOM = 2005,
    AF_BLUE_STRING_GLAGOLITIC_SMALL_TOP = 2037,
    AF_BLUE_STRING_GLAGOLITIC_SMALL_BOTTOM = 2069,
    AF_BLUE_STRING_GOTHIC_TOP = 2101,
    AF_BLUE_STRING_GOTHIC_BOTTOM = 2141,
    AF_BLUE_STRING_GREEK_CAPITAL_TOP = 2161,
    AF_BLUE_STRING_GREEK_CAPITAL_BOTTOM = 2182,
    AF_BLUE_STRING_GREEK_SMALL_BETA_TOP = 2200,
    AF_BLUE_STRING_GREEK_SMALL = 2218,
    AF_BLUE_STRING_GREEK_SMALL_DESCENDER = 2242,
    AF_BLUE_STRING_GUJARATI_TOP = 2266,
    AF_BLUE_STRING_GUJARATI_BOTTOM = 2298,
    AF_BLUE_STRING_GUJARATI_ASCENDER = 2330,
    AF_BLUE_STRING_GUJARATI_DESCENDER = 2380,
    AF_BLUE_STRING_GUJARATI_DIGIT_TOP = 2413,
    AF_BLUE_STRING_GURMUKHI_BASE = 2433,
    AF_BLUE_STRING_GURMUKHI_HEAD = 2465,
    AF_BLUE_STRING_GURMUKHI_TOP = 2497,
    AF_BLUE_STRING_GURMUKHI_BOTTOM = 2529,
    AF_BLUE_STRING_GURMUKHI_DIGIT_TOP = 2561,
    AF_BLUE_STRING_HEBREW_TOP = 2581,
    AF_BLUE_STRING_HEBREW_BOTTOM = 2605,
    AF_BLUE_STRING_HEBREW_DESCENDER = 2623,
    AF_BLUE_STRING_KANNADA_TOP = 2638,
    AF_BLUE_STRING_KANNADA_BOTTOM = 2682,
    AF_BLUE_STRING_KAYAH_LI_TOP = 2714,
    AF_BLUE_STRING_KAYAH_LI_BOTTOM = 2738,
    AF_BLUE_STRING_KAYAH_LI_ASCENDER = 2758,
    AF_BLUE_STRING_KAYAH_LI_DESCENDER = 2766,
    AF_BLUE_STRING_KAYAH_LI_LARGE_DESCENDER = 2778,
    AF_BLUE_STRING_KHMER_TOP = 2799,
    AF_BLUE_STRING_KHMER_SUBSCRIPT_TOP = 2823,
    AF_BLUE_STRING_KHMER_BOTTOM = 2863,
    AF_BLUE_STRING_KHMER_DESCENDER = 2895,
    AF_BLUE_STRING_KHMER_LARGE_DESCENDER = 2929,
    AF_BLUE_STRING_KHMER_SYMBOLS_WAXING_TOP = 3016,
    AF_BLUE_STRING_KHMER_SYMBOLS_WANING_BOTTOM = 3024,
    AF_BLUE_STRING_LAO_TOP = 3032,
    AF_BLUE_STRING_LAO_BOTTOM = 3064,
    AF_BLUE_STRING_LAO_ASCENDER = 3096,
    AF_BLUE_STRING_LAO_LARGE_ASCENDER = 3112,
    AF_BLUE_STRING_LAO_DESCENDER = 3124,
    AF_BLUE_STRING_LATIN_CAPITAL_TOP = 3148,
    AF_BLUE_STRING_LATIN_CAPITAL_BOTTOM = 3164,
    AF_BLUE_STRING_LATIN_SMALL_F_TOP = 3180,
    AF_BLUE_STRING_LATIN_SMALL_TOP = 3194,
    AF_BLUE_STRING_LATIN_SMALL_BOTTOM = 3210,
    AF_BLUE_STRING_LATIN_SMALL_DESCENDER = 3226,
    AF_BLUE_STRING_LATIN_SUBS_CAPITAL_TOP = 3236,
    AF_BLUE_STRING_LATIN_SUBS_CAPITAL_BOTTOM = 3256,
    AF_BLUE_STRING_LATIN_SUBS_SMALL_F_TOP = 3276,
    AF_BLUE_STRING_LATIN_SUBS_SMALL = 3296,
    AF_BLUE_STRING_LATIN_SUBS_SMALL_DESCENDER = 3332,
    AF_BLUE_STRING_LATIN_SUPS_CAPITAL_TOP = 3352,
    AF_BLUE_STRING_LATIN_SUPS_CAPITAL_BOTTOM = 3383,
    AF_BLUE_STRING_LATIN_SUPS_SMALL_F_TOP = 3412,
    AF_BLUE_STRING_LATIN_SUPS_SMALL = 3438,
    AF_BLUE_STRING_LATIN_SUPS_SMALL_DESCENDER = 3463,
    AF_BLUE_STRING_LISU_TOP = 3474,
    AF_BLUE_STRING_LISU_BOTTOM = 3506,
    AF_BLUE_STRING_MALAYALAM_TOP = 3538,
    AF_BLUE_STRING_MALAYALAM_BOTTOM = 3582,
    AF_BLUE_STRING_MEDEFAIDRIN_CAPITAL_TOP = 3614,
    AF_BLUE_STRING_MEDEFAIDRIN_CAPITAL_BOTTOM = 3649,
    AF_BLUE_STRING_MEDEFAIDRIN_SMALL_F_TOP = 3689,
    AF_BLUE_STRING_MEDEFAIDRIN_SMALL_TOP = 3719,
    AF_BLUE_STRING_MEDEFAIDRIN_SMALL_BOTTOM = 3749,
    AF_BLUE_STRING_MEDEFAIDRIN_SMALL_DESCENDER = 3779,
    AF_BLUE_STRING_MEDEFAIDRIN_DIGIT_TOP = 3794,
    AF_BLUE_STRING_MONGOLIAN_TOP_BASE = 3819,
    AF_BLUE_STRING_MONGOLIAN_BOTTOM_BASE = 3863,
    AF_BLUE_STRING_MYANMAR_TOP = 3867,
    AF_BLUE_STRING_MYANMAR_BOTTOM = 3899,
    AF_BLUE_STRING_MYANMAR_ASCENDER = 3931,
    AF_BLUE_STRING_MYANMAR_DESCENDER = 3959,
    AF_BLUE_STRING_NKO_TOP = 3991,
    AF_BLUE_STRING_NKO_BOTTOM = 4015,
    AF_BLUE_STRING_NKO_SMALL_TOP = 4030,
    AF_BLUE_STRING_NKO_SMALL_BOTTOM = 4039,
    AF_BLUE_STRING_OL_CHIKI = 4051,
    AF_BLUE_STRING_OLD_TURKIC_TOP = 4075,
    AF_BLUE_STRING_OLD_TURKIC_BOTTOM = 4090,
    AF_BLUE_STRING_OSAGE_CAPITAL_TOP = 4110,
    AF_BLUE_STRING_OSAGE_CAPITAL_BOTTOM = 4150,
    AF_BLUE_STRING_OSAGE_CAPITAL_DESCENDER = 4180,
    AF_BLUE_STRING_OSAGE_SMALL_TOP = 4195,
    AF_BLUE_STRING_OSAGE_SMALL_BOTTOM = 4235,
    AF_BLUE_STRING_OSAGE_SMALL_ASCENDER = 4275,
    AF_BLUE_STRING_OSAGE_SMALL_DESCENDER = 4300,
    AF_BLUE_STRING_OSMANYA_TOP = 4315,
    AF_BLUE_STRING_OSMANYA_BOTTOM = 4355,
    AF_BLUE_STRING_ROHINGYA_TOP = 4395,
    AF_BLUE_STRING_ROHINGYA_BOTTOM = 4420,
    AF_BLUE_STRING_ROHINGYA_JOIN = 4445,
    AF_BLUE_STRING_SAURASHTRA_TOP = 4448,
    AF_BLUE_STRING_SAURASHTRA_BOTTOM = 4480,
    AF_BLUE_STRING_SHAVIAN_TOP = 4500,
    AF_BLUE_STRING_SHAVIAN_BOTTOM = 4510,
    AF_BLUE_STRING_SHAVIAN_DESCENDER = 4535,
    AF_BLUE_STRING_SHAVIAN_SMALL_TOP = 4545,
    AF_BLUE_STRING_SHAVIAN_SMALL_BOTTOM = 4580,
    AF_BLUE_STRING_SINHALA_TOP = 4595,
    AF_BLUE_STRING_SINHALA_BOTTOM = 4627,
    AF_BLUE_STRING_SINHALA_DESCENDER = 4659,
    AF_BLUE_STRING_SUNDANESE_TOP = 4703,
    AF_BLUE_STRING_SUNDANESE_BOTTOM = 4727,
    AF_BLUE_STRING_SUNDANESE_DESCENDER = 4759,
    AF_BLUE_STRING_TAI_VIET_TOP = 4767,
    AF_BLUE_STRING_TAI_VIET_BOTTOM = 4787,
    AF_BLUE_STRING_TAMIL_TOP = 4799,
    AF_BLUE_STRING_TAMIL_BOTTOM = 4831,
    AF_BLUE_STRING_TELUGU_TOP = 4863,
    AF_BLUE_STRING_TELUGU_BOTTOM = 4891,
    AF_BLUE_STRING_THAI_TOP = 4919,
    AF_BLUE_STRING_THAI_BOTTOM = 4943,
    AF_BLUE_STRING_THAI_ASCENDER = 4971,
    AF_BLUE_STRING_THAI_LARGE_ASCENDER = 4983,
    AF_BLUE_STRING_THAI_DESCENDER = 4995,
    AF_BLUE_STRING_THAI_LARGE_DESCENDER = 5011,
    AF_BLUE_STRING_THAI_DIGIT_TOP = 5019,
    AF_BLUE_STRING_TIFINAGH = 5031,
    AF_BLUE_STRING_VAI_TOP = 5063,
    AF_BLUE_STRING_VAI_BOTTOM = 5095,
    af_blue_1_1 = 5126,
#ifdef AF_CONFIG_OPTION_CJK
    AF_BLUE_STRING_CJK_TOP = af_blue_1_1 + 1,
    AF_BLUE_STRING_CJK_BOTTOM = af_blue_1_1 + 203,
    af_blue_1_1_1 = af_blue_1_1 + 404,
#ifdef AF_CONFIG_OPTION_CJK_BLUE_HANI_VERT
    AF_BLUE_STRING_CJK_LEFT = af_blue_1_1_1 + 1,
    AF_BLUE_STRING_CJK_RIGHT = af_blue_1_1_1 + 204,
    af_blue_1_1_2 = af_blue_1_1_1 + 405,
#else
    af_blue_1_1_2 = af_blue_1_1_1 + 0,
#endif /* AF_CONFIG_OPTION_CJK_BLUE_HANI_VERT */
    af_blue_1_2 = af_blue_1_1_2 + 0,
#else
    af_blue_1_2 = af_blue_1_1 + 0,
#endif /* AF_CONFIG_OPTION_CJK                */


    AF_BLUE_STRING_MAX   /* do not remove */

  } AF_Blue_String;


  FT_LOCAL_ARRAY( char )
  af_blue_strings[];


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                 B L U E   S T R I N G S E T S                 *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /* The next level is to group blue strings into style-specific sets. */


  /* Properties are specific to a writing system.  We assume that a given  */
  /* blue string can't be used in more than a single writing system, which */
  /* is a safe bet.                                                        */
#define AF_BLUE_PROPERTY_LATIN_TOP       ( 1U << 0 )  /* must have value 1 */
#define AF_BLUE_PROPERTY_LATIN_SUB_TOP   ( 1U << 1 )
#define AF_BLUE_PROPERTY_LATIN_NEUTRAL   ( 1U << 2 )
#define AF_BLUE_PROPERTY_LATIN_X_HEIGHT  ( 1U << 3 )
#define AF_BLUE_PROPERTY_LATIN_LONG      ( 1U << 4 )

#define AF_BLUE_PROPERTY_CJK_TOP    ( 1U << 0 )       /* must have value 1 */
#define AF_BLUE_PROPERTY_CJK_HORIZ  ( 1U << 1 )       /* must have value 2 */
#define AF_BLUE_PROPERTY_CJK_RIGHT  AF_BLUE_PROPERTY_CJK_TOP


#define AF_BLUE_STRINGSET_MAX_LEN  8

  /* The AF_Blue_Stringset enumeration values are offsets into the */
  /* `af_blue_stringsets' array.                                   */

  typedef enum  AF_Blue_Stringset_
  {
    AF_BLUE_STRINGSET_ADLM = 0,
    AF_BLUE_STRINGSET_ARAB = 5,
    AF_BLUE_STRINGSET_ARMN = 9,
    AF_BLUE_STRINGSET_AVST = 16,
    AF_BLUE_STRINGSET_BAMU = 19,
    AF_BLUE_STRINGSET_BENG = 22,
    AF_BLUE_STRINGSET_BUHD = 27,
    AF_BLUE_STRINGSET_CAKM = 32,
    AF_BLUE_STRINGSET_CANS = 36,
    AF_BLUE_STRINGSET_CARI = 43,
    AF_BLUE_STRINGSET_CHER = 46,
    AF_BLUE_STRINGSET_COPT = 53,
    AF_BLUE_STRINGSET_CPRT = 58,
    AF_BLUE_STRINGSET_CYRL = 63,
    AF_BLUE_STRINGSET_DEVA = 69,
    AF_BLUE_STRINGSET_DSRT = 75,
    AF_BLUE_STRINGSET_ETHI = 80,
    AF_BLUE_STRINGSET_GEOR = 83,
    AF_BLUE_STRINGSET_GEOK = 90,
    AF_BLUE_STRINGSET_GLAG = 97,
    AF_BLUE_STRINGSET_GOTH = 102,
    AF_BLUE_STRINGSET_GREK = 105,
    AF_BLUE_STRINGSET_GUJR = 112,
    AF_BLUE_STRINGSET_GURU = 118,
    AF_BLUE_STRINGSET_HEBR = 124,
    AF_BLUE_STRINGSET_KNDA = 128,
    AF_BLUE_STRINGSET_KALI = 131,
    AF_BLUE_STRINGSET_KHMR = 137,
    AF_BLUE_STRINGSET_KHMS = 143,
    AF_BLUE_STRINGSET_LAO = 146,
    AF_BLUE_STRINGSET_LATN = 152,
    AF_BLUE_STRINGSET_LATB = 159,
    AF_BLUE_STRINGSET_LATP = 166,
    AF_BLUE_STRINGSET_LISU = 173,
    AF_BLUE_STRINGSET_MLYM = 176,
    AF_BLUE_STRINGSET_MEDF = 179,
    AF_BLUE_STRINGSET_MONG = 187,
    AF_BLUE_STRINGSET_MYMR = 190,
    AF_BLUE_STRINGSET_NKOO = 195,
    AF_BLUE_STRINGSET_NONE = 200,
    AF_BLUE_STRINGSET_OLCK = 201,
    AF_BLUE_STRINGSET_ORKH = 204,
    AF_BLUE_STRINGSET_OSGE = 207,
    AF_BLUE_STRINGSET_OSMA = 215,
    AF_BLUE_STRINGSET_ROHG = 218,
    AF_BLUE_STRINGSET_SAUR = 222,
    AF_BLUE_STRINGSET_SHAW = 225,
    AF_BLUE_STRINGSET_SINH = 231,
    AF_BLUE_STRINGSET_SUND = 235,
    AF_BLUE_STRINGSET_TAML = 239,
    AF_BLUE_STRINGSET_TAVT = 242,
    AF_BLUE_STRINGSET_TELU = 245,
    AF_BLUE_STRINGSET_THAI = 248,
    AF_BLUE_STRINGSET_TFNG = 256,
    AF_BLUE_STRINGSET_VAII = 259,
    af_blue_2_1 = 262,
#ifdef AF_CONFIG_OPTION_CJK
    AF_BLUE_STRINGSET_HANI = af_blue_2_1 + 0,
    af_blue_2_1_1 = af_blue_2_1 + 2,
#ifdef AF_CONFIG_OPTION_CJK_BLUE_HANI_VERT
    af_blue_2_1_2 = af_blue_2_1_1 + 2,
#else
    af_blue_2_1_2 = af_blue_2_1_1 + 0,
#endif /* AF_CONFIG_OPTION_CJK_BLUE_HANI_VERT */
    af_blue_2_2 = af_blue_2_1_2 + 1,
#else
    af_blue_2_2 = af_blue_2_1 + 0,
#endif /* AF_CONFIG_OPTION_CJK                */


    AF_BLUE_STRINGSET_MAX   /* do not remove */

  } AF_Blue_Stringset;


  typedef struct  AF_Blue_StringRec_
  {
    AF_Blue_String  string;
    FT_UShort       properties;

  } AF_Blue_StringRec;


  FT_LOCAL_ARRAY( AF_Blue_StringRec )
  af_blue_stringsets[];

/* */

FT_END_HEADER


#endif /* AFBLUE_H_ */


/* END */
