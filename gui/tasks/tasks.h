// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <atomic>
#include <thread>
#include <string>
#include "../common/database.h"

namespace Tasks
{

/** multi-threaded task */
class Task
{
public:
    Task();
    virtual ~Task() = default;

    void setName(const std::string &taskName);

    /** revert the task to the reset state */
    virtual void reset();

    using ProgressCallback = std::function<void(int)>;

    /** run the task */
    bool run(GUI::Database &db, ProgressCallback callback);

    enum class Status : int
    {
        INVALID = 0,
        RESET,
        RUNNING,
        PROGRESS,
        DONE_OK,
        DONE_ERROR
    };

    /** returns the task status. Thread safe */
    Status status() const
    {
        return m_status.load();
    }

    /** returns true if the task is done */
    bool isFinished() const noexcept
    {
        return (m_status == Status::DONE_OK) || (m_status == Status::DONE_ERROR);
    }

    bool isDone() const noexcept
    {
        return (m_status == Status::DONE_OK);
    }

    /** returns the task name, Thread safe */
    const std::string& name() const noexcept
    {
        return m_name;
    }

protected:
    virtual void execute(GUI::Database &db, ProgressCallback callback) = 0;

    void error(const std::string_view fmt, ...);
    void warning(const std::string_view fmt, ...);
    void info(const std::string_view fmt, ...);
    void done();

    std::string             m_name = {"UNDEFINED"};         ///< task name
    std::atomic<Status>     m_status = {Status::INVALID};
};

};
