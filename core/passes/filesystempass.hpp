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

class LSPass : public Pass
{
public:
    LSPass() : Pass("ls")
    {
    }

    virtual ~LSPass() = default;

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool execute(Database &database) override
    {
        std::filesystem::path path = ".";
        if (!m_params.empty())
        {
            path = m_params.front();
        }

        for (const auto & entry : std::filesystem::directory_iterator(path))
        {
            Logging::logInfo("  %s\n", entry.path().string().c_str());
        }

        return true;
    }

    /**
        returns help text for a pass.
    */
    std::string help() const noexcept override
    {
        std::stringstream ss;
        ss << "ls - list a directory\n";
        return ss.str();
    }

    /**
        returns a one-line short help text for a pass.
    */
    virtual std::string shortHelp() const noexcept
    {
        return "list a directory";
    }

    /**
        Initialize a pass. this is called by registerPass()
    */
    bool init() override
    {
        return true;
    }
};

class CDPass : public Pass
{
public:
    CDPass() : Pass("cd")
    {
    }

    virtual ~CDPass() = default;

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool execute(Database &database) override
    {
        std::filesystem::path path = ".";
        if (m_params.empty())
        {
            Logging::logError("Missing directory parameter\n");
            return false;
        }

        try
        {
            std::filesystem::current_path(m_params.front());
        }
        catch(const std::exception& e)
        {
            Logging::logError("%s\n", e.what());
        }

        return true;
    }

    /**
        returns help text for a pass.
    */
    std::string help() const noexcept override
    {
        std::stringstream ss;
        ss << "cd - change directory\n";
        return ss.str();
    }

    /**
        returns a one-line short help text for a pass.
    */
    virtual std::string shortHelp() const noexcept
    {
        return "change directory";
    }

    /**
        Initialize a pass. this is called by registerPass()
    */
    bool init() override
    {
        return true;
    }
};

class CWDPass : public Pass
{
public:
    CWDPass() : Pass("cwd")
    {
    }

    virtual ~CWDPass() = default;

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool execute(Database &database) override
    {
        auto path = std::filesystem::current_path();
        Logging::logError("%s\n", path.string().c_str());
        return true;
    }

    /**
        returns help text for a pass.
    */
    std::string help() const noexcept override
    {
        std::stringstream ss;
        ss << "cwd - print current directory\n";
        return ss.str();
    }

    /**
        returns a one-line short help text for a pass.
    */
    virtual std::string shortHelp() const noexcept
    {
        return "print current directory";
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
