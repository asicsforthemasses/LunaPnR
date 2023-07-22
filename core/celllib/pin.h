// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "common/dbtypes.h"
#include "common/namedstorage.h"
#include "common/geometry.h"
#include "common/visitor.h"

namespace ChipDB
{

enum class IOType : uint8_t
{
    UNKNOWN  = 0,
    INPUT,
    OUTPUT,
    OUTPUT_TRI,
    IO,
    ANALOG,
    POWER,
    GROUND
};

std::string toString(const IOType &iotype);

/** describes the location of an accessible pin location on a cell. */


struct PinInfo
{
PinInfo() :
        m_iotype(IOType::UNKNOWN),
        m_clock(false),
        m_offset({0,0}),
        m_cap(0.0),
        m_maxCap(0.0),
        m_maxFanOut(0)
        {}

PinInfo(const std::string &name) :
        m_name(name),
        m_iotype(IOType::UNKNOWN),
        m_clock(false),
        m_offset({0,0}),
        m_cap(0.0),
        m_maxCap(0.0),
        m_maxFanOut(0)
        {}

    IMPLEMENT_ACCEPT;

    virtual ~PinInfo() = default;

    std::string name() const noexcept
    {
        return m_name;
    }

    std::string m_name;     ///< pin name

    IOType      m_iotype;   ///< in/out type of pin
    
    bool        m_clock;    ///< true if this is a clock pin
    Coord64     m_offset;   ///< pin offset w.r.t. lower left cell corner

    double      m_cap;      ///< load capacitance of pin in farads (inputs only)
    double      m_maxCap;   ///< maximum load capacitance in farads (outputs only)
    uint32_t    m_maxFanOut;///< maximum number of connections/loads (outputs only)

    std::string m_function; ///< pin function expression from the liberty file (outputs only)
    std::string m_tristateFunction; ///< tri-state function (OUTPUT_TRI pins only)

    bool isOutput() const
    {
        return (m_iotype == IOType::OUTPUT) || (m_iotype == IOType::OUTPUT_TRI)
            || (m_iotype == IOType::IO);
    }

    bool isInput() const
    {
        return (m_iotype == IOType::INPUT) || (m_iotype == IOType::IO);
    }

    bool isPGPin() const
    {
        return (m_iotype == IOType::POWER) || (m_iotype == IOType::GROUND);
    }

    bool isIO() const
    {
        return m_iotype == IOType::IO;
    }

    bool isClock() const
    {
        return m_clock;
    }    

    /** geometry description of pin */
    std::unordered_map<std::string /* layer name */, GeometryObjects> m_pinLayout;

    struct Location
    {
        std::string     m_layer;
        ChipDB::Rect64  m_rect;
    };

    /** valid accessible locations determined by the given routing grid */
    std::vector<Location> m_pinLocations;
};

class PinInfoList
{
    using ContainerType = std::vector<std::shared_ptr<PinInfo> >;

public:

    using iterator = typename ContainerType::iterator;
    using const_iterator = typename ContainerType::const_iterator;

    /** create a pin by name. if a pin by that name already exists, return that */
    KeyObjPair<PinInfo> createPin(const std::string &name);

    void clear()
    {
        m_pins.clear();   
    }

    size_t size() const
    {
        return m_pins.size();
    }

    std::shared_ptr<PinInfo> at(ObjectKey pinKey)
    {
        return m_pins.at(pinKey);
    }

    const std::shared_ptr<PinInfo> at(ObjectKey pinKey) const
    {
        return m_pins.at(pinKey);
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

    /** access pin by key. returns nullptr if not found */
    std::shared_ptr<PinInfo> operator[](ObjectKey pinKey)
    {
        if ((pinKey < m_pins.size()) && (pinKey >= 0))
            return m_pins[pinKey];
        
        return nullptr;
    }

    /** access pin by key. returns nullptr if not found */
    const std::shared_ptr<PinInfo> operator[](ObjectKey pinKey) const
    {
        if ((pinKey < m_pins.size()) && (pinKey >= 0))
            return m_pins[pinKey];
        
        return nullptr;
    }

    /** access pin by name. returns KeyObjPair of pin */
    KeyObjPair<PinInfo> operator[](const std::string &name);

    /** access pin by name. returns KeyObjPair of pin */
    KeyObjPair<PinInfo> operator[](const std::string &name) const;

protected:   

    KeyObjPair<PinInfo> find(const std::string &name) const;
    ContainerType m_pins;
};

};