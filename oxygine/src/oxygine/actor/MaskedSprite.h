#pragma once
#include "../oxygine-include.h"
#include "Sprite.h"

namespace oxygine {
DECLARE_SMART(MaskedSprite, spMaskedSprite);
class MaskedSprite : public Sprite {
   INHERITED(Sprite);

  public:
   DECLARE_COPYCLONE_NEW(MaskedSprite);

   MaskedSprite();
   ~MaskedSprite();

   spSprite getMask() const;
   bool getUseRChannel() const;

   void setMask(spSprite, bool useRChannel = false);

   void serialize(serializedata* data) override;
   void deserialize(const deserializedata* data) override;
   void deserializeLink(const deserializeLinkData*) override;

  protected:
   void render(const RenderState& parentRS, RenderState& rs) override;

  private:
   spSprite _mask;
   bool _useRChannel;
};
}  // namespace oxygine

EDITOR_INCLUDE(MaskedSprite);