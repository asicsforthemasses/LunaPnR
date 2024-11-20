// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <cassert>
#include <cmath>
#include <algorithm>
#include <limits>
#include <iostream>
#include "vector.hpp"
#include "solver.hpp"
#include "sparsematrix.hpp"

namespace LunaCore::Algebra::CGSolver
{

/** No preconditioner
    Mostly for debugging
*/
template<class T>
class NoPreconditioner
{
public:
    NoPreconditioner(const SparseMatrix<T> &mat) {};
    
    /** Solve for and return the pre-conditioned A matix.
        Used internally by the conjugate gradient solver.
    */
    [[nodiscard]] constexpr Vector<T> solve(const Vector<T> &v) const noexcept
    {
        return v;
    }
};

/** A Simple Jacobi/diagonal preconditioner.
    A preconditioner makes the conjugate gradient solver converge more quickly.
*/
template<class T>
class JacobiPreconditioner
{
public:

    /** Create a diagonal/Jacobi preconditioner based on the matrix A.
        @param[in] mat the 'A' matrix of the linear system Ax=b.
    */
    JacobiPreconditioner(const SparseMatrix<T> &mat)
    {
        auto const N = mat.rowCount();
        m_invdiag.resize(N);
        for(std::size_t row=0; row < N; row++)
        {
            auto d = mat.at(row, row);
            if (std::abs(d) < 1.0e-10f )
            {
                m_invdiag.at(row) = 1.0f;
            }
            else
            {
                m_invdiag.at(row) = 1.0f / d;
            }
        }
    }

    /** Solve for and return the pre-conditioned A matix.
        Used internally by the conjugate gradient solver.
    */
    [[nodiscard]] constexpr Vector<T> solve(const Vector<T> &v) const noexcept
    {
        return m_invdiag*v;
    }

protected:
    Vector<T> m_invdiag;    ///< inverted diagonal vector of matrix A.
};


/** Ax = b linear system solver based on conjugate gradient iterations
    @tparam T the datatype of the matrix and vectors.
    @param[in] mat the A matrix.
    @param[in] rhs the b vector.
    @param[out] x   the solution vector.
    @param[in] preconditioner callable 'Vector<T> solve(const Vector<T> &v) const'
    @param[in] tolerance maximum L1 norm of residual / b.
    @param[in] maxIter maximum iterations the solver may use to arrive at a solution.
    @return information about the error and the number of iterations.

    See: https://en.wikipedia.org/wiki/Conjugate_gradient_method
*/
template<typename T>
ComputeInfo solve(const SparseMatrix<T> &mat,
    const Vector<T> &rhs,
    Vector<T> &x,
    auto &preconditioner,
    const float tolerance = 1.0e-5f,
    const std::size_t maxIter = 100)
{
    ComputeInfo info;

    const std::size_t N = mat.rowCount();

    assert(rhs.size() == N);
    assert(x.size() == N);

    auto residual = rhs - mat*x;

    // early out on the trivial solution of x=0
    auto rhsL2 = norm2(rhs);
    if (rhsL2 < 1.0e-20f)
    {
        x.zero();
        info.m_error = 0.0f;
        return info;
    }

    auto zeroThreshold = std::numeric_limits<T>::min();
    const auto threshold = std::max(tolerance*tolerance*rhsL2, zeroThreshold);
    auto residualL2 = norm2(residual);
    if (residualL2 < threshold)
    {
        info.m_error = std::sqrt(residualL2 / rhsL2);
        return info;
    }

    // preconditioning here..
    auto p = preconditioner.solve(residual);

    // iterate to solve
    std::size_t iteration = 0;

    float absNew = dot(residual, p);

    while(iteration < maxIter)
    {
        auto const tmp = mat * p;                   // FIXME: allocate tmp outside the loop..

        auto const alpha = absNew / dot(p, tmp);    // FIXME: possible division by zero
        x = x + alpha * p;
        residual = residual - alpha * tmp;

        residualL2 = norm2(residual);
        if (residualL2 < threshold) break;

        auto z = preconditioner.solve(residual);
        const auto absOld = absNew;
        absNew = dot(residual, z);
        auto const beta = absNew / absOld;
        p = z + beta * p;
        iteration++;
    }

    info.m_iterations = iteration;
    info.m_error = std::sqrt(residualL2 / rhsL2);
    return info;
};

};
