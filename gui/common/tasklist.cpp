
#include "tasklist.h"
#include "common/logging.h"
#include "common/subprocess.h"

#include "tasks/checktiming.h"
#include "tasks/readallfiles.h"
#include "tasks/preflightchecks.h"

#include <cassert>

using namespace GUI;

TaskList::TaskList()
{
    createTask("ReadAllFiles", new Tasks::ReadAllFiles);
    createTask("PreflightChecks", new Tasks::PreflightChecks);
    createTask("CheckTiming", new Tasks::CheckTiming);
}

TaskList::~TaskList()
{
    if (m_thread)
    {
        // make sure we don't crash the program if there is still a thread running
        m_thread->detach();
        m_thread.reset();
    }
}

bool TaskList::executeToTask(Database &db, const std::string &taskName, std::function<void(CallbackInfo)> callback)
{
    // check if the requested task is in the list
    auto taskIter = m_nameToIndex.find(taskName);
    if (taskIter == m_nameToIndex.end())
    {
        // task name not found
        Logging::doLog(Logging::LogType::ERROR, Logging::fmt("Cannot find task %s\n", taskName.c_str()));
        return false;
    }

    auto lastTaskId = taskIter->second;

    if (lastTaskId >= m_tasks.size())
    {
        Logging::doLog(Logging::LogType::ERROR, Logging::fmt("Task ID for %s is out of range\n", taskName.c_str()));
        return false;        
    }

    // reset all the tasks from the lastTask onwards
    for(TaskIndex idx = lastTaskId; idx < m_tasks.size(); idx++)
    {
        m_tasks.at(idx)->reset();
    }

    // find the first task that has not been marked done
    TaskIndex firstTaskId = 0;
    while(firstTaskId < lastTaskId)
    {
        if (!m_tasks.at(firstTaskId)->isDone())
        {
            break;
        }
        firstTaskId++;
    }

    // kill the current thread if there is one running.
    if (m_thread)
    {
        m_thread->detach();
        m_thread.reset();
    }

    m_thread = std::make_unique<std::thread>(
        [this, firstTaskId, lastTaskId, callback, &db]
        {
            taskThread(db, firstTaskId, lastTaskId, callback);
        }
    );

    return false;
}

void TaskList::taskThread(Database &db, TaskIndex firstTask, TaskIndex lastTask, std::function<void(CallbackInfo)> callback)
{
    for(auto idx = firstTask; idx <= lastTask; idx++)
    {
        if (m_tasks.at(idx))
        {
            auto progressCallback = [this, idx, callback](int progress)
            {
                CallbackInfo info;
                info.m_progress = progress;
                info.m_taskIdx  = idx;
                info.m_taskStatus = Tasks::Task::Status::PROGRESS;
                if (callback) callback(info);
            };

            if (!m_tasks.at(idx))
            {
                // unique_ptr is null!
                Logging::doLog(Logging::LogType::ERROR,"TaskList: task ptr is null!\n");
                return;
            }

            // signal start of task
            CallbackInfo info;
            info.m_taskIdx    = idx;
            info.m_taskStatus = Tasks::Task::Status::RUNNING;
            if (callback) callback(info);

            if (!m_tasks.at(idx)->run(db, progressCallback))
            {
                // signal failure
                info.m_taskIdx    = idx;
                info.m_taskStatus = Tasks::Task::Status::DONE_ERROR;
                if (callback) callback(info);
                return;
            }

            // signal end of task
            info.m_taskIdx    = idx;
            info.m_taskStatus = Tasks::Task::Status::DONE_OK;
            if (callback) callback(info);
        }
        else
        {
            // error, task is nullptr
            return;
        }
    }
}

void TaskList::createTask(const std::string &name, Tasks::Task *task)
{
    m_tasks.emplace_back();
    m_tasks.back().reset(task);

    auto iter = m_nameToIndex.find(name);
    assert(iter == m_nameToIndex.end());

    m_nameToIndex[name] = m_tasks.size()-1;
}
