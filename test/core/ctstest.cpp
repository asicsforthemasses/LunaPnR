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

BOOST_AUTO_TEST_SUITE(CTSTest)

void dumpLeaves(LunaCore::CTS::ClockTreeNode *node)
{
    if (node == nullptr) return;
    if (node->isLeaf())
    {
        std::cout << " leaf: ";
        for(auto const& key : node->cells())
        {
            std::cout << " " << key;
        }
        std::cout << "\n";
    }
    else
    {
        for(auto child : node->children())
        {
            dumpLeaves(child);
        }        
    }
}

BOOST_AUTO_TEST_CASE(check_cts)
{
    std::cout << "--== CHECK CTS ==--\n";
        
    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_REQUIRE(leffile.good());
    
    std::ifstream leffile2("test/files/iit_stdcells_extra/fake_ties018.lef");
    BOOST_REQUIRE(leffile2.good());

    ChipDB::Design design;
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(design, leffile));
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(design, leffile2));

    std::ifstream verilogfile("test/files/verilog/femtorv32_quark.v");
    BOOST_REQUIRE(verilogfile.good());

    BOOST_REQUIRE(ChipDB::Verilog::Reader::load(design, verilogfile));
    
    auto mod = design.m_moduleLib->lookupModule("FemtoRV32");
    BOOST_REQUIRE(mod.isValid());

    auto netlist = mod->m_netlist;
    BOOST_REQUIRE(netlist);

    BOOST_CHECK(!LunaCore::CTS::doStuff("clk_doesnt_exist", *netlist));
    BOOST_CHECK(!LunaCore::CTS::doStuff("clk", *netlist));  // fails because cells have not been placed

    // read placement of cells using DEF reader
    std::ifstream deffile("test/files/def/femtorv32_quark.def");
    BOOST_REQUIRE(deffile.good());

    BOOST_REQUIRE(ChipDB::DEF::Reader::load(design, deffile));

    auto clocktree = LunaCore::CTS::doStuff("clk", *netlist);
    BOOST_CHECK(clocktree);  // this should _not_ fail

    dumpLeaves(clocktree.get());
}

BOOST_AUTO_TEST_SUITE_END()