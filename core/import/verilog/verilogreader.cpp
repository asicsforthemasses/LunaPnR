/*
    Copyright (c) 2020,2021 Niels Moseley <n.a.moseley@moseleyinstruments.com>

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
    
*/


#include <sstream>
#include <algorithm>
#include <assert.h>
#include "common/logging.h"
#include "netlist/instance.h"
#include "verilogreader.h"

using namespace ChipDB::Verilog;

constexpr static bool extraDebug = false;

bool Reader::load(Design *design, std::istream &source)
{
    try
    {
        std::stringstream src;
        src << source.rdbuf();

        Lexer lex;
        std::vector<Lexer::token> tokens;
        if (lex.execute(src.str(), tokens))
        {
            ReaderImpl parser(design);
            if (parser.execute(tokens))
            {
                doLog(LOG_INFO,"Verilog netlist parsed.\n");
                doLog(LOG_INFO,"  modules %d\n", design->m_moduleLib.size());
                return true;
            }
        }
    }
    catch(std::runtime_error &e)
    {
        doLog(LOG_ERROR,"%s\n", e.what());
    }
    catch(std::exception const& e)
    {
        doLog(LOG_ERROR,"Unexpected exception %s\n", e.what());
    }
    catch(...)
    {
        doLog(LOG_ERROR,"Unexpected exception\n");
    }

    doLog(LOG_ERROR,"Verilog::Reader failed to load netlist.\n");
    return false;
}


// **********************************************************************
//   ReaderImpl
// **********************************************************************

ReaderImpl::ReaderImpl(Design *design) : 
    m_design(design), 
    m_currentModule{nullptr}, 
    m_currentInstance{nullptr}
{
}

void ReaderImpl::throwOnModuleIsNullptr()
{
    if (m_currentModule == nullptr)
    {
        throw std::runtime_error("Reader: no module selected.");
    }
}

void ReaderImpl::throwOnDesignIsNullptr()
{
    if (m_design == nullptr)
    {
        throw std::runtime_error("Reader: m_design is nullptr.");
    }    
}

void ReaderImpl::throwOnCurInstanceIsNullptr()
{
    if (m_currentInstance == nullptr)
    {
        throw std::runtime_error("Reader: no instance selected.");
    }    
}


void ReaderImpl::onModule(const std::string &modName,
        const std::vector<std::string> &ports)
{
    throwOnDesignIsNullptr();
    
    m_currentModule = m_design->createModule(modName);
    throwOnModuleIsNullptr();

    // we need to create our ports here.
    // this will ensure the correct ordering
    // rather than relying on the 
    // correct input/output order in the verilog file.
    //

    for(auto const& port: ports)
    {
        m_currentModule->createPin(port);
    }
}

void ReaderImpl::onInstance(const std::string &modName, const std::string &insName)
{
    throwOnDesignIsNullptr();
    throwOnModuleIsNullptr();

    // check for cell or module instance
    auto const cellPtr = m_design->m_cellLib.lookup(modName);
    if (cellPtr != nullptr)
    {           
        auto insPtr = new Instance(cellPtr);
        insPtr->m_name = insName;

        if (!m_currentModule->addInstance(insName, insPtr))
        {            
            std::stringstream ss;
            ss << "Failed to create instance " << insName << "\n";
            doLog(LOG_ERROR, ss.str());
        }

        m_currentInstance = insPtr;

        return;
    }
    
    auto modulePtr = m_design->m_moduleLib.lookup(modName);
    if (modulePtr != nullptr)
    {
        auto insPtr = new Instance(modulePtr);
        insPtr->m_name = modName;
        
        if (!m_currentModule->addInstance(insName, insPtr))
        {
            std::stringstream ss;
            ss << "Failed to create module instance " << insName << "\n";
            doLog(LOG_ERROR, ss.str());            
        }

        m_currentInstance = insPtr;

        return;
    }

    // neither a cell or a module could found -> error
    doLog(LOG_ERROR,"Cannot find cell/module '%s' in cell database\n", modName.c_str());
    return;
}

void ReaderImpl::onWire(const std::string &netname, uint32_t start, uint32_t stop)
{
    throwOnDesignIsNullptr();
    throwOnModuleIsNullptr();

    // This callback is only for busses.
    // Here, we expand all busses into individual wires

    if (stop < start)
        std::swap(start,stop);

    for(uint32_t i=start; i<=stop; i++)
    {
        std::stringstream ss;
        ss << netname << "[" << i << "]";
        m_currentModule->createNet(netname);        
    }
}

void ReaderImpl::onWire(const std::string &netname)
{
    throwOnDesignIsNullptr();
    throwOnModuleIsNullptr();

    // This callback is only single wires.
    m_currentModule->createNet(netname);
}

void ReaderImpl::onInput(const std::string &netname)
{
    throwOnDesignIsNullptr();
    throwOnModuleIsNullptr();
    
    // This callback is only single wires.
    auto netPtr = m_currentModule->createNet(netname);
    netPtr->setPortNet(true);

    // add a top-level pin
    auto& pin = m_currentModule->createPin(netname);
    pin.m_iotype = IO_INPUT;

    auto pinInstance = new PinInstance(netname);
    pinInstance->setPinIOType(IO_OUTPUT);    // input ports have output pins!
    pinInstance->connect(0, netPtr);
    netPtr->addConnection(pinInstance, 0);
    m_currentModule->addInstance(netname, pinInstance);    
}

void ReaderImpl::onInput(const std::string &netname, uint32_t start, uint32_t stop)
{
    throwOnDesignIsNullptr();
    throwOnModuleIsNullptr();

    // This callback is only for busses.
    // Here, we expand all busses into individual wires

    if (stop < start)
        std::swap(start,stop);

    for(uint32_t i=start; i<=stop; i++)
    {
        std::stringstream ss;
        ss << netname << "[" << i << "]";
        
        // This callback is only single wires.
        auto netPtr = m_currentModule->createNet(ss.str());
        netPtr->setPortNet(true);

        // add a top-level pin
        auto& pin = m_currentModule->createPin(ss.str());
        pin.m_iotype = IO_INPUT;

        // add a PinInstance for each pin to the netlist
        auto pinInstance = new PinInstance(netname);
        pinInstance->setPinIOType(IO_OUTPUT);    // input ports have output pins!
        pinInstance->connect(0, netPtr);
        netPtr->addConnection(pinInstance, 0);
        m_currentModule->addInstance(netname, pinInstance);
    }

    doLog(LOG_VERBOSE,"Expanded input net %s\n", netname.c_str());
}

void ReaderImpl::onOutput(const std::string &netname)
{
    throwOnDesignIsNullptr();
    throwOnModuleIsNullptr();

    // This callback is only single wires.
    auto netPtr = m_currentModule->createNet(netname);
    netPtr->setPortNet(true);

    // add a top-level pin
    auto& pin = m_currentModule->createPin(netname);
    pin.m_iotype = IO_OUTPUT;

    // add a PinInstance for each pin to the netlist
    auto pinInstance = new PinInstance(netname);
    pinInstance->setPinIOType(IO_INPUT);    // output ports have input pins!
    pinInstance->connect(0, netPtr);
    netPtr->addConnection(pinInstance, 0);    
    m_currentModule->addInstance(netname, pinInstance);
}

void ReaderImpl::onOutput(const std::string &netname, uint32_t start, uint32_t stop)
{
    throwOnDesignIsNullptr();
    throwOnModuleIsNullptr();

    // This callback is only for busses.
    // Here, we expand all busses into individual wires

    if (stop < start)
        std::swap(start,stop);

    for(uint32_t i=start; i<=stop; i++)
    {
        std::stringstream ss;
        ss << netname << "[" << i << "]";
        
        auto netPtr = m_currentModule->createNet(ss.str());
        netPtr->setPortNet(true);

        // add a top-level pin
        auto& pin = m_currentModule->createPin(ss.str());
        pin.m_iotype = IO_OUTPUT;

        // add a PinInstance for each pin to the netlist
        auto pinInstance = new PinInstance(netname);
        pinInstance->setPinIOType(IO_INPUT);    // output ports have input pins!
        pinInstance->connect(0, netPtr);
        netPtr->addConnection(pinInstance, 0);
        m_currentModule->addInstance(ss.str(), pinInstance);
    }

    doLog(LOG_VERBOSE,"Expanded output net %s\n", netname.c_str());
}

/** callback for each instance port in the netlist */
void ReaderImpl::onInstancePort(uint32_t pinIndex, const std::string &netName)
{    
    throwOnDesignIsNullptr();
    throwOnModuleIsNullptr();

    auto netPtr = m_currentModule->m_netlist.m_nets.lookup(netName);
    if (netPtr == nullptr)
    {
        std::stringstream ss;
        ss << "Cannot find net " << netName << " in module " << m_currentModule->m_name << "\n";
        doLog(LOG_ERROR, ss.str());
        return;
    }

    m_currentInstance->connect(pinIndex, netPtr);
    netPtr->addConnection(m_currentInstance, pinIndex);        
}

/** callback for each module instance in the netlist */
void ReaderImpl::onInstanceNamedPort(const std::string &pinName, const std::string &netName)
{
    throwOnDesignIsNullptr();
    throwOnModuleIsNullptr();
    throwOnCurInstanceIsNullptr();

    auto netPtr  = m_currentModule->m_netlist.m_nets.lookup(netName);    
    if (netPtr == nullptr)
    {
        doLog(LOG_WARN,"Cannot connect %s:%s to net %s -- net not found\n", m_currentInstance->m_name.c_str(), 
            pinName.c_str(), netName.c_str());
        return;
    }

    auto pinIndex = m_currentInstance->getPinIndex(pinName);
    if (pinIndex < 0)
    {
        doLog(LOG_WARN,"Cannot connect %s:%s to net %s -- pin not found\n", m_currentInstance->m_name.c_str(), 
            pinName.c_str(), netName.c_str());
        return;
    }

    auto pinInfo       = m_currentInstance->getPinInfo(pinIndex);
    auto pinConnection = m_currentInstance->getConnectedNet(pinIndex);

    m_currentInstance->connect(pinIndex, netPtr);
    netPtr->addConnection(m_currentInstance, pinIndex);
}

void ReaderImpl::onAssign(const std::string &left, const std::string &right)
{
    throwOnDesignIsNullptr();
    throwOnModuleIsNullptr();

    auto outNetPtr = m_currentModule->m_netlist.m_nets.lookup(left);     // output net
    auto inNetPtr  = m_currentModule->m_netlist.m_nets.lookup(right);    // input net
    
    std::stringstream ss;

    ss << "__NETCON" << m_design->createUniqueID();

    auto cellPtr = m_design->m_cellLib.lookup("__NETCON");
    if (cellPtr == nullptr)
    {
        doLog(LOG_ERROR,"Verilog reader: cannot find __NETCON cell in cell library\n");
        return;
    }

    auto insPtr = new Instance(cellPtr);
    insPtr->m_name = ss.str();

    if (!m_currentModule->addInstance(ss.str(), insPtr))
    {            
        std::stringstream ss;
        ss << "Verilog reader: failed to create instance " << ss.str() << "\n";
        doLog(LOG_ERROR, ss);
    }

    // **********************************************************************
    //   find Y and A pins on the __NETCON cell
    // **********************************************************************

    auto pinYIndex = insPtr->getPinIndex("Y");
    if (pinYIndex < 0)
    {
        doLog(LOG_ERROR,"Verilog reader: __NETCON cell does not have a Y pin.\n");
        return;
    }

    auto pinAIndex = insPtr->getPinIndex("A");
    if (pinAIndex < 0)
    {
        doLog(LOG_ERROR,"Verilog reader: __NETCON cell does not have a A pin.\n");
        return;
    }

    // **********************************************************************
    //   connect the nets together
    // **********************************************************************

    insPtr->connect(pinYIndex, outNetPtr);
    insPtr->connect(pinAIndex, inNetPtr);
    outNetPtr->addConnection(insPtr, pinYIndex);
    inNetPtr->addConnection(insPtr, pinAIndex);
}
