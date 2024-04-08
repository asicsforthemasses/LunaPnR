// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <fstream>
#include <filesystem>
#include "common/logging.h"
#include "quickplace/quickplace.hpp"
#include "pass.hpp"

namespace LunaCore::Passes
{

class PlacePass : public Pass
{
public:
    PlacePass() : Pass("place")
    {
        registerNamedParameter("module", "", 1, false);
        registerNamedParameter("ins", "", 3, false);
        registerNamedParameter("fix", "", 0, false);
    }

    virtual ~PlacePass() = default;

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool execute(Database &database) override
    {
        if (m_namedParams.contains("module"))
        {
            auto const& moduleName = m_namedParams["module"].at(0);

            auto modKp = database.m_design.m_moduleLib->lookupModule(moduleName);
            if (!modKp.isValid())
            {
                Logging::logError("Cannot find module %s\n", moduleName.c_str());
                return false;
            }

            return QuickPlace::place(database, *modKp);
        }
        else if (m_namedParams.contains("ins"))
        {
            auto topModule = database.m_design.getTopModule();
            if (!topModule)
            {
                Logging::logError("Top module not set\n");
                return false;
            }

            if (!topModule->m_netlist)
            {
                Logging::logError("Module %s has no netlist\n", topModule->name().c_str());
                return false;
            }

            auto const& insName = m_namedParams["ins"].at(0);
            ChipDB::CoordType xpos, ypos;
            try
            {
                xpos = std::stol(m_namedParams["ins"].at(1));
                ypos = std::stol(m_namedParams["ins"].at(2));
            }
            catch(std::exception &e)
            {
                Logging::logError("Could not convert x or y coordinate to integer\n");
                return false;
            }

            auto &instances = topModule->m_netlist->m_instances;
            auto insKp = instances[insName];

            if (!insKp.isValid())
            {
                Logging::logError("Could not find instance %s\n", insName.c_str());
                return false;
            }

            insKp->m_pos.m_x = xpos;
            insKp->m_pos.m_y = ypos;
            insKp->m_placementInfo = ChipDB::PlacementInfo::PLACED;

            if (m_namedParamDefs.contains("fix"))
            {
                insKp->m_placementInfo = ChipDB::PlacementInfo::PLACEDANDFIXED;
            }

            return true;
        }
        else
        {
            Logging::logError("Missing place type, use -pads, -core or -cell\n");
            return false;
        }

        return true;
    }

    /**
        returns help text for a pass.
    */
    std::string help() const noexcept override
    {
        std::stringstream ss;
        ss << "place - place pads/core/cell\n";
        ss << "  place <place type>\n\n";
        ss << "  Place type options:\n";
        ss << "    -module             : place a module\n";
        ss << "    -ins <name> <x> <y> : place a specific instance at a specified position\n";
        ss << "\n";
        return ss.str();
    }

    /**
        returns a one-line short help text for a pass.
    */
    virtual std::string shortHelp() const noexcept
    {
        return "place core/cell";
    }

    /**
        Initialize a pass. this is called by registerPass()
    */
    bool init() override
    {
        return true;
    }
};


};
