#pragma once


#include <string_view>
#include <string>
#include <mutex>
#include <list>
#include <QObject>

namespace GUI
{

/** multi-threaded string buffer 
 *  used by MMConsole to store log messages coming in from other threads.
*/
class MTStringBuffer
{
public:
    MTStringBuffer(QObject *eventReceiver);

    void print(const std::string &txt);
    void print(const std::string_view &txt);

    std::string pop();
    bool containsString();

protected:
    QObject *m_eventReceiver = nullptr;
    std::mutex m_mutex;
    std::list<std::string> m_buffer;
};

}; //namespace