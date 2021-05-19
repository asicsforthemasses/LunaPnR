#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(VerilogParserTest)

BOOST_AUTO_TEST_CASE(can_parse_netlist)
{
    std::cout << "--== VERILOG PARSER ==--\n";
    
    std::ifstream verilogfile("test/files/verilog/adder2.v");
    BOOST_CHECK(verilogfile.good());

    ChipDB::Verilog::Lexer  lexer;
    ChipDB::Verilog::Parser parser;

    std::stringstream src;
    src << verilogfile.rdbuf();

    std::vector<ChipDB::Verilog::Lexer::token> tokens;
    lexer.execute(src.str(), tokens);

    BOOST_CHECK(tokens.size() > 0);
}

BOOST_AUTO_TEST_SUITE_END()