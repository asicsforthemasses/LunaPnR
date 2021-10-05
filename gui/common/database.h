#pragma once

#include "lunacore.h"
#include "layerrenderinfo.h"
#include "hatchlibrary.h"

namespace GUI
{

class Database
{
public:

    /** clear the entire database */
    void clear();

    const ChipDB::Design& design() const
    {
        return m_design;
    }

    ChipDB::Design& design()
    {
        return m_design;
    }

    const ChipDB::CellLib& cellLib() const
    {
        return m_design.m_cellLib;
    }

    ChipDB::CellLib& cellLib()
    {
        return m_design.m_cellLib;
    }

    const ChipDB::TechLib& techLib() const
    {
        return m_design.m_techLib;
    }

    ChipDB::TechLib& techLib()
    {
        return m_design.m_techLib;
    }

    const ChipDB::Netlist& netlist() const
    {
        return m_design.m_netlist;
    }

    ChipDB::Netlist& netlist()
    {
        return m_design.m_netlist;
    }

    const ChipDB::ModuleLib& moduleLib() const
    {
        return m_design.m_moduleLib;
    }

    ChipDB::ModuleLib& moduleLib()
    {
        return m_design.m_moduleLib;
    }

    LayerRenderInfoDB m_layerRenderInfoDB;
    HatchLibrary      m_hatchLib;
    
protected:    
    ChipDB::Design  m_design;
};

};