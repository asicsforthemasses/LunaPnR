// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <fstream>
#include <filesystem>
#include "common/logging.h"
#include "import/import.h"
#include "pass.hpp"
#include "passes.hpp"

namespace LunaCore::Passes
{

class ScriptPass : public Pass
{
public:
    ScriptPass() : Pass("script")
    {
    }

    virtual ~ScriptPass() = default;

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool execute(Database &database) override
    {
        for(auto scriptFilename : m_params)
        {
            if (!std::filesystem::is_regular_file(scriptFilename))
            {
                std::stringstream ss;
                ss << "Cannot load " << scriptFilename << "\n";
                Logging::doLog(Logging::LogType::ERROR, ss.str());
                return false;
            }

            std::ifstream infile(scriptFilename);
            if (!infile.is_open())
            {
                std::stringstream ss;
                ss << "Cannot load " << scriptFilename << "\n";
                Logging::doLog(Logging::LogType::ERROR, ss.str());
                return false;
            }

            std::stringstream ss;
            ss << "Running script " << scriptFilename << "\n";
            Logging::doLog(Logging::LogType::INFO, ss.str());

            std::string line;
            while(std::getline(infile, line))
            {
                // do some hacking to allow line comments.
                // remove leading and trailing whitespace.
                line = trim(line);

                if (line.empty() || line.starts_with('#'))
                {
                    continue;
                }

                if (!Passes::run(database, line))
                {
                    ss << "Script " << scriptFilename << " error\n";
                    Logging::doLog(Logging::LogType::INFO, ss.str());
                    return false;
                }
            }

            ss.str("");
            ss << "Script " << scriptFilename << " done\n";
            Logging::doLog(Logging::LogType::INFO, ss.str());
        }

        return true;
    }

    /**
        returns help text for a pass.
    */
    std::string help() const noexcept override
    {
        std::stringstream ss;
        ss << "info - query the database\n";
        ss << "  info <info type> [name1 name2 ...]\n\n";
        ss << "  Info type options:\n";
        ss << "    -cells   : show all the cells\n";
        ss << "\n";
        return ss.str();
    }

    /**
        returns a one-line short help text for a pass.
    */
    virtual std::string shortHelp() const noexcept
    {
        return "query the datatbase";
    }

    /**
        Initialize a pass. this is called by registerPass()
    */
    bool init() override
    {
        return true;
    }

protected:
    std::string trim(const std::string& str,
                    const std::string& whitespace = " \t")
    {
        const auto strBegin = str.find_first_not_of(whitespace);
        if (strBegin == std::string::npos)
            return ""; // no content

        const auto strEnd = str.find_last_not_of(whitespace);
        const auto strRange = strEnd - strBegin + 1;

        return str.substr(strBegin, strRange);
    }

#if 0
    std::string trimString(const std::string& str,
                   const std::string& fill = " ",
                   const std::string& whitespace = " \t")
    {
        // trim first
        auto result = trim(str, whitespace);

        // replace sub ranges
        auto beginSpace = result.find_first_of(whitespace);
        while (beginSpace != std::string::npos)
        {
            const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
            const auto range = endSpace - beginSpace;

            result.replace(beginSpace, range, fill);

            const auto newStart = beginSpace + fill.length();
            beginSpace = result.find_first_of(whitespace, newStart);
        }

        return result;
    }
#endif

};


};
