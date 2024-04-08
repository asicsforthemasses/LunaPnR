// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <fstream>
#include <filesystem>
#include "common/logging.h"
#include "common/strutils.hpp"
#include "pass.hpp"

namespace LunaCore::Passes
{

class SetPass : public Pass
{
public:
    SetPass() : Pass("set")
    {
        registerNamedParameter("loglevel", "", 1, false);
        registerNamedParameter("top", "", 1, false);
        registerNamedParameter("cell", "", 1, false);
        registerNamedParameter("class", "", 1, false);
        registerNamedParameter("subclass", "", 1, false);
    }

    virtual ~SetPass() = default;

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool execute(Database &database) override
    {
        if (m_namedParams.contains("loglevel"))
        {
            auto lvl = m_namedParams.at("loglevel").front();

            if (LunaCore::toupper(lvl) == "DEBUG")
            {
                Logging::setLogLevel(Logging::LogType::DEBUG);
            }
            else if (LunaCore::toupper(lvl) == "VERBOSE")
            {
                Logging::setLogLevel(Logging::LogType::VERBOSE);
            }
            else
            {
                Logging::setLogLevel(Logging::LogType::INFO);
            }
            return true;
        }
        else if (m_namedParams.contains("top"))
        {
            auto const& moduleName = m_namedParams.at("top").front();

            if (!database.m_design.setTopModule(moduleName))
            {
                Logging::logError("Could not set top module to %s\n", moduleName.c_str());
                return false;
            }
            return true;
        }
        else if (m_namedParams.contains("cell"))
        {
            auto cellName = m_namedParams.at("cell").front();

            auto cellKp = database.m_design.m_cellLib->lookupCell(cellName);
            if (!cellKp.isValid())
            {
                Logging::logError("Cannot find cell %s\n", cellName.c_str());
                return false;
            }

            if (m_namedParams.contains("subclass"))
            {
                cellKp->m_subclass.fromString(m_namedParams.at("subclass").at(0));
            }

            if (m_namedParams.contains("class"))
            {
                cellKp->m_class.fromString(m_namedParams.at("class").at(0));
            }

            return true;
        }
        else
        {
            Logging::logError("Missing parameters, use -loglevel, -top or -cell\n");
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
        ss << "set - set stuff\n";
        ss << "  set <set type>\n\n";
        ss << "  set type options:\n";
        ss << "    -loglevel <level>                    : set log level to NORMAL, DEBUG or VERBOSE.\n";
        ss << "    -top <module name>                   : set top level module\n";
        ss << "    -cell <cell name> -class <class>     : change cell class\n";
        ss << "    -cell <cell name> -subclass <sclass> : change cell subclass\n";
        ss << "\n";
        return ss.str();
    }

    /**
        returns a one-line short help text for a pass.
    */
    virtual std::string shortHelp() const noexcept
    {
        return "set stuff";
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
