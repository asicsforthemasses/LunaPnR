// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
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
#include "netlist/netlist.h"
#include "design/design.h"
#include "pynets.h"
#include "pynet.h"

struct PyNetsIterator
{
    using iterator   = ChipDB::NamedStorage<ChipDB::Net>::iterator;
    using value_type = typename iterator::value_type;

    std::shared_ptr<ChipDB::Netlist> m_netlist;
    iterator m_iter;

    iterator end()
    {
        if (!m_netlist)
        {
            return iterator{};
        }
        return m_netlist->m_nets.end();
    }

    iterator begin()
    {
        if (!m_netlist)
        {
            return iterator{};
        }        
        return m_netlist->m_nets.begin();
    }
};

/** container for LunaCore::Cell */
struct PyNets : public Python::TypeTemplate<PyNetsIterator>
{
    /** set internal values of PyCell */
    static int pyInit(PyNets *self, PyObject *args, PyObject *kwds)
    {
        //std::cout << "PyNets::Init\n";

        // do not use ok() here, as it checks for
        // m_holder to be != nullptr.
        if (self->m_holder != nullptr)
        {
            //std::cout << "  Shared pointer created\n";

            self->m_holder->reset(new PyNetsIterator());
            self->obj()->m_iter = PyNetsIterator::iterator();

            // get a pointer to 
            auto designPtr = reinterpret_cast<ChipDB::Design*>(PyCapsule_Import("Luna.DesignPtr", 0));
            if (designPtr == nullptr)
            {
                PyErr_Format(PyExc_RuntimeError, "Unable to access design database");
                return -1;
            }

            auto topModule = designPtr->getTopModule();
            if (!topModule)
            {
                PyErr_Format(PyExc_RuntimeError, "Top module has not been set");
                return -1;
            }

            self->obj()->m_netlist = topModule->m_netlist;
        }
        else
        {
            PyErr_Format(PyExc_RuntimeError, "Internal error: PyNets m_holder is nullptr!");
            return -1;
        }

        return 0;   /* success */
    };

    static PyObject* getNet(PyNets *self, PyObject *args)
    {
        if (self->ok())
        {
            if (!self->obj()->m_netlist)
            {
                PyErr_Format(PyExc_RuntimeError, "Top module has no netlist");
                return nullptr;
            }

            ChipDB::InstanceObjectKey key;
            if (PyArg_ParseTuple(args,"L", &key))
            {
                auto net = self->obj()->m_netlist->m_nets[key];
                if (!net)
                {
                    PyErr_Format(PyExc_ValueError, "Net with key %ld not found", key);
                    return nullptr;                    
                }                
                return Python::toPython(net);
            }

            // clear previous PyArg_ParseTuple exception
            PyErr_Clear();

            const char *netName = nullptr;
            if (PyArg_ParseTuple(args,"s", &netName))
            {
                auto net = self->obj()->m_netlist->m_nets[netName];
                if (!net.isValid())
                {
                    PyErr_Format(PyExc_ValueError, "Net %s not found", netName);
                    return nullptr;
                }

                return Python::toPython(net.ptr());
            }

            PyErr_Format(PyExc_ValueError, "getInstance requires a key or name as argument");
            return nullptr;
        }
        
        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");        
        return nullptr;         
    }

    static PyObject* pyIter(PyNets *self)
    {
        //std::cout << "PyNets::Iter\n";

        if (self->ok())
        {
            self->obj()->m_iter = self->obj()->begin();
            Py_INCREF(self);
            return (PyObject*)self;
        }

        return nullptr;
    };

    static PyObject* pyIterNext(PyNets *self)
    {
        //std::cout << "PyNets::IterNext\n";

        if (self->ok())
        {
            if (self->obj()->m_iter == self->obj()->end())
            {
                return nullptr; // no more object, stop iteration
            }

            auto kvpair = *self->obj()->m_iter;
            PyObject *cellObject = Python::toPython(kvpair.ptr());

            self->obj()->m_iter++;
            return (PyObject*)cellObject;
        }

        return nullptr;
    };

    /** set internal values of PyCell */
    static PyObject* pyStr(PyObject *self)
    {
        return Python::toPython(PyNets::PythonObjectName);
    };

    static constexpr const char *PythonObjectName = "Nets";
    static constexpr const char *PythonObjectDoc  = "Nets object";
};

// cppcheck-suppress "suppressed_error_id"
static PyMemberDef PyNetsMembers[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}  /* Sentinel */
};

static PyGetSetDef PyNetsGetSet[] =     // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}
};

static PyMethodDef PyNetsMethods[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {"getNet", (PyCFunction)PyNets::getNet, METH_VARARGS, "Lookup and return a net (by name or by key)"},
    {nullptr}  /* Sentinel */
};

PyTypeObject PyNetsType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    PyNets::PythonObjectName,       /* tp_name */
    sizeof(PyNets),                 /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)PyNets::pyDeAlloc,  /* tp_dealloc */
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
    PyNets::pyStr,                  /* tp_str */
    nullptr,                        /* tp_getattro */
    nullptr,                        /* tp_setattro */
    nullptr,                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,        /* tp_flags */
    PyNets::PythonObjectDoc,        /* tp_doc */
    nullptr,                        /* tp_traverse */
    nullptr,                        /* tp_clear */
    nullptr,                        /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    (getiterfunc)PyNets::pyIter,         /* tp_iter */
    (iternextfunc)PyNets::pyIterNext,    /* tp_iternext */
    PyNetsMethods,                  /* tp_methods */
    PyNetsMembers,                  /* tp_members */
    PyNetsGetSet,                   /* tp_getset */
    nullptr,                        /* tp_base */
    nullptr,                        /* tp_dict */
    nullptr,                        /* tp_descr_get */
    nullptr,                        /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)PyNets::pyInit,       /* tp_init */
    nullptr,                        /* tp_alloc */
    PyNets::pyNewCall,
};
