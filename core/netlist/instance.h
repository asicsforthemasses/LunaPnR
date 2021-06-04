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
    Instance(const Cell *cell) : m_parent(nullptr), m_cell(cell), m_orientation(ORIENT_R0), m_flags(0)
    {
        m_pinToNet.resize(cell->m_pins.size());
    }

    Instance(const Cell *cell, Instance *parent) : m_parent(parent), m_cell(cell), m_orientation(ORIENT_R0), m_flags(0)
    {
        m_pinToNet.resize(cell->m_pins.size());
    }

    IMPLEMENT_ACCEPT;

    std::string m_name;     ///< name of instance

    Instance    *m_parent;  ///< parent instance (must be a module, used for flat netlist name resolution)

    /** get access to the cell/module */
    const Cell* cell() const
    {
        return m_cell;
    }

    /** return the underlying cell/module name */
    virtual std::string getArchetypeName() const;

    /** returns true if the instance is a module */
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

    /** get the number of pins on this instance */
    const size_t getNumberOfPins() const;

    /** connect pin with specified index to the given net. 
     *  returns true if succesful.
    */
    bool connect(ssize_t pinIndex, Net *net);

    /** connect pin with specified name to the given net. 
     *  returns true if succesful.
    */    
    bool connect(const std::string &pinName, Net *net);

    /** returns the net connected to a pin with a given index.
     *  if the pin does not exist, it return nullptr.
    */
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

    /** returns an iterable object to the connections of the instance */
    auto connections()
    {
        return ConnectionIterator(*this);
    }

    /** returns an iterable object to the connections of the instance */
    auto connections() const
    {
        return ConstConnectionIterator(*this);
    }

    /** returns an iterable object to the pin information of the instance */
    auto pinInfos() const
    {
        return PinInfoIterator(m_cell);
    }

    /** return the cell size of the instance */
    const Coord64 cellSize() const
    {
        if (m_cell == nullptr)
            return Coord64{0,0};
            
        return m_cell->m_size;
    }

    /** return the center position of the instance */
    Coord64 getCenter() const
    {
        if (m_cell != nullptr)
            return Coord64{m_pos.m_x + m_cell->m_size.m_x/2, m_pos.m_y + m_cell->m_size.m_y/2};
        else
            return m_pos;
    }

    Coord64     m_pos;          ///< lower-left position of the instance
    Orientation m_orientation;  ///< orientation of the cell instance

    uint32_t    m_flags;        ///< non-persistent generic flags that can be used by algorithms

protected:
    std::vector<Net*>   m_pinToNet;  ///< connections from pin to net
    const Cell* m_cell;
};

};
