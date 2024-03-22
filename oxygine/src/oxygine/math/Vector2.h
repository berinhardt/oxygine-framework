#pragma once
#include "../oxygine-include.h"
#include "ScalarMath.h"
//#include "../math/vector3.h"

namespace oxygine
{
    template <typename T>
    using VectorT2 = glm::vec<2, T, glm::defaultp>;
    typedef VectorT2<float> Vector2;
    typedef VectorT2<double> VectorD2;
    typedef VectorT2<int> Point;
    typedef VectorT2<short> PointS;
}
