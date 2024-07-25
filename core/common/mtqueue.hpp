// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

namespace LunaCore
{

/**
    A multithreaded queue
*/

template<typename T>
class MTQueue
{
public:

    /** blocking push */
    void push(const T& item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push_back(item);
        if (m_queue.size() == 1)
        {
            m_notEmptyCv.notify_all();
        }
    }

    /** blocking push */
    void push(T&& item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.emplace_back(item);
        if (m_queue.size() == 1)
        {
            m_notEmptyCv.notify_all();
        }
    }

    /** non-blocking pop */
    bool tryPop(T& item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) return false;
        item = std::move(m_queue.front());
        m_queue.pop_front();

        if (m_queue.empty()) m_emptyCv.notify_all();

        return true;
    }

    /** blocking pop */
    T pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_queue.empty())
        {
            auto item = std::move(m_queue.front());
            m_queue.pop_front();

            if (m_queue.empty()) m_emptyCv.notify_all();

            return std::move(item);
        }

        m_notEmptyCv.wait(lock,
            [this]() { return !m_queue.empty(); }
        );

        auto item = std::move(m_queue.front());
        m_queue.pop_front();

        if (m_queue.empty()) m_emptyCv.notify_all();

        return std::move(item);
    }

    /** blocking clear */
    void clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.clear();
    }

    /** waits/blocks for the queue to become empty  */
    void waitEmpty()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_queue.empty()) return;

        m_emptyCv.wait(lock,
            [this]() { return m_queue.empty(); }
        );
    }

    /** returns true if the queue is empty. blocking */
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    /** returns the number of elements in the queue. blocking. */
    std::size_t size() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

private:
    std::mutex    m_mutex;
    std::deque<T> m_queue;
    std::condition_variable m_emptyCv;
    std::condition_variable m_notEmptyCv;
};

};
