// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(PadringTest)

BOOST_AUTO_TEST_CASE(read_padring_config)
{
    auto oldLevel = Logging::getLogLevel();
    Logging::setLogLevel(Logging::LogType::DEBUG);

    std::cout << "--== READ PADRING CONFIG ==--\n";

    LunaCore::Database db;

    std::ifstream leffile("test/files/iit_stdcells/lib/ami035/lib/iit035_stdcells.lef");
    BOOST_REQUIRE(leffile.is_open());
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(db.m_design, leffile));

    std::ifstream verilogfile("test/files/padring/padring.v");
    BOOST_REQUIRE(verilogfile.is_open());
    BOOST_REQUIRE(ChipDB::Verilog::Reader::load(db.m_design, verilogfile));

    BOOST_REQUIRE(db.m_design.setTopModule("top"));

    std::ifstream padringconfig("test/files/padring/padring.conf");
    BOOST_REQUIRE(padringconfig.is_open());

    LunaCore::Padring::Padring padring;

    BOOST_REQUIRE(LunaCore::Padring::read(padringconfig, db, padring));

    Logging::setLogLevel(oldLevel);
};

BOOST_AUTO_TEST_SUITE_END()
