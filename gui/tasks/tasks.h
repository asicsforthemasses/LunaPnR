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
    Task(const std::string &taskName);

    /** run the task in a thread */
    bool run(GUI::Database &db);

    /** hard abort the thread */
    void abort();

    /** block/wait until the task is done */
    void wait();

    enum class Status : int
    {
        INVALID = 0,
        RUNNING,
        DONE_OK,
        DONE_ERROR
    };

    Status status() const
    {
        return m_status.load();
    }

    int progress() const
    {
        return m_progress.load();
    }

    /** returns true if the task is done */
    bool isDone() const noexcept
    {
        return (m_status == Status::DONE_OK) || (m_status == Status::DONE_ERROR);
    }

    const std::string& name() const noexcept
    {
        return m_name;
    }

protected:
    virtual void execute(GUI::Database &db) = 0;

    void error(const std::string &txt);
    void info(const std::string &txt);
    void done();

    std::string             m_name;             ///< task name
    std::atomic<Status>     m_status = {Status::INVALID};
    std::thread             *m_thread = nullptr;
    std::atomic<int>        m_progress = 0;     ///< task progress from 0 .. 100
};

};
