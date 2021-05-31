#pragma once

#include <stdint.h>
#include <string>
#include "common/dbtypes.h"
#include "common/visitor.h"

namespace ChipDB
{

enum PinIOType : uint8_t
{
    IO_UNKNOWN  = 0,
    IO_INPUT,
    IO_OUTPUT,
    IO_OUTPUT_TRI,
    IO_IO,
    IO_POWER,
    IO_GROUND
};

std::string toString(const PinIOType &iotype);

struct PinInfo
{
PinInfo() :
        m_iotype(IO_UNKNOWN),
        m_clock(false),
        m_offset({0,0}),
        m_cap(0.0),
        m_maxCap(0.0),
        m_maxFanOut(0)
        {}

PinInfo(const std::string &name) :
        m_name(name),
        m_iotype(IO_UNKNOWN),
        m_clock(false),
        m_offset({0,0}),
        m_cap(0.0),
        m_maxCap(0.0),
        m_maxFanOut(0)
        {}

    IMPLEMENT_ACCEPT;

    std::string m_name;     ///< pin name

    PinIOType   m_iotype;   ///< in/out type of pin
    
    bool        m_clock;    ///< true if this is a clock pin
    Coord64     m_offset;   ///< pin offset w.r.t. lower left cell corner

    double      m_cap;      ///< load capacitance of pin in farads (inputs only)
    double      m_maxCap;   ///< maximum load capacitance in farads (outputs only)
    uint32_t    m_maxFanOut;///< maximum number of connections/loads (outputs only)

    std::string m_function; ///< pin function expression from the liberty file (outputs only)
    std::string m_tristateFunction; ///< tri-state function (OUTPUT_TRI pins only)

    bool isOutput() const
    {
        return (m_iotype == IO_OUTPUT) || (m_iotype == IO_OUTPUT_TRI)
            || (m_iotype == IO_IO);
    }

    bool isInput() const
    {
        return (m_iotype == IO_INPUT) || (m_iotype == IO_IO);
    }

    bool isPGPin() const
    {
        return (m_iotype == IO_POWER) || (m_iotype == IO_GROUND);
    }

    bool isIO() const
    {
        return m_iotype == IO_IO;
    }

    bool isClock() const
    {
        return m_clock;
    }    
};

class PinInfoList
{
public:

    PinInfo& createPin(const std::string &name);

    void resize(size_t num)
    {
        m_pins.resize(num);
    }

    size_t size() const
    {
        return m_pins.size();
    }

    auto at(size_t index)
    {
        return m_pins.at(index);
    }

    auto at(size_t index) const
    {
        return m_pins.at(index);
    }

    auto begin()
    {
        return m_pins.begin();
    }

    auto end()
    {
        return m_pins.end();
    }

    auto begin() const
    {
        return m_pins.begin();
    }

    auto end() const
    {
        return m_pins.end();
    }

    /** access pin directly with bounds checking */
    PinInfo* operator[](const ssize_t index)
    {
        if ((index < m_pins.size()) && (index >= 0))
            return &m_pins[index];
        
        return nullptr;
    }

    /** access pin directly with bounds checking */
    const PinInfo* operator[](const ssize_t index) const
    {
        if ((index < m_pins.size()) && (index >= 0))
            return &m_pins[index];
        
        return nullptr;
    }

    PinInfo* lookup(const std::string &name);
    const PinInfo* lookup(const std::string &name) const;

    /** find the index of a pin by name, returns -1 if not found */
    ssize_t lookupIndex(const std::string &name) const;

protected:   
    std::vector<PinInfo>::iterator find(const std::string &name);
    std::vector<PinInfo>::const_iterator find(const std::string &name) const;
    std::vector<PinInfo> m_pins;
};

};