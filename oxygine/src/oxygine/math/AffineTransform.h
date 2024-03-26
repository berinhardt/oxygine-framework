#pragma once
#include "../oxygine-include.h"
#include "Matrix.h"
#include "Vector2.h"

namespace oxygine
{
    template<class T>
    class AffineTransformT
    {
    public:

        typedef VectorT2<T> vector2;
        typedef AffineTransformT<T> affineTransform;
        typedef VectorT4<T> column;
        typedef Matrix4T<T> matrix4;
        typedef Matrix2T<T> matrix2;

        AffineTransformT() {}
        AffineTransformT(T a_, T b_, T c_, T d_, T x_, T y_): _transform(a_, b_, c_,d_), _translate(x_, y_) {}
        AffineTransformT(const matrix2 &f, const vector2 &l): _transform(f), _translate(l) {}


        explicit AffineTransformT(const matrix4& m) : _transform(m[0][0],m[0][1],m[1][0],m[1][1]), _translate(m[3][0], m[3][1])
        { }

        void identity()
        {
            _translate = glm::zero<Vector2>();
            _transform = matrix2(1.0f);
        }

        static affineTransform getIdentity()
        {
            affineTransform t;
            t.identity();
            return t;
        }

        void translate(const vector2& v)
        {
            _translate += _transform*v;
        }

        affineTransform translated(const vector2& v) const
        {
            affineTransform t = *this;
            t.translate(v);
            return t;
        }

        void scale(const vector2& v)
        {
            _transform[0][0] *= v.x;
            _transform[0][1] *= v.x;
            _transform[1][0] *= v.y;
            _transform[1][1] *= v.y;
        }

        affineTransform scaled(const vector2& v) const
        {
            affineTransform t = *this;
            t.scale(v);
            return t;
        }

        void rotate(T v)
        {
            T sin_ = scalar::sin(v);
            T cos_ = scalar::cos(v);

            affineTransform rot(cos_, sin_, -sin_, cos_, 0, 0);
            *this = *this * rot;
        }

        affineTransform rotated(T v) const
        {
            affineTransform t = *this;
            t.rotate(v);
            return t;
        }

        void invert()
        {
            affineTransform t = *this;
            _transform = glm::inverse(_transform);
            _translate = -_transform*t._translate;
        }

        affineTransform inverted() const
        {
            affineTransform t = *this;
            t.invert();
            return t;
        }

        operator matrix4() const
        {
            return toMatrix();
        }

        matrix4 toMatrix() const
        {
            matrix4 m(_transform);
            m[3] = column(_translate, .0f, 1.0f);
            return m;
        }


        static affineTransform& multiply(affineTransform& out, const affineTransform& t1, const affineTransform& t2)
        {
            out._transform = t1._transform*t2._transform;
            out._translate = t2._transform*t1._translate+t2._translate;
            return out;
        }


        affineTransform operator * (const affineTransform& t2) const
        {
            affineTransform r;
            multiply(r, *this, t2);
            return r;
        }

        vector2 transform(const vector2& v) const
        {
            return _transform*v+_translate;
        }


        matrix2 _transform;
        vector2 _translate;
    };

    typedef AffineTransformT<float> AffineTransform;
}
