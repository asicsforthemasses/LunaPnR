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

BOOST_AUTO_TEST_SUITE(VerilogAssignTest)


BOOST_AUTO_TEST_CASE(can_read_assign_statements)
{
    std::cout << "--== VERILOG ASSIGN TEST ==--\n";
    
    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));

    std::ifstream verilogfile("test/files/verilog/assign_test.v");
    BOOST_CHECK(verilogfile.good());

    BOOST_CHECK(ChipDB::Verilog::Reader::load(design, verilogfile));

    // check the design
    std::cout << "  Found " << design.m_moduleLib->size() << " modules\n";
    BOOST_CHECK(design.m_moduleLib->size() == 1);
    BOOST_CHECK(design.m_moduleLib->lookupModule("testmodule").isValid());
    auto mod = design.m_moduleLib->lookupModule("testmodule");

    BOOST_CHECK(mod.isValid());
    if (!mod.isValid())
    {
        return;
    }

    BOOST_CHECK(mod->m_netlist);

    std::cout << "  module has " << mod->m_netlist->m_instances.size() << " instances:\n";
    BOOST_CHECK(mod->m_netlist->m_instances.size() != 0);

    for(auto ins : mod->m_netlist->m_instances)
    {
        switch(ins->insType())
        {
        case ChipDB::InstanceType::MODULE:
            std::cout << "    module " << ins->name() << "\n";
            break;
        case ChipDB::InstanceType::CELL:
            {
                std::cout << "    cell " << ins->name() << " " << ins->getArchetypeName() << "\tarea: " << ins->getArea() << " um²\n";
                BOOST_CHECK(ins->getArea() >= 0);   // NETCON cells have area 0
            }
            break;
        case ChipDB::InstanceType::PIN:
            {
                std::cout << "    pin  " << ins->name() << " " << ins->getArchetypeName() << "\tarea: " << ins->getArea() << " um²\n";
            }
            break;
        default:
            break;            
        }
    }
    
    std::cout << "  module has " << mod->m_netlist->m_nets.size() << " nets\n";
    BOOST_CHECK(mod->m_netlist->m_nets.size() == 8);
    for(auto net : mod->m_netlist->m_nets)
    {
        std::cout << "    " << net->name() << "\n";
    }

    BOOST_CHECK(mod->m_pins.size() == 8);
    std::cout << "  module pins:\n";
    for(auto pin : mod->m_pins)
    {
        std::cout << "    " << pin->m_name << "\n";
    }

    // check that module pins have a __pin instance in the netlist
    for(auto modPin : mod->m_pins)
    {
        BOOST_CHECK(mod->m_netlist->m_instances.at(modPin->m_name).isValid());
    }

    // write to DOT
    std::ofstream dotfile("test/files/results/verilogassigntest.dot");
    BOOST_CHECK(dotfile.good());
    if (dotfile.good())
    {
        BOOST_CHECK(LunaCore::Dot::Writer::write(dotfile, mod.ptr(), nullptr));
    }
}

BOOST_AUTO_TEST_SUITE_END()
