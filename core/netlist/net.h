#pragma once

#include <string>
#include <vector>

namespace ChipDB
{

class Instance; // pre-declaration

class Net
{
public:
    Net() : m_isPortNet(false) {}
    
    std::string m_name;         ///< net name
    bool        m_isPortNet;    ///< when true, this net connectes to a module port

    void setPortNet(bool isPortNet)
    {
        m_isPortNet = isPortNet;
    }

    struct NetConnect
    {
        Instance *m_instance;
        size_t    m_pinIndex;
    };

    /** connect the net to the instance pin */
    void addConnection(Instance *instance, size_t pinIndex);

    std::vector<NetConnect> m_connections;
};

};