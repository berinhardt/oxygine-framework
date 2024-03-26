#pragma once
#include "../oxygine-include.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <algorithm>
#include <math.h>
#include <stdlib.h>

namespace oxygine
{
    template <class T>
    using Matrix4T = glm::mat<4,4,T,glm::defaultp>;
    typedef Matrix4T<float> Matrix4;
    typedef Matrix4T<double> Matrix4D;

}
