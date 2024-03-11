// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "pass.hpp"

#include <memory>
#include "common/logging.h"

namespace LunaCore::Passes
{

struct Passes
{
    bool executePass(Database &database, const std::string &passName, ArgList args)
    {
        if (!m_passes.contains(passName))
        {
            Logging::doLog(Logging::LogType::ERROR, "Pass %s not found\n", passName.c_str());
            return false;
        }

        return m_passes.at(passName)->execute(database, args);
    }

    bool registerPass(Pass *pass)
    {
        if (pass == nullptr)
        {
            Logging::doLog(Logging::LogType::ERROR, "registerPass: Pass is nullptr\n");
            return false;
        }

        if (pass->name().empty())
        {
            Logging::doLog(Logging::LogType::ERROR, "registerPass: Pass name cannot be empty\n");
            return false;
        }

        if (m_passes.contains(pass->name()))
        {
            Logging::doLog(Logging::LogType::WARNING, "registerPass: Pass with name %s already exists, replacing.\n", pass->name().c_str());
        }

        m_passes[pass->name()].reset(pass);
        return true;
    }

    std::unordered_map<std::string /* pass name */, std::unique_ptr<Pass>> m_passes;
};

static Passes gs_passes;

bool executePass(Database &database, const std::string &passName, ArgList args)
{
    return gs_passes.executePass(database, passName, args);
}

bool registerPass(Pass *pass)
{
    return gs_passes.registerPass(pass);
}

void Pass::processParameters(ArgList args)
{
    m_namedParams.clear();
    m_params.clear();

    bool namedParam = false;
    std::string paramName;
    for(auto const& arg : args)
    {
        if (namedParam)
        {
            m_namedParams[paramName] = arg;
            namedParam = false;
        }
        else
        {
            if (arg.starts_with('-'))
            {
                paramName = arg.substr(1);
                namedParam = true;
            }
            else
            {
                m_params.push_back(arg);
            }
        }
    }
}

};