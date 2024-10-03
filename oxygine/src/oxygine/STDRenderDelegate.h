#pragma once
#include "RenderDelegate.h"
#include "STDRenderer.h"
#include "oxygine-include.h"

namespace oxygine {
DECLARE_SMART(STDRenderDelegate, spSTDRenderDelegate);
class STDRenderDelegate : public RenderDelegate {
  public:
   static STDRenderDelegate* instance;

   STDRenderDelegate() {}

   void render(ClipRectActor*, const RenderState& rs, RenderState& nrs) override;
   void render(MaskedSprite*, const RenderState& rs, RenderState& nrs) override;
   void doRender(Sprite*, const RenderState& rs) override;
   void doRender(TextField*, const RenderState& rs) override;
   void doRender(ColorRectSprite*, const RenderState& rs) override;
   void doRender(ProgressBar*, const RenderState& rs) override;

  protected:
};

}  // namespace oxygine