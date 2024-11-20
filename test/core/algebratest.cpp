// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include "algebra/algebra.hpp"
#include <boost/test/unit_test.hpp>

using namespace LunaCore;

BOOST_AUTO_TEST_SUITE(AlgebraTest)

BOOST_AUTO_TEST_CASE(test_simple_eigen_stuff)
{
    std::cout << "--== TEST ALGEBRA ==--\n";

    Algebra::SparseMatrix<float> m(2);
    m(0,0) = 3;
    m(1,0) = 2.5;
    m(0,1) = -1;
    m(1,1) = m(1,0) + m(0,1);

    BOOST_CHECK(m(1,1) == 1.5);

}

BOOST_AUTO_TEST_CASE(SDSolver_1)
{
    std::cout << "--== TEST ALGEBRA::SDSOLVER_1 ==--\n";

    const size_t N = 2;
    Algebra::Vector<float> vx(N);     // x vector
    Algebra::Vector<float> vb(N);     // b vector

    vb = "1, 2";
    vx.zero();

    Algebra::SparseMatrix<float> mat(N);

    const std::array<float, N*N> matEntries = {
         1,  2,
         3,  5
    };

    auto iter = matEntries.begin();
    for(size_t row = 0; row < N; row++)
    {
        for(size_t col = 0; col < N; col++)
        {
            mat.at(row, col) = *iter++;
        }        
    }

    std::cout << "  Matrix: " << mat << "\n";

    auto info = Algebra::SDSolver::solve(mat, vb, vx, 1.0e-5f, 1000);

    std::cout << "  Iterations = " << info.m_iterations << "\n";
    std::cout << "  Error      = " << info.m_error << "\n";
}

BOOST_AUTO_TEST_CASE(CGSolver_1)
{
    std::cout << "--== TEST ALGEBRA::CGSOLVER_1 ==--\n";

    const size_t N = 2;
    Algebra::Vector<float> vx(N);     // x vector
    Algebra::Vector<float> vb(N);     // b vector

    vb = "1, 2";
    vx.zero();

    Algebra::SparseMatrix<float> mat(N);

    const std::array<float, N*N> matEntries = {
         1,  2,
         3,  5
    };

    auto iter = matEntries.begin();
    for(size_t row = 0; row < N; row++)
    {
        for(size_t col = 0; col < N; col++)
        {
            mat.at(row, col) = *iter++;
        }        
    }

    std::cout << "  Matrix: " << mat << "\n";

    Algebra::CGSolver::NoPreconditioner noprecon(mat);
    auto info = Algebra::CGSolver::solve(mat, vb, vx, noprecon, 1.0e-5f, 1000);

    std::cout << "  Iterations = " << info.m_iterations << "\n";
    std::cout << "  Error      = " << info.m_error << "\n";
}


#if 0
BOOST_AUTO_TEST_CASE(SDSolver)
{
    std::cout << "--== TEST ALGEBRA::SDSOLVER ==--\n";

    // solve Ax + b = 0
    const size_t N = 10;
    Algebra::Vector<float> vx(N);     // x vector
    Algebra::Vector<float> vb(N);     // b vector

    vb = "26.717, 29.451, 30.383, 18.453, 28.383, 19.795, 33.162, 29.254, 25.458, 30.753";
    //vx.zero();
    vx = "1,2,3,4,5,6,7,8,9,10";
    //vx = 0.5f*vx;

    Algebra::SparseMatrix<float> mat(N);

    const std::array<float,100> matEntries = {
        0.229695,   0.690635,   0.616636,   0.211123,   0.832392,   0.370716,   0.734110,   0.699650,   0.187762,   0.359942,
        0.863408,   0.985869,   0.404228,   0.476515,   0.273468,   0.320162,   0.858238,   0.728686,   0.743069,   0.168414,
        0.835193,   0.811203,   0.901006,   0.858847,   0.012385,   0.820319,   0.772809,   0.216854,   0.133299,   0.845927,
        0.765512,   0.714526,   0.379682,   0.220327,   0.317389,   0.285239,   0.809550,   0.232699,   0.047876,   0.298018,
        0.777579,   0.483532,   0.277070,   0.010964,   0.930385,   0.194816,   0.946667,   0.748971,   0.035425,   0.700505,
        0.221320,   0.400438,   0.207084,   0.055486,   0.756033,   0.609477,   0.079978,   0.019625,   0.824724,   0.235287,
        0.742477,   0.739054,   0.483942,   0.644075,   0.447451,   0.635992,   0.422765,   0.102596,   0.827036,   0.963706,
        0.207270,   0.280237,   0.814382,   0.801900,   0.206579,   0.600032,   0.099114,   0.831088,   0.478120,   0.655645,
        0.740534,   0.935769,   0.169096,   0.506901,   0.184819,   0.895788,   0.998351,   0.483706,   0.273516,   0.069275,
        0.729925,   0.946078,   0.926757,   0.561323,   0.521301,   0.549666,   0.115890,   0.311166,   0.689658,   0.769348};
    
    auto iter = matEntries.begin();
    for(size_t row = 0; row < N; row++)
    {
        for(size_t col = 0; col < N; col++)
        {
            mat.at(row, col) = *iter++;
        }        
    }

    Algebra::SDSolver::solve(mat, vb, vx, 1.0e-5f, 1000);
    
    std::array<float,N> xvalues{1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,8.0f,9.0f,10.0f};
    Algebra::Vector<float> checkX(xvalues);

    // create the sum of the absolute element errors
    auto diff = (vx - checkX);
    auto error = Algebra::dot(diff, diff);

#if 0
    std::cout << "iterations: " << solver.iterations() << "\n";
    std::cout << "est. error: " << solver.error() << "\n";
    std::cout << "mat: " << mat << "\n";
    std::cout << "vb : " << vb << "\n";
    std::cout << "vx : " << vx << "\n";
    std::cout << "diff: " << diff << "\n";
    std::cout << "error: " << error << "\n";
#endif

    BOOST_CHECK(error < 1e-4f);
}
#endif


#if 0
BOOST_AUTO_TEST_CASE(Solver)
{
    std::cout << "--== TEST ALGEBRA QP ==--\n";

    // solve Ax + b = 0
    const size_t N = 10;
    Algebra::Vector<float> vx(N);     // x vector
    Algebra::Vector<float> vb(N);     // b vector

    vb = "26.717, 29.451, 30.383, 18.453, 28.383, 19.795, 33.162, 29.254, 25.458, 30.753";
    vx.zero();
    vx = "1,2,3,4,5,6,7,8,9,10";
    vx = 0.5f*vx;

    Algebra::SparseMatrix<float> mat(N);

    const std::array<float,100> matEntries = {
        0.229695,   0.690635,   0.616636,   0.211123,   0.832392,   0.370716,   0.734110,   0.699650,   0.187762,   0.359942,
        0.863408,   0.985869,   0.404228,   0.476515,   0.273468,   0.320162,   0.858238,   0.728686,   0.743069,   0.168414,
        0.835193,   0.811203,   0.901006,   0.858847,   0.012385,   0.820319,   0.772809,   0.216854,   0.133299,   0.845927,
        0.765512,   0.714526,   0.379682,   0.220327,   0.317389,   0.285239,   0.809550,   0.232699,   0.047876,   0.298018,
        0.777579,   0.483532,   0.277070,   0.010964,   0.930385,   0.194816,   0.946667,   0.748971,   0.035425,   0.700505,
        0.221320,   0.400438,   0.207084,   0.055486,   0.756033,   0.609477,   0.079978,   0.019625,   0.824724,   0.235287,
        0.742477,   0.739054,   0.483942,   0.644075,   0.447451,   0.635992,   0.422765,   0.102596,   0.827036,   0.963706,
        0.207270,   0.280237,   0.814382,   0.801900,   0.206579,   0.600032,   0.099114,   0.831088,   0.478120,   0.655645,
        0.740534,   0.935769,   0.169096,   0.506901,   0.184819,   0.895788,   0.998351,   0.483706,   0.273516,   0.069275,
        0.729925,   0.946078,   0.926757,   0.561323,   0.521301,   0.549666,   0.115890,   0.311166,   0.689658,   0.769348};
    
    auto iter = matEntries.begin();
    for(size_t row = 0; row < N; row++)
    {
        for(size_t col = 0; col < N; col++)
        {
            mat.at(row, col) = *iter++;
        }        
    }

    Algebra::CGSolver::JacobiPreconditioner precon(mat);
    auto info = Algebra::CGSolver::solve(mat, vb, vx, precon, 1.0e-5f, 1000);
    
    std::array<float,N> xvalues{1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,8.0f,9.0f,10.0f};
    Algebra::Vector<float> checkX(xvalues);

    // create the sum of the absolute element errors
    auto diff = (vx - checkX);
    auto error = Algebra::dot(diff, diff);

#if 0
    std::cout << "iterations: " << solver.iterations() << "\n";
    std::cout << "est. error: " << solver.error() << "\n";
    std::cout << "mat: " << mat << "\n";
    std::cout << "vb : " << vb << "\n";
    std::cout << "vx : " << vx << "\n";
    std::cout << "diff: " << diff << "\n";
    std::cout << "error: " << error << "\n";
#endif

    BOOST_CHECK(error < 1e-4f);
}
#endif

BOOST_AUTO_TEST_SUITE_END()
