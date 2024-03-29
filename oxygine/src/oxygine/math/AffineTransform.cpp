#include "AffineTransform.h"

namespace oxygine {
AffineTransform::AffineTransform() : translation(0, 0), scale(1, 1), rotation(0), matrix(1), dirty(false) {}
AffineTransform::AffineTransform(const Vector2 &t, const Vector2 &s, float r) : translation(t), scale(s), rotation(r), matrix(1), dirty(true) {}
AffineTransform::AffineTransform(const AffineTransform& t) : translation(t.translation), scale(t.scale), rotation(t.rotation), dirty(t.dirty) {
   if (!t.dirty) matrix = t.matrix;
}
AffineTransform& AffineTransform::operator=(const AffineTransform& t) {
   translation = t.translation;
   scale = t.scale;
   rotation = t.rotation;
   dirty = t.dirty;
   if (!dirty) matrix = t.matrix;
   return *this;
}
void AffineTransform::translate(const Vector2& pos) {
   translation += pos;
}
void AffineTransform::rescale(const Vector2& s) {
   setScale({s.x * scale.x, s.y * scale.y});
}
void AffineTransform::rotate(float r) {
   setRotation(rotation + r);
}

void AffineTransform::setTranslation(const Vector2& pos) {
   translation = pos;
}
void AffineTransform::setScale(const Vector2& s) {
   if (s != scale)
      dirty = true;
   scale = s;
}
void AffineTransform::setRotation(float r) {
   if (r != rotation)
      dirty = true;
   rotation = r;
}
const glm::mat2& AffineTransform::getMatrix() const {
   updateMatrix();
   return matrix;
}

void AffineTransform::updateMatrix() const {
   if (dirty) {
      dirty = false;
      glm::vec2 cs(glm::cos(rotation), glm::sin(rotation));
      matrix = {cs.x * scale.x, -cs.y,
                cs.y, cs.x * scale.y};
   }
}
AffineTransform& AffineTransform::compose(const AffineTransform& t) {
   translate(t.getTranslation() * getMatrix());
   rescale(t.getScale());
   rotate(t.getRotation());
   return *this;
}
AffineTransform& AffineTransform::identity() {
   setTranslation(glm::zero<Vector2>());
   setScale(Vector2(1.0f,1.0f));
   setRotation(0.0f);
   return *this;
}
AffineTransform AffineTransform::operator*(const AffineTransform& t) const {
   AffineTransform at(*this);   
   at.compose(t);
   return at;
}
Vector2 AffineTransform::applyScale(const Vector2& size) const {
   return {getScale().x * size.x, getScale().y * size.y};
}

Vector2 AffineTransform::applyInverse(const Vector2& size) const {
   return size * glm::inverse(getMatrix()) - getTranslation();
}
Vector2 AffineTransform::applyT(const Vector2& size) const {
   return size  + getTranslation();
}
Vector2 AffineTransform::applySR(const Vector2& size) const {
   return size * getMatrix();
}
Vector2 AffineTransform::apply(const Vector2& size) const {
   return applyT(applySR(size));
}
AffineTransform& AffineTransform::invert() {
   setTranslation(-getTranslation());
   setScale(Vector2(1/getScale().x, 1/getScale().y));
   setRotation(-getRotation());
   return *this;
}
AffineTransform AffineTransform::inverse() const {
   AffineTransform t(*this);
   t.invert();
   return t;
}
}
