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
    virtual void decorate(const ChipDB::Instance *ins, std::ostream &os) = 0;
};

/** Writer writes module netlist in Graphviz DOT format */
class Writer
{
public:
    static bool write(const ChipDB::Module *module,
        std::ostream &os, AbstractNodeDecorator *decorator = nullptr);

    static bool write(const ChipDB::Instance *moduleInstance,
        std::ostream &os, AbstractNodeDecorator *decorator = nullptr);

protected:
    Writer(const ChipDB::Module *module);

    bool execute(std::ostream &os, AbstractNodeDecorator *decorator);

#if 0
    struct netPin
    {
        netPin() : m_driver(false), m_toppin(false), m_insPtr(nullptr) {}
        netPin(const netPin &pin) = default;
        netPin(netPin &pin) = default;

        std::string m_insName;
        bool        m_driver;
        bool        m_toppin;

        ChipDB::Instance*       m_insPtr;
        ChipDB::Instance::Pin   m_pin;

        bool operator==(const netPin &other) const
        {
            if (!m_pin.isValid()) return false;
            if (!other.m_pin.isValid()) return false;

            return (m_insPtr == other.m_insPtr) && (m_pin.m_pinIndex == other.m_pin.m_pinIndex);
        }

        bool operator!=(const netPin &other) const
        {
            return (m_insPtr != other.m_insPtr) || (m_pin.m_pinIndex != other.m_pin.m_pinIndex);
        }     

        bool operator==(netPin &other) const
        {
            return (m_insPtr == other.m_insPtr) && (m_pin.m_pinIndex == other.m_pin.m_pinIndex);
        }

        bool operator!=(netPin &other) const
        {
            return (m_insPtr != other.m_insPtr) || (m_pin.m_pinIndex != other.m_pin.m_pinIndex);
        }            
    };
#endif

    void writeInputs(std::ostream &os, const ChipDB::Instance *instance);
    void writeOutputs(std::ostream &os, const ChipDB::Instance *instance);
    std::string escapeString(const std::string &txt);

    const ChipDB::Module *m_module;
};

};  // namespace