#pragma once
#include "../oxygine-include.h"
#include "Vector2.h"

namespace oxygine
{
    template <class T>
    using VectorT3 = glm::vec<3, T, glm::defaultp>;
    typedef VectorT3<float> Vector3;
    typedef VectorT3<double> VectorD3;
}
