#pragma once
#include "../oxygine-include.h"
#include "Vector3.h"

namespace oxygine
{
    template <class T>
    using VectorT4 = glm::vec<4, T, glm::defaultp>;
    typedef VectorT4<float> Vector4;
    typedef VectorT4<double> VectorD4;
}
