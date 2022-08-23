// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "lunacore.h"
#include "layerrenderinfo.h"
#include "hatchlibrary.h"
#include "projectsetup.h"

namespace GUI
{

class Database
{
public:
    Database() {}

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

    std::shared_ptr<ChipDB::CellLib> cellLib() const
    {
        return m_design.m_cellLib;
    }

    std::shared_ptr<ChipDB::TechLib> techLib() const
    {
        return m_design.m_techLib;
    }

    std::shared_ptr<ChipDB::ModuleLib> moduleLib() const
    {
        return m_design.m_moduleLib;
    }

    std::shared_ptr<ChipDB::Floorplan> floorplan() const
    {
        return m_design.m_floorplan;
    }

    LayerRenderInfoDB m_layerRenderInfoDB;
    HatchLibrary      m_hatchLib;
    ProjectSetup      m_projectSetup;

protected:    
    ChipDB::Design  m_design;
};

};