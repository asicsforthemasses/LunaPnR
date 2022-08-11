#pragma once

#include <atomic>
#include <thread>
#include "../common/database.h"

namespace Tasks
{

/** multi-threaded task */
class Task
{
public:
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
    bool isDone() const
    {
        return (m_status == Status::DONE_OK) || (m_status == Status::DONE_ERROR);
    }

protected:
    virtual void execute(GUI::Database &db) = 0;

    void error(const std::string &txt);
    void done();

    std::atomic<Status>     m_status = {Status::INVALID};
    std::thread             *m_thread = nullptr;
    std::atomic<int>        m_progress = 0;
};

};
