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
    Pass(const std::string &name) : m_name(name)
    {
        registerNamedParameter("help", "");
    }

    virtual ~Pass() = default;

    [[nodiscard]] const std::string& name() const noexcept
    {
        return m_name;
    }

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool run(Database &database, const ArgList &args)
    {
        if (!processParameters(args))
        {
            return false;
        }

        return execute(database);
    }

    /**
        returns help text for a pass.
    */
    virtual std::string help() const noexcept
    {
        std::stringstream ss;
        ss << "No help available for " << m_name << "\n";
        return ss.str();
    }

    /**
        returns a one-line short help text for a pass.
    */
    virtual std::string shortHelp() const noexcept
    {
        return "no short help";
    }

    /**
        Initialize a pass. this is called by registerPass()
    */
    virtual bool init()
    {
        return false;
    }

protected:

    /** implementer must override this for each pass */
    [[nodiscard]] virtual bool execute(Database &database) = 0;

    /** register a named parameter so it can be recoqnized by the argument parser.
        if required is true, the argmument parser will return with an error if the
        parameter is missing.
     */
    void registerNamedParameter(const std::string &name,
        const std::string &defaultValue = "",
        int argCount = 0,
        bool required = false);

    /** parse the args and fill m_namedParams and m_params */
    bool processParameters(ArgList args);

    struct NamedParameterDefinition
    {
        std::string m_name;         ///< name of the parameter
        std::string m_default;      ///< parameter default value
        int  m_argCount{0};         ///< number of arguments of the parameter
        bool m_required{false};     ///< true if the pass requires this argument
    };

    std::unordered_map<std::string /* param name */, NamedParameterDefinition> m_namedParamDefs;

    std::unordered_map<std::string /* param name */, std::vector<std::string /* param value */> > m_namedParams;
    std::list<std::string> m_params;

    std::string m_name;
};

bool runPass(Database &database, const std::string &passName, ArgList args);
bool registerPass(Pass *pass);
void displayMainHelp();

};