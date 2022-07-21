#ifndef OT_LAYOUT_GSUB_GSUB_HH
#define OT_LAYOUT_GSUB_GSUB_HH

// TODO(garretrieger): move to new layout.
#include "../../../hb-ot-layout-gsubgpos.hh"
#include "Common.hh"
#include "SubstLookup.hh"

using OT::Layout::GSUB::SubstLookup;

namespace OT {
namespace Layout {
namespace GSUB {

/*
 * GSUB -- Glyph Substitution
 * https://docs.microsoft.com/en-us/typography/opentype/spec/gsub
 */

struct GSUB : GSUBGPOS
{
  using Lookup = SubstLookup;

  static constexpr hb_tag_t tableTag = HB_OT_TAG_GSUB;

  const SubstLookup& get_lookup (unsigned int i) const
  { return static_cast<const SubstLookup &> (GSUBGPOS::get_lookup (i)); }

  bool subset (hb_subset_context_t *c) const
  {
    hb_subset_layout_context_t l (c, tableTag, c->plan->gsub_lookups, c->plan->gsub_langsys, c->plan->gsub_features);
    return GSUBGPOS::subset<SubstLookup> (&l);
  }

  bool sanitize (hb_sanitize_context_t *c) const
  { return GSUBGPOS::sanitize<SubstLookup> (c); }

  HB_INTERNAL bool is_blocklisted (hb_blob_t *blob,
                                   hb_face_t *face) const;

  void closure_lookups (hb_face_t      *face,
                        const hb_set_t *glyphs,
                        hb_set_t       *lookup_indexes /* IN/OUT */) const
  { GSUBGPOS::closure_lookups<SubstLookup> (face, glyphs, lookup_indexes); }

  typedef GSUBGPOS::accelerator_t<GSUB> accelerator_t;
};


}
}

struct GSUB_accelerator_t : Layout::GSUB::GSUB::accelerator_t {
  GSUB_accelerator_t (hb_face_t *face) : Layout::GSUB::GSUB::accelerator_t (face) {}
};


}

#endif  /* OT_LAYOUT_GSUB_GSUB_HH */
