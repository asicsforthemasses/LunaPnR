#include "tasks.h"

using namespace Tasks;

bool Task::run(GUI::Database &db)
{
    if (m_thread != nullptr) return false;
    m_thread = new std::thread([this, &db] {this->execute(db); } );

    return true;
}

void Task::error(const std::string &txt)
{
    std::cerr << txt;
    m_status.store(Status::DONE_ERROR);
}

void Task::done()
{
    m_status.store(Status::DONE_OK);
}

void Task::wait()
{
    if (m_thread == nullptr)
    {
        return;
    }

    if (m_thread->joinable())
    {
        m_thread->join();
    }

    delete m_thread;

    m_thread = nullptr;
}

void Task::abort()
{
    if (m_thread != nullptr)
    {
        delete m_thread;
    }
}
