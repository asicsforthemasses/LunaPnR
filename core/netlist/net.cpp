// SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
//
// SPDX-License-Identifier: GPL-3.0-only

#include <algorithm>
#include "net.h"

using namespace ChipDB;

bool Net::hasConnection(InstanceObjectKey insKey, PinObjectKey pinKey) const
{
    const NetConnect connection(insKey, pinKey);

    auto iter = std::find_if(m_connections.begin(), m_connections.end(),
        [connection](auto listConn)
        {
            return listConn == connection;
        }
    );

    return (iter != m_connections.end());
}

void Net::addConnection(InstanceObjectKey insKey, PinObjectKey pinKey)
{
    m_connections.emplace_back(NetConnect(insKey, pinKey));
}

bool Net::removeConnection(InstanceObjectKey insKey, PinObjectKey pinKey)
{
    const NetConnect connection(insKey, pinKey);

    auto iter = std::find_if(m_connections.begin(), m_connections.end(),
        [connection](auto listConn)
        {
            return listConn == connection;
        }
    );

    if (iter != m_connections.end())
    {
        m_connections.erase(iter);
        return true;
    }

    return false;
}

