// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QEvent>
#include <QString>
#include "../tasks/tasks.h"

namespace GUI
{

class TaskListEvent : public QEvent
{
public:
    static constexpr auto EventType = (QEvent::Type)(QEvent::User+1);

    virtual ~TaskListEvent() = default;

    TaskListEvent(QString taskName, Tasks::Task::Status status, size_t taskIndex, int progress = 0)
        : QEvent(EventType), m_taskName(taskName), m_taskStatus(status), m_progress(progress)
    {
    }

    /** return the task status */
    [[nodiscard]] constexpr auto status() const noexcept
    {
        return m_taskStatus;
    }

    /** return the task index */
    [[nodiscard]] constexpr auto index() const noexcept
    {
        return m_taskIndex;
    }

    /** return the progress 0 .. 100 */
    [[nodiscard]] constexpr auto progress() const noexcept
    {
        return m_progress;
    }

    /** return the task name */
    [[nodiscard]] auto const name() const noexcept
    {
        return m_taskName;
    }

protected:
    QString m_taskName;
    Tasks::Task::Status m_taskStatus = Tasks::Task::Status::INVALID;
    size_t m_taskIndex = 0;         ///< index into TaskList array
    int m_progress     = 0;
};

};
