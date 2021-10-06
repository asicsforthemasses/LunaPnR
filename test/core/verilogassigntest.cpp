#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(VerilogAssignTest)


BOOST_AUTO_TEST_CASE(can_read_assign_statements)
{
    std::cout << "--== VERILOG ASSIGN TEST ==--\n";
    
    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(&design, leffile));

    std::ifstream verilogfile("test/files/verilog/assign_test.v");
    BOOST_CHECK(verilogfile.good());

    BOOST_CHECK(ChipDB::Verilog::Reader::load(&design, verilogfile));

    // check the design
    std::cout << "  Found " << design.m_moduleLib.size() << " modules\n";
    BOOST_CHECK(design.m_moduleLib.size() == 1);
    BOOST_CHECK(design.m_moduleLib.lookup("testmodule") != nullptr);
    auto mod = design.m_moduleLib.lookup("testmodule");
    if (mod == nullptr)
        return;

    BOOST_CHECK(mod->m_netlist);

    std::cout << "  module has " << mod->m_netlist->m_instances.size() << " instances:\n";
    BOOST_CHECK(mod->m_netlist->m_instances.size() != 0);

    for(auto ins : mod->m_netlist->m_instances)
    {
        switch(ins->m_insType)
        {
        case ChipDB::InstanceBase::INS_MODULE:
            std::cout << "    module " << ins->m_name << "\n";
            break;
        case ChipDB::InstanceBase::INS_CELL:
            {
                std::cout << "    cell " << ins->m_name << " " << ins->getArchetypeName() << "\tarea: " << ins->getArea() << " um²\n";
                BOOST_CHECK(ins->getArea() >= 0);   // NETCON cells have area 0
            }
            break;
        case ChipDB::InstanceBase::INS_PIN:
            {
                std::cout << "    pin  " << ins->m_name << " " << ins->getArchetypeName() << "\tarea: " << ins->getArea() << " um²\n";
            }
            break;
        default:
            break;            
        }
    }
    
    std::cout << "  module has " << mod->m_netlist->m_nets.size() << " nets\n";
    BOOST_CHECK(mod->m_netlist->m_nets.size() == 8);
    for(auto const net : mod->m_netlist->m_nets)
    {
        std::cout << "    " << net->m_name << "\n";
    }

    BOOST_CHECK(mod->m_pins.size() == 8);
    std::cout << "  module pins:\n";
    for(auto const &pin : mod->m_pins)
    {
        std::cout << "    " << pin.m_name << "\n";
    }

    // check that module pins have a __pin instance in the netlist
    for(auto const& modPin : mod->m_pins)
    {
        BOOST_CHECK(mod->m_netlist->m_instances.lookup(modPin.m_name) != nullptr);
    }

    // write to DOT
    std::ofstream dotfile("test/files/results/verilogassigntest.dot");
    BOOST_CHECK(dotfile.good());
    if (dotfile.good())
    {
        BOOST_CHECK(LunaCore::Dot::Writer::write(dotfile, mod, nullptr));
    }
}

BOOST_AUTO_TEST_SUITE_END()
