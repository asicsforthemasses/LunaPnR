#pragma once

#include<limits>
#include<cmath>

namespace Helpers
{

constexpr bool compare(const float &v1, const float &v2)
{
    return std::fabs(v1-v2) <= std::numeric_limits<float>::epsilon();
}

constexpr bool compare(const double &v1, const double &v2)
{
    return std::fabs(v1-v2) <= std::numeric_limits<double>::epsilon();
}

};