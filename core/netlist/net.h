#pragma once

#include <string>
#include <vector>
#include "common/dbtypes.h"
#include "common/visitor.h"

namespace ChipDB
{

class InstanceBase; // pre-declaration

class Net
{
public:
    Net() : m_id(-1), m_flags(0), m_isPortNet(false), m_isClockNet(false) {}
    
    Net(const std::string &name) : m_name(name), m_id(-1), m_flags(0), 
        m_isPortNet(false), m_isClockNet(false) {}

    IMPLEMENT_ACCEPT;

    std::string m_name;         ///< net name
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
        ObjectKey   m_instanceKey;
        size_t      m_pinIndex;
    };

    /** connect the net to the instance pin.
     *  it checks if the requested connection already exists
     *  and if it does, ignores it.
    */
    void addConnection(ObjectKey instanceKey, size_t pinIndex);

    std::vector<NetConnect> m_connections;
};

};