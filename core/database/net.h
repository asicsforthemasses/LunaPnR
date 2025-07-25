// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only


#pragma once

#include <string>
#include <vector>
#include "dbtypes.h"
#include "visitor.h"

namespace ChipDB
{

//class Instance; // pre-declaration

class Net
{
public:
    Net() : m_id(-1), m_flags(0), m_isPortNet(false), m_isClockNet(false) {}

    Net(const std::string &name) : m_name(name), m_id(-1), m_flags(0),
        m_isPortNet(false), m_isClockNet(false) {}

    IMPLEMENT_ACCEPT;

    std::string name() const noexcept
    {
        return m_name;
    }

    int32_t     m_id;           ///< unique ID
    uint32_t    m_flags;        ///< non-persistent flags that can be used by algorithms
    bool        m_isPortNet;    ///< when true, this net connects to a module port
    bool        m_isClockNet;   ///< when true, this net is a clock net

    void setPortNet(bool isPortNet)
    {
        m_isPortNet = isPortNet;
    }

    void setClockNet(bool isClockNet)
    {
        m_isClockNet = isClockNet;
    }

    struct NetConnect
    {
        NetConnect() = default;
        NetConnect(InstanceObjectKey insKey, PinObjectKey pinKey)
            : m_instanceKey(insKey), m_pinKey(pinKey) {}

        InstanceObjectKey   m_instanceKey{ChipDB::ObjectNotFound};
        PinObjectKey        m_pinKey{ChipDB::ObjectNotFound};

        friend constexpr bool operator==(const NetConnect &lhs, const NetConnect &rhs)
        {
            return (lhs.m_instanceKey == rhs.m_instanceKey) && (lhs.m_pinKey == rhs.m_pinKey);
        }

        friend constexpr bool operator!=(const NetConnect &lhs, const NetConnect &rhs)
        {
            return (lhs.m_instanceKey != rhs.m_instanceKey) || (lhs.m_pinKey != rhs.m_pinKey);
        }
    };

    /** returns true if the net already has a connection to (ins,pin) */
    bool hasConnection(InstanceObjectKey insKey, PinObjectKey pinKey) const;

    /** add a connection from net to (ins,pin). It does not check if a connection already exists */
    void addConnection(InstanceObjectKey insKey, PinObjectKey pinKey);

    /** remove a connection from net to (ins, pin). returns true if a connection was removed */
    bool removeConnection(InstanceObjectKey insKey, PinObjectKey pinKey);

    /** return the number of connections */
    size_t numberOfConnections() const noexcept
    {
        return m_connections.size();
    }

    using iterator          = typename std::vector<NetConnect>::iterator;
    using const_iterator    = typename std::vector<NetConnect>::const_iterator;
    using value_type        = NetConnect;

    /** connections iterator end */
    [[nodiscard]] const_iterator begin() const
    {
        return m_connections.begin();
    }

    /** connections iterator end */
    [[nodiscard]] const_iterator end() const
    {
        return m_connections.end();
    }

    /** connections iterator begin */
    [[nodiscard]] iterator begin()
    {
        return m_connections.begin();
    }

    /** connections iterator end */
    [[nodiscard]] iterator end()
    {
        return m_connections.end();
    }

protected:
    std::string m_name;         ///< net name
    std::vector<NetConnect> m_connections;
};

};