// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/IterativeLinearSolvers>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(GDS2Test)

BOOST_AUTO_TEST_CASE(test_float_versions)
{
    std::cout << "--== GDS2 FLOAT CONVERSION ==--\n";

    auto flt1 = LunaCore::GDS2::IEEE2GDSFloat(3.1415927);

    auto flt1_ = LunaCore::GDS2::GDS2Float2IEEE(flt1);

    std::cout << flt1_ << "\n";

    BOOST_CHECK(flt1_ == 3.1415927);

}

BOOST_AUTO_TEST_SUITE_END()