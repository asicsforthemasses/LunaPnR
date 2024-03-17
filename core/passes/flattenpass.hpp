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

class FlattenPass : public Pass
{
public:
    FlattenPass() : Pass("flatten")
    {
        registerNamedParameter("module", "", 1, true);
    }

    virtual ~FlattenPass() = default;

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool execute(Database &database) override
    {
        if (m_namedParams.contains("module"))
        {
            Logging::logError("Not implemented yet\n");
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
        ss << "flatten - flatten netlist of module\n";
        ss << "  flatten <arguments>\n\n";
        ss << "  argument options:\n";
        ss << "    -module  : the name of the module to flatten\n";
        ss << "\n";
        return ss.str();
    }

    /**
        returns a one-line short help text for a pass.
    */
    virtual std::string shortHelp() const noexcept
    {
        return "flatten netlist of module";
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
