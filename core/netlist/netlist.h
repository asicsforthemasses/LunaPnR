#pragma once

#include <vector>
#include <string>
#include "celllib/celllib.h"
#include "common/visitor.h"

namespace ChipDB
{

class AbstractInstance; // pre-declaration
class Net;

struct Pin
{
    Net    *m_net;      ///< connected net
};

class Net
{
public:
    std::string m_name;         ///< net name
    bool        m_isPortNet;    ///< when true, this net connectes to a module port

    void setPortNet(bool isPortNet)
    {
        m_isPortNet = isPortNet;
    }

    struct NetConnect
    {
        AbstractInstance *m_instance;
        size_t           m_pinIndex;
    };

    /** connect the net to the instance pin */
    void addConnection(AbstractInstance *instance, size_t pinIndex);

    std::vector<NetConnect> m_connections;
};

class AbstractInstance
{
public:

    virtual ~AbstractInstance() {}

    IMPLEMENT_ACCEPT;

    virtual ssize_t lookupPinIndex(const std::string &name) const = 0;
    virtual const Pin* lookupPin(const std::string &name) const = 0;
    virtual const Pin* lookupPin(size_t pinIndex) const = 0;
    virtual Pin* lookupPin(const std::string &name) = 0;
    virtual Pin* lookupPin(size_t pinIndex) = 0;

    std::string m_insName;
    std::vector<Pin> m_pins;

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

    virtual bool isModule() const
    {
        return false;
    }

    const Cell* getCell() const
    {
        return m_cell;
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

protected:
    const Module *m_module;
};

class Netlist
{
    
};

};  // namespace