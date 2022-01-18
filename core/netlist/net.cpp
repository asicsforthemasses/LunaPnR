
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
    m_connections.push_back(NetConnect(insKey, pinKey));
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

