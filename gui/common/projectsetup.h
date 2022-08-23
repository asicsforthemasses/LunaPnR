/*
    LunaPnR Source Code

    SPDX-License-Identifier: GPL-3.0-only
    SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/

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

    std::vector<RegionSetup> m_regions;

    std::string m_openSTALocation{"/usr/local/bin/sta"};    ///< this comes from the Luna config file, not the project file.

    bool readFromJSON(std::istream &is);
    bool writeToJSON(std::ostream &os) const;
};

};
