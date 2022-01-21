#pragma once

#include <iostream>
#include <vector>
#include "celllib/pin.h"
#include "netlist/instance.h"
#include "netlist/netlist.h"

namespace LunaCore::Dot
{

class AbstractNodeDecorator
{
public:
    virtual void decorate(const std::shared_ptr<ChipDB::InstanceBase> ins, std::ostream &os) = 0;
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

    void writeInputs(std::ostream &os, const std::shared_ptr<ChipDB::InstanceBase> ins);
    void writeOutputs(std::ostream &os, const std::shared_ptr<ChipDB::InstanceBase> ins);
    std::string escapeString(const std::string &txt);

    const std::shared_ptr<ChipDB::Module> m_module;
};

};  // namespace