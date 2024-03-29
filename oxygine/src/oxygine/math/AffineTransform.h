#pragma once
#include "../oxygine-include.h"
#include "Matrix.h"
#include "Vector2.h"

namespace oxygine
{
    class AffineTransform {
        public:
        AffineTransform();
        AffineTransform(const Vector2& t,const Vector2& s, float r);
        AffineTransform(const AffineTransform&);
        ~AffineTransform(){};

        public:
        AffineTransform& operator=(const AffineTransform&);
        public:
        void translate(const Vector2& pos);
        void rescale(const Vector2& scale);
        void rotate(float r);

        AffineTransform inverse() const;
        AffineTransform& invert();
        AffineTransform& identity();

        AffineTransform& compose(const AffineTransform&);
        AffineTransform operator*(const AffineTransform&) const;
        AffineTransform& operator*=(const AffineTransform& t) { return compose(t); }

        Vector2 applyScale(const Vector2&) const;
        Vector2 applyInverse(const Vector2&) const;
        Vector2 apply(const Vector2&) const;
        Vector2 applyT(const Vector2&) const;
        Vector2 applySR(const Vector2&) const;

        void setTranslation(const Vector2& pos);
        void setScale(const Vector2& pos);
        void setRotation(float r);

        const Vector2& getTranslation() const { return translation; }
        const Vector2& getScale() const { return scale; }
        float getRotation() const { return rotation; }
        const glm::mat2& getMatrix() const;
        
        private:
        void updateMatrix() const;
        Vector2 translation;
        Vector2 scale;
        float rotation;
        mutable glm::mat2 matrix;
        mutable bool dirty;
    };
}

