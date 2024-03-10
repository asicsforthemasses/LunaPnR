#pragma once
#include <list>
#include <string>
#include <functional>
#include <unordered_map>

namespace ChipDB
{

struct IObserver
{
    virtual void update(int id) = 0;
};

/** Subject interface of observer */
class Subject
{
public:
    using updateCallback = std::function<void(int id)>;

    void notify();

    void addObserver(IObserver *observer);
    void addObserver(const std::string &name, updateCallback cb);

    void removeObserver(IObserver *observer);
    void removeObserver(const std::string &name);

    void setSubjectID(int id);

protected:
    int m_subjectId{-1};

    std::list<IObserver*> m_observers;
    std::unordered_map<std::string, updateCallback> m_callbacks;
};

};