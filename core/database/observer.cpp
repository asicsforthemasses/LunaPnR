#include "observer.h"

namespace ChipDB
{

void Subject::setSubjectID(int id)
{
    m_subjectId = id;
}

void Subject::notify()
{
    for(auto &observer : m_observers)
    {
        observer->update(m_subjectId);
    }

    for(auto &cb : m_callbacks)
    {
        if (cb.second)
        {
            cb.second(m_subjectId);
        }
    }
}

void Subject::addObserver(IObserver *observer)
{
    auto iter = std::find(m_observers.begin(), m_observers.end(), observer);
    if (iter == m_observers.end())
    {
        m_observers.push_back(observer);
    }
}

void Subject::addObserver(const std::string &name, updateCallback cb)
{
    m_callbacks[name] = cb;
}

void Subject::removeObserver(IObserver *observer)
{
    auto iter = std::find(m_observers.begin(), m_observers.end(), observer);
    if (iter != m_observers.end())
    {
        m_observers.erase(iter);
    }
}

void Subject::removeObserver(const std::string &name)
{
    m_callbacks.erase(name);
}

};
