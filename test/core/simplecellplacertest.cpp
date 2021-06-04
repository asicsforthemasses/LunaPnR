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
    std::cout << "--== CHECK SIMEPLCELLPLACER ==--\n";

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(&design, leffile));

    std::ifstream verilogfile("test/files/verilog/nerv_tsmc018.v");
    BOOST_CHECK(verilogfile.good());

    ChipDB::Verilog::Reader::load(&design, verilogfile);    

    auto mod = design.m_moduleLib.lookup("nerv");
    BOOST_CHECK(mod != nullptr);

    LunaCore::SimpleCellPlacer::place(&mod->m_netlist, ChipDB::Rect64{{0,0},{200000,200000}}, 10000);

    auto hpwl = LunaCore::HPWLCalculator::calc(&mod->m_netlist);
    BOOST_CHECK(hpwl > 0);
    std::cout << "  HPWL = " << hpwl << " nm\n";
};


BOOST_AUTO_TEST_SUITE_END()