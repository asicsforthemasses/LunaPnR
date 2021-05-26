#include <limits.h>
#include <memory>
#include <algorithm>
#include "common/logging.h"
#include "celllib/celllib.h"
#include "dotwriter.h"

using namespace LunaCore::Dot;

Writer::Writer(const ChipDB::Module *module) 
    : m_module(module)
{
}


bool Writer::execute(std::ostream &os, AbstractNodeDecorator *decorator)
{
    // write DOT header
    os << "digraph G {\n";
    os << "  rankdir=LR;\n"; 
    //os << "  splines=ortho;\n";
    os << "  node [shape=record style=filled];\n";
    os << "  labelloc=\"t\";\n";
    os << "  fontsize  = 30;\n";
    os << "  label=\"" << escapeString(m_module->m_name) << "\";\n";

    // write all instances as nodes
    for(auto const ins : m_module->m_instances)
    {
        os << "  " << escapeString(ins->m_name) << " [label = \"{{";
        writeInputs(os, ins);
        os << "}|" << escapeString(ins->m_name) << "\\n" << escapeString(ins->getArchetypeName()) << "|{";
        writeOutputs(os, ins);
        os << "}} \";";

        if (decorator != nullptr)
        {
            decorator->decorate(ins, os);
        }

        os << "];\n";
    }

    // write all pins of the module
    for(auto const& pin : m_module->m_pins)
    {
        auto const pinName = pin.m_name;
        os << "  " << escapeString(pinName) << " [shape=\"circle\", label = \""  << pinName << "\"];\n";
    }

    // stream out all the connections
    for(auto const net : m_module->m_nets)
    {   
        if (net == nullptr)
            continue;
    
        // find the driver on the net
        std::string driverName;
        std::string driverPinName;
        ssize_t driverIdx = -1;
        ssize_t idx = 0;
        
        constexpr ssize_t tlpinIdx = SSIZE_MAX;

        for(auto const& conn : net->m_connections)
        {          
            auto pin = conn.m_instance->pin(conn.m_pinIndex);
            if (pin.isValid())
            {
                if (pin.m_info->isOutput())
                {
                    // net driver pin
                    driverPinName = pin.m_info->m_name;
                    driverName = conn.m_instance->m_name;
                    driverIdx  = idx;
                }
            }
            idx++;
        }

        // if no driver was found, perhaps it was a top-level pin?
        if (driverIdx == -1)
        {
            auto pinInfo = m_module->lookupPin(net->m_name);

            // top level inputs are circuit level outputs!
            if (pinInfo->isInput())
            {
                driverPinName = net->m_name;
                driverIdx = tlpinIdx;
            }
        }

        // write out all connections from net driver
        // to the receiving instances

        idx = 0;
        for(auto const& conn : net->m_connections)
        {
            if (driverIdx != idx)
            {            
                auto dstPin = conn.m_instance->pin(conn.m_pinIndex);
                if (dstPin.isValid())
                {
                    if (driverIdx != tlpinIdx)
                    {
                        // non top-level pin driver                        
                        os << "  " << escapeString(driverName) << ":" << escapeString(driverPinName) << "->";
                    }
                    else
                    {
                        // top-level pin driver
                        os << "  " << escapeString(driverPinName) << "->";
                    }

                    auto dstIns = dstPin.m_info->m_name;
                    os << escapeString(conn.m_instance->m_name) << ":" << escapeString(dstPin.m_info->m_name) << ";\n";
                }
            }

            idx++;
        }

        // check if the net is a top-level net
        // and make a connection from the net driver to the top-level pin
        if ((driverIdx != tlpinIdx) && (driverIdx != -1) && net->m_isPortNet)
        {
            os << "  " << escapeString(driverName) << ":" << escapeString(driverPinName) << "->";
            os << escapeString(net->m_name) << ";\n";
        }
    }

    os << "\n}\n";
    return true;
}

bool Writer::write(const ChipDB::Instance *modInstance,
        std::ostream &os, AbstractNodeDecorator *decorator)
{
    if (modInstance == nullptr)
    {
        doLog(LOG_ERROR,"Dot::Writer module instance is nullptr\n");
        return false;        
    }

    if (!modInstance->isModule())
    {
        doLog(LOG_ERROR,"Dot::Writer instance is not a module\n");
        return false;                
    }

    auto module = dynamic_cast<const ChipDB::Module*>(modInstance->cell());

    if (module == nullptr)
    {
        doLog(LOG_ERROR,"Dot::Writer module is nullptr\n");
        return false;
    }

    std::unique_ptr<Writer> writer(new Writer(module));
    return writer->execute(os, decorator);
}

bool Writer::write(const ChipDB::Module *module,
        std::ostream &os, AbstractNodeDecorator *decorator)
{
    if (module == nullptr)
    {
        doLog(LOG_ERROR,"Dot::Writer module is nullptr\n");
        return false;
    }

    std::unique_ptr<Writer> writer(new Writer(module));
    return writer->execute(os, decorator);
}

void Writer::writeInputs(std::ostream &os, const ChipDB::Instance *ins)
{
    if (ins == nullptr)
        return;

    bool first = true;
    for(auto pin : *ins)
    {
        // skip power/ground pins
        if (pin.m_info->isPGPin())
            continue;

        if (pin.m_info->isInput())
        {
            if (!first)
                os << "|";

            first = false;
            os << "<" << escapeString(pin.m_info->m_name) << "> " << escapeString(pin.m_info->m_name);
        }
    }
}

void Writer::writeOutputs(std::ostream &os, const ChipDB::Instance *ins)
{
    if (ins == nullptr)
        return;

    bool first = true;
    for(auto pin : *ins)
    {
        // skip power/ground pins
        if (pin.m_info->isPGPin())
            continue;

        if (pin.m_info->isOutput())
        {
            if (!first)
                os << "|";

            first = false;
            os << "<" << escapeString(pin.m_info->m_name) << "> " << escapeString(pin.m_info->m_name);

        }
    }
}

std::string Writer::escapeString(const std::string &txt)
{
    std::string result;
    for(auto c : txt)
    {
        if (c == '[')
        {
            result += "__";
        }
        else if (c == ']')
        {
            result += "__";
        }
        else
        {
            result += c;
        }    
    }
    return result;
}
