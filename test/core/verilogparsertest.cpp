// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(VerilogParserTest)

class CustomParser : public ChipDB::Verilog::Parser
{
public:
    CustomParser() : m_insCount(0), m_outputCount(0), m_inputCount(0), m_wireCount(0), m_modCount(0) {}

    virtual void onModule(const std::string &modName,
        const std::vector<std::string> &ports) override
    {
        m_modCount++;
    };

    virtual void onInstance(const std::string &modname, const std::string &insname) override
    {
        m_insCount++;
    };

    /** callback for each wire in the netlist with [start:stop] range */
    virtual void onWire(const std::string &netname, uint32_t start, uint32_t stop) override
    {
        m_wireCount += std::max(start, stop) - std::min(start, stop);
    };

    /** callback for each wire in the netlist that does not have a range */
    virtual void onWire(const std::string &netname) override
    {
        m_wireCount++;
    };

    /** callback for each module input port with optional [start:stop] range */
    virtual void onInput(const std::string &netname, uint32_t start, uint32_t stop) override
    {
        m_inputCount += std::max(start, stop) - std::min(start, stop);
    };

    /** callback for each module single wire input port */
    virtual void onInput(const std::string &netname) override
    {
        m_inputCount++;
    };

    /** callback for each module output port with optional [start:stop] range */
    virtual void onOutput(const std::string &netname, uint32_t start, uint32_t stop) override
    {
        m_outputCount += std::max(start, stop) - std::min(start, stop);
    };

    /** callback for each module single wire output port */
    virtual void onOutput(const std::string &netname) override
    {
        m_outputCount++;
    };


    uint32_t m_insCount;
    uint32_t m_outputCount;
    uint32_t m_inputCount;
    uint32_t m_wireCount;
    uint32_t m_modCount;
};

BOOST_AUTO_TEST_CASE(can_parse_netlist)
{
    std::cout << "--== VERILOG PARSER ==--\n";

    std::ifstream verilogfile("test/files/verilog/adder2.v");
    BOOST_CHECK(verilogfile.good());

    ChipDB::Verilog::Lexer  lexer;
    CustomParser parser;

    std::stringstream src;
    src << verilogfile.rdbuf();

    std::vector<ChipDB::Verilog::Lexer::token> tokens;
    lexer.execute(src.str(), tokens);
    BOOST_CHECK(tokens.size() > 0);

    BOOST_CHECK(parser.execute(tokens));

    BOOST_CHECK(parser.m_insCount == 24);
    BOOST_CHECK(parser.m_inputCount == 5);
    BOOST_CHECK(parser.m_outputCount == 3);
    BOOST_CHECK(parser.m_wireCount == 21);
    BOOST_CHECK(parser.m_modCount == 1);
}

BOOST_AUTO_TEST_SUITE_END()