// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include <memory>
#include <array>
#include <iostream>

#include "../../common/database.h"

#include "pyvectorofstrings.h"

/** read-only container for std::vector<std::string> */
struct PyProjectVerilogFiles : public PyVectorOfStrings
{
    /** set internal values of PyVectorOfStrings */
    static int pyInit(PyVectorOfStrings *self, PyObject *args, PyObject *kwds)
    {
        if (self->m_holder == nullptr)
        {
            return -1; /* error */
        }

        self->m_holder->reset(new PyVectorOfStringsIterator());

        // get a pointer to database
        auto designPtr = reinterpret_cast<GUI::Database*>(PyCapsule_Import("LunaExtra.DatabasePtr", 0));
        if (designPtr == nullptr)
        {
            PyErr_Format(PyExc_RuntimeError, "Unable to access design database");
            return -1;
        }

        self->obj()->m_vector = &designPtr->m_projectSetup.m_verilogFiles;
        return 0;   /* success */
    };

    /** set internal values of PyCell */
    static PyObject* pyStr(PyObject *self)
    {
        return Python::toPython(PyProjectVerilogFiles::PythonObjectName);
    };

    static constexpr const char *PythonObjectName = "ProjectVerilogFiles";
    static constexpr const char *PythonObjectDoc  = "Project ProjectVerilog files object";
};

// cppcheck-suppress "suppressed_error_id"
static PyMemberDef PyProjectVerilogFilesMembers[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}  /* Sentinel */
};

static PyGetSetDef PyProjectVerilogFilesGetSet[] =     // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}
};

static PyMethodDef PyProjectVerilogFilesMethods[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {"at", (PyCFunction)PyProjectVerilogFiles::getString, METH_VARARGS, "Lookup and return a string by index"},
    {nullptr}  /* Sentinel */
};

PyTypeObject PyProjectVerilogFilesType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    PyProjectVerilogFiles::PythonObjectName,       /* tp_name */
    sizeof(PyProjectVerilogFiles),                 /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)PyProjectVerilogFiles::pyDeAlloc,  /* tp_dealloc */
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
    PyProjectVerilogFiles::pyStr,                  /* tp_str */
    nullptr,                        /* tp_getattro */
    nullptr,                        /* tp_setattro */
    nullptr,                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,        /* tp_flags */
    PyProjectVerilogFiles::PythonObjectDoc,        /* tp_doc */
    nullptr,                        /* tp_traverse */
    nullptr,                        /* tp_clear */
    nullptr,                        /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    (getiterfunc)PyProjectVerilogFiles::pyIter,         /* tp_iter */
    (iternextfunc)PyProjectVerilogFiles::pyIterNext,    /* tp_iternext */
    PyProjectVerilogFilesMethods,                  /* tp_methods */
    PyProjectVerilogFilesMembers,                  /* tp_members */
    PyProjectVerilogFilesGetSet,                   /* tp_getset */
    nullptr,                        /* tp_base */
    nullptr,                        /* tp_dict */
    nullptr,                        /* tp_descr_get */
    nullptr,                        /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)PyProjectVerilogFiles::pyInit,       /* tp_init */
    nullptr,                        /* tp_alloc */
    PyProjectVerilogFiles::pyNewCall,
};
