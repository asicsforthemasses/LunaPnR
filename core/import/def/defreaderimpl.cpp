// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <array>
#include <string>
#include <memory>
#include "defreaderimpl.h"
#include "design/design.h"
#include "common/logging.h"

using namespace ChipDB::DEF;

ReaderImpl::ReaderImpl(Design &design) 
    : m_design(design) 
{
}

void ReaderImpl::onDesign(const std::string &designName)
{
    Logging::doLog(Logging::LogType::INFO, "DEFReader: design %s\n", designName.c_str());

    auto modKeyPtr = m_design.m_moduleLib->lookupModule(designName);
    if (!modKeyPtr.isValid())
    {        
        m_module.reset();
        std::stringstream ss;
        ss << "Cannot find the specified DEF design " << designName;
        error(ss.str());
        return;
    }

    m_module = modKeyPtr.ptr();
}

void ReaderImpl::onEndDesign(const std::string &designName)
{
    Logging::doLog(Logging::LogType::VERBOSE, "DEFReader: end design %s\n", designName.c_str());
    m_module.reset();
}

void ReaderImpl::onComponent(const std::string &insName, const std::string &archetype)
{
    Logging::doLog(Logging::LogType::VERBOSE, "DEFReader: ins %s archetype %s\n", insName.c_str(), archetype.c_str());
    
    if (!m_module) return;    
    
    auto netlist = m_module->m_netlist;
    if (!netlist) return;

    auto insKeyPtr = netlist->lookupInstance(insName);
    if (insKeyPtr.isValid())
    {
        m_instance = insKeyPtr.ptr();
    }
    else
    {
        m_instance.reset();
    }
};

void ReaderImpl::onComponentPlacement(const ChipDB::Coord64 &pos, 
    const ChipDB::PlacementInfo placement,
    const ChipDB::Orientation orient)
{

    if (!m_module) return;
    if (!m_instance) return;

    m_instance->m_pos = pos;
    m_instance->m_orientation = orient;
    m_instance->m_placementInfo = placement;
};

void ReaderImpl::onEndParse()
{
    Logging::doLog(Logging::LogType::VERBOSE, "DEFReader: done\n");
}
