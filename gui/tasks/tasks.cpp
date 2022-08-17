#include "tasks.h"
#include "common/logging.h"

using namespace Tasks;

Task::Task(const std::string &taskName) : m_name(taskName)
{   
}

bool Task::run(GUI::Database &db)
{
    if (m_thread != nullptr) return false;
    m_thread = new std::thread([this, &db] {this->execute(db); } );

    return true;
}

void Task::error(const std::string &txt)
{
    Logging::doLog(Logging::LogType::ERROR, txt);
    m_status.store(Status::DONE_ERROR);
}

void Task::info(const std::string &txt)
{
    Logging::doLog(Logging::LogType::INFO, txt);
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
