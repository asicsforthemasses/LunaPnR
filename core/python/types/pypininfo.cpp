// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include <memory>
#include <array>
#include <iostream>

#include "../converters.h"
#include "pypininfo.h"
#include "typetemplate.h"

/** container for LunaCore::PinInfo */
struct PyPinInfo : public Python::TypeTemplate<ChipDB::PinInfo>
{
    static PyObject* getName(PyPinInfo *self, void *closure)
    {
        //std::cout << "PyCell::getName\n";
        if (self->ok())
        {
            return Python::toPython(self->obj()->name());
        }

        return nullptr;
    };

    static PyObject* getIOType(PyPinInfo *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(ChipDB::toString(self->obj()->m_iotype));
        }
        return nullptr;
    };

    static PyObject* getCapacitance(PyPinInfo *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->m_cap);
        }
        return nullptr;
    };

    static PyObject* getMaxCapacitance(PyPinInfo *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->m_maxCap);
        }
        return nullptr;
    };

    static PyObject* getClock(PyPinInfo *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->m_clock);
        }
        return nullptr;
    };

    static PyObject* getOffset(PyPinInfo *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->m_offset);
        }
        return nullptr;
    };

    static PyObject* getFunction(PyPinInfo *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->m_function);
        }
        return nullptr;
    };

    static PyObject* getTristateFunction(PyPinInfo *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->m_tristateFunction);
        }
        return nullptr;
    };

    /** set internal values of PyCell */
    static int pyInit(PyPinInfo *self, PyObject *args, PyObject *kwds)
    {
        return 0;   /* success */
    };

    /** set internal values of PyCell */
    static PyObject* pyStr(PyObject *self)
    {
        return Python::toPython(PyPinInfo::PythonObjectName);
    };

    static constexpr const char *PythonObjectName = "PinInfo";
    static constexpr const char *PythonObjectDoc  = "PinInfo object";
};

// cppcheck-suppress "suppressed_error_id"
static PyMemberDef PyPinInfoMembers[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}  /* Sentinel */
};

static PyGetSetDef PyPinInfoGetSet[] =     // NOLINT(modernize-avoid-c-arrays)
{
    {"name", (getter)PyPinInfo::getName, nullptr, "pin name", nullptr /* closure */},
    {"function", (getter)PyPinInfo::getFunction, nullptr, "pin function (outputs only)", nullptr /* closure */},
    {"ioType", (getter)PyPinInfo::getIOType, nullptr, "pin IO type", nullptr /* closure */},
    {"tristateFunction", (getter)PyPinInfo::getTristateFunction, nullptr, "pin tri-state function", nullptr /* closure */},
    {"capacitance", (getter)PyPinInfo::getCapacitance, nullptr, "pin load capacitance (input only)", nullptr /* closure */},
    {"maxCapacitance", (getter)PyPinInfo::getMaxCapacitance, nullptr, "pin max load capacitance (output only)", nullptr /* closure */},
    {"clock", (getter)PyPinInfo::getClock, nullptr, "boolean to signify clock pin", nullptr /* closure */},
    {"offset", (getter)PyPinInfo::getOffset, nullptr, "pin location w.r.t lower left cell coordinate", nullptr /* closure */},
    {nullptr}
};

static PyMethodDef PyPinInfoMethods[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}  /* Sentinel */
};

PyTypeObject PyPinInfoType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    PyPinInfo::PythonObjectName,       /* tp_name */
    sizeof(PyPinInfo),                 /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)PyPinInfo::pyDeAlloc,  /* tp_dealloc */
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
    PyPinInfo::pyStr,                  /* tp_str */
    nullptr,                        /* tp_getattro */
    nullptr,                        /* tp_setattro */
    nullptr,                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,        /* tp_flags */
    PyPinInfo::PythonObjectDoc,        /* tp_doc */
    nullptr,                        /* tp_traverse */
    nullptr,                        /* tp_clear */
    nullptr,                        /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    nullptr,                        /* tp_iter */
    nullptr,                        /* tp_iternext */
    PyPinInfoMethods,                  /* tp_methods */
    PyPinInfoMembers,                  /* tp_members */
    PyPinInfoGetSet,                   /* tp_getset */
    nullptr,                        /* tp_base */
    nullptr,                        /* tp_dict */
    nullptr,                        /* tp_descr_get */
    nullptr,                        /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)PyPinInfo::pyInit,       /* tp_init */
    nullptr,                        /* tp_alloc */
    PyPinInfo::pyNewCall
};

PyObject* Python::toPython(std::shared_ptr<ChipDB::PinInfo> ptr)
{
    // create a new PyCell oject
    auto pinInfoObject = reinterpret_cast<PyPinInfo*>(PyObject_CallObject((PyObject*)&PyPinInfoType, nullptr));
    if (pinInfoObject->m_holder != nullptr)
    {
        *pinInfoObject->m_holder = ptr;
        return (PyObject*)pinInfoObject;
    }
    return nullptr;
};
