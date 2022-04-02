/*  LunaPnR Source Code
 
    SPDX-License-Identifier: GPL-3.0-only
    SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/

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
#include "pyinstance.h"

struct PyInstancesIterator
{
    using iterator   = ChipDB::NamedStorage<ChipDB::InstanceBase>::iterator;
    using value_type = std::shared_ptr<ChipDB::InstanceBase>;

    std::shared_ptr<ChipDB::Netlist> m_netlist;
    iterator m_iter;

    iterator end()
    {
        if (!m_netlist)
        {
            return iterator();
        }
        return m_netlist->m_instances.end();
    }

    iterator begin()
    {
        if (!m_netlist)
        {
            return iterator();
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
            self->obj()->m_iter = self->obj()->end();

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
static PyMemberDef PyCellMembers[] =    // NOLINT(modernize-avoid-c-arrays)
{/*
    {"first", T_OBJECT_EX, offsetof(Noddy, first), nullptr,
    "first name"},
    {"last", T_OBJECT_EX, offsetof(Noddy, last), nullptr,
    "last name"},
    {"number", T_INT, offsetof(Noddy, number), nullptr,
    "noddy number"},
*/
    {nullptr}  /* Sentinel */
};

static PyGetSetDef PyCellGetSet[] =     // NOLINT(modernize-avoid-c-arrays)
{
    //{"name", (getter)PyCell::getName, nullptr, "", nullptr /* closure */},
    //{"number", (getter)PyCell::getNumber, (setter)PyCell::setNumber, "", nullptr /* closure */},
    {nullptr}
};

static PyMethodDef PyCellMethods[] =    // NOLINT(modernize-avoid-c-arrays)
{
//    {"name", (PyCFunction)PyCell::name, METH_NOARGS, "Return the cell name"},
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
    PyCellMethods,                  /* tp_methods */
    PyCellMembers,                  /* tp_members */
    PyCellGetSet,                   /* tp_getset */
    nullptr,                        /* tp_base */
    nullptr,                        /* tp_dict */
    nullptr,                        /* tp_descr_get */
    nullptr,                        /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)PyInstances::pyInit,       /* tp_init */
    nullptr,                        /* tp_alloc */
    PyInstances::pyNewCall,
};
