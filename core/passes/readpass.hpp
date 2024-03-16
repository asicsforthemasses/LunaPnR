// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <fstream>
#include <filesystem>
#include "common/logging.h"
#include "import/import.h"
#include "pass.hpp"

namespace LunaCore::Passes
{

class ReadPass : public Pass
{
public:
    ReadPass() : Pass("read")
    {
        registerNamedParameter("verilog", "", 0, false);
        registerNamedParameter("lib", "", 0, false);
        registerNamedParameter("lef", "", 0, false);
        registerNamedParameter("sdc", "", 0, false);
    }

    virtual ~ReadPass() = default;

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool execute(Database &database) override
    {
        if (m_namedParams.contains("verilog"))
        {
            for(auto const& fname : m_params)
            {
                if (!std::filesystem::is_regular_file(fname))
                {
                    std::stringstream ss;
                    ss << "'"<< fname << "' is not a file\n";
                    Logging::doLog(Logging::LogType::ERROR, ss.str());
                    return false;
                }

                std::ifstream infile(fname);
                if (!infile.is_open())
                {
                    std::stringstream ss;
                    ss << "Cannot open file '"<< fname << "'\n";
                    Logging::doLog(Logging::LogType::ERROR, ss.str());
                    return false;
                }

                ChipDB::Verilog::Reader reader;
                if (!reader.load(database.m_design, infile))
                {
                    std::stringstream ss;
                    ss << "Failed to load '"<< fname << "'\n";
                    Logging::doLog(Logging::LogType::ERROR, ss.str());
                    return false;
                }
            }
        }
        else if (m_namedParams.contains("lib"))
        {
            for(auto const& fname : m_params)
            {
                if (!std::filesystem::is_regular_file(fname))
                {
                    std::stringstream ss;
                    ss << "'"<< fname << "' is not a file\n";
                    Logging::doLog(Logging::LogType::ERROR, ss.str());
                    return false;
                }

                std::ifstream infile(fname);
                if (!infile.is_open())
                {
                    std::stringstream ss;
                    ss << "Cannot open file '"<< fname << "'\n";
                    Logging::doLog(Logging::LogType::ERROR, ss.str());
                    return false;
                }

                ChipDB::Liberty::Reader reader;
                if (!reader.load(database.m_design, infile))
                {
                    std::stringstream ss;
                    ss << "Failed to load '"<< fname << "'\n";
                    Logging::doLog(Logging::LogType::ERROR, ss.str());
                    return false;
                }
            }
        }
        else if (m_namedParams.contains("lef"))
        {
            for(auto const& fname : m_params)
            {
                if (!std::filesystem::is_regular_file(fname))
                {
                    std::stringstream ss;
                    ss << "'"<< fname << " is not a file'\n";
                    Logging::doLog(Logging::LogType::ERROR, ss.str());
                    return false;
                }

                std::ifstream infile(fname);
                if (!infile.is_open())
                {
                    std::stringstream ss;
                    ss << "Cannot open file '"<< fname << "'\n";
                    Logging::doLog(Logging::LogType::ERROR, ss.str());
                    return false;
                }

                ChipDB::LEF::Reader reader;
                if (!reader.load(database.m_design, infile))
                {
                    std::stringstream ss;
                    ss << "Failed to load '"<< fname << "'\n";
                    Logging::doLog(Logging::LogType::ERROR, ss.str());
                    return false;
                }
            }
        }
        else if (m_namedParams.contains("sdc"))
        {
            Logging::doLog(Logging::LogType::ERROR, "Not implemented.\n");
            return false;

        }
        else
        {
            doLog(Logging::LogType::ERROR, "Missing file type, use -verilog, -lib, -lef or -sdc\n");
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
        ss << "read - read files into the database\n";
        ss << "  read <file type> file1 file2 ...\n\n";
        ss << "  File type options:\n";
        ss << "    -verilog : read a Verilog netlist\n";
        ss << "    -lib     : read a Liberty timing file\n";
        ss << "    -lef     : read a LEF layout file\n";
        ss << "    -sdc     : read timing specification file\n";
        ss << "\n";
        ss << "Note: the order of the files is important; specify lower hierarchy files first.\n";
        return ss.str();
    }

    /**
        returns a one-line short help text for a pass.
    */
    virtual std::string shortHelp() const noexcept
    {
        return "read files into the datatbase";
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
