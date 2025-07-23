// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "testhelpers.h"
#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(VisitorTest)

class NetLoadVisitor : public ChipDB::ConstVisitor
{
public:
    NetLoadVisitor(ChipDB::Netlist &netlist) : m_netlist(netlist), m_capacitance(0), m_inputCount(0) {};

    virtual void visit(const ChipDB::Instance *instance) override {};
    virtual void visit(const ChipDB::Module *mod) override {};
    virtual void visit(const ChipDB::Cell *cell) override {};
    virtual void visit(const ChipDB::PinInfo *pinfo) override {};
    virtual void visit(const ChipDB::Netlist *nl) override {};
    virtual void visit(const ChipDB::LayerInfo *layer) override {};

    virtual void visit(const ChipDB::Net *net) override
    {
        for(auto const& conn : *net)
        {
            if (conn.m_instanceKey == ChipDB::ObjectNotFound)
                continue;

            auto ins = m_netlist.m_instances.at(conn.m_instanceKey);

            if (ins)
            {
                auto pin = ins->getPin(conn.m_pinKey);

                m_capacitance += pin.m_pinInfo->m_cap;

                if (pin.m_pinInfo->isInput())
                    m_inputCount++;
            }
        }
    };

    double m_capacitance;
    size_t m_inputCount;

protected:
    ChipDB::Netlist &m_netlist;
};

BOOST_AUTO_TEST_CASE(iterator_clock_net)
{
    std::cout << "--== VISITOR TEST ==--\n";

    ChipDB::Design design;

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));

    std::ifstream libertyfile("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib");
    BOOST_CHECK(libertyfile.good());
    BOOST_CHECK(ChipDB::Liberty::Reader::load(design, libertyfile));

    std::ifstream verilogfile("test/files/verilog/nerv_tsmc018.v");
    BOOST_CHECK(verilogfile.good());

    ChipDB::Verilog::Reader::load(design, verilogfile);

    auto mod = design.m_moduleLib->lookupModule("nerv");
    BOOST_CHECK(mod.isValid());
    if (!mod.isValid())
        return;

    BOOST_CHECK(mod->m_netlist);

    auto net = mod->m_netlist->m_nets.at("clock");
    BOOST_CHECK(net.isValid());
    if (!net.isValid())
    {
        return;
    }

    NetLoadVisitor v(*mod->m_netlist.get());
    net->accept(&v);
    std::cout << "Clock network capacitive load = " << v.m_capacitance << " Farad\n";
    std::cout << "Clock network inputs          = " << v.m_inputCount << "\n";
    BOOST_CHECK(Helpers::compare(v.m_capacitance, 3.07438e-11));
    BOOST_CHECK(v.m_inputCount = 1101);
}

BOOST_AUTO_TEST_SUITE_END()
