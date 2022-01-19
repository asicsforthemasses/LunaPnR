#include <climits>
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
    os << "  label=\"" << escapeString(m_module->name()) << "\";\n";

    if (!m_module->m_netlist)
    {
        doLog(LOG_ERROR, "Cannot write netlist of a black box");
        return false;
    }

    // write all instances as nodes
    for(auto const insKeyObjPair : m_module->m_netlist->m_instances)
    {
        if (insKeyObjPair->m_insType == ChipDB::InstanceType::PIN)
        {
            os << "  " << escapeString(insKeyObjPair->name()) << R"( [shape="circle", label = ")"  << insKeyObjPair->name() << "\"];\n";
        }
        else
        {
            os << "  " << escapeString(insKeyObjPair->name()) << " [label = \"{{";
            writeInputs(os, insKeyObjPair.ptr());
            os << "}|" << escapeString(insKeyObjPair->name()) << "\\n" << escapeString(insKeyObjPair->getArchetypeName()) << "|{";
            writeOutputs(os, insKeyObjPair.ptr());
            os << "}} \";";
            if (decorator != nullptr)
            {
                decorator->decorate(insKeyObjPair.ptr(), os);
            }

            os << "];\n";            
        }
    }

    // stream out all the connections
    for(auto const net : m_module->m_netlist->m_nets)
    {   
        if (net == nullptr)
            continue;
    
        // find the driver on the net
        std::string driverName;
        std::string driverPinName;
        ChipDB::InstanceType driverInsType = ChipDB::InstanceType::ABSTRACT;

        ssize_t driverIdx = -1;
        ssize_t idx = 0;        
        for(auto const& conn : net->m_connections)
        {    
            auto pinInfo = conn.m_instance->getPinInfo(conn.m_pinIndex);
            if (pinInfo != nullptr)
            {
                if (pinInfo->isOutput())
                {
                    // net driver pin
                    driverPinName = pinInfo->m_name;
                    driverName    = conn.m_instance->m_name;
                    driverInsType = conn.m_instance->m_insType;
                    driverIdx  = idx;
                }
            }
            else
            {
                doLog(LOG_ERROR,"Dot::Writer: pinInfo == nullptr on instance %s %s\n",
                    conn.m_instance->m_name.c_str(), conn.m_instance->getArchetypeName().c_str());
                return false;
            }
            idx++;
        }

        if (driverName.empty())
        {
            doLog(LOG_WARN,"Dot::Writer: no driver name found for net %s\n", net->m_name.c_str());
        }

        // write out all connections from net driver
        // to the receiving instances

        idx = 0;
        for(auto const& conn : net->m_connections)
        {
            if (driverIdx != idx)
            {            
                auto dstPinInfo = conn.m_instance->getPinInfo(conn.m_pinIndex);
                if (dstPinInfo != nullptr)
                {
                    if (driverInsType != ChipDB::InstanceType::PIN)
                    {
                        os << "  " << escapeString(driverName) << ":" << escapeString(driverPinName) << "->";
                    }
                    else
                    {
                        os << "  " << escapeString(driverPinName) << "->";
                    }

                    if (conn.m_instance->m_insType != ChipDB::InstanceType::PIN)
                        os << escapeString(conn.m_instance->m_name) << ":" << escapeString(dstPinInfo->m_name) << ";\n";
                    else
                        os << escapeString(conn.m_instance->m_name) << ";\n";
                }
            }

            idx++;
        }

#if 0
        // check if the net is a top-level net
        // and make a connection from the net driver to the top-level pin
        if ((driverIdx != tlpinIdx) && (driverIdx != -1) && net->m_isPortNet)
        {
            os << "  " << escapeString(driverName) << ":" << escapeString(driverPinName) << "->";
            os << escapeString(net->m_name) << ";\n";
        }
#endif
    }

    os << "\n}\n";
    return true;
}

bool Writer::write(std::ostream &os, const std::shared_ptr<ChipDB::Instance> modInstance,
    AbstractNodeDecorator *decorator)
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

    auto cell   = modInstance->cell();
    auto module = dynamic_cast<ChipDB::Module*>(cell.get());

    if (module == nullptr)
    {
        doLog(LOG_ERROR,"Dot::Writer module is nullptr\n");
        return false;
    }

    std::unique_ptr<Writer> writer(new Writer(module));
    return writer->execute(os, decorator);
}

bool Writer::write(std::ostream &os, const std::shared_ptr<ChipDB::Module> module,
    AbstractNodeDecorator *decorator)
{
    if (module == nullptr)
    {
        doLog(LOG_ERROR,"Dot::Writer module is nullptr\n");
        return false;
    }

    std::unique_ptr<Writer> writer(new Writer(module));
    return writer->execute(os, decorator);
}

void Writer::writeInputs(std::ostream &os, const std::shared_ptr<ChipDB::InstanceBase> ins)
{
    if (ins == nullptr)
        return;

    bool first = true;
    for(ChipDB::PinObjectKey pinKey=0; pinKey < ins->getNumberOfPins(); ++pinKey)
    {
        auto const& pin = ins->getPin(pinKey);

        if (!pin.isValid())
            continue;

        // skip power/ground pins
        if (pin.m_pinInfo->isPGPin())
            continue;

        if (pin.m_pinInfo->isInput())
        {
            if (!first)
                os << "|";

            first = false;
            os << "<" << escapeString(pin.name()) << "> " << escapeString(pin.name());
        }
    }
}

void Writer::writeOutputs(std::ostream &os, const std::shared_ptr<ChipDB::InstanceBase> ins)
{
    if (!ins)
        return;

    bool first = true;
    for(ChipDB::PinObjectKey pinKey=0; pinKey < ins->getNumberOfPins(); ++pinKey)
    {
        auto const& pin = ins->getPin(pinKey);
        
        if (!pin.isValid())
            continue;

        // skip power/ground pins        
        if (pin.m_pinInfo->isPGPin())
            continue;

        if (pin.m_pinInfo->isOutput())
        {
            if (!first)
                os << "|";

            first = false;
            os << "<" << escapeString(pin.name()) << "> " << escapeString(pin.name());
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
