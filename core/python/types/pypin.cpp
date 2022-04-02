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
#include "pypin.h"
#include "pypininfolist.h"

/** container for LunaCore::Cell */
struct PyPin : public Python::TypeTemplate<ChipDB::InstanceBase::Pin, 
    Python::ValueContainer<ChipDB::InstanceBase::Pin> >
{
    static PyObject* getName(PyPin *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->name());
        }
        
        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return nullptr;
    };

#if 0
    static PyObject* getArchetype(PyPin *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->getArchetypeName());
        }

        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return nullptr;
    }

    static PyObject* getPosition(PyPin *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->m_pos);
        }

        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return nullptr;
    }

    static int setPosition(PyPin *self, PyObject *value, void *closure)
    {
        if (self->ok())
        {
            if (!PyArg_ParseTuple(value, "LL", &self->obj()->m_pos.m_x, &self->obj()->m_pos.m_y))
            {
                return -1;
            }
            return 0; // success
        }

        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return -1;
    }

    static PyObject* getPlacementInfo(PyPin *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(ChipDB::toString(self->obj()->m_placementInfo));
        }

        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return nullptr;
    }

    static PyObject* getOrientation(PyPin *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(ChipDB::toString(self->obj()->m_orientation));
        }

        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return nullptr;
    }

    static PyObject* getSize(PyPin *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->instanceSize());
        }
        
        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");        
        return nullptr;        
    };

    static PyObject* getArea(PyPin *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->getArea());
        }
        
        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");        
        return nullptr;        
    };

#endif

    /** set internal values of PyPin */
    static int pyInit(PyPin *self, PyObject *args, PyObject *kwds)
    {
        return 0;   /* success */
    };

    /** set internal values of PyPin */
    static PyObject* pyStr(PyObject *self)
    {
        //std::cout << "pyStr\n";
        return Python::toPython(PyPin::PythonObjectName);
    };

    static constexpr const char *PythonObjectName = "InstancePin";
    static constexpr const char *PythonObjectDoc  = "InstancePin object";
};

// cppcheck-suppress "suppressed_error_id"
static PyMemberDef PyPinMembers[] =    // NOLINT(modernize-avoid-c-arrays)
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

static PyGetSetDef PyPinGetSet[] =     // NOLINT(modernize-avoid-c-arrays)
{
    {"name", (getter)PyPin::getName, nullptr, "instance name", nullptr /* closure */},
    {nullptr}
};

static PyMethodDef PyPinMethods[] =    // NOLINT(modernize-avoid-c-arrays)
{
//    {"name", (PyCFunction)PyPin::name, METH_NOARGS, "Return the cell name"},
    {nullptr}  /* Sentinel */
};

PyTypeObject PyPinType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    PyPin::PythonObjectName,       /* tp_name */
    sizeof(PyPin),                 /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)PyPin::pyDeAlloc,  /* tp_dealloc */
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
    PyPin::pyStr,                  /* tp_str */
    nullptr,                        /* tp_getattro */
    nullptr,                        /* tp_setattro */
    nullptr,                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,        /* tp_flags */
    PyPin::PythonObjectDoc,        /* tp_doc */
    nullptr,                        /* tp_traverse */
    nullptr,                        /* tp_clear */
    nullptr,                        /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    nullptr,                        /* tp_iter */
    nullptr,                        /* tp_iternext */
    PyPinMethods,                  /* tp_methods */
    PyPinMembers,                  /* tp_members */
    PyPinGetSet,                   /* tp_getset */
    nullptr,                        /* tp_base */
    nullptr,                        /* tp_dict */
    nullptr,                        /* tp_descr_get */
    nullptr,                        /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)PyPin::pyInit,       /* tp_init */
    nullptr,                        /* tp_alloc */
    PyPin::pyNewCall
};

PyObject* Python::toPython(const ChipDB::InstanceBase::Pin &pin)
{
    // create a new PyPin oject
    auto pinObject = reinterpret_cast<PyPin*>(PyObject_CallObject((PyObject*)&PyPinType, nullptr));
    if (pinObject->m_holder != nullptr)
    {
        *pinObject->m_holder = pin;
        return (PyObject*)pinObject;
    }
    return nullptr;
};
