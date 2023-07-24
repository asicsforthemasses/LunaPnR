// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <QEvent>
#include <QApplication>
#include "mtstringbuffer.h"

using namespace GUI;

MTStringBuffer::MTStringBuffer(QObject *eventReceiver) : m_eventReceiver(eventReceiver)
{
}

void MTStringBuffer::print(const std::string &txt)
{
    print(Logging::LogType::INFO, txt);
}

void MTStringBuffer::print(const std::string_view &txt)
{
    print(Logging::LogType::INFO, txt);
}

void MTStringBuffer::print(const Logging::LogType &logType, const std::string &txt)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    m_buffer.push_back(LogString{logType, txt});

    if (m_eventReceiver != nullptr)
    {
        auto event = new QEvent(QEvent::User);
        QApplication::postEvent(m_eventReceiver, event);
    }
}

void MTStringBuffer::print(const Logging::LogType &logType, const std::string_view &txt)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    m_buffer.push_back(LogString{logType, std::string(txt)});

    if (m_eventReceiver != nullptr)
    {
        auto event = new QEvent(QEvent::User);
        QApplication::postEvent(m_eventReceiver, event);
    }    
}

MTStringBuffer::LogString MTStringBuffer::pop()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    auto logString = m_buffer.front();
    m_buffer.pop_front();
    return logString;
}

bool MTStringBuffer::containsString()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    return !m_buffer.empty();
}
