#pragma once

#include <QEvent>
#include <map>
#include <string>
#include <thread>
#include <functional>

#include "database.h"
#include "tasks/tasks.h"

namespace GUI
{

class TaskList
{
public:
    TaskList();
    virtual ~TaskList();

    size_t numberOfTasks() const noexcept
    {
        return m_tasks.size();
    }

    using TaskIndex = size_t;

    struct CallbackInfo
    {
        TaskIndex m_taskIdx = 0;
        Tasks::Task::Status m_taskStatus = Tasks::Task::Status::INVALID;
        int m_progress = 0;
    };

    bool executeToTask(Database &db, const std::string &taskName, std::function<void(CallbackInfo)> callback);

protected:
    void createTask(const std::string &taskName, Tasks::Task *task);

    std::vector<std::unique_ptr<Tasks::Task> > m_tasks;

    using TaskName  = std::string;
    std::map<TaskName, TaskIndex> m_nameToIndex;

    void taskThread(Database &db, TaskIndex firstTask, TaskIndex lastTask, std::function<void(CallbackInfo)> callback);
    std::unique_ptr<std::thread> m_thread;
};

};
