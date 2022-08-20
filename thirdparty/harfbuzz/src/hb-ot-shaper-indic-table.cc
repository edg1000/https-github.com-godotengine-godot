/* == Start of generated table == */
/*
 * The following table is generated by running:
 *
 *   ./gen-indic-table.py IndicSyllabicCategory.txt IndicPositionalCategory.txt Blocks.txt
 *
 * on files with these headers:
 *
 * # IndicSyllabicCategory-14.0.0.txt
 * # Date: 2021-05-22, 01:01:00 GMT [KW, RP]
 * # IndicPositionalCategory-14.0.0.txt
 * # Date: 2021-05-22, 01:01:00 GMT [KW, RP]
 * # Blocks-14.0.0.txt
 * # Date: 2021-01-22, 23:29:00 GMT [KW]
 */

#include "hb.hh"

#ifndef HB_NO_OT_SHAPE

#include "hb-ot-shaper-indic.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-macros"

#include "hb-ot-shaper-indic-machine.hh"
#include "hb-ot-shaper-khmer-machine.hh"
#include "hb-ot-shaper-myanmar-machine.hh"

/* indic */
#define OT_X I_Cat(X)
#define OT_C I_Cat(C)
#define OT_V I_Cat(V)
#define OT_N I_Cat(N)
#define OT_H I_Cat(H)
#define OT_ZWNJ I_Cat(ZWNJ)
#define OT_ZWJ I_Cat(ZWJ)
#define OT_M I_Cat(M)
#define OT_SM I_Cat(SM)
#define OT_A I_Cat(A)
#define OT_VD I_Cat(VD)
#define OT_PLACEHOLDER I_Cat(PLACEHOLDER)
#define OT_DOTTEDCIRCLE I_Cat(DOTTEDCIRCLE)
#define OT_RS I_Cat(RS)
#define OT_Repha I_Cat(Repha)
#define OT_Ra I_Cat(Ra)
#define OT_CM I_Cat(CM)
#define OT_Symbol I_Cat(Symbol)
#define OT_CS I_Cat(CS)
/* khmer */
#define OT_VAbv K_Cat(VAbv)
#define OT_VBlw K_Cat(VBlw)
#define OT_VPre K_Cat(VPre)
#define OT_VPst K_Cat(VPst)
#define OT_Robatic K_Cat(Robatic)
#define OT_Xgroup K_Cat(Xgroup)
#define OT_Ygroup K_Cat(Ygroup)
/* myanmar */
static_assert (OT_VAbv == M_Cat(VAbv), "");
static_assert (OT_VBlw == M_Cat(VBlw), "");
static_assert (OT_VPre == M_Cat(VPre), "");
static_assert (OT_VPst == M_Cat(VPst), "");
#define OT_IV M_Cat(IV)
#define OT_As M_Cat(As)
#define OT_DB M_Cat(DB)
#define OT_GB M_Cat(GB)
#define OT_MH M_Cat(MH)
#define OT_MR M_Cat(MR)
#define OT_MW M_Cat(MW)
#define OT_MY M_Cat(MY)
#define OT_PT M_Cat(PT)
#define OT_VS M_Cat(VS)
#define OT_ML M_Cat(ML)


#define _OT_A    OT_A            /*  53 chars; A */
#define _OT_As   OT_As           /*   1 chars; As */
#define _OT_C    OT_C            /* 478 chars; C */
#define _OT_CM   OT_CM           /*   1 chars; CM */
#define _OT_CS   OT_CS           /*   2 chars; CS */
#define _OT_DC   OT_DOTTEDCIRCLE /*   1 chars; DOTTEDCIRCLE */
#define _OT_H    OT_H            /*  11 chars; H */
#define _OT_M    OT_M            /* 143 chars; M */
#define _OT_MH   OT_MH           /*   1 chars; MH */
#define _OT_ML   OT_ML           /*   1 chars; ML */
#define _OT_MR   OT_MR           /*   1 chars; MR */
#define _OT_MW   OT_MW           /*   2 chars; MW */
#define _OT_MY   OT_MY           /*   3 chars; MY */
#define _OT_N    OT_N            /*  17 chars; N */
#define _OT_GB   OT_PLACEHOLDER  /* 165 chars; PLACEHOLDER */
#define _OT_PT   OT_PT           /*   8 chars; PT */
#define _OT_R    OT_Ra           /*  14 chars; Ra */
#define _OT_Rf   OT_Repha        /*   1 chars; Repha */
#define _OT_Rt   OT_Robatic      /*   3 chars; Robatic */
#define _OT_SM   OT_SM           /*  55 chars; SM */
#define _OT_S    OT_Symbol       /*  22 chars; Symbol */
#define _OT_V    OT_V            /* 172 chars; V */
#define _OT_VA   OT_VAbv         /*  18 chars; VAbv */
#define _OT_VB   OT_VBlw         /*   7 chars; VBlw */
#define _OT_VL   OT_VPre         /*   5 chars; VPre */
#define _OT_VR   OT_VPst         /*  13 chars; VPst */
#define _OT_VS   OT_VS           /*  16 chars; VS */
#define _OT_X    OT_X            /*   2 chars; X */
#define _OT_Xg   OT_Xgroup       /*   7 chars; Xgroup */
#define _OT_Yg   OT_Ygroup       /*   4 chars; Ygroup */
#define _OT_ZWJ  OT_ZWJ          /*   1 chars; ZWJ */
#define _OT_ZWNJ OT_ZWNJ         /*   1 chars; ZWNJ */

#define _POS_T   POS_ABOVE_C     /*  22 chars; ABOVE_C */
#define _POS_A   POS_AFTER_MAIN  /*   3 chars; AFTER_MAIN */
#define _POS_AP  POS_AFTER_POST  /*  50 chars; AFTER_POST */
#define _POS_AS  POS_AFTER_SUB   /*  51 chars; AFTER_SUB */
#define _POS_C   POS_BASE_C      /* 833 chars; BASE_C */
#define _POS_BS  POS_BEFORE_SUB  /*  25 chars; BEFORE_SUB */
#define _POS_B   POS_BELOW_C     /*  13 chars; BELOW_C */
#define _POS_X   POS_END         /*  71 chars; END */
#define _POS_R   POS_POST_C      /*  13 chars; POST_C */
#define _POS_L   POS_PRE_C       /*   5 chars; PRE_C */
#define _POS_LM  POS_PRE_M       /*  14 chars; PRE_M */
#define _POS_SM  POS_SMVD        /* 129 chars; SMVD */

#pragma GCC diagnostic pop

#define INDIC_COMBINE_CATEGORIES(S,M) ((S) | ((M) << 8))

#define _(S,M) INDIC_COMBINE_CATEGORIES (_OT_##S, _POS_##M)


static const uint16_t indic_table[] = {


#define indic_offset_0x0028u 0


  /* Basic Latin */

  /* 0028 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X), _(GB,C),  _(X,X),  _(X,X),
  /* 0030 */ _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C),
  /* 0038 */ _(GB,C), _(GB,C),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),

#define indic_offset_0x00b0u 24


  /* Latin-1 Supplement */

  /* 00B0 */  _(X,X),  _(X,X),_(SM,SM),_(SM,SM),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 00B8 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 00C0 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 00C8 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 00D0 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X), _(GB,C),

#define indic_offset_0x0900u 64


  /* Devanagari */

  /* 0900 */_(SM,SM),_(SM,SM),_(SM,SM),_(SM,SM),  _(V,C),  _(V,C),  _(V,C),  _(V,C),
  /* 0908 */  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),
  /* 0910 */  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0918 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0920 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0928 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0930 */  _(R,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0938 */  _(C,C),  _(C,C), _(M,AS), _(M,AS),  _(N,X), _(S,SM), _(M,AS), _(M,LM),
  /* 0940 */ _(M,AS), _(M,AS), _(M,AS), _(M,AS), _(M,AS), _(M,AS), _(M,AS), _(M,AS),
  /* 0948 */ _(M,AS), _(M,AS), _(M,AS), _(M,AS), _(M,AS),  _(H,B), _(M,LM), _(M,AS),
  /* 0950 */  _(X,X), _(A,SM), _(A,SM),_(SM,SM),_(SM,SM), _(M,AS), _(M,AS), _(M,AS),
  /* 0958 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0960 */  _(V,C),  _(V,C), _(M,AS), _(M,AS),  _(X,X),  _(X,X), _(GB,C), _(GB,C),
  /* 0968 */ _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C),
  /* 0970 */  _(X,X),  _(X,X),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),
  /* 0978 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),

  /* Bengali */

  /* 0980 */ _(GB,C),_(SM,SM),_(SM,SM),_(SM,SM),  _(X,X),  _(V,C),  _(V,C),  _(V,C),
  /* 0988 */  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(X,X),  _(X,X),  _(V,C),
  /* 0990 */  _(V,C),  _(X,X),  _(X,X),  _(V,C),  _(V,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0998 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 09A0 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 09A8 */  _(C,C),  _(X,X),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 09B0 */  _(R,C),  _(X,X),  _(C,C),  _(X,X),  _(X,X),  _(X,X),  _(C,C),  _(C,C),
  /* 09B8 */  _(C,C),  _(C,C),  _(X,X),  _(X,X),  _(N,X), _(S,SM), _(M,AP), _(M,LM),
  /* 09C0 */ _(M,AP), _(M,AS), _(M,AS), _(M,AS), _(M,AS),  _(X,X),  _(X,X), _(M,LM),
  /* 09C8 */ _(M,LM),  _(X,X),  _(X,X), _(M,AP), _(M,AP),  _(H,B),  _(C,C),  _(X,X),
  /* 09D0 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X), _(M,AP),
  /* 09D8 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(C,C),  _(C,C),  _(X,X),  _(C,C),
  /* 09E0 */  _(V,C),  _(V,C), _(M,AS), _(M,AS),  _(X,X),  _(X,X), _(GB,C), _(GB,C),
  /* 09E8 */ _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C),
  /* 09F0 */  _(R,C),  _(C,C),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 09F8 */  _(X,X),  _(X,X),  _(X,X),  _(X,X), _(GB,C),  _(X,X),_(SM,SM),  _(X,X),

  /* Gurmukhi */

  /* 0A00 */  _(X,X),_(SM,SM),_(SM,SM),_(SM,SM),  _(X,X),  _(V,C),  _(V,C),  _(V,C),
  /* 0A08 */  _(V,C),  _(V,C),  _(V,C),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(V,C),
  /* 0A10 */  _(V,C),  _(X,X),  _(X,X),  _(V,C),  _(V,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0A18 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0A20 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0A28 */  _(C,C),  _(X,X),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0A30 */  _(R,C),  _(X,X),  _(C,C),  _(C,C),  _(X,X),  _(C,C),  _(C,C),  _(X,X),
  /* 0A38 */  _(C,C),  _(C,C),  _(X,X),  _(X,X),  _(N,X),  _(X,X), _(M,AP), _(M,LM),
  /* 0A40 */ _(M,AP), _(M,AP), _(M,AP),  _(X,X),  _(X,X),  _(X,X),  _(X,X), _(M,AP),
  /* 0A48 */ _(M,AP),  _(X,X),  _(X,X), _(M,AP), _(M,AP),  _(H,B),  _(X,X),  _(X,X),
  /* 0A50 */  _(X,X),  _(M,B),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 0A58 */  _(X,X),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(X,X),  _(C,C),  _(X,X),
  /* 0A60 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X), _(GB,C), _(GB,C),
  /* 0A68 */ _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C),
  /* 0A70 */_(SM,SM),_(SM,SM),  _(C,C),  _(C,C),  _(X,X), _(CM,C),  _(X,X),  _(X,X),
  /* 0A78 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),

  /* Gujarati */

  /* 0A80 */  _(X,X),_(SM,SM),_(SM,SM),_(SM,SM),  _(X,X),  _(V,C),  _(V,C),  _(V,C),
  /* 0A88 */  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(X,X),  _(V,C),
  /* 0A90 */  _(V,C),  _(V,C),  _(X,X),  _(V,C),  _(V,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0A98 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0AA0 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0AA8 */  _(C,C),  _(X,X),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0AB0 */  _(R,C),  _(X,X),  _(C,C),  _(C,C),  _(X,X),  _(C,C),  _(C,C),  _(C,C),
  /* 0AB8 */  _(C,C),  _(C,C),  _(X,X),  _(X,X),  _(N,X), _(S,SM), _(M,AP), _(M,LM),
  /* 0AC0 */ _(M,AP), _(M,AP), _(M,AP), _(M,AP), _(M,AP), _(M,AS),  _(X,X), _(M,AS),
  /* 0AC8 */ _(M,AS), _(M,AP),  _(X,X), _(M,AP), _(M,AP),  _(H,B),  _(X,X),  _(X,X),
  /* 0AD0 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 0AD8 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 0AE0 */  _(V,C),  _(V,C), _(M,AP), _(M,AP),  _(X,X),  _(X,X), _(GB,C), _(GB,C),
  /* 0AE8 */ _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C),
  /* 0AF0 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 0AF8 */  _(X,X),  _(C,C), _(A,SM),  _(N,X), _(A,SM),  _(N,X),  _(N,X),  _(N,X),

  /* Oriya */

  /* 0B00 */  _(X,X),_(SM,BS),_(SM,SM),_(SM,SM),  _(X,X),  _(V,C),  _(V,C),  _(V,C),
  /* 0B08 */  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(X,X),  _(X,X),  _(V,C),
  /* 0B10 */  _(V,C),  _(X,X),  _(X,X),  _(V,C),  _(V,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0B18 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0B20 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0B28 */  _(C,C),  _(X,X),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0B30 */  _(R,C),  _(X,X),  _(C,C),  _(C,C),  _(X,X),  _(C,C),  _(C,C),  _(C,C),
  /* 0B38 */  _(C,C),  _(C,C),  _(X,X),  _(X,X),  _(N,X), _(S,SM), _(M,AP),  _(M,A),
  /* 0B40 */ _(M,AP), _(M,AS), _(M,AS), _(M,AS), _(M,AS),  _(X,X),  _(X,X), _(M,LM),
  /* 0B48 */  _(M,A),  _(X,X),  _(X,X), _(M,AP), _(M,AP),  _(H,B),  _(X,X),  _(X,X),
  /* 0B50 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(N,X),  _(M,A), _(M,AP),
  /* 0B58 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(C,C),  _(C,C),  _(X,X),  _(C,C),
  /* 0B60 */  _(V,C),  _(V,C), _(M,AS), _(M,AS),  _(X,X),  _(X,X), _(GB,C), _(GB,C),
  /* 0B68 */ _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C),
  /* 0B70 */  _(X,X),  _(C,C),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 0B78 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),

  /* Tamil */

  /* 0B80 */  _(X,X),  _(X,X),_(SM,SM),  _(X,X),  _(X,X),  _(V,C),  _(V,C),  _(V,C),
  /* 0B88 */  _(V,C),  _(V,C),  _(V,C),  _(X,X),  _(X,X),  _(X,X),  _(V,C),  _(V,C),
  /* 0B90 */  _(V,C),  _(X,X),  _(V,C),  _(V,C),  _(V,C),  _(C,C),  _(X,X),  _(X,X),
  /* 0B98 */  _(X,X),  _(C,C),  _(C,C),  _(X,X),  _(C,C),  _(X,X),  _(C,C),  _(C,C),
  /* 0BA0 */  _(X,X),  _(X,X),  _(X,X),  _(C,C),  _(C,C),  _(X,X),  _(X,X),  _(X,X),
  /* 0BA8 */  _(C,C),  _(C,C),  _(C,C),  _(X,X),  _(X,X),  _(X,X),  _(C,C),  _(C,C),
  /* 0BB0 */  _(R,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0BB8 */  _(C,C),  _(C,C),  _(X,X),  _(X,X),  _(X,X),  _(X,X), _(M,AP), _(M,AP),
  /* 0BC0 */ _(M,AS), _(M,AP), _(M,AP),  _(X,X),  _(X,X),  _(X,X), _(M,LM), _(M,LM),
  /* 0BC8 */ _(M,LM),  _(X,X), _(M,AP), _(M,AP), _(M,AP),  _(H,T),  _(X,X),  _(X,X),
  /* 0BD0 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X), _(M,AP),
  /* 0BD8 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 0BE0 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X), _(GB,C), _(GB,C),
  /* 0BE8 */ _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C),
  /* 0BF0 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 0BF8 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),

  /* Telugu */

  /* 0C00 */_(SM,SM),_(SM,SM),_(SM,SM),_(SM,SM),_(SM,SM),  _(V,C),  _(V,C),  _(V,C),
  /* 0C08 */  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(X,X),  _(V,C),  _(V,C),
  /* 0C10 */  _(V,C),  _(X,X),  _(V,C),  _(V,C),  _(V,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0C18 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0C20 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0C28 */  _(C,C),  _(X,X),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0C30 */  _(R,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0C38 */  _(C,C),  _(C,C),  _(X,X),  _(X,X),  _(N,X), _(S,SM), _(M,BS), _(M,BS),
  /* 0C40 */ _(M,BS), _(M,BS), _(M,BS), _(M,AS), _(M,AS),  _(X,X), _(M,BS), _(M,BS),
  /* 0C48 */ _(M,BS),  _(X,X), _(M,BS), _(M,BS), _(M,BS),  _(H,T),  _(X,X),  _(X,X),
  /* 0C50 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X), _(M,BS), _(M,BS),  _(X,X),
  /* 0C58 */  _(C,C),  _(C,C),  _(C,C),  _(X,X),  _(X,X),  _(C,C),  _(X,X),  _(X,X),
  /* 0C60 */  _(V,C),  _(V,C), _(M,BS), _(M,BS),  _(X,X),  _(X,X), _(GB,C), _(GB,C),
  /* 0C68 */ _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C),
  /* 0C70 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 0C78 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),

  /* Kannada */

  /* 0C80 */ _(GB,C),_(SM,SM),_(SM,SM),_(SM,SM),  _(X,X),  _(V,C),  _(V,C),  _(V,C),
  /* 0C88 */  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(X,X),  _(V,C),  _(V,C),
  /* 0C90 */  _(V,C),  _(X,X),  _(V,C),  _(V,C),  _(V,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0C98 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0CA0 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0CA8 */  _(C,C),  _(X,X),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0CB0 */  _(R,C),  _(C,C),  _(C,C),  _(C,C),  _(X,X),  _(C,C),  _(C,C),  _(C,C),
  /* 0CB8 */  _(C,C),  _(C,C),  _(X,X),  _(X,X),  _(N,X), _(S,SM), _(M,BS), _(M,BS),
  /* 0CC0 */ _(M,BS), _(M,BS), _(M,BS), _(M,AS), _(M,AS),  _(X,X), _(M,BS), _(M,AS),
  /* 0CC8 */ _(M,AS),  _(X,X), _(M,AS), _(M,AS), _(M,BS),  _(H,T),  _(X,X),  _(X,X),
  /* 0CD0 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X), _(M,AS), _(M,AS),  _(X,X),
  /* 0CD8 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(C,C),  _(C,C),  _(X,X),
  /* 0CE0 */  _(V,C),  _(V,C), _(M,BS), _(M,BS),  _(X,X),  _(X,X), _(GB,C), _(GB,C),
  /* 0CE8 */ _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C),
  /* 0CF0 */  _(X,X), _(CS,C), _(CS,C),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 0CF8 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),

  /* Malayalam */

  /* 0D00 */_(SM,SM),_(SM,SM),_(SM,SM),_(SM,SM), _(GB,C),  _(V,C),  _(V,C),  _(V,C),
  /* 0D08 */  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(X,X),  _(V,C),  _(V,C),
  /* 0D10 */  _(V,C),  _(X,X),  _(V,C),  _(V,C),  _(V,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0D18 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0D20 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0D28 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0D30 */  _(R,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 0D38 */  _(C,C),  _(C,C),  _(C,C), _(M,AS), _(M,AS), _(S,SM), _(M,AP), _(M,AP),
  /* 0D40 */ _(M,AP), _(M,AP), _(M,AP), _(M,AP), _(M,AP),  _(X,X), _(M,LM), _(M,LM),
  /* 0D48 */ _(M,LM),  _(X,X), _(M,AP), _(M,AP), _(M,AP),  _(H,T), _(Rf,X),  _(X,X),
  /* 0D50 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(C,C),  _(C,C),  _(C,C), _(M,AP),
  /* 0D58 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(V,C),
  /* 0D60 */  _(V,C),  _(V,C), _(M,AP), _(M,AP),  _(X,X),  _(X,X), _(GB,C), _(GB,C),
  /* 0D68 */ _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C),
  /* 0D70 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 0D78 */  _(X,X),  _(X,X),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),

#define indic_offset_0x1000u 1216


  /* Myanmar */

  /* 1000 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(R,C),  _(C,C),  _(C,C),  _(C,C),
  /* 1008 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 1010 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 1018 */  _(C,C),  _(C,C),  _(C,C),  _(R,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 1020 */  _(C,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),
  /* 1028 */  _(V,C),  _(V,C),  _(V,C), _(VR,R), _(VR,R), _(VA,T), _(VA,T), _(VB,B),
  /* 1030 */ _(VB,B), _(VL,L), _(A,SM), _(VA,T), _(VA,T), _(VA,T), _(A,SM),  _(N,X),
  /* 1038 */_(SM,SM),  _(H,X), _(As,X), _(MY,X), _(MR,X), _(MW,X), _(MH,X),  _(C,C),
  /* 1040 */ _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C),
  /* 1048 */ _(GB,C), _(GB,C), _(GB,C), _(GB,C),  _(X,X),  _(X,X),  _(C,C),  _(X,X),
  /* 1050 */  _(C,C),  _(C,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C), _(VR,R), _(VR,R),
  /* 1058 */ _(VB,B), _(VB,B),  _(R,C),  _(C,C),  _(C,C),  _(C,C), _(MY,X), _(MY,X),
  /* 1060 */ _(ML,X),  _(C,C), _(VR,R), _(PT,X), _(PT,X),  _(C,C),  _(C,C), _(VR,R),
  /* 1068 */ _(VR,R), _(PT,X), _(PT,X), _(PT,X), _(PT,X), _(PT,X),  _(C,C),  _(C,C),
  /* 1070 */  _(C,C), _(VA,T), _(VA,T), _(VA,T), _(VA,T),  _(C,C),  _(C,C),  _(C,C),
  /* 1078 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 1080 */  _(C,C),  _(C,C), _(MW,X), _(VR,R), _(VL,L), _(VA,T), _(VA,T),_(SM,SM),
  /* 1088 */_(SM,SM),_(SM,SM),_(SM,SM),_(SM,SM),_(SM,SM),_(SM,SM),  _(C,C),_(SM,SM),
  /* 1090 */ _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C),
  /* 1098 */ _(GB,C), _(GB,C),_(SM,SM),_(SM,SM),_(SM,SM), _(VA,T),  _(X,X),  _(X,X),

#define indic_offset_0x1780u 1376


  /* Khmer */

  /* 1780 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 1788 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 1790 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 1798 */  _(C,C),  _(C,C),  _(R,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* 17A0 */  _(C,C),  _(C,C),  _(C,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),
  /* 17A8 */  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(V,C),
  /* 17B0 */  _(V,C),  _(V,C),  _(V,C),  _(V,C),  _(X,X),  _(X,X), _(VR,R), _(VA,T),
  /* 17B8 */ _(VA,T), _(VA,T), _(VA,T), _(VB,B), _(VB,B), _(VB,B), _(VA,T), _(VR,R),
  /* 17C0 */ _(VR,R), _(VL,L), _(VL,L), _(VL,L), _(VR,R), _(VR,R), _(Xg,X), _(Yg,X),
  /* 17C8 */ _(Yg,X), _(Rt,X), _(Rt,X), _(Xg,X), _(Rt,X), _(Xg,X), _(Xg,X), _(Xg,X),
  /* 17D0 */ _(Xg,X), _(Xg,X),  _(H,X), _(Yg,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 17D8 */  _(X,X), _(GB,C),  _(X,X),  _(X,X), _(S,SM), _(Yg,X),  _(X,X),  _(X,X),
  /* 17E0 */ _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C),
  /* 17E8 */ _(GB,C), _(GB,C),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),

#define indic_offset_0x1cd0u 1488


  /* Vedic Extensions */

  /* 1CD0 */ _(A,SM), _(A,SM), _(A,SM),  _(X,X), _(A,SM), _(A,SM), _(A,SM), _(A,SM),
  /* 1CD8 */ _(A,SM), _(A,SM), _(A,SM), _(A,SM), _(A,SM), _(A,SM), _(A,SM), _(A,SM),
  /* 1CE0 */ _(A,SM), _(A,SM), _(A,SM), _(A,SM), _(A,SM), _(A,SM), _(A,SM), _(A,SM),
  /* 1CE8 */ _(A,SM), _(S,SM), _(S,SM), _(S,SM), _(S,SM), _(A,SM), _(S,SM), _(S,SM),
  /* 1CF0 */ _(S,SM), _(S,SM),  _(C,C),  _(C,C), _(A,SM),  _(C,C),  _(C,C), _(A,SM),
  /* 1CF8 */ _(A,SM), _(A,SM), _(GB,C),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),

#define indic_offset_0x2008u 1536


  /* General Punctuation */

  /* 2008 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),_(ZWNJ,X),_(ZWJ,X),  _(X,X),  _(X,X),
  /* 2010 */ _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C),  _(X,X),  _(X,X),
  /* 2018 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 2020 */  _(X,X),  _(X,X), _(GB,C),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),

#define indic_offset_0x2070u 1568


  /* Superscripts and Subscripts */

  /* 2070 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),_(SM,SM),  _(X,X),  _(X,X),  _(X,X),
  /* 2078 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 2080 */  _(X,X),  _(X,X),_(SM,SM),_(SM,SM),_(SM,SM),  _(X,X),  _(X,X),  _(X,X),

#define indic_offset_0x25f8u 1592


  /* Geometric Shapes */

  /* 25F8 */  _(X,X),  _(X,X),  _(X,X), _(GB,C), _(GB,C), _(GB,C), _(GB,C),  _(X,X),

#define indic_offset_0xa8e0u 1600


  /* Devanagari Extended */

  /* A8E0 */ _(A,SM), _(A,SM), _(A,SM), _(A,SM), _(A,SM), _(A,SM), _(A,SM), _(A,SM),
  /* A8E8 */ _(A,SM), _(A,SM), _(A,SM), _(A,SM), _(A,SM), _(A,SM), _(A,SM), _(A,SM),
  /* A8F0 */ _(A,SM), _(A,SM), _(S,SM), _(S,SM), _(S,SM), _(S,SM), _(S,SM), _(S,SM),
  /* A8F8 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(V,C), _(M,AS),

#define indic_offset_0xa9e0u 1632


  /* Myanmar Extended-B */

  /* A9E0 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C), _(VA,T),  _(X,X),  _(C,C),
  /* A9E8 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* A9F0 */ _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C), _(GB,C),
  /* A9F8 */ _(GB,C), _(GB,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(X,X),

#define indic_offset_0xaa60u 1664


  /* Myanmar Extended-A */

  /* AA60 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* AA68 */  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),  _(C,C),
  /* AA70 */  _(X,X),  _(C,C),  _(C,C),  _(C,C), _(GB,C), _(GB,C), _(GB,C),  _(X,X),
  /* AA78 */  _(X,X),  _(X,X),  _(C,C), _(PT,X),  _(N,X),  _(N,X),  _(C,C),  _(C,C),

#define indic_offset_0xfe00u 1696


  /* Variation Selectors */

  /* FE00 */ _(VS,X), _(VS,X), _(VS,X), _(VS,X), _(VS,X), _(VS,X), _(VS,X), _(VS,X),
  /* FE08 */ _(VS,X), _(VS,X), _(VS,X), _(VS,X), _(VS,X), _(VS,X), _(VS,X), _(VS,X),

#define indic_offset_0x11300u 1712


  /* Grantha */

  /* 11300 */  _(X,X),_(SM,SM),_(SM,SM),_(SM,SM),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 11308 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 11310 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 11318 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 11320 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 11328 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 11330 */  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),  _(X,X),
  /* 11338 */  _(X,X),  _(X,X),  _(X,X),  _(N,X),  _(N,X),  _(X,X),  _(X,X),  _(X,X),

}; /* Table items: 1776; occupancy: 69% */

uint16_t
hb_indic_get_categories (hb_codepoint_t u)
{
  switch (u >> 12)
  {
    case 0x0u:
      if (unlikely (u == 0x00A0u)) return _(GB,C);
      if (hb_in_range<hb_codepoint_t> (u, 0x0028u, 0x003Fu)) return indic_table[u - 0x0028u + indic_offset_0x0028u];
      if (hb_in_range<hb_codepoint_t> (u, 0x00B0u, 0x00D7u)) return indic_table[u - 0x00B0u + indic_offset_0x00b0u];
      if (hb_in_range<hb_codepoint_t> (u, 0x0900u, 0x0D7Fu)) return indic_table[u - 0x0900u + indic_offset_0x0900u];
      break;

    case 0x1u:
      if (hb_in_range<hb_codepoint_t> (u, 0x1000u, 0x109Fu)) return indic_table[u - 0x1000u + indic_offset_0x1000u];
      if (hb_in_range<hb_codepoint_t> (u, 0x1780u, 0x17EFu)) return indic_table[u - 0x1780u + indic_offset_0x1780u];
      if (hb_in_range<hb_codepoint_t> (u, 0x1CD0u, 0x1CFFu)) return indic_table[u - 0x1CD0u + indic_offset_0x1cd0u];
      break;

    case 0x2u:
      if (unlikely (u == 0x25CCu)) return _(DC,C);
      if (hb_in_range<hb_codepoint_t> (u, 0x2008u, 0x2027u)) return indic_table[u - 0x2008u + indic_offset_0x2008u];
      if (hb_in_range<hb_codepoint_t> (u, 0x2070u, 0x2087u)) return indic_table[u - 0x2070u + indic_offset_0x2070u];
      if (hb_in_range<hb_codepoint_t> (u, 0x25F8u, 0x25FFu)) return indic_table[u - 0x25F8u + indic_offset_0x25f8u];
      break;

    case 0xAu:
      if (hb_in_range<hb_codepoint_t> (u, 0xA8E0u, 0xA8FFu)) return indic_table[u - 0xA8E0u + indic_offset_0xa8e0u];
      if (hb_in_range<hb_codepoint_t> (u, 0xA9E0u, 0xA9FFu)) return indic_table[u - 0xA9E0u + indic_offset_0xa9e0u];
      if (hb_in_range<hb_codepoint_t> (u, 0xAA60u, 0xAA7Fu)) return indic_table[u - 0xAA60u + indic_offset_0xaa60u];
      break;

    case 0xFu:
      if (hb_in_range<hb_codepoint_t> (u, 0xFE00u, 0xFE0Fu)) return indic_table[u - 0xFE00u + indic_offset_0xfe00u];
      break;

    case 0x11u:
      if (hb_in_range<hb_codepoint_t> (u, 0x11300u, 0x1133Fu)) return indic_table[u - 0x11300u + indic_offset_0x11300u];
      break;

    default:
      break;
  }
  return _(X,X);
}

#undef _
#undef INDIC_COMBINE_CATEGORIES

#undef _OT_A
#undef _OT_As
#undef _OT_C
#undef _OT_CM
#undef _OT_CS
#undef _OT_DC
#undef _OT_H
#undef _OT_M
#undef _OT_MH
#undef _OT_ML
#undef _OT_MR
#undef _OT_MW
#undef _OT_MY
#undef _OT_N
#undef _OT_GB
#undef _OT_PT
#undef _OT_R
#undef _OT_Rf
#undef _OT_Rt
#undef _OT_SM
#undef _OT_S
#undef _OT_V
#undef _OT_VA
#undef _OT_VB
#undef _OT_VL
#undef _OT_VR
#undef _OT_VS
#undef _OT_X
#undef _OT_Xg
#undef _OT_Yg
#undef _OT_ZWJ
#undef _OT_ZWNJ

#undef _POS_T
#undef _POS_A
#undef _POS_AP
#undef _POS_AS
#undef _POS_C
#undef _POS_BS
#undef _POS_B
#undef _POS_X
#undef _POS_R
#undef _POS_L
#undef _POS_LM
#undef _POS_SM

#endif

/* == End of generated table == */
