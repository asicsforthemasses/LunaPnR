#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(DotWriterTest)

BOOST_AUTO_TEST_CASE(can_write_netlist_to_dot)
{
    std::cout << "--== DOT WRITER ==--\n";

    ChipDB::Design design;

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());    
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));

    std::ifstream libertyfile("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib");
    BOOST_CHECK(libertyfile.good());    
    BOOST_CHECK(ChipDB::Liberty::Reader::load(design, libertyfile));

    // dump FAX1 cell 
    auto cellKeyObjPair = design.m_cellLib.lookupCell("FAX1");
    BOOST_CHECK(cellKeyObjPair.isValid());

    for(auto pinInfo : cellKeyObjPair->m_pins)
    {
        std::cout << "  " << pinInfo->m_name << " " << toString(pinInfo->m_iotype) << "\n";
    }

    std::ifstream verilogfile("test/files/verilog/adder8.v");
    BOOST_CHECK(verilogfile.good());

    BOOST_CHECK(ChipDB::Verilog::Reader::load(design, verilogfile));

    auto modulePtr = design.m_moduleLib.lookupModule("adder8");
    BOOST_CHECK(modulePtr.isValid());

    std::ofstream ofile("test/files/results/adder8.dot");
    BOOST_CHECK(ofile.good());

    LunaCore::Dot::Writer::write(ofile, modulePtr.ptr());
}

BOOST_AUTO_TEST_SUITE_END()
