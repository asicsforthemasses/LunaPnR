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
    //note: make sure it can be a constexpr by not using std::fabs
    if (v1 > v2)
    {
        return (v2-v1) <= std::numeric_limits<double>::epsilon();
    }
    else
    {
        return (v1-v2) <= std::numeric_limits<double>::epsilon();
    }
    //return std::fabs(v1-v2) <= std::numeric_limits<double>::epsilon();
}

};