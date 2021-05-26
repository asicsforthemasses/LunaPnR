#include <algorithm>
#include "pin.h"

using namespace ChipDB;

PinInfo& PinInfoList::createPin(const std::string &name)
{
    auto iter = find(name);
    if (iter == m_pins.end())
    {
        m_pins.emplace_back(PinInfo(name));
        return m_pins.back();
    }
    else
    {
        return *iter;
    }
}

PinInfo* PinInfoList::lookup(const std::string &name)
{
    auto iter = find(name);
    if (iter == m_pins.end())
    {
        return nullptr;
    }
    else
    {
        return &(*iter);
    }
}

const PinInfo* PinInfoList::lookup(const std::string &name) const
{
    auto iter = find(name);
    if (iter == m_pins.end())
    {
        return nullptr;
    }
    else
    {
        return &(*iter);
    }
}

ssize_t PinInfoList::lookupIndex(const std::string &name) const
{
    size_t idx = 0;
    for(auto const& p : m_pins)
    {
        if (p.m_name == name)
        {
            return idx;
        }
        idx++;
    }
    return -1;
}

std::vector<PinInfo>::iterator PinInfoList::find(const std::string &name)
{
    auto iter = std::find_if(m_pins.begin(), m_pins.end(), 
        [name](auto const& p)
        {
            return p.m_name == name;
        }
    );

    return iter;
}

std::vector<PinInfo>::const_iterator PinInfoList::find(const std::string &name) const
{
    auto iter = std::find_if(m_pins.begin(), m_pins.end(), 
        [name](auto const& p)
        {
            return p.m_name == name;
        }
    );

    return iter;
}

