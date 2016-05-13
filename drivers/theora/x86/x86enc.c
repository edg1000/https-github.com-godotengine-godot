/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggTheora SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE Theora SOURCE CODE IS COPYRIGHT (C) 2002-2009                *
 * by the Xiph.Org Foundation and contributors http://www.xiph.org/ *
 *                                                                  *
 ********************************************************************

  function:
    last mod: $Id: x86state.c 15675 2009-02-06 09:43:27Z tterribe $

 ********************************************************************/
#include "x86enc.h"

#if defined(OC_X86_ASM)

#include "../cpu.c"

void oc_enc_vtable_init_x86(oc_enc_ctx *_enc){
  ogg_uint32_t cpu_flags;
  cpu_flags=oc_cpu_flags_get();
  oc_enc_vtable_init_c(_enc);
  if(cpu_flags&OC_CPU_X86_MMX){
    _enc->opt_vtable.frag_sub=oc_enc_frag_sub_mmx;
    _enc->opt_vtable.frag_sub_128=oc_enc_frag_sub_128_mmx;
    _enc->opt_vtable.frag_recon_intra=oc_frag_recon_intra_mmx;
    _enc->opt_vtable.frag_recon_inter=oc_frag_recon_inter_mmx;
    _enc->opt_vtable.fdct8x8=oc_enc_fdct8x8_mmx;
  }
  if(cpu_flags&OC_CPU_X86_MMXEXT){
    _enc->opt_vtable.frag_sad=oc_enc_frag_sad_mmxext;
    _enc->opt_vtable.frag_sad_thresh=oc_enc_frag_sad_thresh_mmxext;
    _enc->opt_vtable.frag_sad2_thresh=oc_enc_frag_sad2_thresh_mmxext;
    _enc->opt_vtable.frag_satd_thresh=oc_enc_frag_satd_thresh_mmxext;
    _enc->opt_vtable.frag_satd2_thresh=oc_enc_frag_satd2_thresh_mmxext;
    _enc->opt_vtable.frag_intra_satd=oc_enc_frag_intra_satd_mmxext;
    _enc->opt_vtable.frag_copy2=oc_enc_frag_copy2_mmxext;
  }
  if(cpu_flags&OC_CPU_X86_SSE2){
# if defined(OC_X86_64_ASM)
    /*_enc->opt_vtable.fdct8x8=oc_enc_fdct8x8_x86_64sse2;*/
# endif
  }
}
#endif
