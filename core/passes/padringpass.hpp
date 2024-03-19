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

class PadringPass : public Pass
{
public:
    PadringPass() : Pass("padring")
    {
        registerNamedParameter("place", "", 1, true);
        registerNamedParameter("module", "", 1, true);
    }

    virtual ~PadringPass() = default;

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool execute(Database &database) override
    {
        if (!m_namedParams.contains("module"))
        {
            Logging::logError("padring requires a module parameter\n");
            return false;
        }

        auto moduleName = m_namedParams.at("module").at(0);
        if (!database.m_design.setTopModule(moduleName))
        {
            Logging::logError("module %s not found\n", moduleName.c_str());
            return false;
        }

        if (m_namedParams.contains("place"))
        {
            auto ioFileName = m_namedParams.at("place").front();
            std::ifstream iofile(ioFileName);

            if (!iofile.is_open())
            {
                Logging::logError("Cannot open IO file %s\n", ioFileName.c_str());
                return false;
            }

            LunaCore::Padring::Padring padring;

            LunaCore::Padring::ConfigReader ioreader(iofile);
            if (!LunaCore::Padring::read(iofile, database, padring))
            {
                Logging::logError("Failed to read io file %s\n", ioFileName.c_str());
                return false;
            }

            if (!LunaCore::Padring::place(database, padring))
            {
                Logging::logError("Failed to place io pads\n");
                return false;
            }

            return true;
        }
        return false;
    }

    /**
        returns help text for a pass.
    */
    std::string help() const noexcept override
    {
        std::stringstream ss;
        ss << "padring - padring placement\n";
        ss << "  padring <option>\n\n";
        ss << "  padring options:\n";
        ss << "    -place <io filename> : place the padring\n";
        ss << "\n";
        return ss.str();
    }

    /**
        returns a one-line short help text for a pass.
    */
    virtual std::string shortHelp() const noexcept
    {
        return "padring - padring placement";
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
