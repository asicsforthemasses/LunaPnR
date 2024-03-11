// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <span>
#include <list>
#include <unordered_map>

#include "database/database.h"

namespace LunaCore::Passes
{

using ArgList = std::span<std::string>;

/** Baseclass for a pass */
class Pass
{
public:
    Pass(const std::string &name) : m_name(name) {}

    virtual ~Pass() = default;

    [[nodiscard]] const std::string& name() const noexcept
    {
        return m_name;
    }

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] virtual bool execute(Database &database, const ArgList &args)
    {
        return false;
    }

    /**
        returns help text for a pass.
    */
    virtual std::string help() const noexcept
    {
        std::stringstream ss;
        ss << "Not help available for " << m_name << "\n";
        return ss.str();
    }

    /**
        Initialize a pass. this is called by registerPass()
    */
    virtual bool init()
    {
        return false;
    }

protected:

    void processParameters(ArgList args);

    std::unordered_map<std::string /* param name*/, std::string /* param value */> m_namedParams;
    std::list<std::string> m_params;

    std::string m_name;
};

bool executePass(Database &database, const std::string &passName, ArgList args);
bool registerPass(Pass *pass);

};