// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <fstream>
#include <filesystem>
#include "common/logging.h"
#include "pass.hpp"

namespace LunaCore::Passes
{

class CTSPass : public Pass
{
public:
    CTSPass() : Pass("cts")
    {
        registerNamedParameter("net", "", 1, false);
    }

    virtual ~CTSPass() = default;

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool execute(Database &database) override
    {
        if (m_namedParams.contains("net"))
        {
            Logging::logError("Not implemented yet\n");
            return false;
        }
        else
        {
            Logging::logError("Missing cts type, use -net\n");
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
        ss << "cts - clock tree synthesis\n";
        ss << "  cts <net>\n\n";
        ss << "  check type options:\n";
        ss << "    -net  : synthesize clock tree for the specified net\n";
        ss << "\n";
        return ss.str();
    }

    /**
        returns a one-line short help text for a pass.
    */
    virtual std::string shortHelp() const noexcept
    {
        return "clock tree synthesis";
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
