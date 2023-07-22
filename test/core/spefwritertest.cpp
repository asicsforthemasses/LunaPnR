// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(SPEFWriterTest)


BOOST_AUTO_TEST_CASE(can_write_spef)
{
    std::cout << "--== SPEF WRITER ==--\n";
    
    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_REQUIRE(leffile.good());

    ChipDB::Design design;
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(design, leffile));

    std::ifstream verilogfile("test/files/verilog/adder2.v");
    BOOST_REQUIRE(verilogfile.good());

    BOOST_REQUIRE(ChipDB::Verilog::Reader::load(design, verilogfile));

    // check the design
    BOOST_REQUIRE(design.m_moduleLib->size() == 1);
    BOOST_REQUIRE(design.m_moduleLib->lookupModule("adder2").isValid());
    
    auto mod = design.m_moduleLib->lookupModule("adder2");

    BOOST_CHECK(LunaCore::SPEF::write("test/files/results/adders2.spef", mod.ptr()));
}

BOOST_AUTO_TEST_SUITE_END()
