
#include <algorithm>
#include "net.h"

using namespace ChipDB;

void Net::addConnection(ObjectKey instanceKey, size_t pinIndex)
{
    auto iter = std::find_if(m_connections.begin(), m_connections.end(),
        [instanceKey, pinIndex](auto conn)
        {
            return (conn.m_instanceKey == instanceKey) && (conn.m_pinIndex == pinIndex);
        }
    );

    if (iter == m_connections.end())
    {
        // connection does not yet exist -> add it
        NetConnect conn = {instanceKey, pinIndex};
        m_connections.push_back(conn);
    }
}
