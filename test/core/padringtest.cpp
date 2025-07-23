// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
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
    Logging::setLogLevel(Logging::LogType::VERBOSE);

    std::cout << "--== READ PADRING CONFIG ==--\n";

    LunaCore::Database db;

    std::ifstream leffile("test/files/iit_stdcells/lib/ami035/lib/iit035_stdcells.lef");
    BOOST_REQUIRE(leffile.is_open());
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(db.m_design, leffile));

    leffile.close();
    leffile.open("test/files/iit_stdcells_extra/fake_pad_fillers35.lef");
    BOOST_REQUIRE(leffile.is_open());
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(db.m_design, leffile));

    std::ifstream verilogfile("test/files/padring/padring.v");
    BOOST_REQUIRE(verilogfile.is_open());
    BOOST_REQUIRE(ChipDB::Verilog::Reader::load(db.m_design, verilogfile));

    BOOST_REQUIRE(db.m_design.setTopModule("top"));

    db.m_design.m_floorplan->clear();
    db.m_design.m_floorplan->setCoreSize({1000000, 1000000});
    db.m_design.m_floorplan->setIO2CoreMargins({10000, 10000, 10000, 10000});
    db.m_design.m_floorplan->setCornerCellSize({300000,300000});
    db.m_design.m_floorplan->setIOMargins({300000,300000,300000,300000});

    // fix the PDK
    LunaCore::Passes::registerAllPasses();
    LunaCore::Passes::run(db, "set -cell PADNC -subclass SPACER");
    LunaCore::Passes::run(db, "set -cell PADNC5 -subclass SPACER");
    LunaCore::Passes::run(db, "set -cell PADNC10 -subclass SPACER");
    LunaCore::Passes::run(db, "set -cell PADNC50 -subclass SPACER");

    std::ifstream padringconfig("test/files/padring/padring.conf");
    BOOST_REQUIRE(padringconfig.is_open());

    LunaCore::Padring::Padring padring;

    BOOST_REQUIRE(LunaCore::Padring::read(padringconfig, db, padring));
    BOOST_CHECK(LunaCore::Padring::place(db, padring));

    Logging::setLogLevel(oldLevel);
};

BOOST_AUTO_TEST_SUITE_END()
