#pragma once 
#include <Python.h>
#include <memory>

namespace Python
{

/** non-owning raw pointer holder for use with TypeTemplate. */
template<typename MyType>
class ValueContainer
{
public:
    ValueContainer() = default;
    ValueContainer(const MyType &value) : m_value(value) {}
    ValueContainer(MyType *ptr) : m_value(*ptr) {}

    constexpr MyType* get() 
    {
        return &m_value;
    }

    constexpr operator bool() const
    {
        return true;
    }

protected:
    MyType m_value;
};

/** non-owning raw pointer holder for use with TypeTemplate. */
template<typename MyType>
class RawPointer
{
public:
    RawPointer() = default;
    RawPointer(MyType *ptr) : m_ptr(ptr) {}

    constexpr MyType* get() 
    {
        return m_ptr;
    }

    constexpr operator bool() const
    {
        return m_ptr != nullptr;
    }

protected:
    MyType *m_ptr = nullptr;
};

template<typename MyType, typename TypeHolder = typename std::shared_ptr<MyType> >
struct TypeTemplate
{    
    PyObject_HEAD
    
    TypeTemplate() 
    {
        //std::cout << "TypeTemplate constructor called\n";
    }

    TypeHolder *m_holder;

    auto* obj() const
    {
        return m_holder->get();
    }

    constexpr bool ok() const
    {
        return (m_holder != nullptr) && (*m_holder);
    }
 
    /** allocate memory for the PyCell */
    static PyObject* pyNewCall(PyTypeObject *type, PyObject *args, PyObject *kwds)
    {
        //std::cout << type->tp_name << " pyNewCall\n";

        auto *self = reinterpret_cast<TypeTemplate*>(type->tp_alloc(type, 0));
        if (self != nullptr)
        {
            self->m_holder = new TypeHolder();
        }

        return (PyObject*)self;
    }

    static void pyDeAlloc(TypeTemplate *self)
    {
        auto tp = Py_TYPE(self);
        //std::cout << tp->tp_name << " pyDeAlloc\n";

        /* add additional memory free operations here, if necessary */

        if (self->m_holder != nullptr)
        {
            delete self->m_holder;
        }

        if (tp != nullptr)
        {
            tp->tp_free((PyObject*)self);
        }        
    }
};

};