// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <map>
#include <string>
#include <thread>
#include <functional>

#include <QEvent>
#include <QObject>

#include "database.h"
#include "tasks/tasks.h"

namespace GUI
{

class TaskList
{
public:
    TaskList(QObject *projectManager);
    virtual ~TaskList();

    size_t numberOfTasks() const noexcept
    {
        return m_tasks.size();
    }

    const Tasks::Task* at(size_t index) const
    {
        return m_tasks.at(index).get();
    }

    bool executeToTask(Database &db, const std::string &taskName);

protected:
    void createTask(const std::string &taskName, Tasks::Task *task);

    QObject  *m_projectManager = nullptr;

    std::vector<std::unique_ptr<Tasks::Task> > m_tasks;

    using TaskName  = std::string;
    std::map<TaskName, size_t> m_nameToIndex;

    void taskThread(Database &db, size_t firstTask, size_t lastTask, QObject *projectManager);
    std::unique_ptr<std::thread> m_thread;
};

};
