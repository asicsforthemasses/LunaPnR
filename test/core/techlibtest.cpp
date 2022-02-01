/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/


#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TechlibTest)

BOOST_AUTO_TEST_CASE(various_tech_tests)
{
    std::cout << "--== TECHLIB TEST ==--\n";

    //FIXME: actually add tests.. 
    ChipDB::TechLib techlib;
}

BOOST_AUTO_TEST_SUITE_END()
