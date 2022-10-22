// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(DEFReaderTest)

BOOST_AUTO_TEST_CASE(can_read_def)
{
    std::cout << "--== DEF READER ==--\n";
    
    std::ifstream deffile("test/files/def/testcase01.def");
    BOOST_REQUIRE(deffile.good());

    auto logLevel = Logging::getLogLevel();
    Logging::setLogLevel(Logging::LogType::INFO);

    ChipDB::Design design;

    bool result;
    try
    {
        result = ChipDB::DEF::Reader::load(design, deffile);
    }
    catch(std::runtime_error &e)
    {
        std::cerr << e.what() << "\n";
    }
    catch(...)
    {
        std::cerr << "!!!\n";
    }

    BOOST_CHECK(result);

    Logging::setLogLevel(logLevel);
}

BOOST_AUTO_TEST_SUITE_END()
