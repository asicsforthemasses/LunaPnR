// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <functional>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

PyMODINIT_FUNC PyInit_ConsoleRedirect(void);

#ifdef NO_SSIZE_T
    #include <type_traits>
    typedef std::make_signed<size_t>::type ssize_t;
#endif

namespace Scripting::PyConsoleRedirect
{

struct PyStdout
{
    PyObject_HEAD
    std::function<void(const char *, ssize_t strLen)> writeFunc;

    static PyObject* pyWrite(PyObject *self, PyObject *args);
    static PyObject* pyFlush(PyObject *self, PyObject *args);
};

}; //namespace
