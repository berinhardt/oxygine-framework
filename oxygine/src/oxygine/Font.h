#pragma once
#include "oxygine-include.h"
#include "core/Object.h"
#include "math/Rect.h"
#include <unordered_set>

namespace oxygine {
struct glyph
{
   RectF src;

   int          ch;
   glyphOptions opt;

   short sw;
   short sh;

   short offset_x;
   short offset_y;

   short advance_x;
   short advance_y;

   spNativeTexture texture;

   bool operator==(const glyph& r) const {
      return ch == r.ch && opt == r.opt;
   }
};

struct GlyphHasher
{
   std::size_t operator()(const glyph& k) const {
      return std::hash<size_t>()(k.ch + k.opt);
   }
};

struct Symbol;
class Font : public ObjectBase {
public:

   Font();
   ~Font();

   void    init(const char* name, int size, int baselineDistance, int lineHeight, bool sdf = false);

   void    addGlyph(const glyph& g);
   void    sortGlyphs() {}

   uint8_t getHash() const {
      return _hash;
   }

   void rehash() {
      _hash++;
   }

   void clear();

   void setScale(float scale) {
      _scale = scale;
   }

   void setBaselineDistance(int d) {
      _baselineDistance = d;
   }

   const glyph* getGlyph(int code, const glyphOptions& opt) const;
   int          getBaselineDistance() const;
   int          getSize() const;
   float        getScale() const;

   virtual void BiDiPass(std::vector<text::Symbol*>&) const {}

protected:

   const glyph* findGlyph(int code, const glyphOptions& opt) const;

   virtual bool loadGlyph(int code, glyph&, const glyphOptions& opt) {
      return false;
   }

   typedef std::unordered_set<glyph, GlyphHasher> glyphs;

   glyphs _glyphs;
   bool _ignoreOptions;

   float _scale;
   bool _sdf;

   int _size;
   int _baselineDistance;
   uint8_t _hash;
};
}
