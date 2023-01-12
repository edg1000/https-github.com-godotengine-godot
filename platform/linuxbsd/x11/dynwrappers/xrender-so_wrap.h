#ifndef DYLIBLOAD_WRAPPER_XRENDER
#define DYLIBLOAD_WRAPPER_XRENDER
// This file is generated. Do not edit!
// see https://github.com/hpvb/dynload-wrapper for details
// generated by ./generate-wrapper.py 0.3 on 2022-12-02 12:55:28
// flags: ./generate-wrapper.py --include /usr/include/X11/extensions/Xrender.h --sys-include <X11/extensions/Xrender.h> --soname libXrender.so.1 --init-name xrender --output-header xrender-so_wrap.h --output-implementation xrender-so_wrap.c
//
// NOTE: Generated from Xrender 0.9.10.
// This has been handpatched to workaround some issues with the generator that
// will be eventually fixed. In this case, non-existent symbols inherited from
// libX11, but absent in libXrender.so.1, were removed.
#include <stdint.h>

#define XRenderQueryExtension XRenderQueryExtension_dylibloader_orig_xrender
#define XRenderQueryVersion XRenderQueryVersion_dylibloader_orig_xrender
#define XRenderQueryFormats XRenderQueryFormats_dylibloader_orig_xrender
#define XRenderQuerySubpixelOrder XRenderQuerySubpixelOrder_dylibloader_orig_xrender
#define XRenderSetSubpixelOrder XRenderSetSubpixelOrder_dylibloader_orig_xrender
#define XRenderFindVisualFormat XRenderFindVisualFormat_dylibloader_orig_xrender
#define XRenderFindFormat XRenderFindFormat_dylibloader_orig_xrender
#define XRenderFindStandardFormat XRenderFindStandardFormat_dylibloader_orig_xrender
#define XRenderQueryPictIndexValues XRenderQueryPictIndexValues_dylibloader_orig_xrender
#define XRenderCreatePicture XRenderCreatePicture_dylibloader_orig_xrender
#define XRenderChangePicture XRenderChangePicture_dylibloader_orig_xrender
#define XRenderSetPictureClipRectangles XRenderSetPictureClipRectangles_dylibloader_orig_xrender
#define XRenderSetPictureClipRegion XRenderSetPictureClipRegion_dylibloader_orig_xrender
#define XRenderSetPictureTransform XRenderSetPictureTransform_dylibloader_orig_xrender
#define XRenderFreePicture XRenderFreePicture_dylibloader_orig_xrender
#define XRenderComposite XRenderComposite_dylibloader_orig_xrender
#define XRenderCreateGlyphSet XRenderCreateGlyphSet_dylibloader_orig_xrender
#define XRenderReferenceGlyphSet XRenderReferenceGlyphSet_dylibloader_orig_xrender
#define XRenderFreeGlyphSet XRenderFreeGlyphSet_dylibloader_orig_xrender
#define XRenderAddGlyphs XRenderAddGlyphs_dylibloader_orig_xrender
#define XRenderFreeGlyphs XRenderFreeGlyphs_dylibloader_orig_xrender
#define XRenderCompositeString8 XRenderCompositeString8_dylibloader_orig_xrender
#define XRenderCompositeString16 XRenderCompositeString16_dylibloader_orig_xrender
#define XRenderCompositeString32 XRenderCompositeString32_dylibloader_orig_xrender
#define XRenderCompositeText8 XRenderCompositeText8_dylibloader_orig_xrender
#define XRenderCompositeText16 XRenderCompositeText16_dylibloader_orig_xrender
#define XRenderCompositeText32 XRenderCompositeText32_dylibloader_orig_xrender
#define XRenderFillRectangle XRenderFillRectangle_dylibloader_orig_xrender
#define XRenderFillRectangles XRenderFillRectangles_dylibloader_orig_xrender
#define XRenderCompositeTrapezoids XRenderCompositeTrapezoids_dylibloader_orig_xrender
#define XRenderCompositeTriangles XRenderCompositeTriangles_dylibloader_orig_xrender
#define XRenderCompositeTriStrip XRenderCompositeTriStrip_dylibloader_orig_xrender
#define XRenderCompositeTriFan XRenderCompositeTriFan_dylibloader_orig_xrender
#define XRenderCompositeDoublePoly XRenderCompositeDoublePoly_dylibloader_orig_xrender
#define XRenderParseColor XRenderParseColor_dylibloader_orig_xrender
#define XRenderCreateCursor XRenderCreateCursor_dylibloader_orig_xrender
#define XRenderQueryFilters XRenderQueryFilters_dylibloader_orig_xrender
#define XRenderSetPictureFilter XRenderSetPictureFilter_dylibloader_orig_xrender
#define XRenderCreateAnimCursor XRenderCreateAnimCursor_dylibloader_orig_xrender
#define XRenderAddTraps XRenderAddTraps_dylibloader_orig_xrender
#define XRenderCreateSolidFill XRenderCreateSolidFill_dylibloader_orig_xrender
#define XRenderCreateLinearGradient XRenderCreateLinearGradient_dylibloader_orig_xrender
#define XRenderCreateRadialGradient XRenderCreateRadialGradient_dylibloader_orig_xrender
#define XRenderCreateConicalGradient XRenderCreateConicalGradient_dylibloader_orig_xrender
#include <X11/extensions/Xrender.h>
#undef XRenderQueryExtension
#undef XRenderQueryVersion
#undef XRenderQueryFormats
#undef XRenderQuerySubpixelOrder
#undef XRenderSetSubpixelOrder
#undef XRenderFindVisualFormat
#undef XRenderFindFormat
#undef XRenderFindStandardFormat
#undef XRenderQueryPictIndexValues
#undef XRenderCreatePicture
#undef XRenderChangePicture
#undef XRenderSetPictureClipRectangles
#undef XRenderSetPictureClipRegion
#undef XRenderSetPictureTransform
#undef XRenderFreePicture
#undef XRenderComposite
#undef XRenderCreateGlyphSet
#undef XRenderReferenceGlyphSet
#undef XRenderFreeGlyphSet
#undef XRenderAddGlyphs
#undef XRenderFreeGlyphs
#undef XRenderCompositeString8
#undef XRenderCompositeString16
#undef XRenderCompositeString32
#undef XRenderCompositeText8
#undef XRenderCompositeText16
#undef XRenderCompositeText32
#undef XRenderFillRectangle
#undef XRenderFillRectangles
#undef XRenderCompositeTrapezoids
#undef XRenderCompositeTriangles
#undef XRenderCompositeTriStrip
#undef XRenderCompositeTriFan
#undef XRenderCompositeDoublePoly
#undef XRenderParseColor
#undef XRenderCreateCursor
#undef XRenderQueryFilters
#undef XRenderSetPictureFilter
#undef XRenderCreateAnimCursor
#undef XRenderAddTraps
#undef XRenderCreateSolidFill
#undef XRenderCreateLinearGradient
#undef XRenderCreateRadialGradient
#undef XRenderCreateConicalGradient
#ifdef __cplusplus
extern "C" {
#endif
#define XRenderQueryExtension XRenderQueryExtension_dylibloader_wrapper_xrender
#define XRenderQueryVersion XRenderQueryVersion_dylibloader_wrapper_xrender
#define XRenderQueryFormats XRenderQueryFormats_dylibloader_wrapper_xrender
#define XRenderQuerySubpixelOrder XRenderQuerySubpixelOrder_dylibloader_wrapper_xrender
#define XRenderSetSubpixelOrder XRenderSetSubpixelOrder_dylibloader_wrapper_xrender
#define XRenderFindVisualFormat XRenderFindVisualFormat_dylibloader_wrapper_xrender
#define XRenderFindFormat XRenderFindFormat_dylibloader_wrapper_xrender
#define XRenderFindStandardFormat XRenderFindStandardFormat_dylibloader_wrapper_xrender
#define XRenderQueryPictIndexValues XRenderQueryPictIndexValues_dylibloader_wrapper_xrender
#define XRenderCreatePicture XRenderCreatePicture_dylibloader_wrapper_xrender
#define XRenderChangePicture XRenderChangePicture_dylibloader_wrapper_xrender
#define XRenderSetPictureClipRectangles XRenderSetPictureClipRectangles_dylibloader_wrapper_xrender
#define XRenderSetPictureClipRegion XRenderSetPictureClipRegion_dylibloader_wrapper_xrender
#define XRenderSetPictureTransform XRenderSetPictureTransform_dylibloader_wrapper_xrender
#define XRenderFreePicture XRenderFreePicture_dylibloader_wrapper_xrender
#define XRenderComposite XRenderComposite_dylibloader_wrapper_xrender
#define XRenderCreateGlyphSet XRenderCreateGlyphSet_dylibloader_wrapper_xrender
#define XRenderReferenceGlyphSet XRenderReferenceGlyphSet_dylibloader_wrapper_xrender
#define XRenderFreeGlyphSet XRenderFreeGlyphSet_dylibloader_wrapper_xrender
#define XRenderAddGlyphs XRenderAddGlyphs_dylibloader_wrapper_xrender
#define XRenderFreeGlyphs XRenderFreeGlyphs_dylibloader_wrapper_xrender
#define XRenderCompositeString8 XRenderCompositeString8_dylibloader_wrapper_xrender
#define XRenderCompositeString16 XRenderCompositeString16_dylibloader_wrapper_xrender
#define XRenderCompositeString32 XRenderCompositeString32_dylibloader_wrapper_xrender
#define XRenderCompositeText8 XRenderCompositeText8_dylibloader_wrapper_xrender
#define XRenderCompositeText16 XRenderCompositeText16_dylibloader_wrapper_xrender
#define XRenderCompositeText32 XRenderCompositeText32_dylibloader_wrapper_xrender
#define XRenderFillRectangle XRenderFillRectangle_dylibloader_wrapper_xrender
#define XRenderFillRectangles XRenderFillRectangles_dylibloader_wrapper_xrender
#define XRenderCompositeTrapezoids XRenderCompositeTrapezoids_dylibloader_wrapper_xrender
#define XRenderCompositeTriangles XRenderCompositeTriangles_dylibloader_wrapper_xrender
#define XRenderCompositeTriStrip XRenderCompositeTriStrip_dylibloader_wrapper_xrender
#define XRenderCompositeTriFan XRenderCompositeTriFan_dylibloader_wrapper_xrender
#define XRenderCompositeDoublePoly XRenderCompositeDoublePoly_dylibloader_wrapper_xrender
#define XRenderParseColor XRenderParseColor_dylibloader_wrapper_xrender
#define XRenderCreateCursor XRenderCreateCursor_dylibloader_wrapper_xrender
#define XRenderQueryFilters XRenderQueryFilters_dylibloader_wrapper_xrender
#define XRenderSetPictureFilter XRenderSetPictureFilter_dylibloader_wrapper_xrender
#define XRenderCreateAnimCursor XRenderCreateAnimCursor_dylibloader_wrapper_xrender
#define XRenderAddTraps XRenderAddTraps_dylibloader_wrapper_xrender
#define XRenderCreateSolidFill XRenderCreateSolidFill_dylibloader_wrapper_xrender
#define XRenderCreateLinearGradient XRenderCreateLinearGradient_dylibloader_wrapper_xrender
#define XRenderCreateRadialGradient XRenderCreateRadialGradient_dylibloader_wrapper_xrender
#define XRenderCreateConicalGradient XRenderCreateConicalGradient_dylibloader_wrapper_xrender
extern int (*XRenderQueryExtension_dylibloader_wrapper_xrender)( Display*, int*, int*);
extern int (*XRenderQueryVersion_dylibloader_wrapper_xrender)( Display*, int*, int*);
extern int (*XRenderQueryFormats_dylibloader_wrapper_xrender)( Display*);
extern int (*XRenderQuerySubpixelOrder_dylibloader_wrapper_xrender)( Display*, int);
extern int (*XRenderSetSubpixelOrder_dylibloader_wrapper_xrender)( Display*, int, int);
extern XRenderPictFormat* (*XRenderFindVisualFormat_dylibloader_wrapper_xrender)( Display*,const Visual*);
extern XRenderPictFormat* (*XRenderFindFormat_dylibloader_wrapper_xrender)( Display*, unsigned long,const XRenderPictFormat*, int);
extern XRenderPictFormat* (*XRenderFindStandardFormat_dylibloader_wrapper_xrender)( Display*, int);
extern XIndexValue* (*XRenderQueryPictIndexValues_dylibloader_wrapper_xrender)( Display*,const XRenderPictFormat*, int*);
extern Picture (*XRenderCreatePicture_dylibloader_wrapper_xrender)( Display*, Drawable,const XRenderPictFormat*, unsigned long,const XRenderPictureAttributes*);
extern void (*XRenderChangePicture_dylibloader_wrapper_xrender)( Display*, Picture, unsigned long,const XRenderPictureAttributes*);
extern void (*XRenderSetPictureClipRectangles_dylibloader_wrapper_xrender)( Display*, Picture, int, int,const XRectangle*, int);
extern void (*XRenderSetPictureClipRegion_dylibloader_wrapper_xrender)( Display*, Picture, Region);
extern void (*XRenderSetPictureTransform_dylibloader_wrapper_xrender)( Display*, Picture, XTransform*);
extern void (*XRenderFreePicture_dylibloader_wrapper_xrender)( Display*, Picture);
extern void (*XRenderComposite_dylibloader_wrapper_xrender)( Display*, int, Picture, Picture, Picture, int, int, int, int, int, int, unsigned int, unsigned int);
extern GlyphSet (*XRenderCreateGlyphSet_dylibloader_wrapper_xrender)( Display*,const XRenderPictFormat*);
extern GlyphSet (*XRenderReferenceGlyphSet_dylibloader_wrapper_xrender)( Display*, GlyphSet);
extern void (*XRenderFreeGlyphSet_dylibloader_wrapper_xrender)( Display*, GlyphSet);
extern void (*XRenderAddGlyphs_dylibloader_wrapper_xrender)( Display*, GlyphSet,const Glyph*,const XGlyphInfo*, int,const char*, int);
extern void (*XRenderFreeGlyphs_dylibloader_wrapper_xrender)( Display*, GlyphSet,const Glyph*, int);
extern void (*XRenderCompositeString8_dylibloader_wrapper_xrender)( Display*, int, Picture, Picture,const XRenderPictFormat*, GlyphSet, int, int, int, int,const char*, int);
extern void (*XRenderCompositeString16_dylibloader_wrapper_xrender)( Display*, int, Picture, Picture,const XRenderPictFormat*, GlyphSet, int, int, int, int,const unsigned short*, int);
extern void (*XRenderCompositeString32_dylibloader_wrapper_xrender)( Display*, int, Picture, Picture,const XRenderPictFormat*, GlyphSet, int, int, int, int,const unsigned int*, int);
extern void (*XRenderCompositeText8_dylibloader_wrapper_xrender)( Display*, int, Picture, Picture,const XRenderPictFormat*, int, int, int, int,const XGlyphElt8*, int);
extern void (*XRenderCompositeText16_dylibloader_wrapper_xrender)( Display*, int, Picture, Picture,const XRenderPictFormat*, int, int, int, int,const XGlyphElt16*, int);
extern void (*XRenderCompositeText32_dylibloader_wrapper_xrender)( Display*, int, Picture, Picture,const XRenderPictFormat*, int, int, int, int,const XGlyphElt32*, int);
extern void (*XRenderFillRectangle_dylibloader_wrapper_xrender)( Display*, int, Picture,const XRenderColor*, int, int, unsigned int, unsigned int);
extern void (*XRenderFillRectangles_dylibloader_wrapper_xrender)( Display*, int, Picture,const XRenderColor*,const XRectangle*, int);
extern void (*XRenderCompositeTrapezoids_dylibloader_wrapper_xrender)( Display*, int, Picture, Picture,const XRenderPictFormat*, int, int,const XTrapezoid*, int);
extern void (*XRenderCompositeTriangles_dylibloader_wrapper_xrender)( Display*, int, Picture, Picture,const XRenderPictFormat*, int, int,const XTriangle*, int);
extern void (*XRenderCompositeTriStrip_dylibloader_wrapper_xrender)( Display*, int, Picture, Picture,const XRenderPictFormat*, int, int,const XPointFixed*, int);
extern void (*XRenderCompositeTriFan_dylibloader_wrapper_xrender)( Display*, int, Picture, Picture,const XRenderPictFormat*, int, int,const XPointFixed*, int);
extern void (*XRenderCompositeDoublePoly_dylibloader_wrapper_xrender)( Display*, int, Picture, Picture,const XRenderPictFormat*, int, int, int, int,const XPointDouble*, int, int);
extern int (*XRenderParseColor_dylibloader_wrapper_xrender)( Display*, char*, XRenderColor*);
extern Cursor (*XRenderCreateCursor_dylibloader_wrapper_xrender)( Display*, Picture, unsigned int, unsigned int);
extern XFilters* (*XRenderQueryFilters_dylibloader_wrapper_xrender)( Display*, Drawable);
extern void (*XRenderSetPictureFilter_dylibloader_wrapper_xrender)( Display*, Picture,const char*, XFixed*, int);
extern Cursor (*XRenderCreateAnimCursor_dylibloader_wrapper_xrender)( Display*, int, XAnimCursor*);
extern void (*XRenderAddTraps_dylibloader_wrapper_xrender)( Display*, Picture, int, int,const XTrap*, int);
extern Picture (*XRenderCreateSolidFill_dylibloader_wrapper_xrender)( Display*,const XRenderColor*);
extern Picture (*XRenderCreateLinearGradient_dylibloader_wrapper_xrender)( Display*,const XLinearGradient*,const XFixed*,const XRenderColor*, int);
extern Picture (*XRenderCreateRadialGradient_dylibloader_wrapper_xrender)( Display*,const XRadialGradient*,const XFixed*,const XRenderColor*, int);
extern Picture (*XRenderCreateConicalGradient_dylibloader_wrapper_xrender)( Display*,const XConicalGradient*,const XFixed*,const XRenderColor*, int);
int initialize_xrender(int verbose);
#ifdef __cplusplus
}
#endif
#endif
