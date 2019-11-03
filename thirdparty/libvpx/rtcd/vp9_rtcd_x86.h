#ifndef VP9_RTCD_H_
#define VP9_RTCD_H_

#ifdef RTCD_C
#define RTCD_EXTERN
#else
#define RTCD_EXTERN extern
#endif

/*
 * VP9
 */

#include "vp9/common/vp9_common.h"

#ifdef __cplusplus
extern "C" {
#endif

void vp9_iht16x16_256_add_c(const tran_low_t *input, uint8_t *output, int pitch, int tx_type);
void vp9_iht16x16_256_add_sse2(const tran_low_t *input, uint8_t *output, int pitch, int tx_type);
RTCD_EXTERN void (*vp9_iht16x16_256_add)(const tran_low_t *input, uint8_t *output, int pitch, int tx_type);

void vp9_iht4x4_16_add_c(const tran_low_t *input, uint8_t *dest, int dest_stride, int tx_type);
void vp9_iht4x4_16_add_sse2(const tran_low_t *input, uint8_t *dest, int dest_stride, int tx_type);
RTCD_EXTERN void (*vp9_iht4x4_16_add)(const tran_low_t *input, uint8_t *dest, int dest_stride, int tx_type);

void vp9_iht8x8_64_add_c(const tran_low_t *input, uint8_t *dest, int dest_stride, int tx_type);
void vp9_iht8x8_64_add_sse2(const tran_low_t *input, uint8_t *dest, int dest_stride, int tx_type);
RTCD_EXTERN void (*vp9_iht8x8_64_add)(const tran_low_t *input, uint8_t *dest, int dest_stride, int tx_type);

void vp9_rtcd(void);

#ifdef RTCD_C
#include "vpx_ports/x86.h"
static void setup_rtcd_internal(void)
{
    int flags = x86_simd_caps();

    vp9_iht16x16_256_add = vp9_iht16x16_256_add_c;
    if (flags & HAS_SSE2) vp9_iht16x16_256_add = vp9_iht16x16_256_add_sse2;

    vp9_iht4x4_16_add = vp9_iht4x4_16_add_c;
    if (flags & HAS_SSE2) vp9_iht4x4_16_add = vp9_iht4x4_16_add_sse2;

    vp9_iht8x8_64_add = vp9_iht8x8_64_add_c;
    if (flags & HAS_SSE2) vp9_iht8x8_64_add = vp9_iht8x8_64_add_sse2;
}
#endif

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
