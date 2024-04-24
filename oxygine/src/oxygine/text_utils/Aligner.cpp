// clang-formater off
#include "Aligner.h"

#include <assert.h>

#include <sstream>

#include "../Font.h"
#include "../res/ResFont.h"

namespace oxygine {
extern uint32_t decodeSymbol(int sym);
namespace text {
#define GSCALE 1
// #define ALIGNER_LOG
#ifdef ALIGNER_LOG
#define DBG_LOG(...) logs::messageln(__VA_ARGS__)
#else
#define DBG_LOG(...)
#endif
Aligner::Aligner(const TextStyle& Style, spSTDMaterial mt, const Font* font, float gscale, const Vector2& size) : width((int)size.x),
                                                                                                                  height((int)size.y),
                                                                                                                  _x(0),
                                                                                                                  _y(0),
                                                                                                                  _lineWidth(0),
                                                                                                                  bounds(0, 0, 0, 0),
                                                                                                                  style(Style),
                                                                                                                  _scale(gscale),
                                                                                                                  _font(font),
                                                                                                                  mat(mt) {
   trimTopLine = style.trimLineHeight;
   _line.reserve(50);
   _lineSkip = (int)(_font->getBaselineDistance() * style.baselineScale) + style.linesOffset;
   DBG_LOG("_lineSkip [%d] = %d x %f + %d", _lineSkip, _font->getBaselineDistance(), style.baselineScale, style.linesOffset);
   _padding = _font->getPadding();
   _offY = _lineSkip;
   options = Style.options;

   DBG_LOG("ALIGN BOUNDS [%d, %d]x[%d, %d]", bounds.pos.x, bounds.pos.y, bounds.size.x, bounds.size.y);
}

Aligner::~Aligner() {}

int Aligner::offsetY() const {
   if (!trimTopLine)
      return 0;
   else
      return _offY + _padding;
}

int Aligner::_alignX(int rx) {
   int tx = 0;

   switch (getStyle().hAlign) {
      case TextStyle::HALIGN_LEFT:
      case TextStyle::HALIGN_DEFAULT:
         tx = 0;
         break;
      case TextStyle::HALIGN_MIDDLE:
         tx = width / 2 - rx / 2;
         break;
      case TextStyle::HALIGN_RIGHT:
         tx = width - rx;
         break;
   }
   return tx;
}

int Aligner::_alignY(int ry) {
   int ty = 0;

   switch (getStyle().vAlign) {
      case TextStyle::VALIGN_BASELINE:
         ty = -getLineSkip();
         break;
      case TextStyle::VALIGN_TOP:
      case TextStyle::VALIGN_DEFAULT:
         ty = 0;
         break;
      case TextStyle::VALIGN_MIDDLE:
         ty = height / 2 - ry / 2;
         break;
      case TextStyle::VALIGN_BOTTOM:
         ty = height - ry;
         break;
   }
   return ty;
}

void Aligner::begin() {
   _x = 0;
   _y = 0;
   DBG_LOG("Aligner::begin(_y = %d)", _y);
   width = int(width * _scale);
   height = int(height * _scale);

   bounds = Rect(_alignX(0), _alignY(0), 0, 0);
   DBG_LOG("ALIGN BOUNDS [%d, %d]x[%d, %d]", bounds.pos.x, bounds.pos.y, bounds.size.x, bounds.size.y);
   nextLine();
}

void Aligner::end() {
   int ry = _y;
   if (getStyle().multiline) {
      nextLine();
      _y -= getLineSkip();
      DBG_LOG("Aligner::end MULTILINE(_y = %d)", _y);
   } else {
      _alignLine(_line);
   }

   DBG_LOG("Aligner::end %s I(%d - %d)", trimTopLine ? "TRIM" : "NO TRIM", ry, offsetY());
   ry -= offsetY();
   DBG_LOG("Aligner::end %s F(%d - %d)", trimTopLine ? "TRIM" : "NO TRIM", ry, offsetY());

   bounds.setY(_alignY(ry));
   bounds.setHeight(ry);
}

int Aligner::getLineWidth() const {
   return _lineWidth;
}

int Aligner::getLineSkip() const {
   return _lineSkip;
}

void Aligner::_alignLine(line& ln) {
   if (!ln.empty()) {
      int ws_off = ((options >> 12) & 0xF) +
                   ((options >> 8) & 0xF) * 2.0f;

      if (_font->BiDiPass(ln)) {
         int ox = 0;
         int oy = ln[0]->y - ln[0]->gl.offset_y;

         for (size_t i = 0; i < ln.size(); ++i) {
            Symbol* s = ln[i];

            if (s->code != s->gl.ch) {
               const glyph* gl = _font->getGlyph(s->code, options);

               if (gl) s->gl = *gl;
               s->y = oy + s->gl.offset_y;
            }
            s->x = ox + s->gl.offset_x + ws_off / 2;
            ox += s->gl.advance_x + ws_off / 2;
         }
      }

      // calculate real text width
      int rx = 0;
      int ox = 0;

      for (size_t i = 0; i < ln.size(); ++i) {
         Symbol& s = *ln[i];
         ox = std::min(ox, (int)s.x);
         rx = std::max(s.x + s.gl.advance_x + ws_off, rx);
         _offY = std::min((int)s.y, _offY);
      }
      rx -= ox;
      int tx = _alignX(rx);

      for (size_t i = 0; i < ln.size(); ++i) {
         Symbol& s = *ln[i];
         s.x += tx;
      }

      _lineWidth = rx;

      bounds.setX(std::min(tx, bounds.getX()));
      bounds.setWidth(std::max(_lineWidth, bounds.getWidth()));
      DBG_LOG("BOUNDS WIDTH %d :: %d", _lineWidth, bounds.getWidth());
   }
}

void Aligner::_nextLine(line& ln) {
   _y += getLineSkip();
   DBG_LOG("Aligner::_nextLine 1(_y = %d)", _y);
   _alignLine(ln);
   DBG_LOG("Aligner::_nextLine 2(_y = %d)", _y);

   _lineWidth = 0;

   _x = 0;
}

void Aligner::nextLine() {
   // assert(multiline == true); commented, becase even if multiline is false - there are breakLine markers, they could be used anyway

   _nextLine(_line);
   _line.clear();
}

float Aligner::getScale() const {
   return _scale;
}

int Aligner::putSymbol(Symbol& s) {
   if (_line.empty() && (s.code == ' ')) return 0;

   _line.push_back(&s);

   // optional.. remove?
   // if ((_line.size() == 1) && (s.gl.offset_x < 0)) _x -= s.gl.offset_x;

   int ws_off = ((options >> 12) & 0xF) +
                ((options >> 8) & 0xF) * 2.0f;
#ifdef ALIGNER_LOG
   std::string code;
   charCode2Bytes(code, s.gl.ch);
#endif
   DBG_LOG("SYMBOL %s PRE _xy [%d, %d] OFFSET [%d,%d]", code.c_str(), _x, _y, s.gl.offset_x, s.gl.offset_y);
   s.x = _x + s.gl.offset_x + ws_off / 2;
   s.y = _y + s.gl.offset_y - ws_off / 2;
   DBG_LOG("SYMBOL %s [%d, %d] x [%d] OFF[%d]", code.c_str(), s.x, s.y, s.gl.advance_x, ws_off);
   _x += s.gl.advance_x + getStyle().kerning + ws_off / 2;

   int rx = s.x + s.gl.advance_x + ws_off;

   _lineWidth = std::max(rx, _lineWidth);

   //
   if ((_lineWidth > width) && getStyle().multiline && (width > 0) && (_line.size() > 1)) {
      int lastWordPos = (int)_line.size() - 1;

      for (; lastWordPos > 0; --lastWordPos) {
         if ((_line[lastWordPos]->code == ' ') && (_line[lastWordPos - 1]->code != ' ')) break;
      }

      if (!lastWordPos) {
         if (style.breakLongWords)
            lastWordPos = (int)_line.size() - 1;
         else
            return 0;
      }

      int delta = (int)_line.size() - lastWordPos;
      line leftPart;
      leftPart.resize(delta + 1);
      leftPart.assign(_line.begin() + lastWordPos, _line.end());
      _line.resize(lastWordPos);
      nextLine();

      // line = leftPart;

      for (size_t i = 0; i < leftPart.size(); ++i) {
         putSymbol(*leftPart[i]);
      }

      return 0;
   }

   assert(_x > -1000);

   return 0;
}
}  // namespace text
}  // namespace oxygine
