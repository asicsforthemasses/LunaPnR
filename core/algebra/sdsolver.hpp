// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <cassert>
#include <list>
#include <vector>
#include <algorithm>
#include <iterator>
#include "vector.hpp"
#include "solver.hpp"
#include "sparsematrix.hpp"

namespace LunaCore::Algebra::SDSolver
{

/** Ax = b linear system solver based on steepest descent iterations
    @tparam T the datatype of the matrix and vectors.
    @param[in] mat the A matrix.
    @param[in] rhs the b vector.
    @param[out] x   the solution vector.
    @param[in] tolerance maximum L1 norm of residual / b.
    @param[in] maxIter maximum iterations the solver may use to arrive at a solution.
    @return information about the error and the number of iterations.
*/

template<typename T>
ComputeInfo solve(const SparseMatrix<T> &mat,
    const Vector<T> &rhs,
    Vector<T> &x,
    const float tolerance = 1.0e-5f,
    const std::size_t maxIter = 100)
{
    ComputeInfo info;

    auto r = rhs - mat*x;   // initial residual
    auto delta = dot(r,r);
    auto delta0 = delta;
    auto tol2  = tolerance*tolerance;

    info.m_iterations = 0;
    while(info.m_iterations < maxIter)
    {
        if (delta <= tol2*delta0)
        {
            info.m_error = sqrt(dot(r,r) / norm2(rhs));
            return info;
        }

        auto q     = mat*r;
        auto alpha = delta / dot(r,q);
        x = x + alpha*r;
        
        if ((info.m_iterations % 50) == 49)
        {
            r = rhs - mat*x;
        }
        else
        {
            r = r - alpha*q;
        }

        delta = dot(r,r);
        info.m_iterations++;
    }

    info.m_error = sqrt(dot(r,r) / norm2(rhs));
    return info;
};

};
