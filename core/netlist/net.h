#pragma once

#include <string>
#include <vector>
#include "common/visitor.h"

namespace ChipDB
{

class InstanceBase; // pre-declaration

class Net
{
public:
    Net() : m_flags(0), m_isPortNet(false), m_isClockNet(false) {}
    
    IMPLEMENT_ACCEPT;

    std::string m_name;         ///< net name
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
        InstanceBase *m_instance;
        size_t        m_pinIndex;
    };

    /** connect the net to the instance pin.
     *  it checks if the requested connection already exists
     *  and if it does, ignores it.
    */
    void addConnection(InstanceBase *instance, size_t pinIndex);

    std::vector<NetConnect> m_connections;
};

};