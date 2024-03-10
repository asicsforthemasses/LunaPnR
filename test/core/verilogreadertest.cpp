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

BOOST_AUTO_TEST_SUITE(VerilogReaderTest)


BOOST_AUTO_TEST_CASE(can_read_netlist)
{
    std::cout << "--== VERILOG NETLIST READER ==--\n";

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));

    std::ifstream verilogfile("test/files/verilog/adder2.v");
    BOOST_CHECK(verilogfile.good());

    BOOST_CHECK(ChipDB::Verilog::Reader::load(design, verilogfile));

    // check the design
    std::cout << "  Found " << design.m_moduleLib->size() << " modules\n";
    BOOST_CHECK(design.m_moduleLib->size() == 1);
    BOOST_CHECK(design.m_moduleLib->lookupModule("adder2").isValid());

    auto mod = design.m_moduleLib->lookupModule("adder2");

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
                BOOST_CHECK(ins->getArea() >= 0);    // NETCON cells have area 0
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
    BOOST_CHECK(mod->m_netlist->m_nets.size() == 29);
    for(auto const net : mod->m_netlist->m_nets)
    {
        std::cout << "    " << net->name() << "\n";
    }

    BOOST_CHECK(mod->m_pins.size() == 8);
    std::cout << "  module pins:\n";
    for(auto pin : mod->m_pins)
    {
        std::cout << "    " << pin->name() << "\n";
    }

    // check that module pins have a __pin instance in the netlist
    for(auto modPin : mod->m_pins)
    {
        BOOST_CHECK(mod->m_netlist->m_instances.at(modPin->m_name).isValid());
    }
}

BOOST_AUTO_TEST_CASE(can_read_multiplier)
{
    std::cout << "--== VERILOG NETLIST READER MULTIPLIER ==--\n";

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_REQUIRE(leffile.good());

    ChipDB::Design design;
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(design, leffile));

    std::ifstream verilogfile("test/files/verilog/multiplier.v");
    BOOST_REQUIRE(verilogfile.good());

    BOOST_REQUIRE(ChipDB::Verilog::Reader::load(design, verilogfile));

    // check the design
    std::cout << "  Found " << design.m_moduleLib->size() << " modules\n";
    BOOST_CHECK(design.m_moduleLib->size() == 1);
    BOOST_CHECK(design.m_moduleLib->lookupModule("multiplier").isValid());

    auto mod = design.m_moduleLib->lookupModule("multiplier");
    if (mod.isValid())
    {
        std::cout << "  module has " << mod->m_netlist->m_instances.size() << " instances\n";
        BOOST_CHECK(mod->m_netlist->m_instances.size() != 0);

        std::cout << "  module has " << mod->m_netlist->m_nets.size() << " nets\n";
        BOOST_CHECK(mod->m_netlist->m_nets.size() != 0);

        std::cout << "  module has " << mod->m_pins.size() << " pins\n";
        BOOST_CHECK(mod->m_pins.size() != 0);

        // check that module pins have a __pin instance in the netlist
        for(auto modPin : mod->m_pins)
        {
            if (!mod->m_netlist->m_instances.at(modPin->m_name).isValid())
            {
                std::cout << "  missing pin instance for pin '" << modPin->name() << "'\n";
            }
            BOOST_CHECK(mod->m_netlist->m_instances.at(modPin->m_name).isValid());
        }

        // determine cell area
        double area = 0;
        for(auto ins : mod->m_netlist->m_instances)
        {
            area += ins->getArea();
        }

        std::cout << "  module area " << area << " um²\n";
    }

    // check if all instances have all pins connected
    std::size_t unconnectedPins = 0;
    auto const& netlist = mod->m_netlist;
    for(auto ins : netlist->m_instances)
    {
        int pinIndex = 0;
        for(auto connection : ins->connections())
        {
            auto checkPin = ins->getPin(pinIndex);
            if ((checkPin.m_pinInfo) && (checkPin.m_pinInfo->isPGPin())) continue;    // skip power and ground pins.

            if (connection == ChipDB::ObjectNotFound)
            {
                unconnectedPins++;
                Logging::doLog(Logging::LogType::ERROR, Logging::fmt("Pin with index %d (name is %s) on instance %s is unconnected\n",
                    pinIndex, checkPin.name().c_str(), ins->name().c_str()));
            }
            pinIndex++;
        }
    }
    BOOST_CHECK(unconnectedPins == 0);
}

BOOST_AUTO_TEST_CASE(can_read_nerv32)
{
    std::cout << "--== VERILOG NETLIST READER NERV ==--\n";

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_REQUIRE(leffile.good());

    ChipDB::Design design;
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(design, leffile));

    std::ifstream verilogfile("test/files/verilog/nerv_tsmc018.v");
    BOOST_REQUIRE(verilogfile.good());

    BOOST_REQUIRE(ChipDB::Verilog::Reader::load(design, verilogfile));

    // check the design
    std::cout << "  Found " << design.m_moduleLib->size() << " modules\n";
    BOOST_CHECK(design.m_moduleLib->size() == 1);
    BOOST_CHECK(design.m_moduleLib->lookupModule("nerv").isValid());

    auto mod = design.m_moduleLib->lookupModule("nerv");
    BOOST_REQUIRE(mod.isValid());

    std::cout << "  module has " << mod->m_netlist->m_instances.size() << " instances\n";
    BOOST_CHECK(mod->m_netlist->m_instances.size() != 0);

    std::cout << "  module has " << mod->m_netlist->m_nets.size() << " nets\n";
    BOOST_CHECK(mod->m_netlist->m_nets.size() != 0);

    std::cout << "  module has " << mod->m_pins.size() << " pins\n";
    BOOST_CHECK(mod->m_pins.size() != 0);

    // check that module pins have a __pin instance in the netlist
    for(auto modPin : mod->m_pins)
    {
        BOOST_CHECK(mod->m_netlist->m_instances.at(modPin->name()).isValid());
    }

    // determine cell area
    double area = 0;
    for(auto ins : mod->m_netlist->m_instances)
    {
        area += ins->getArea();
    }

    std::cout << "  module area " << area << " um²\n";

    auto netlist = mod->m_netlist;

    // check that there is a clk instance
    auto clkins = netlist->lookupInstance("clock");
    BOOST_CHECK(clkins.isValid());

    // check that this has one output pin.
    BOOST_CHECK(clkins->getNumberOfPins() == 1);
    auto pin = clkins->getPin(0);

    BOOST_REQUIRE(pin.isValid());
    BOOST_CHECK(pin.m_pinInfo->isOutput());

    // check that the clk pin is connected to a net
    BOOST_CHECK(pin.netKey() != ChipDB::ObjectNotFound);

    auto net = netlist->lookupNet(pin.netKey());
    BOOST_REQUIRE(net);

    std::cout << "  clk net has " << net->numberOfConnections() << " connections\n";
    BOOST_CHECK(net->numberOfConnections() > 1);
}

BOOST_AUTO_TEST_CASE(can_read_picorv32)
{
    std::cout << "--== VERILOG NETLIST READER PICORV32 ==--\n";

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_REQUIRE(leffile.good());

    std::ifstream leffile2("test/files/iit_stdcells_extra/fake_ties018.lef");
    BOOST_REQUIRE(leffile2.good());

    ChipDB::Design design;
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(design, leffile));
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(design, leffile2));

    std::ifstream verilogfile("test/files/verilog/picorv32.v");
    BOOST_REQUIRE(verilogfile.good());

    BOOST_REQUIRE(ChipDB::Verilog::Reader::load(design, verilogfile));

    // check the design
    std::cout << "  Found " << design.m_moduleLib->size() << " modules\n";
    BOOST_CHECK(design.m_moduleLib->size() == 1);
    BOOST_CHECK(design.m_moduleLib->lookupModule("picorv32").isValid());

    auto mod = design.m_moduleLib->lookupModule("picorv32");
    BOOST_REQUIRE(mod.isValid());

    BOOST_CHECK(mod->m_netlist.get() != nullptr);

    std::cout << "  module has " << mod->m_netlist->m_instances.size() << " instances\n";
    BOOST_CHECK(mod->m_netlist->m_instances.size() != 0);

    std::cout << "  module has " << mod->m_netlist->m_nets.size() << " nets\n";
    BOOST_CHECK(mod->m_netlist->m_nets.size() != 0);

    std::cout << "  module has " << mod->m_pins.size() << " pins\n";
    BOOST_CHECK(mod->m_pins.size() != 0);

    // check that module pins have a __pin instance in the netlist
    for(auto modPin : mod->m_pins)
    {
        BOOST_CHECK(mod->m_netlist->m_instances.at(modPin->m_name).isValid());
    }

    // determine cell area
    double area = 0;
    for(auto ins : mod->m_netlist->m_instances)
    {
        area += ins->getArea();
    }

    std::cout << "  module area " << area << " um²\n";

    auto netlist = mod->m_netlist;

    // check that there is a clk instance
    auto clkins = netlist->lookupInstance("clk");
    BOOST_CHECK(clkins.isValid());

    // check that this has one output pin.
    BOOST_CHECK(clkins->getNumberOfPins() == 1);
    auto pin = clkins->getPin(0);

    BOOST_REQUIRE(pin.isValid());
    BOOST_CHECK(pin.m_pinInfo->isOutput());

    // check that the clk pin is connected to a net
    BOOST_CHECK(pin.netKey() != ChipDB::ObjectNotFound);

    auto net = netlist->lookupNet(pin.netKey());
    BOOST_REQUIRE(net);

    std::cout << "  clk net has " << net->numberOfConnections() << " connections\n";
    BOOST_CHECK(net->numberOfConnections() > 1);
}

BOOST_AUTO_TEST_CASE(can_read_FemtoRV32)
{
    std::cout << "--== VERILOG NETLIST READER FemtoRV32 ==--\n";

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_REQUIRE(leffile.good());

    std::ifstream leffile2("test/files/iit_stdcells_extra/fake_ties018.lef");
    BOOST_REQUIRE(leffile2.good());

    ChipDB::Design design;
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(design, leffile));
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(design, leffile2));

    std::ifstream verilogfile("test/files/verilog/femtorv32_quark.v");
    BOOST_REQUIRE(verilogfile.good());

    BOOST_REQUIRE(ChipDB::Verilog::Reader::load(design, verilogfile));

    // check the design
    std::cout << "  Found " << design.m_moduleLib->size() << " modules\n";
    BOOST_CHECK(design.m_moduleLib->size() == 1);
    BOOST_CHECK(design.m_moduleLib->lookupModule("FemtoRV32").isValid());

    auto mod = design.m_moduleLib->lookupModule("FemtoRV32");
    BOOST_REQUIRE(mod.isValid());

    BOOST_CHECK(mod->m_netlist.get() != nullptr);

    std::cout << "  module has " << mod->m_netlist->m_instances.size() << " instances\n";
    BOOST_CHECK(mod->m_netlist->m_instances.size() != 0);

    std::cout << "  module has " << mod->m_netlist->m_nets.size() << " nets\n";
    BOOST_CHECK(mod->m_netlist->m_nets.size() != 0);

    std::cout << "  module has " << mod->m_pins.size() << " pins\n";
    BOOST_CHECK(mod->m_pins.size() != 0);

    // check that module pins have a __pin instance in the netlist
    for(auto modPin : mod->m_pins)
    {
        BOOST_CHECK(mod->m_netlist->m_instances.at(modPin->m_name).isValid());
    }

    // determine cell area
    double area = 0;
    for(auto ins : mod->m_netlist->m_instances)
    {
        area += ins->getArea();
    }

    std::cout << "  module area " << area << " um²\n";

    auto netlist = mod->m_netlist;

    // check that there is a clk instance
    auto clkins = netlist->lookupInstance("clk");
    BOOST_CHECK(clkins.isValid());

    // check that this has one output pin.
    BOOST_CHECK(clkins->getNumberOfPins() == 1);
    auto pin = clkins->getPin(0);

    BOOST_REQUIRE(pin.isValid());
    BOOST_CHECK(pin.m_pinInfo->isOutput());

    // check that the clk pin is connected to a net
    BOOST_CHECK(pin.netKey() != ChipDB::ObjectNotFound);

    auto net = netlist->lookupNet(pin.netKey());
    BOOST_REQUIRE(net);

    std::cout << "  clk net has " << net->numberOfConnections() << " connections\n";
    BOOST_CHECK(net->numberOfConnections() > 1);

    // check that the clock network is connected to the pin instance
    auto const insKey = clkins.key();
    auto iter = std::find_if(net->begin(), net->end(),
        [insKey](auto const netconnect)
        {
            return (netconnect.m_instanceKey == insKey);
        }
    );

    BOOST_CHECK_MESSAGE(iter != net->end(), "pin instance was not found on clock net..");

    auto driver = netlist->lookupInstance(iter->m_instanceKey);
    BOOST_REQUIRE(driver);

    std::cout << "  clk source: " << driver->name() << " archetype: " << driver->getArchetypeName() << "\n";
    std::cout << "         pin: " << ChipDB::toString(driver->getPin(0).m_pinInfo->m_iotype) << "\n";
}


BOOST_AUTO_TEST_SUITE_END()
