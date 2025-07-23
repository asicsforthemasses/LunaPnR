// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <array>
#include <stdexcept>
#include "visitor.h"
#include "cell.h"
#include "module.h"
#include "net.h"

namespace ChipDB
{

enum class InstanceType
{
    UNKNOWN = 0,
    ABSTRACT,
    CELL,
    MODULE,
    PIN,
    NETCON
};

std::string toString(const InstanceType &t);

class Instance
{
public:
    Instance() : m_cell(nullptr) {}

    Instance(const std::string &name, InstanceType instype, const std::shared_ptr<Cell> cell)
        : m_name(name), m_insType(instype), m_cell(cell)
    {
        m_pinToNet.resize(cell->getNumberOfPins(), ChipDB::ObjectNotFound);
    }

    virtual ~Instance() = default;

    IMPLEMENT_ACCEPT

    [[nodiscard]] constexpr auto insType() const noexcept
    {
        return m_insType;
    }

    /** returns true if the instance is a module */
    [[nodiscard]] constexpr bool isModule() const noexcept
    {
        return m_insType == InstanceType::MODULE;
    }

    [[nodiscard]] constexpr bool isCell() const noexcept
    {
        return m_insType == InstanceType::CELL;
    }

    [[nodiscard]] constexpr bool isAbstract() const noexcept
    {
        return m_insType == InstanceType::ABSTRACT;
    }

    [[nodiscard]] constexpr bool isPin() const noexcept
    {
        return m_insType == InstanceType::PIN;
    }

    /** returns true if the instance is placed or placed and fixed */
    [[nodiscard]] constexpr bool isPlaced() const
    {
        return ((m_placementInfo == PlacementInfo::PLACED) ||
            (m_placementInfo == PlacementInfo::PLACEDANDFIXED));
    }

    /** returns true if the instance is placed and fixed */
    [[nodiscard]] constexpr bool isFixed() const
    {
        return (m_placementInfo == PlacementInfo::PLACEDANDFIXED);
    }

    /** get area in um² */
    [[nodiscard]] double getArea() const noexcept;

    /** return the underlying cell/module name */
    [[nodiscard]] std::string getArchetypeName() const noexcept;

    /** return the cell size of the instance */
    [[nodiscard]] const Coord64 instanceSize() const noexcept
    {
        if (m_cell == nullptr)
            return Coord64{0,0};

        return m_cell->m_size;
    }

    /** get access to the cell/module, if there is one */
    [[nodiscard]] const std::shared_ptr<Cell> cell() const noexcept
    {
        return m_cell;
    }

    /** get access to the cell/module, if there is one */
    [[nodiscard]] const ChipDB::PinInfoList& pins() const
    {
        if (!m_cell)
        {
            throw std::runtime_error("Instance::pins() cannot return information because m_cell is nullptr.");
        }

        return m_cell->m_pins;
    }

    /** return the center position of the instance */
    [[nodiscard]] Coord64 getCenter() const noexcept
    {
        if (m_cell != nullptr)
            return Coord64{m_pos.m_x + m_cell->m_size.m_x/2, m_pos.m_y + m_cell->m_size.m_y/2};
        else
            return m_pos;
    }

    /** set the center position of the instance */
    void setCenter(const ChipDB::Coord64 &p) noexcept
    {
        if (m_cell != nullptr)
        {
            m_pos = Coord64{p.m_x - m_cell->m_size.m_x/2, p.m_y - m_cell->m_size.m_y/2};
        }
        else
        {
            m_pos = p;
        }
    }

    /** get the name of the instance */
    [[nodiscard]] std::string name() const noexcept
    {
        return m_name;
    }

    /** get the name of the instance */
    [[nodiscard]] ChipDB::Rect64 rect() const noexcept
    {
        if (m_cell)
        {
            return {m_pos, m_pos+ m_cell->m_size};
        }
        return {m_pos,m_pos};
    }

    struct Pin
    {
        constexpr bool isValid() const
        {
            return ((m_pinKey != ObjectNotFound) && m_pinInfo);
        }

        std::string name() const
        {
            if (m_pinInfo)
            {
                return m_pinInfo->name();
            }
            return "INVALID PININFO";
        }

        NetObjectKey    m_netKey = ObjectNotFound;  ///< key of connected net
        PinObjectKey    m_pinKey = ObjectNotFound;  ///< key of this pin
        std::shared_ptr<PinInfo> m_pinInfo;         ///< information about the pin

        constexpr PinObjectKey pinKey() const
        {
            return m_pinKey;
        }

        constexpr NetObjectKey netKey() const
        {
            return m_netKey;
        }
    };

    virtual Pin getPin(PinObjectKey pinKey) const;
    virtual Pin getPin(const std::string &pinName) const;
    virtual bool setPinNet(PinObjectKey pinKey, NetObjectKey netKey);
    virtual bool disconnectPin(PinObjectKey pinKey);

    virtual size_t getNumberOfPins() const;

    class ConnectionIterators
    {
    public:
        ConnectionIterators(const Instance *ins) : m_instance(ins) {}

        auto begin()
        {
            return m_instance->m_pinToNet.begin();
        }

        auto end()
        {
            return m_instance->m_pinToNet.end();
        }

        auto begin() const
        {
            return m_instance->m_pinToNet.begin();
        }

        auto end() const
        {
            return m_instance->m_pinToNet.end();
        }

    protected:
        const Instance *m_instance;
    };

    auto connections()
    {
        return ConnectionIterators(this);
    }

    auto connections() const
    {
        return ConnectionIterators(this);
    }

    /** returns true if this is a CORE filler cell */
    [[nodiscard]] bool isCoreFiller() const noexcept;

    /** returns true if this is a CORE decap cell */
    [[nodiscard]] bool isCoreDecap() const noexcept;

    Coord64         m_pos{0,0};                                     ///< lower-left position of the instance
    Orientation     m_orientation{Orientation::R0};                 ///< orientation of the cell instance
    PlacementInfo   m_placementInfo{PlacementInfo::UNPLACED};       ///< placement status
    uint32_t        m_flags{0};                                     ///< non-persistent generic flags that can be used by algorithms

protected:
    const std::shared_ptr<Cell> m_cell;                 ///< access to pins of cell
    std::vector<NetObjectKey>   m_pinToNet;             ///< connections from pin to net
    std::string     m_name;                             ///< name of the instance
    InstanceType    m_insType{InstanceType::UNKNOWN};
};

};
