#pragma once
#include "oxygine-include.h"

namespace oxygine {
class RenderDelegate {
  public:
   virtual ~RenderDelegate();

   virtual void render(Actor*, const RenderState& rs, RenderState& nrs);
   virtual void render(ClipRectActor*, const RenderState& rs, RenderState& nrs) {}
   virtual void render(MaskedSprite*, const RenderState& rs, RenderState& nrs) {}

   virtual void doRender(Sprite*, const RenderState& rs) {}
   virtual void doRender(TextField*, const RenderState& rs) {}
   virtual void doRender(ColorRectSprite*, const RenderState& rs) {}
   virtual void doRender(ProgressBar*, const RenderState& rs) {}

   static void setCurrent(void*) {}
};
}  // namespace oxygine