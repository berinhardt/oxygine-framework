#include "ProgressBar.h"

#include <sstream>

#include "../RenderDelegate.h"
#include "../RenderState.h"
#include "../STDRenderDelegate.h"
#include "../STDRenderer.h"
#include "../Serialize.h"
#include "../glm-config.h"

namespace oxygine {

ProgressBar::ProgressBar() : _progress(1.0f), _direction(dir_0) {
   AnimationFrame f(STDRenderer::white);
   f.setSize(200, 60);
   f.setDestRect(RectF(0, 0, 200, 60));
   setAnimFrame(f);
}

ProgressBar::~ProgressBar() {
}

void ProgressBar::copyFrom(const ProgressBar& src, cloneOptions opt) {
   inherited::copyFrom(src, opt);
   _progress = src._progress;
   _direction = src._direction;
   _originalFrame = src._originalFrame;
}

void ProgressBar::animFrameChanged(const AnimationFrame& f) {
   _originalFrame = f;
   _update();
   inherited::animFrameChanged(f);
}

void ProgressBar::_update() {
   if (!_frame.getDiffuse().base)
      return;

   RectF newSrc = _originalFrame.getSrcRect();
   RectF newDest = _originalFrame.getDestRect();

   float inv_progress = 1.0f - _progress;
   switch (_direction) {
      case dir_180:
         newSrc.pos.x += newSrc.size.x * inv_progress;
         newDest.pos.x += newDest.size.x * inv_progress;
      // break; do not break
      case dir_0:
         newSrc.size.x = newSrc.size.x * _progress;
         newDest.size.x = newDest.size.x * _progress;
         break;
      case dir_90:
         newSrc.pos.y += newSrc.size.y * inv_progress;
         newDest.pos.y += newDest.size.y * inv_progress;
      // break; do not break
      case dir_270:
         newSrc.size.y = newSrc.size.y * _progress;
         newDest.size.y = newDest.size.y * _progress;
         break;
      default:
         break;
   }

   Vector2 newSize = _originalFrame.getSize() * _progress;
   _frame.init(_frame.getResAnim(), _frame.getDiffuse(), newSrc, newDest, newSize);
   //_vstyle._material.srcRect = newSrc;
}

void addVertex(uint32_t rgba, const Vector2& A, const Vector2& B, const Vector2& C, const Vector2& P, const Vector2& Q) {
   STDRenderer* renderer = STDRenderer::getCurrent();

   vertexPCT2 vertices[4];
   vertexPCT2* next = vertices;
   next->color = rgba;
   next->z = 0;
   next->x = A.x;
   next->y = A.y;
   Vector2 uv = P * A + Q;
   next->u = uv.x;
   next->v = uv.y;
   next++;

   next->color = rgba;
   next->z = 0;
   next->x = B.x;
   next->y = B.y;
   uv = P * B + Q;
   next->u = uv.x;
   next->v = uv.y;
   next++;

   next->color = rgba;
   next->z = 0;
   next->x = C.x;
   next->y = C.y;
   uv = P * C + Q;
   next->u = uv.x;
   next->v = uv.y;
   next++;

   next->color = rgba;
   next->z = 0;
   next->x = B.x;
   next->y = B.y;
   uv = P * B + Q;
   next->u = uv.x;
   next->v = uv.y;
   renderer->addVertices(vertices, sizeof(vertices));
}
void ProgressBar::doRender(const RenderState& rs) {
   if (((_direction != __dir_radial_ccw) && (_direction != dir_radial_cw)) || (_progress == 1.0f)) {
      inherited::doRender(rs);
      return;
   }

   _mat->apply();

   const Diffuse& df = _frame.getDiffuse();
   if (df.base) {
      unsigned int color = rs.getFinalColor(getColor()).rgba();

      RectF destRect = inherited::getDestRect();

      RectF srcRect = _frame.getSrcRect();
      Vector2 uv = srcRect.pos;
      const Vector2& duv = srcRect.size;
      uv += duv / 2.0f;

      Vector2 O = destRect.pos;
      const Vector2& size = destRect.size;
      O += size / 2.f;

      float progress = _progress;
      float fP = MATH_PI * 2.f * (progress - 1.f / 8.f);

      Vector2 S(size.x / 2.f, -size.x / 2.f);
      S = glm::rotate(S, fP);
      Vector2 A = O - size / 2.f;
      Vector2 B = A;
      Vector2 C = O + size / 2.f;
      Vector2 D = C;
      Vector2 F = O;
      B.y += size.y;
      D.y -= size.y;
      F.y -= size.y / 2.f;

      Vector2 quadrant = S - O;
      if (quadrant.x < -size.x / 2.f)
         S *= -size.x / 2.f / quadrant.x;
      else if (quadrant.x > size.x / 2.f)
         S *= +size.x / 2.f / quadrant.x;
      if (quadrant.y < -size.y / 2.f)
         S *= -size.y / 2.f / quadrant.y;
      else if (quadrant.y > size.y / 2.f)
         S *= +size.y / 2.f / quadrant.y;

      O = rs.transform.apply(O);
      F = rs.transform.apply(F);
      A = rs.transform.apply(A);
      B = rs.transform.apply(B);
      C = rs.transform.apply(C);
      D = rs.transform.apply(D);
      S = rs.transform.apply(S);

      Vector2 P = duv / (C - A);
      Vector2 Q = uv - O * P;

      if (progress <= 3.f / 8.f) {
         // SFO
         addVertex(color, S, F, O, P, Q);
         if (progress > 1.f / 8.f) {
            // SFD
            addVertex(color, S, F, D, P, Q);
         }
      } else if (progress <= 5.f / 8.f) {
         // OFD
         // OSD
         // SCD
         addVertex(color, D, F, O, P, Q);
         addVertex(color, S, D, O, P, Q);
         addVertex(color, S, C, D, P, Q);
      } else {
         // BCD
         // OBS
         addVertex(color, F, D, O, P, Q);
         addVertex(color, S, B, O, P, Q);
         addVertex(color, B, C, D, P, Q);
         if (progress > 7.f / 8.f) {
            // BAS
            addVertex(color, S, A, B, P, Q);
         }
      }
   }
}

std::string ProgressBar::dump(const dumpOptions& options) const {
   std::stringstream stream;
   stream << "{ProgressBar}\n";

   const char* dir = "dir_0";
   switch (_direction) {
      case dir_90:
         dir = "dir_90";
         break;
      case dir_180:
         dir = "dir_180";
         break;
      case dir_270:
         dir = "dir_270";
         break;
      case dir_radial_cw:
         dir = "dir_radial_cw";
         break;
      case __dir_radial_ccw:
         dir = "dir_radial_ccw";
         break;
      default:
         break;
   }
   stream << " direction=" << dir << "";

   stream << "\n"
          << inherited::dump(options);

   return stream.str();
}
void ProgressBar::setProgress(float f) {
   _progress = scalar::clamp(f, 0.0f, 1.0f);

   Event ev(PROGRESS_CHANGED);
   dispatchEvent(&ev);

   if (_direction == __dir_radial_ccw || _direction == dir_radial_cw)
      return;
   _update();
}

void ProgressBar::setDirection(direction dir) {
   _direction = dir;
   if (_direction == __dir_radial_ccw || _direction == dir_radial_cw) {
      _frame = _originalFrame;
      return;
   }
   _update();
}

float ProgressBar::getProgress() const {
   return _progress;
}

ProgressBar::direction ProgressBar::getDirection() const {
   return _direction;
}

void ProgressBar::serialize(serializedata* data) {
   inherited::serialize(data);
   pugi::xml_node node = data->node;
   data->node.set_name("ProgressBar");
   data->node.append_attribute("progress").set_value(_progress);
   data->node.append_attribute("direction").set_value((int)_direction);
}

void ProgressBar::deserialize(const deserializedata* data) {
   inherited::deserialize(data);
   _direction = (direction)data->node.attribute("direction").as_int();
   _progress = data->node.attribute("progress").as_float(1.0f);

   _update();
}
}  // namespace oxygine