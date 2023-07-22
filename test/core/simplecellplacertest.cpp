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

BOOST_AUTO_TEST_SUITE(SimpleCellPlacerTest)

BOOST_AUTO_TEST_CASE(check_simplecellplacer)
{
    std::cout << "--== CHECK SIMPLECELLPLACER ==--\n";

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));

    std::ifstream verilogfile("test/files/verilog/nerv_tsmc018.v");
    BOOST_CHECK(verilogfile.good());

    ChipDB::Verilog::Reader::load(design, verilogfile);    

    auto mod = design.m_moduleLib->lookupModule("nerv");
    BOOST_CHECK(mod.isValid());

    LunaCore::SimpleCellPlacer::place(mod->m_netlist.get(), ChipDB::Rect64{{0,0},{650000,650000}}, 10000);

    auto hpwl = LunaCore::NetlistTools::calcHPWL(*mod->m_netlist.get());
    BOOST_CHECK(hpwl > 0);
    std::cout << "  HPWL = " << hpwl << " nm\n";

    auto area = LunaCore::NetlistTools::calcTotalCellArea(*mod->m_netlist.get());
    BOOST_CHECK(area > 0.0);
    std::cout << "  Area = " << area << " um²\n";
};


BOOST_AUTO_TEST_SUITE_END()