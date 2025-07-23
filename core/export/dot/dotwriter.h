// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only


#pragma once

#include <iostream>
#include <vector>
#include "database/database.h"

namespace LunaCore::Dot
{

class AbstractNodeDecorator
{
public:
    virtual void decorate(const std::shared_ptr<ChipDB::Instance> ins, std::ostream &os) = 0;
};

/** Writer writes module netlist in Graphviz DOT format */
class Writer
{
public:
    static bool write(std::ostream &os, const std::shared_ptr<ChipDB::Module> module,
        AbstractNodeDecorator *decorator = nullptr);

    static bool write(std::ostream &os, const std::shared_ptr<ChipDB::Instance> modInstance,
        AbstractNodeDecorator *decorator = nullptr);

protected:
    Writer(const std::shared_ptr<ChipDB::Module> module);

    bool execute(std::ostream &os, AbstractNodeDecorator *decorator);

    void writeInputs(std::ostream &os, const std::shared_ptr<ChipDB::Instance> ins);
    void writeOutputs(std::ostream &os, const std::shared_ptr<ChipDB::Instance> ins);
    std::string escapeString(const std::string &txt);

    const std::shared_ptr<ChipDB::Module> m_module;
};

};  // namespace