// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
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
    std::lock_guard<std::mutex> guard(m_mutex);
    m_buffer.push_back(txt);

    if (m_eventReceiver != nullptr)
    {
        auto event = new QEvent(QEvent::User);
        QApplication::postEvent(m_eventReceiver, event);
    }
}

void MTStringBuffer::print(const std::string_view &txt)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    m_buffer.push_back(std::string(txt));

    if (m_eventReceiver != nullptr)
    {
        auto event = new QEvent(QEvent::User);
        QApplication::postEvent(m_eventReceiver, event);
    }    
}

std::string MTStringBuffer::pop()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    auto str = m_buffer.front();
    m_buffer.pop_front();
    return str;
}

bool MTStringBuffer::containsString()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    return !m_buffer.empty();
}
