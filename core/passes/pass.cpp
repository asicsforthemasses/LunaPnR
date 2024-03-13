// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "pass.hpp"

#include <memory>
#include <map>
#include <algorithm>

#include "common/logging.h"

namespace LunaCore::Passes
{

struct Passes
{
    bool runPass(Database &database, const std::string &passName, ArgList args)
    {
        if (passName == "help")
        {
            displayMainHelp();
            return true;
        }

        if (!m_passes.contains(passName))
        {
            Logging::doLog(Logging::LogType::ERROR, "Pass %s not found\n", passName.c_str());
            return false;
        }

        return m_passes.at(passName)->run(database, args);
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

    void displayMainHelp()
    {
        Logging::doLog(Logging::LogType::INFO, "Main help:\n");

        for(auto const& pass : m_passes)
        {
            auto passName = pass.first;
            std::string indentStr;
            int spaceCount = std::max(0UL, 30 - passName.length());
            indentStr.assign(spaceCount, ' ');

            Logging::doLog(Logging::LogType::INFO, "    %s%s%s\n",
                passName.c_str(),
                indentStr.c_str(),
                pass.second->shortHelp().c_str() );
        }
    }

    std::map<std::string /* pass name */, std::unique_ptr<Pass>> m_passes;
};



static Passes gs_passes;

bool runPass(Database &database, const std::string &passName, ArgList args)
{
    return gs_passes.runPass(database, passName, args);
}

bool registerPass(Pass *pass)
{
    return gs_passes.registerPass(pass);
}

enum class ParamState
{
    IDLE,
    NAMEDPARAM
};

void Pass::registerNamedParameter(const std::string &name,
    const std::string &defaultValue,
    int argCount,
    bool required)
{
    m_namedParamDefs[name].m_name = name;
    m_namedParamDefs[name].m_argCount = argCount;
    m_namedParamDefs[name].m_required = required;
    m_namedParamDefs[name].m_default = defaultValue;
}

bool Pass::processParameters(ArgList args)
{
    auto paramState = ParamState::IDLE;

    //FIXME: check the numner of parameter arguments
    // and produce an error if there is no match
    //
    // also add support for variadic parameters:
    // just read until there is another named parameter
    // or an end of line

    m_namedParams.clear();
    m_params.clear();

    std::string paramName;
    int paramArgCount{0};
    for(auto const& arg : args)
    {
        switch(paramState)
        {
        case ParamState::IDLE:
            if (arg.starts_with('-'))
            {
                paramName = arg.substr(1);

                auto iter = m_namedParamDefs.find(paramName);
                if (iter != m_namedParamDefs.end())
                {
                    paramArgCount = iter->second.m_argCount;
                    if (paramArgCount != 0)
                    {
                        paramState = ParamState::NAMEDPARAM;
                    }

                    m_namedParams[paramName];
                }
                else
                {
                    // error cannot find named parameter!
                    std::stringstream ss;
                    ss << "Pass " << m_name << " does not recognize parameter " << paramName << "\n";
                    Logging::doLog(Logging::LogType::ERROR, ss.str());
                    return false;
                }
            }
            else
            {
                m_params.push_back(arg);
            }
            break;
        case ParamState::NAMEDPARAM:
            paramArgCount--;
            if (paramArgCount == 0)
            {
                paramState = ParamState::IDLE;
            }

            if (arg.starts_with('-'))
            {
                std::stringstream ss;
                ss << "Unexpectedly found a named parameter " << arg.substr(1) << " while parsing " << paramName << "\n";
                Logging::doLog(Logging::LogType::ERROR, ss.str());
                return false;
            }

            m_namedParams[paramName].push_back(arg);
            break;
        }
    }

    // if -help was specified, display it
    if (m_namedParams.contains("help"))
    {
        Logging::doLog(Logging::LogType::INFO,  help());
        return false;
    }

    // check that all the required parameters are there
    for (auto const paramDef : m_namedParamDefs)
    {
        if (paramDef.second.m_required)
        {
            if (!m_namedParams.contains(paramDef.first))
            {
                std::stringstream ss;
                ss << "Required parameter " << paramDef.first << " is missing.\n";
                Logging::doLog(Logging::LogType::ERROR, ss.str());
                return false;
            }
        }
    }

    return true;
}

};