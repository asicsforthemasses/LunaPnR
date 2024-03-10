// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "tasklist.h"
#include "common/logging.h"
#include "common/subprocess.h"
#include "customevents.h"

#include "tasks/checktiming.h"
#include "tasks/ctstask.h"
#include "tasks/readallfiles.h"
#include "tasks/preflightchecks.h"
#include "tasks/createfloorplan.h"
#include "tasks/globalroute.h"
#include "tasks/place.h"

#include <QApplication>

#include <cassert>

using namespace GUI;

TaskList::TaskList(QObject *projectManager) : m_projectManager(projectManager)
{
    //Note: the order must reflect the order in the ProjectManager.
    createTask("ReadAllFiles", new Tasks::ReadAllFiles);
    createTask("PreflightChecks", new Tasks::PreflightChecks);
    createTask("CreateFloorplan", new Tasks::CreateFloorplan);
    createTask("PLACE", new Tasks::Place);
    createTask("CTS", new Tasks::CTS);
    createTask("TIMINGREPORT1", new Tasks::CheckTiming(Tasks::CheckTiming::Mode::CTS));
    createTask("GLOBALROUTE", new Tasks::GlobalRoute);
    createTask("TIMINGREPORT2", new Tasks::CheckTiming(Tasks::CheckTiming::Mode::WITHSPEF));
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

bool TaskList::executeToTask(Database &db, const std::string &taskName)
{
    // check if the requested task is in the list
    auto taskIter = m_nameToIndex.find(taskName);
    if (taskIter == m_nameToIndex.end())
    {
        // task name not found
        Logging::doLog(Logging::LogType::ERROR, Logging::fmt("TaskList: Cannot find task %s\n", taskName.c_str()));
        return false;
    }

    auto lastTaskId = taskIter->second;

    if (lastTaskId >= m_tasks.size())
    {
        Logging::doLog(Logging::LogType::ERROR, Logging::fmt("TaskList: Task ID for %s is out of range\n", taskName.c_str()));
        return false;
    }

    // reset all the tasks from the lastTask onwards
    for(size_t idx = lastTaskId; idx < m_tasks.size(); idx++)
    {
        m_tasks.at(idx)->reset();
    }

    // find the first task that has not been marked done
    size_t firstTaskId = 0;
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
        [this, firstTaskId, lastTaskId, &db]
        {
            taskThread(db, firstTaskId, lastTaskId, m_projectManager);
        }
    );

    return false;
}

void TaskList::taskThread(Database &db, size_t firstTask, size_t lastTask, QObject *projectManager)
{
    for(auto idx = firstTask; idx <= lastTask; idx++)
    {
        if (m_tasks.at(idx))
        {
            if (!m_tasks.at(idx))
            {
                // unique_ptr is null!
                Logging::doLog(Logging::LogType::ERROR,"TaskList: task ptr is null!\n");
                return;
            }

            // signal start of task
            const QString taskName = QString::fromStdString(m_tasks.at(idx)->name());
            auto event = new GUI::TaskListEvent(taskName, Tasks::Task::Status::RUNNING, idx);
            QApplication::postEvent(m_projectManager, event);

            auto progressCallback = [this, taskName, idx, projectManager](int progress)
            {
                auto event = new GUI::TaskListEvent(taskName, Tasks::Task::Status::PROGRESS, idx, progress);
                QApplication::postEvent(m_projectManager, event);
            };

            if (!m_tasks.at(idx)->run(db, progressCallback))
            {
                // signal failure
                auto event = new GUI::TaskListEvent(taskName, Tasks::Task::Status::DONE_ERROR, idx, 0);
                QApplication::postEvent(m_projectManager, event);
                return;
            }

            // signal end of task
            event = new GUI::TaskListEvent(taskName, Tasks::Task::Status::DONE_OK, idx, 100);
            QApplication::postEvent(m_projectManager, event);
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
    assert(task != nullptr);

    m_tasks.emplace_back();
    m_tasks.back().reset(task);
    m_tasks.back()->setName(name);

    auto iter = m_nameToIndex.find(name);
    assert(iter == m_nameToIndex.end());

    m_nameToIndex[name] = m_tasks.size()-1;
}
