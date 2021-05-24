#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(VerilogReaderTest)


BOOST_AUTO_TEST_CASE(can_read_netlist)
{
    std::cout << "--== VERILOG NETLIST READER ==--\n";
    
    std::ifstream verilogfile("test/files/verilog/adder2.v");
    BOOST_CHECK(verilogfile.good());

    //std::stringstream src;
    //src << verilogfile.rdbuf();

    ChipDB::Design design;
    ChipDB::Verilog::Reader::load(&design, verilogfile);
}

BOOST_AUTO_TEST_SUITE_END()
