#pragma once

#include <vector>
#include <string>
#include "celllib/celllib.h"
#include "common/visitor.h"
#include "net.h"

namespace ChipDB
{

#if 0

class AbstractInstance; // pre-declaration

class AbstractInstance
{
public:

    virtual ~AbstractInstance() {}

    IMPLEMENT_ACCEPT;

    struct Pin
    {
        Net* m_connection;
    };

    virtual ssize_t lookupPinIndex(const std::string &name) const = 0;
    virtual const Pin* lookupPin(const std::string &name) const = 0;
    virtual const Pin* lookupPin(size_t pinIndex) const = 0;
    virtual Pin* lookupPin(const std::string &name) = 0;
    virtual Pin* lookupPin(size_t pinIndex) = 0;

    std::string         m_insName;
    std::vector<Net*>   m_pinToNet;     ///< pin connection to net

    virtual const PinInfo& lookupPinInfo(size_t pinIndex) const = 0;

    /** get the underlying cell or module name */
    virtual std::string getArchetypeName() const
    {
        return "";
    }

    virtual bool isModule() const
    {
        return false;
    }
};


class CellInstance : public AbstractInstance
{
public:

    IMPLEMENT_ACCEPT;

    CellInstance(const Cell *cell) : m_cell(cell)
    {
        m_pins.resize(cell->m_pins.size());
    }

    virtual ssize_t lookupPinIndex(const std::string &name) const override;
    virtual const Pin* lookupPin(const std::string &name) const override;
    virtual const Pin* lookupPin(size_t pinIndex) const override;
    virtual Pin* lookupPin(const std::string &name);
    virtual Pin* lookupPin(size_t pinIndex);    

    virtual const PinInfo& lookupPinInfo(size_t pinIndex) const = 0;

    virtual bool isModule() const
    {
        return false;
    }

    const Cell* getCell() const
    {
        return m_cell;
    }

    virtual std::string getArchetypeName() const
    {
        if (m_cell != nullptr)
            return m_cell->m_name;
        else
            return "UNKNOWN";
    }

protected:
    const Cell *m_cell;   // weak ptr to cell
};


class ModuleInstance : public AbstractInstance
{
public:

    IMPLEMENT_ACCEPT;

    ModuleInstance(const Module *module) : m_module(module)
    {
        m_pins.resize(module->m_pins.size());
    }

    virtual ssize_t lookupPinIndex(const std::string &name) const override;
    virtual const Pin* lookupPin(const std::string &name) const override;
    virtual const Pin* lookupPin(size_t pinIndex) const override;
    virtual Pin* lookupPin(const std::string &name);
    virtual Pin* lookupPin(size_t pinIndex);       

    virtual bool isModule() const
    {
        return true;
    }

    virtual std::string getArchetypeName() const
    {
        if (m_module != nullptr)
            return m_module->m_name;
        else
            return "UNKNOWN";
    }

    const Module* getModule() const
    {
        return m_module;
    }

protected:
    const Module *m_module;
};
#endif

class Netlist
{
    
};

};  // namespace