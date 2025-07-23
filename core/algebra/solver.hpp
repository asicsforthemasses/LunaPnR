// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <cstdlib>
#include <iostream>

namespace LunaCore::Algebra
{

/** result type for 'solve' that provides solution accuracy information and number of iterations. */
struct ComputeInfo
{
    std::size_t m_iterations{0};    ///< number of iterations used to deliver the solution
    float       m_error{0.0f};      ///< error sqrt(|residual|^2 / |b|^2)
};

};

inline std::ostream& operator<<(std::ostream& os, const LunaCore::Algebra::ComputeInfo &info)
{
    os << "Iterations: " << info.m_iterations << " MSE: " << info.m_error;
    return os;
}
