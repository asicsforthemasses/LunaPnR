// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "lunacore.h"

namespace GUI
{

struct RegionSetup
{
    std::string         m_regionName;   ///< region name
    ChipDB::Rect64      m_regionSize;   ///< region size including halo
    ChipDB::Margins64   m_regionHalo;   ///< region halo margins
    std::string         m_site;         ///< site name, corresponding to LEF file.
};

/** LunaPnR project information, used by the project browser/manager. */
struct ProjectSetup
{
    std::vector<std::string> m_lefFiles;
    std::vector<std::string> m_libFiles;
    std::vector<std::string> m_verilogFiles;
    std::vector<std::string> m_timingConstraintFiles;
    std::vector<std::string> m_layerFiles;

    std::string m_ctsBuffer;            ///< architype name of clock buffer.
    float       m_ctsMaxCap{0.2e-12};   ///< maximum capacity of CTS subtree.

    std::vector<RegionSetup> m_regions;

    std::string m_openSTALocation{"/usr/local/bin/sta"};    ///< this comes from the Luna config file, not the project file.
    std::string m_floorplanScriptLocation;                  ///< if this is not empty, this script is used to generate the floorplan

    bool readFromJSON(std::istream &is);
    bool writeToJSON(std::ostream &os) const;

    bool readFromTOML(std::istream &is);
    bool writeToTOML(std::ostream &os) const;
};

};
