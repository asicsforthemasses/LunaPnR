// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <cstdarg>
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

void Task::error(const std::string_view fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    Logging::logError(fmt, args);
    va_end(args);

    m_status.store(Status::DONE_ERROR);
}

void Task::warning(const std::string_view fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    Logging::logWarning(fmt, args);
    va_end(args);
}

void Task::info(const std::string_view fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    Logging::logInfo(fmt, args);
    va_end(args);
}

void Task::done()
{
    m_status.store(Status::DONE_OK);
}

void Task::reset()
{
    m_status.store(Status::RESET);
}
