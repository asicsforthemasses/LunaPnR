// SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(VerilogWriterTest)

BOOST_AUTO_TEST_CASE(test_null_behaviour)
{
    BOOST_CHECK(LunaCore::Verilog::Writer::write(std::cout, nullptr) == false);
}

BOOST_AUTO_TEST_CASE(can_write_netlist_to_verilog)
{
    std::cout << "--== VERILOG WRITER (adder2) ==--\n";

    ChipDB::Design design;

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());    
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));

    std::ifstream libertyfile("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib");
    BOOST_CHECK(libertyfile.good());    
    BOOST_CHECK(ChipDB::Liberty::Reader::load(design, libertyfile));

    std::ifstream verilogfile("test/files/verilog/adder2.v");
    BOOST_CHECK(verilogfile.good());

    BOOST_CHECK(ChipDB::Verilog::Reader::load(design, verilogfile));

    auto modulePtr = design.m_moduleLib->lookupModule("adder2");
    BOOST_CHECK(modulePtr.isValid());

    std::ofstream ofile("test/files/results/adder2_export.v");
    BOOST_CHECK(ofile.good());

    if (ofile.good())
    {
        BOOST_CHECK(LunaCore::Verilog::Writer::write(ofile, modulePtr.ptr()));
    }
}

BOOST_AUTO_TEST_CASE(can_write_adder8_to_verilog)
{
    std::cout << "--== VERILOG WRITER (adder8) ==--\n";

    ChipDB::Design design;

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());    
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));

    std::ifstream libertyfile("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib");
    BOOST_CHECK(libertyfile.good());    
    BOOST_CHECK(ChipDB::Liberty::Reader::load(design, libertyfile));

    std::ifstream verilogfile("test/files/verilog/adder8.v");
    BOOST_CHECK(verilogfile.good());

    BOOST_CHECK(ChipDB::Verilog::Reader::load(design, verilogfile));

    auto modulePtr = design.m_moduleLib->lookupModule("adder8");
    BOOST_CHECK(modulePtr.isValid());

    std::ofstream ofile("test/files/results/adder8_export.v");
    BOOST_CHECK(ofile.good());

    if (ofile.good())
    {
        BOOST_CHECK(LunaCore::Verilog::Writer::write(ofile, modulePtr.ptr()));
    }
}

BOOST_AUTO_TEST_SUITE_END()
