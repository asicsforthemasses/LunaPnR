// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <fstream>
#include <filesystem>
#include "common/logging.h"
#include "export/export.h"
#include "pass.hpp"

namespace LunaCore::Passes
{

class WritePass : public Pass
{
public:
    WritePass() : Pass("write")
    {
        registerNamedParameter("verilog", "", 2, false);
        registerNamedParameter("def", "", 2, false);
        registerNamedParameter("txt", "", 2, false);
        registerNamedParameter("gds2", "", 2, false);
    }

    virtual ~WritePass() = default;

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool execute(Database &database) override
    {
        if (m_namedParams.contains("verilog"))
        {
            auto params = m_namedParams.at("verilog");
            if (params.size() != 2)
            {
                std::stringstream ss;
                ss << "read -verilog requires exactly two parameters\n";
                Logging::logError(ss.str());
                return false;
            }

            auto moduleName = params.at(0);
            auto fname      = params.at(1);

            auto modKp = database.m_design.m_moduleLib->lookupModule(moduleName);
            if (!modKp.isValid())
            {
                std::stringstream ss;
                ss << "cannot find module '"<< moduleName << "'\n";
                Logging::logError(ss.str());
                return false;
            }

            if (std::filesystem::is_directory(fname))
            {
                std::stringstream ss;
                ss << "'"<< fname << "' is a directory\n";
                Logging::logError(ss.str());
                return false;
            }

            std::ofstream outfile(fname, std::ios::out | std::ios::trunc);
            if (!outfile.is_open())
            {
                std::stringstream ss;
                ss << "Cannot open file '"<< fname << "'\n";
                Logging::logError(ss.str());
                return false;
            }

            if (!LunaCore::Verilog::Writer::write(outfile, modKp.ptr()))
            {
                std::stringstream ss;
                ss << "Failed to write '"<< fname << "'\n";
                Logging::logError(ss.str());
                return false;
            }
            else
            {
                std::stringstream ss;
                ss << "Module '" << modKp->name() << "' has been exported to " << fname << "'\n";
                Logging::logInfo(ss.str());
                return true;
            }
        }
        else if (m_namedParams.contains("txt"))
        {
            auto params = m_namedParams.at("txt");
            if (params.size() != 2)
            {
                std::stringstream ss;
                ss << "read -txt requires exactly two parameters\n";
                Logging::logError(ss.str());
                return false;
            }

            auto moduleName = params.at(0);
            auto fname      = params.at(1);

            auto modKp = database.m_design.m_moduleLib->lookupModule(moduleName);
            if (!modKp.isValid())
            {
                std::stringstream ss;
                ss << "cannot find module '"<< moduleName << "'\n";
                Logging::logError(ss.str());
                return false;
            }

            if (std::filesystem::is_directory(fname))
            {
                std::stringstream ss;
                ss << "'"<< fname << "' is a directory\n";
                Logging::logError(ss.str());
                return false;
            }

            std::ofstream outfile(fname);
            if (!outfile.is_open())
            {
                std::stringstream ss;
                ss << "Cannot open file '"<< fname << "'\n";
                Logging::logError(ss.str());
                return false;
            }

            if (!LunaCore::TXT::write(outfile, modKp.ptr()))
            {
                std::stringstream ss;
                ss << "Failed to write '"<< fname << "'\n";
                Logging::logError(ss.str());
                return false;
            }
            else
            {
                std::stringstream ss;
                ss << "Module '" << modKp->name() << "' has been exported to " << fname << "'\n";
                Logging::logInfo(ss.str());
                return true;
            }
        }
        else if (m_namedParams.contains("def"))
        {
            auto params = m_namedParams.at("def");
            if (params.size() != 2)
            {
                std::stringstream ss;
                ss << "read -def requires exactly two parameters\n";
                Logging::logError(ss.str());
                return false;
            }

            auto moduleName = params.at(0);
            auto fname      = params.at(1);

            auto modKp = database.m_design.m_moduleLib->lookupModule(moduleName);
            if (!modKp.isValid())
            {
                std::stringstream ss;
                ss << "cannot find module '"<< moduleName << "'\n";
                Logging::logError(ss.str());
                return false;
            }

            if (std::filesystem::is_directory(fname))
            {
                std::stringstream ss;
                ss << "'"<< fname << "' is a directory\n";
                Logging::logError(ss.str());
                return false;
            }

            std::ofstream outfile(fname, std::ios::out | std::ios::trunc);
            if (!outfile.is_open())
            {
                std::stringstream ss;
                ss << "Cannot open file '"<< fname << "'\n";
                Logging::logError(ss.str());
                return false;
            }

            if (!LunaCore::DEF::write(outfile, modKp.ptr()))
            {
                std::stringstream ss;
                ss << "Failed to write '"<< fname << "'\n";
                Logging::logError(ss.str());
                return false;
            }
            else
            {
                std::stringstream ss;
                ss << "Module '" << modKp->name() << "' has been exported to " << fname << "'\n";
                Logging::logInfo(ss.str());
                return true;
            }
        }
        else if (m_namedParams.contains("gds2"))
        {
            auto params = m_namedParams.at("gds2");
            if (params.size() != 2)
            {
                std::stringstream ss;
                ss << "read -gds2 requires exactly two parameters\n";
                Logging::logError(ss.str());
                return false;
            }

            auto moduleName = params.at(0);
            auto fname      = params.at(1);

            if (std::filesystem::is_directory(fname))
            {
                std::stringstream ss;
                ss << "'"<< fname << "' is a directory\n";
                Logging::logError(ss.str());
                return false;
            }

            std::ofstream outfile(fname, std::ios::out | std::ios::trunc);
            if (!outfile.is_open())
            {
                std::stringstream ss;
                ss << "Cannot open file '"<< fname << "'\n";
                Logging::logError(ss.str());
                return false;
            }

            if (!LunaCore::GDS2::write(outfile, database, moduleName))
            {
                std::stringstream ss;
                ss << "Failed to write '"<< fname << "'\n";
                Logging::logError(ss.str());
                return false;
            }
            else
            {
                std::stringstream ss;
                ss << "Module '" << moduleName << "' has been exported to " << fname << "'\n";
                Logging::logInfo(ss.str());
                return true;
            }
        }
        else
        {
            Logging::logError("Missing file type, use -verilog, -def, -gds2 or -txt\n");
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
        ss << "write - write files into the database\n";
        ss << "  write <file type> <design> <filename>\n\n";
        ss << "  File type options:\n";
        ss << "    -verilog : write a Verilog netlist\n";
        ss << "    -def     : write a DEF design file\n";
        ss << "    -gds2    : write a GDS2 design file\n";
        ss << "    -txt     : write a TXT file\n";
        ss << "\n";
        return ss.str();
    }

    /**
        returns a one-line short help text for a pass.
    */
    virtual std::string shortHelp() const noexcept
    {
        return "write a design to a file";
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
