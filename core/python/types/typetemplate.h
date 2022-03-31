#pragma once 
#include <Python.h>
#include <memory>

namespace Python
{

template<typename MyType, typename TypeHolder = typename std::shared_ptr<MyType> >
struct TypeTemplate
{    
    PyObject_HEAD
    
    TypeTemplate() 
    {
        std::cout << "TypeTemplate constructor called\n";
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
        std::cout << "TypeTemplate pyNewCall\n";

        auto *self = reinterpret_cast<TypeTemplate*>(type->tp_alloc(type, 0));
        if (self != nullptr)
        {
            std::cout << "Type holder created\n";
            self->m_holder = new TypeHolder();
        }

        return (PyObject*)self;
    }

    static void pyDeAlloc(TypeTemplate *self)
    {
        std::cout << "TypeTemplate pyDeAlloc\n";

        /* add additional memory free operations here, if necessary */

        if (self->m_holder != nullptr)
        {
            delete self->m_holder;
        }

        auto tp = Py_TYPE(self);
        if (tp != nullptr)
        {
            tp->tp_free((PyObject*)self);
        }        
    }
};

};