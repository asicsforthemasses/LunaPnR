// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <fstream>
#include <filesystem>
#include "common/logging.h"
#include "pass.hpp"

namespace LunaCore::Passes
{

class PlacePass : public Pass
{
public:
    PlacePass() : Pass("place")
    {
        registerNamedParameter("core", "", 0, false);
        registerNamedParameter("cell", "", 0, false);
    }

    virtual ~PlacePass() = default;

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool execute(Database &database) override
    {
        if (m_namedParams.contains("core"))
        {
            Logging::logError("Not implemented yet\n");
            return false;
        }
        else if (m_namedParams.contains("cell"))
        {
            Logging::logError("Not implemented yet\n");
            return false;
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
        ss << "    -core    : place all core cells\n";
        ss << "    -cell    : place a specific cell at a specified position\n";
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
