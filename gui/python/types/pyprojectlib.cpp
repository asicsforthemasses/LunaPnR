// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
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
struct PyProjectLibFiles : public PyVectorOfStrings
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

        self->obj()->m_vector = &designPtr->m_projectSetup.m_libFiles;
        return 0;   /* success */
    };

    /** set internal values of PyCell */
    static PyObject* pyStr(PyObject *self)
    {
        return Python::toPython(PyProjectLibFiles::PythonObjectName);
    };

    static constexpr const char *PythonObjectName = "ProjectLibFiles";
    static constexpr const char *PythonObjectDoc  = "Project LIB files object";
};

// cppcheck-suppress "suppressed_error_id"
static PyMemberDef PyProjectLibFilesMembers[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}  /* Sentinel */
};

static PyGetSetDef PyProjectLibFilesGetSet[] =     // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}
};

static PyMethodDef PyProjectLibFilesMethods[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {"at", (PyCFunction)PyProjectLibFiles::getString, METH_VARARGS, "Lookup and return a string by index"},
    {nullptr}  /* Sentinel */
};

PyTypeObject PyProjectLibFilesType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    PyProjectLibFiles::PythonObjectName,       /* tp_name */
    sizeof(PyProjectLibFiles),                 /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)PyProjectLibFiles::pyDeAlloc,  /* tp_dealloc */
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
    PyProjectLibFiles::pyStr,                  /* tp_str */
    nullptr,                        /* tp_getattro */
    nullptr,                        /* tp_setattro */
    nullptr,                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,        /* tp_flags */
    PyProjectLibFiles::PythonObjectDoc,        /* tp_doc */
    nullptr,                        /* tp_traverse */
    nullptr,                        /* tp_clear */
    nullptr,                        /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    (getiterfunc)PyProjectLibFiles::pyIter,         /* tp_iter */
    (iternextfunc)PyProjectLibFiles::pyIterNext,    /* tp_iternext */
    PyProjectLibFilesMethods,                  /* tp_methods */
    PyProjectLibFilesMembers,                  /* tp_members */
    PyProjectLibFilesGetSet,                   /* tp_getset */
    nullptr,                        /* tp_base */
    nullptr,                        /* tp_dict */
    nullptr,                        /* tp_descr_get */
    nullptr,                        /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)PyProjectLibFiles::pyInit,       /* tp_init */
    nullptr,                        /* tp_alloc */
    PyProjectLibFiles::pyNewCall,
};
