// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QColor>
#include <string_view>
#include <string>
#include <mutex>
#include <list>
#include <QObject>
#include "common/logging.h"

namespace GUI
{

/** multi-threaded string buffer 
 *  used by MMConsole to store log messages coming in from other threads.
*/
class MTStringBuffer
{
public:
    struct LogString
    {
        Logging::LogType    m_logType{Logging::LogType::INFO};
        std::string         m_txt;
    };

    MTStringBuffer(QObject *eventReceiver);

    void print(const std::string &txt);
    void print(const std::string_view &txt);

    void print(const Logging::LogType &logType, const std::string &txt);
    void print(const Logging::LogType &logType, const std::string_view &txt);

    LogString pop();
    bool containsString();

protected:
    QObject *m_eventReceiver = nullptr;
    std::mutex m_mutex;

    std::list<LogString> m_buffer;
};

}; //namespace