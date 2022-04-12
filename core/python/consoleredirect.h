#pragma once
#include <functional>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

PyMODINIT_FUNC PyInit_ConsoleRedirect(void);

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
