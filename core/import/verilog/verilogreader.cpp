// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only


#include <sstream>
#include <algorithm>
#include <cassert>
#include "common/logging.h"
#include "netlist/instance.h"
#include "verilogreader.h"

using namespace ChipDB::Verilog;

constexpr static bool extraDebug = false;

bool Reader::load(Design &design, std::istream &source)
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
                Logging::doLog(Logging::LogType::INFO,"Verilog netlist parsed.\n");
                Logging::doLog(Logging::LogType::INFO,"  modules %d\n", design.m_moduleLib->size());
                return true;
            }
        }
        else
        {
            Logging::doLog(Logging::LogType::ERROR,"Verilog::Reader failed to load netlist.\n");
            return false;
        }
    }
    catch(std::runtime_error &e)
    {
        Logging::doLog(Logging::LogType::ERROR,"%s\n", e.what());
    }
    catch(std::exception const& e)
    {
        Logging::doLog(Logging::LogType::ERROR,"Unexpected exception %s\n", e.what());
    }
    catch(...)
    {
        Logging::doLog(Logging::LogType::ERROR,"Unexpected exception\n");
    }

    Logging::doLog(Logging::LogType::ERROR,"Verilog::Reader failed to load netlist.\n");
    return false;
}


// **********************************************************************
//   ReaderImpl
// **********************************************************************

ReaderImpl::ReaderImpl(Design &design) : m_design(design)
{
}

void ReaderImpl::throwOnModuleIsNullptr()
{
    if (!m_currentModule)
    {
        throw std::runtime_error("Reader: no module selected.");
    }
}

void ReaderImpl::throwOnCurInstanceIsNullptr()
{
    if (!m_currentInsKeyObjPair.isValid())
    {
        throw std::runtime_error("Reader: no instance selected.");
    }    
}


void ReaderImpl::onModule(const std::string &modName,
        const std::vector<std::string> &ports)
{    
    m_currentModule = m_design.m_moduleLib->createModule(modName).ptr();
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
    throwOnModuleIsNullptr();

    // check for cell or module instance
    auto const cellKeyObjPtr = m_design.m_cellLib->lookupCell(modName);
    if (cellKeyObjPtr.isValid())
    {           
        auto insPtr = std::make_shared<Instance>(insName, ChipDB::InstanceType::CELL, cellKeyObjPtr.ptr());
        auto insKeyObjPair = m_currentModule->addInstance(insPtr);

        if (!insKeyObjPair.isValid())
        {            
            std::stringstream ss;
            ss << "Failed to create instance " << insName << "\n";
            Logging::doLog(Logging::LogType::ERROR, ss.str());
            return;
        }

        m_currentInsKeyObjPair = insKeyObjPair;
        return;
    }
    
    auto moduleKeyObjPair = m_design.m_moduleLib->lookupModule(modName);
    if (moduleKeyObjPair.isValid())
    {
        auto insPtr = std::make_shared<Instance>(modName, ChipDB::InstanceType::MODULE, moduleKeyObjPair.ptr());
        auto insKeyObjPair = m_currentModule->addInstance(insPtr);
        if (!insKeyObjPair.isValid())
        {
            std::stringstream ss;
            ss << "Failed to create module instance " << insName << "\n";
            Logging::doLog(Logging::LogType::ERROR, ss.str());            
        }

        m_currentInsKeyObjPair = insKeyObjPair;
        return;
    }

    // neither a cell or a module could found -> error
    Logging::doLog(Logging::LogType::ERROR,"Cannot find cell/module '%s' in cell database\n", modName.c_str());
    return;
}

void ReaderImpl::onWire(const std::string &netname, uint32_t start, uint32_t stop)
{
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
    throwOnModuleIsNullptr();

    // This callback is only single wires.
    m_currentModule->createNet(netname);
}

void ReaderImpl::onInput(const std::string &netname)
{
    throwOnModuleIsNullptr();
    
    // This callback is only single wires.
    auto netPtr = m_currentModule->createNet(netname);
    netPtr->setPortNet(true);

    // add a top-level pin
    auto pin = m_currentModule->createPin(netname);
    pin->m_iotype = ChipDB::IOType::INPUT;

    auto pinInstance = std::make_shared<ChipDB::Instance>(netname, ChipDB::InstanceType::PIN, 
        m_design.m_cellLib->lookupCell("__INPIN").ptr());

    auto pinInsKeyObjPair = m_currentModule->addInstance(pinInstance);
    if (!m_currentModule->connect(netname, "Y", netname))   // output on the inner level
    {
        Logging::doLog(Logging::LogType::ERROR,"VerilogReader::ReaderImpl::onInput: cannot connect to pin Instance!\n");
    }
}

void ReaderImpl::onInput(const std::string &netname, uint32_t start, uint32_t stop)
{
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
        auto pin = m_currentModule->createPin(ss.str());
        pin->m_iotype = ChipDB::IOType::INPUT;

        // add a PinInstance for each pin to the netlist
        auto pinInstance = std::make_shared<ChipDB::Instance>(netname, ChipDB::InstanceType::PIN, 
        m_design.m_cellLib->lookupCell("__INPIN").ptr());

        auto pinInsKeyObjPair = m_currentModule->addInstance(pinInstance);
        if (!m_currentModule->connect(netname, "Y", netname))    // output on the inner level
        {
            Logging::doLog(Logging::LogType::ERROR,"VerilogReader::ReaderImpl::onInput: cannot connect to pin Instance!\n");
        }
    }

    Logging::doLog(Logging::LogType::VERBOSE,"Expanded input net %s\n", netname.c_str());
}

void ReaderImpl::onOutput(const std::string &netname)
{
    throwOnModuleIsNullptr();

    // This callback is only single wires.
    auto netKeyObjPair = m_currentModule->createNet(netname);
    netKeyObjPair->setPortNet(true);

    // add a top-level pin
    auto pin = m_currentModule->createPin(netname);
    pin->m_iotype = ChipDB::IOType::OUTPUT;

    // add a PinInstance for each pin to the netlist
    auto pinInstance = std::make_shared<ChipDB::Instance>(netname, ChipDB::InstanceType::PIN, 
        m_design.m_cellLib->lookupCell("__OUTPIN").ptr());

    auto pinInsKeyObjPair = m_currentModule->addInstance(pinInstance);

    if (!m_currentModule->connect(netname, "A", netname))    // input on the inner level
    {
        Logging::doLog(Logging::LogType::ERROR,"VerilogReader::ReaderImpl::onOutput: cannot connect to pin Instance!\n");        
    }
}

void ReaderImpl::onOutput(const std::string &netname, uint32_t start, uint32_t stop)
{
    throwOnModuleIsNullptr();

    // This callback is only for busses.
    // Here, we expand all busses into individual wires

    if (stop < start)
        std::swap(start,stop);

    for(uint32_t i=start; i<=stop; i++)
    {
        std::stringstream ss;
        ss << netname << "[" << i << "]";
        
        auto netKeyObjPair = m_currentModule->createNet(ss.str());
        netKeyObjPair->setPortNet(true);

        // add a top-level pin
        auto pin = m_currentModule->createPin(ss.str());
        pin->m_iotype = ChipDB::IOType::OUTPUT;

        // add a PinInstance for each pin to the netlist
        auto pinInstance = std::make_shared<ChipDB::Instance>(netname, ChipDB::InstanceType::PIN, 
            m_design.m_cellLib->lookupCell("__OUTPIN").ptr());

        auto pinInsKeyObjPair = m_currentModule->addInstance(pinInstance);

        if (!m_currentModule->connect(netname, "A", netname))    // input on the inner level
        {
            Logging::doLog(Logging::LogType::ERROR,"VerilogReader::ReaderImpl::onOutput: cannot connect to pin Instance!\n");        
        }
    }

    Logging::doLog(Logging::LogType::VERBOSE,"Expanded output net %s\n", netname.c_str());
}

/** callback for each instance port in the netlist */
void ReaderImpl::onInstancePort(uint32_t pinIndex, const std::string &netName)
{    
    throwOnModuleIsNullptr();

    auto netKeyObjPair = m_currentModule->m_netlist->m_nets.at(netName);
    if (!netKeyObjPair.isValid())
    {
        std::stringstream ss;
        ss << "Cannot find net " << netName << " in module " << m_currentModule->name() << "\n";
        Logging::doLog(Logging::LogType::ERROR, ss.str());
        return;
    }

    if (!m_currentModule->connect(m_currentInsKeyObjPair.key(), pinIndex, netKeyObjPair.key()))
    {
        Logging::doLog(Logging::LogType::ERROR,"VerilogReader::ReaderImpl::onInstancePort: cannot connect to instance!\n");
    }
}

/** callback for each module instance in the netlist */
void ReaderImpl::onInstanceNamedPort(const std::string &pinName, const std::string &netName)
{
    throwOnModuleIsNullptr();
    throwOnCurInstanceIsNullptr();

    auto netKeyObjPair  = m_currentModule->m_netlist->m_nets.at(netName);    
    if (!netKeyObjPair.isValid())
    {
        Logging::doLog(Logging::LogType::WARNING,"Cannot connect %s:%s to net %s -- net not found\n", m_currentInsKeyObjPair->name().c_str(), 
            pinName.c_str(), netName.c_str());
        return;
    }

    auto pin = m_currentInsKeyObjPair->getPin(pinName);
    if (!pin.isValid())
    {
        Logging::doLog(Logging::LogType::WARNING,"Cannot connect %s:%s to net %s -- pin not found\n", m_currentInsKeyObjPair->name().c_str(), 
            pinName.c_str(), netName.c_str());
        return;
    }

    if (!m_currentModule->connect(m_currentInsKeyObjPair.key(), pin.m_pinKey, netKeyObjPair.key()))
    {
        Logging::doLog(Logging::LogType::WARNING,"Cannot connect %s:%s to net %s -- Module->connect returned false\n", m_currentInsKeyObjPair->name().c_str(), 
            pinName.c_str(), netName.c_str());
    }
}

void ReaderImpl::onAssign(const std::string &left, const std::string &right)
{
    throwOnModuleIsNullptr();

    auto outNetPtr = m_currentModule->m_netlist->m_nets[left];     // output net
    auto inNetPtr  = m_currentModule->m_netlist->m_nets[right];    // input net
    
    std::stringstream ss;

    ss << "__NETCON" << m_design.createUniqueID();

    auto cellKeyObjPair = m_design.m_cellLib->lookupCell("__NETCON");
    if (!cellKeyObjPair.isValid())
    {
        Logging::doLog(Logging::LogType::ERROR,"Verilog reader: cannot find __NETCON cell in cell library\n");
        return;
    }

    auto insPtr = std::make_shared<Instance>(ss.str(), ChipDB::InstanceType::CELL, cellKeyObjPair.ptr());
    auto insKeyObjPair = m_currentModule->addInstance(insPtr);

    if (!insKeyObjPair.isValid())
    {            
        std::stringstream ss2;
        ss2 << "Verilog reader: failed to create instance " << ss.str() << "\n";
        Logging::doLog(Logging::LogType::ERROR, ss2);
    }

    // **********************************************************************
    //   find Y and A pins on the __NETCON cell
    // **********************************************************************

    auto pinY = insPtr->getPin("Y");
    if (!pinY.isValid())
    {
        Logging::doLog(Logging::LogType::ERROR,"Verilog reader: __NETCON cell does not have a Y pin.\n");
        return;
    }

    auto pinA = insPtr->getPin("A");
    if (!pinA.isValid())
    {
        Logging::doLog(Logging::LogType::ERROR,"Verilog reader: __NETCON cell does not have a A pin.\n");
        return;
    }

    // **********************************************************************
    //   connect the nets together
    // **********************************************************************

    m_currentModule->connect(insKeyObjPair.key(), pinY.pinKey(), outNetPtr.key());
    m_currentModule->connect(insKeyObjPair.key(), pinA.pinKey(), inNetPtr.key());
}
