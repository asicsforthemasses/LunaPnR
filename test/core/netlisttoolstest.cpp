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
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));

    std::ifstream verilogfile("test/files/verilog/adder2.v");
    BOOST_CHECK(verilogfile.good());

    BOOST_CHECK(ChipDB::Verilog::Reader::load(design, verilogfile));

    // check the design    
    auto mod = design.m_moduleLib->lookupModule("adder2");
    BOOST_CHECK(mod.isValid());
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
    std::ifstream leffile2("test/files/iit_stdcells_extra/fake_ties018.lef");
    BOOST_CHECK(leffile.good());
    BOOST_CHECK(leffile2.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile2));

    std::ifstream verilogfile("test/files/verilog/femtorv32_quark.v");
    BOOST_CHECK(verilogfile.good());

    BOOST_CHECK(ChipDB::Verilog::Reader::load(design, verilogfile));

    // check the design    
    auto mod = design.m_moduleLib->lookupModule("FemtoRV32");
    BOOST_CHECK(mod.isValid());
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
    BOOST_CHECK(histogram[1] == 186);   // assign nets that are just for naming aliases.

    size_t degenerateCount = 0;
    for(auto net : mod->m_netlist->m_nets)
    {
        if (net->numberOfConnections() == 1)
        {
            doLog(LOG_INFO, "  Degenerate net: %s\n", net->name().c_str());
            degenerateCount++;
        }
        if (degenerateCount == 10)
        {
            doLog(LOG_INFO, "  skipping other nets..\n");
            break;
        }
    }

    setLogLevel(ll);
}

BOOST_AUTO_TEST_CASE(remove_netcons)
{
    std::cout << "--== CHECK REMOVE NETCONS ==--\n";
    std::ifstream verilogfile("test/files/verilog/netcon_test.v");
    BOOST_CHECK(verilogfile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::Verilog::Reader::load(design, verilogfile));

    // check the design    
    auto mod = design.m_moduleLib->lookupModule("netcon");
    BOOST_CHECK(mod.isValid());
    BOOST_CHECK(mod->m_netlist);    

    auto ll = getLogLevel();

    setLogLevel(LOG_VERBOSE);

    //FIXME:
    //LunaCore::NetlistTools::removeNetconInstances(*mod->m_netlist.get());
    setLogLevel(ll);
}

BOOST_AUTO_TEST_SUITE_END()