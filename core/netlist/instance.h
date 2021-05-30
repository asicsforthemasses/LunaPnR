#pragma once

#include <string>
#include <vector>
#include "celllib/celllib.h"
#include "netlist/net.h"
#include "common/visitor.h"

namespace ChipDB
{

class Net;  // pre-declaration

class Instance
{
public:
    Instance(const Cell *cell) : m_cell(cell) 
    {
        m_pinToNet.resize(cell->m_pins.size());
    }

    IMPLEMENT_ACCEPT;

    std::string m_name; ///< name of instance

    /** get access to the cell/module */
    const Cell* cell() const
    {
        return m_cell;
    }

    /** return the cell/module name */
    virtual std::string getArchetypeName() const;

    bool isModule() const;
    
    /** get pin information from the underlying cell or module 
     *  returns nullptr if pin not found.
    */
    const PinInfo* getPinInfo(ssize_t pinIndex) const;

    /** get pin information from the underlying cell or module 
     *  returns nullptr if pin not found.
    */
    const PinInfo* getPinInfo(const std::string &pinName) const;


    /** get pin index by name. returns -1 when not found. 
    */
    const ssize_t getPinIndex(const std::string &pinName) const;

    /** connect pin with specified index to the given net. 
     *  returns true if succesful.
    */
    bool connect(ssize_t pinIndex, Net *net);

    /** connect pin with specified name to the given net. 
     *  returns true if succesful.
    */    
    bool connect(const std::string &pinName, Net *net);

    Net* getConnectedNet(ssize_t pinIndex);

    struct ConnectionIterator
    {
        ConnectionIterator(Instance &ins) : m_ins(ins) {}

        auto begin()
        {
            return m_ins.m_pinToNet.begin();
        }

        auto end()
        {
            return m_ins.m_pinToNet.end();
        }

        Instance& m_ins;
    };


    struct ConstConnectionIterator
    {
        ConstConnectionIterator(const Instance &ins) : m_ins(ins) {}

        auto begin() const
        {
            return m_ins.m_pinToNet.begin();
        }

        auto end() const
        {
            return m_ins.m_pinToNet.end();
        }

        const Instance& m_ins;
    };

    struct PinInfoIterator
    {
        PinInfoIterator(const Cell *cell) : m_cell(cell) {}

        auto begin() const
        {
            return m_cell->m_pins.begin();
        }

        auto end() const
        {
            return m_cell->m_pins.end();
        }

        const Cell *m_cell;
    };

    auto connections()
    {
        return ConnectionIterator(*this);
    }

    auto connections() const
    {
        return ConstConnectionIterator(*this);
    }

    auto pinInfos() const
    {
        return PinInfoIterator(m_cell);
    }

protected:
    std::vector<Net*>   m_pinToNet;  ///< connections from pin to net
    const Cell* m_cell;
};

};
