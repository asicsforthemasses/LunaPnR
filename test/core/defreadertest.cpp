// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
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

    // create a module that appears in the def file
    // so the reader won't barf.
    design.m_moduleLib->createModule("design");

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

BOOST_AUTO_TEST_CASE(can_read_def2)
{
    std::cout << "--== DEF READER 2 ==--\n";
    
    std::ifstream deffile("test/files/def/testcase02.def");
    BOOST_REQUIRE(deffile.good());

    auto logLevel = Logging::getLogLevel();
    Logging::setLogLevel(Logging::LogType::INFO);

    ChipDB::Design design;

    // create a module that appears in the def file
    // so the reader won't barf.
    design.m_moduleLib->createModule("gcd");

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

BOOST_AUTO_TEST_CASE(can_read_def3)
{
    std::cout << "--== DEF READER MAC32 ==--\n";

    ChipDB::Design design;

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_REQUIRE(leffile.good());
    
    std::ifstream leffile2("test/files/iit_stdcells_extra/fake_ties018.lef");
    BOOST_REQUIRE(leffile2.good());

    std::ifstream libertyfile("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib");
    BOOST_REQUIRE(libertyfile.good());

    std::ifstream libertyfile2("test/files/iit_stdcells_extra/fake_ties018.lib");
    BOOST_REQUIRE(libertyfile2.good());

    std::ifstream verilogfile("test/files/verilog/mac32_netlist.v");
    BOOST_REQUIRE(verilogfile.good());

    bool lefresult = false;
    BOOST_CHECK_NO_THROW(lefresult = ChipDB::LEF::Reader::load(design, leffile));
    BOOST_REQUIRE(lefresult);

    bool lefresult2 = false;
    BOOST_CHECK_NO_THROW(lefresult2 = ChipDB::LEF::Reader::load(design, leffile2));
    BOOST_REQUIRE(lefresult2);

    bool libresult = false;
    BOOST_CHECK_NO_THROW(libresult = ChipDB::Liberty::Reader::load(design, libertyfile));
    BOOST_REQUIRE(libresult);

    bool libresult2 = false;
    BOOST_CHECK_NO_THROW(libresult2 = ChipDB::Liberty::Reader::load(design, libertyfile2));
    BOOST_REQUIRE(libresult2);

    bool verilogresult = false;
    BOOST_CHECK_NO_THROW(verilogresult = ChipDB::Verilog::Reader::load(design, verilogfile));
    BOOST_REQUIRE(verilogresult);

    std::ifstream deffile("test/files/def/mac32_placement.def");
    BOOST_REQUIRE(deffile.good());

    auto logLevel = Logging::getLogLevel();
    Logging::setLogLevel(Logging::LogType::INFO);

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

    BOOST_REQUIRE(design.setTopModule("mac32"));

    auto modPtr = design.getTopModule();
    BOOST_REQUIRE(modPtr);

    auto netlist = modPtr->m_netlist;
    BOOST_REQUIRE(netlist);

    // check that all instances have been placed
    for(auto insKeyPair : netlist->m_instances)
    {
        BOOST_REQUIRE(insKeyPair.isValid());
        BOOST_CHECK(insKeyPair->isPlaced());
    }

    Logging::setLogLevel(logLevel);
}


BOOST_AUTO_TEST_SUITE_END()
