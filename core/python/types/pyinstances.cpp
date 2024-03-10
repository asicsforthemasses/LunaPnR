// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
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
#include "database/database.h"
#include "pyinstance.h"

struct PyInstancesIterator
{
    using iterator   = ChipDB::NamedStorage<ChipDB::Instance>::iterator;
    using value_type = std::shared_ptr<ChipDB::Instance>;

    std::shared_ptr<ChipDB::Netlist> m_netlist;
    iterator m_iter;

    iterator end()
    {
        if (!m_netlist)
        {
            return iterator{};
        }
        return m_netlist->m_instances.end();
    }

    iterator begin()
    {
        if (!m_netlist)
        {
            return iterator{};
        }
        return m_netlist->m_instances.begin();
    }
};

/** container for LunaCore::Cell */
struct PyInstances : public Python::TypeTemplate<PyInstancesIterator>
{
    /** set internal values of PyCell */
    static int pyInit(PyInstances *self, PyObject *args, PyObject *kwds)
    {
        //std::cout << "PyInstances::Init\n";

        // do not use ok() here, as it checks for
        // m_holder to be != nullptr.
        if (self->m_holder != nullptr)
        {
            //std::cout << "  Shared pointer created\n";

            self->m_holder->reset(new PyInstancesIterator());
            self->obj()->m_iter = PyInstancesIterator::iterator();

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
            PyErr_Format(PyExc_RuntimeError, "Internal error: PyInstances m_holder is nullptr!");
            return -1;
        }

        return 0;   /* success */
    };

    static PyObject* getInstance(PyInstances *self, PyObject *args)
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
                auto ins = self->obj()->m_netlist->m_instances[key];
                if (!ins)
                {
                    PyErr_Format(PyExc_ValueError, "Instance with key %ld not found", key);
                    return nullptr;
                }
                return Python::toPython(ins);
            }

            // clear previous PyArg_ParseTuple exception
            PyErr_Clear();

            const char *instanceName = nullptr;
            if (PyArg_ParseTuple(args,"s", &instanceName))
            {
                auto ins = self->obj()->m_netlist->m_instances[instanceName];
                if (!ins.isValid())
                {
                    PyErr_Format(PyExc_ValueError, "Instance %s not found", instanceName);
                    return nullptr;
                }

                return Python::toPython(ins.ptr());
            }

            PyErr_Format(PyExc_ValueError, "getInstance requires a key or name as argument");
            return nullptr;
        }

        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return nullptr;
    }

    static PyObject* pyIter(PyInstances *self)
    {
        //std::cout << "PyInstances::Iter\n";

        if (self->ok())
        {
            self->obj()->m_iter = self->obj()->begin();
            Py_INCREF(self);
            return (PyObject*)self;
        }

        return nullptr;
    };

    static PyObject* pyIterNext(PyInstances *self)
    {
        //std::cout << "PyInstances::IterNext\n";

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
        return Python::toPython(PyInstances::PythonObjectName);
    };

    static constexpr const char *PythonObjectName = "Instances";
    static constexpr const char *PythonObjectDoc  = "Instances object";
};

// cppcheck-suppress "suppressed_error_id"
static PyMemberDef PyInstancesMembers[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}  /* Sentinel */
};

static PyGetSetDef PyInstancesGetSet[] =     // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}
};

static PyMethodDef PyInstancesMethods[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {"getInstance", (PyCFunction)PyInstances::getInstance, METH_VARARGS, "Lookup and return an instance (by name or by key)"},
    {nullptr}  /* Sentinel */
};

PyTypeObject PyInstancesType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    PyInstances::PythonObjectName,       /* tp_name */
    sizeof(PyInstances),                 /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)PyInstances::pyDeAlloc,  /* tp_dealloc */
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
    PyInstances::pyStr,                  /* tp_str */
    nullptr,                        /* tp_getattro */
    nullptr,                        /* tp_setattro */
    nullptr,                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,        /* tp_flags */
    PyInstances::PythonObjectDoc,        /* tp_doc */
    nullptr,                        /* tp_traverse */
    nullptr,                        /* tp_clear */
    nullptr,                        /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    (getiterfunc)PyInstances::pyIter,         /* tp_iter */
    (iternextfunc)PyInstances::pyIterNext,    /* tp_iternext */
    PyInstancesMethods,                  /* tp_methods */
    PyInstancesMembers,                  /* tp_members */
    PyInstancesGetSet,                   /* tp_getset */
    nullptr,                        /* tp_base */
    nullptr,                        /* tp_dict */
    nullptr,                        /* tp_descr_get */
    nullptr,                        /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)PyInstances::pyInit,       /* tp_init */
    nullptr,                        /* tp_alloc */
    PyInstances::pyNewCall,
};
