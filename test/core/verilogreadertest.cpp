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
    
    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(&design, leffile));

    std::ifstream verilogfile("test/files/verilog/adder2.v");
    BOOST_CHECK(verilogfile.good());

    ChipDB::Verilog::Reader::load(&design, verilogfile);

    // check the design
    std::cout << "  Found " << design.m_moduleLib.size() << " modules\n";
    BOOST_CHECK(design.m_moduleLib.size() == 1);
    BOOST_CHECK(design.m_moduleLib.lookup("adder2") != nullptr);
    
    auto mod = design.m_moduleLib.lookup("adder2");

    std::cout << "  module has " << mod->m_instances.size() << " instances:\n";
    for(auto ins : mod->m_instances)
    {
        if (ins->isModule())
        {
            auto modIns = dynamic_cast<ChipDB::ModuleInstance*>(ins);
            std::cout << "    module " << modIns->m_insName << "\n";
        }
        else
        {
            auto cellIns = dynamic_cast<ChipDB::CellInstance*>(ins);
            std::cout << "    cell " << cellIns->m_insName << " " << cellIns->getCell()->m_name << "\tarea: " << cellIns->getCell()->m_area << " um²\n";
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
