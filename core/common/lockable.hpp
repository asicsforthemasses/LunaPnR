#pragma once
#include <shared_mutex>

/** Helper class to provide resource locking in a multithreaded environment.

    Obtain a read-write lock by calling the lock() method.
    Obtain a read-only lock by calling the cLock() method.

    Multiple readers can access the resource in the case of a read-only lock.

    Usage:
@code
    using LockableDatabase = Lockable<Database>;

    // Locks the database for read-write access and gets a resource accessor.
    auto db = LockableDatabase.lock();

    // interace with the database
    db->addItem(...)

    // when the accessor 'db' goes out of scope, the resource is unlocked automatically.
    // or if the resource needs to be unlocked earlier, use db.unlock();
@endcode

*/
template<typename T>
class Lockable
{
public:

    class Accessor
    {
    public:
        Accessor(const Accessor&) = delete;
        Accessor& operator=(const Accessor&) = delete;

        Accessor(Lockable &lockable) : m_lockable(lockable)
        {
            m_lockable._lock();
            m_ownsLock = true;
        }

        virtual ~Accessor()
        {
            m_lockable._unlock();
        }

        constexpr T* operator->()
        {
            if (!m_ownsLock) throw std::runtime_error("ConstAccessor: lock not owned while accessing object");

            return m_lockable.getPtr();
        }

        constexpr T& ref()
        {
            if (!m_ownsLock) throw std::runtime_error("ConstAccessor: lock not owned while accessing object");

            return m_lockable.getRef();
        }

        constexpr T* ptr()
        {
            if (!m_ownsLock) throw std::runtime_error("ConstAccessor: lock not owned while accessing object");

            return m_lockable.getPtr();
        }

        void unlock() noexcept      ///< unlock the read-write resource.
        {
            m_ownsLock = false;
            m_lockable._unlock();
        }

        constexpr auto ownsLock() const noexcept
        {
            return m_ownsLock;
        }

    private:
        bool  m_ownsLock{false};
        Lockable& m_lockable;
    };

    class ConstAccessor
    {
    public:
        ConstAccessor(const ConstAccessor&) = delete;
        ConstAccessor& operator=(const ConstAccessor&) = delete;

        ConstAccessor(const Lockable &lockable) : m_lockable(lockable)
        {
            m_lockable.lockShared();
            m_ownsLock = true;
        }

        virtual ~ConstAccessor()
        {
            m_lockable.unlockShared();
        }

        constexpr const T* operator->()
        {
            if (!m_ownsLock) throw std::runtime_error("ConstAccessor: lock not owned while accessing object");

            return m_lockable.getPtr();
        }

        constexpr const T* ptr()
        {
            if (!m_ownsLock) throw std::runtime_error("ConstAccessor: lock not owned while accessing object");

            return m_lockable.getPtr();
        }

        constexpr const T& ref()
        {
            if (!m_ownsLock) throw std::runtime_error("ConstAccessor: lock not owned while accessing object");

            return m_lockable.getConstRef();
        }

        void unlock() noexcept      ///< unlock the read-only resource.
        {
            m_ownsLock = false;
            m_lockable.unlockShared();
        }

        constexpr auto ownsLock() const noexcept
        {
            return m_ownsLock;
        }

    private:
        bool  m_ownsLock{false};
        const Lockable& m_lockable;
    };

    /** lock the resource for read-write access and return the accessor */
    auto lock()
    {
        return Accessor(*this);
    }

    /** lock the resource for read-only access and return the accessor */
    auto lock() const
    {
        return ConstAccessor(*this);
    }

    /** lock the resource for read-only access and return the accessor */
    auto cLock()
    {
        return ConstAccessor(*this);
    }

protected:

    const T* getPtr() const
    {
        return &m_object;
    }

    T* getPtr()
    {
        return &m_object;
    }

    const T& getRef() const
    {
        return m_object;
    }

    T& getRef()
    {
        return m_object;
    }

    inline void _lock()
    {
        m_mutex.lock();
    }

    inline void _unlock()
    {
        m_mutex.unlock();
    }

    inline bool tryLock()
    {
        return m_mutex.try_lock();
    }

    inline void lockShared() const
    {
        m_mutex.lock_shared();
    }

    inline void unlockShared() const
    {
        m_mutex.unlock_shared();
    }

    inline bool tryLockShared() const
    {
        return m_mutex.try_lock_shared();
    }

    mutable std::shared_mutex m_mutex;
    T m_object;
};
