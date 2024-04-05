
#include "lunacore.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(AlgebraTests)

BOOST_AUTO_TEST_CASE(cgsolvertest)
{
    std::cout << "--== CG SOLVER TEST ==--\n";

    Logging::setLogLevel(Logging::LogType::INFO);

    LunaCore::Algebra::SparseMatrix<float> m;

    m.at(0,0) = 10;
    m.at(0,1) = 1;
    m.at(1,1) = 20;

    LunaCore::Algebra::Vector<float> b(2);
    b.at(0) = 5;
    b.at(1) = 40;

    LunaCore::Algebra::CGSolver::Preconditioner<float> precon(m);

    const float tol = 1e-3f;
    LunaCore::Algebra::Vector<float> x(2);
    auto info = LunaCore::Algebra::CGSolver::solve(m, b, x, precon, tol);

#if 0
    for(auto const &value : x)
    {
        std::cout << value << "\n";
    }
#endif

    Logging::logInfo("  Solver iterations: %lu\n", info.m_iterations);
    Logging::logInfo("         error     : %f  (tol = %f)\n", info.m_error, tol);
    BOOST_CHECK(info.m_error < tol);
};

BOOST_AUTO_TEST_CASE(cgsolvertest2)
{
    //see: https://en.wikipedia.org/wiki/Conjugate_gradient_method
    std::cout << "--== CG SOLVER TEST2 ==--\n";

    LunaCore::Algebra::SparseMatrix<float> A;
    A.at(0,0) = 4.0f;
    A.at(0,1) = 1.0f;
    A.at(1,0) = 1.0f;
    A.at(1,1) = 3.0f;

    LunaCore::Algebra::Vector<float> b(2);
    b.at(0) = 1.0f;
    b.at(1) = 2.0f;

    LunaCore::Algebra::CGSolver::Preconditioner<float> precon(A);

    const float tol = 1e-3f;
    LunaCore::Algebra::Vector<float> x(2);
    auto info = LunaCore::Algebra::CGSolver::solve(A, b, x, precon, tol);

    // solution is x(0) = 1/11 and x(1) = 7/11
    const std::array<float,2> expected = {1.0f/11.0f, 7.0f/11.0f};
    std::size_t index = 0;
    for(auto ex : expected)
    {
        BOOST_CHECK( std::abs(x.at(index) - ex) < tol);
        index++;
    }
}

BOOST_AUTO_TEST_SUITE_END()
