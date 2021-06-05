
#include <algorithm>
#include "net.h"

using namespace ChipDB;

void Net::addConnection(InstanceBase *instance, size_t pinIndex)
{
    auto iter = std::find_if(m_connections.begin(), m_connections.end(),
        [instance, pinIndex](auto conn)
        {
            return (conn.m_instance == instance) && (conn.m_pinIndex == pinIndex);
        }
    );

    if (iter == m_connections.end())
    {
        // connection does not yet exist -> add it
        NetConnect conn = {.m_instance = instance, .m_pinIndex = pinIndex};
        m_connections.push_back(conn);
    }
}

