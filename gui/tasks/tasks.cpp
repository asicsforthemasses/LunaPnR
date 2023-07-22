// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "tasks.h"
#include "common/logging.h"

using namespace Tasks;

Task::Task()
{   
}

void Task::setName(const std::string &taskName)
{
    m_name = taskName;
}

bool Task::run(GUI::Database &db, ProgressCallback callback)
{
    execute(db, callback);
    return m_status.load() == Status::DONE_OK;
}

void Task::error(const std::string &txt)
{
    Logging::doLog(Logging::LogType::ERROR, txt);
    m_status.store(Status::DONE_ERROR);
}

void Task::warning(const std::string &txt)
{
    Logging::doLog(Logging::LogType::WARNING, txt);
}

void Task::info(const std::string &txt)
{
    Logging::doLog(Logging::LogType::INFO, txt);
}

void Task::done()
{
    m_status.store(Status::DONE_OK);
}

void Task::reset()
{
    m_status.store(Status::RESET);
}
