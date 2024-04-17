#pragma once
#include <cassert>

namespace LunaCore
{

/** round up to a multiple of 'multiple' */
constexpr auto roundUp(auto const value, auto const multiple)
{
    assert(value >= 0);

    if (multiple == 0) return value;

    auto remainder = value % multiple;
    if (remainder == 0) return value;

    return value + multiple - remainder;
}

};