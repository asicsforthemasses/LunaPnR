// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include <memory>
#include <array>
#include <iostream>

#include "../converters.h"
#include "typetemplate.h"
#include "pynet.h"
#include "design/design.h"

struct PyNetConstIterator
{
    using ContainerType = std::shared_ptr<ChipDB::Net>;
    using iterator = ChipDB::Net::const_iterator;
    using const_iterator = ChipDB::Net::const_iterator;

    ContainerType m_net;
    const_iterator m_iter;

    [[nodiscard]] const_iterator end() const
    {
        if (m_net)
        {
            return m_net->end();
        }
        return const_iterator{};
    }

    [[nodiscard]] const_iterator begin() const
    {
        if (m_net)
        {
            return m_net->begin();
        }
        return const_iterator{};
    }
};

/** container for LunaCore::Cell */
struct PyNet : public Python::TypeTemplate<PyNetConstIterator>
{
    static PyObject* getName(PyNet *self, void *closure)
    {
        if (self->ok())
        {
            if (self->obj()->m_net)
            {
                return Python::toPython(self->obj()->m_net->name());            
            }
        }
        
        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return nullptr;
    };

    /** set internal values of PyNet */
    static int pyInit(PyNet *self, PyObject *args, PyObject *kwds)
    {
        //std::cout << "PyNets::Init\n";

        // do not use ok() here, as it checks for
        // m_holder to be != nullptr.
        if (self->m_holder != nullptr)
        {
            //std::cout << "  Shared pointer created\n";

            self->m_holder->reset(new PyNetConstIterator());
            self->obj()->m_iter = PyNetConstIterator::iterator();
        }
        else
        {
            PyErr_Format(PyExc_RuntimeError, "Internal error: PyNets m_holder is nullptr!");
            return -1;
        }

        return 0;   /* success */
    };

    static PyObject* pyIter(PyNet *self)
    {
        if (self->ok())
        {
            self->obj()->m_iter = self->obj()->begin();
            Py_INCREF(self);
            return (PyObject*)self;
        }

        return nullptr;
    };

    static PyObject* pyIterNext(PyNet *self)
    {
        if (self->ok())
        {
            if (self->obj()->m_iter == self->obj()->end())
            {
                return nullptr; // no more object, stop iteration
            }

            auto const& netConnectObj = *self->obj()->m_iter;
            auto *netConnObject = Py_BuildValue("ll", netConnectObj.m_instanceKey, netConnectObj.m_pinKey);

            self->obj()->m_iter++;
            return netConnObject;
        }

        return nullptr;
    };

    /** set internal values of PyNet */
    static PyObject* pyStr(PyObject *self)
    {
        //std::cout << "pyStr\n";
        return Python::toPython(PyNet::PythonObjectName);
    };

    static constexpr const char *PythonObjectName = "Net";
    static constexpr const char *PythonObjectDoc  = "Net object";
};

// cppcheck-suppress "suppressed_error_id"
static PyMemberDef PyNetMembers[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}  /* Sentinel */
};

static PyGetSetDef PyNetGetSet[] =     // NOLINT(modernize-avoid-c-arrays)
{
    {"name", (getter)PyNet::getName, nullptr, "net name", nullptr /* closure */},
    {nullptr}
};

static PyMethodDef PyNetMethods[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}  /* Sentinel */
};

PyTypeObject PyNetType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    PyNet::PythonObjectName,       /* tp_name */
    sizeof(PyNet),                 /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)PyNet::pyDeAlloc,  /* tp_dealloc */
    0,                              /* tp_print */
    nullptr,                        /* tp_getattr */
    nullptr,                        /* tp_setattr */
    nullptr,                        /* tp_reserved */
    nullptr,                        /* tp_repr */
    nullptr,                        /* tp_as_number */
    nullptr,                        /* tp_as_sequence */
    nullptr,                        /* tp_as_mapping */
    nullptr,                        /* tp_hash  */
    nullptr,                        /* tp_call */
    PyNet::pyStr,                   /* tp_str */
    nullptr,                        /* tp_getattro */
    nullptr,                        /* tp_setattro */
    nullptr,                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,        /* tp_flags */
    PyNet::PythonObjectDoc,         /* tp_doc */
    nullptr,                        /* tp_traverse */
    nullptr,                        /* tp_clear */
    nullptr,                        /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    (getiterfunc)PyNet::pyIter,     /* tp_iter */
    (iternextfunc)PyNet::pyIterNext,/* tp_iternext */
    PyNetMethods,                   /* tp_methods */
    PyNetMembers,                   /* tp_members */
    PyNetGetSet,                    /* tp_getset */
    nullptr,                        /* tp_base */
    nullptr,                        /* tp_dict */
    nullptr,                        /* tp_descr_get */
    nullptr,                        /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)PyNet::pyInit,        /* tp_init */
    nullptr,                        /* tp_alloc */
    PyNet::pyNewCall
};

PyObject* Python::toPython(std::shared_ptr<ChipDB::Net> netPtr)
{
    // create a new PyNet object
    auto netObject = reinterpret_cast<PyNet*>(PyObject_CallObject((PyObject*)&PyNetType, nullptr));
    if (netObject->m_holder != nullptr)
    {
        (*netObject->m_holder)->m_net = netPtr;
        return (PyObject*)netObject;
    }
    return nullptr;
};

