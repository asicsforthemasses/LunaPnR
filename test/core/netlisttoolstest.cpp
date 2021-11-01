#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(NETLISTTOOLSTEST)

BOOST_AUTO_TEST_CASE(check_histogram)
{
    std::cout << "--== CHECK NETLISTTOOLS HISTOGRAM ==--\n";
    
    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(&design, leffile));

    std::ifstream verilogfile("test/files/verilog/adder2.v");
    BOOST_CHECK(verilogfile.good());

    BOOST_CHECK(ChipDB::Verilog::Reader::load(&design, verilogfile));

    // check the design    
    auto mod = design.m_moduleLib.lookup("adder2");
    BOOST_CHECK(mod != nullptr);
    BOOST_CHECK(mod->m_netlist);

    auto histogram = LunaCore::NetlistTools::calcNetlistHistogram(mod->m_netlist.get());

    auto ll = getLogLevel();

    setLogLevel(LOG_VERBOSE);

    for(auto pair : histogram)
    {
        doLog(LOG_VERBOSE, "  size %d -> %d occurences\n", pair.first, pair.second);
    }

    BOOST_CHECK(histogram[2] == 17);
    BOOST_CHECK(histogram[3] == 12);

    setLogLevel(ll);
}

BOOST_AUTO_TEST_CASE(check_histogram_2)
{
    std::cout << "--== CHECK NETLISTTOOLS HISTOGRAM 2 ==--\n";
    
    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(&design, leffile));

    std::ifstream verilogfile("test/files/verilog/femtorv32_quark.v");
    BOOST_CHECK(verilogfile.good());

    BOOST_CHECK(ChipDB::Verilog::Reader::load(&design, verilogfile));

    // check the design    
    auto mod = design.m_moduleLib.lookup("FemtoRV32");
    BOOST_CHECK(mod != nullptr);
    BOOST_CHECK(mod->m_netlist);

    auto histogram = LunaCore::NetlistTools::calcNetlistHistogram(mod->m_netlist.get());

    auto ll = getLogLevel();

    setLogLevel(LOG_VERBOSE);

    for(auto pair : histogram)
    {
        doLog(LOG_VERBOSE, "  size %d -> %d occurences\n", pair.first, pair.second);
    }

    BOOST_CHECK(histogram[2] == 5979);
    BOOST_CHECK(histogram[3] == 450);
    BOOST_CHECK(histogram[1] == 0);     /// sanity check!

    setLogLevel(ll);
}

BOOST_AUTO_TEST_SUITE_END()