// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <string>
#include <iostream>
#include "consoleredirect.h"

// inspiration:
// https://github.com/mloskot/workshop/blob/master/python/emb/emb.cpp

PyObject* Scripting::PyConsoleRedirect::PyStdout::pyWrite(PyObject *self, PyObject *args)
{
    auto *pyStdout = reinterpret_cast<Scripting::PyConsoleRedirect::PyStdout*>(self);
    if (pyStdout->writeFunc)
    {
        ssize_t strLen = 0;
        const char *str;

        //static int counter = 0;

        if (PyArg_ParseTuple(args, "s#", &str, &strLen))
        {
            //std::cout << (counter++) << " PyWrite pre\n";
            //std::cout << "'" << str << "'" << "\n";
            pyStdout->writeFunc(str, strLen);
            //std::cout << "PyWrite post\n";
            Py_RETURN_NONE;
        }
        else
        {
            // FIXME: handle conversion error
            std::cerr << "Error in pyWrite!\n";
            return nullptr;
        }
    }

    Py_RETURN_NONE;
}

PyObject* Scripting::PyConsoleRedirect::PyStdout::pyFlush(PyObject *self, PyObject *args)
{
    Py_RETURN_NONE;
};


namespace Scripting::PyConsoleRedirect
{

PyMethodDef PyStdout_methods[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {"write", PyStdout::pyWrite, METH_VARARGS, "sys.stdout.write"},
    {"flush", PyStdout::pyFlush, METH_VARARGS, "sys.stdout.flush"},
    {nullptr, nullptr, 0, nullptr} // sentinel
};

PyTypeObject PyStdoutType =
{
    PyVarObject_HEAD_INIT(nullptr, 0)
    "PyStdoutType",       /* tp_name */
    sizeof(PyStdout),     /* tp_basicsize */
    0,                    /* tp_itemsize */
    nullptr,                    /* tp_dealloc */
    0,                    /* tp_print */
    nullptr,                    /* tp_getattr */
    nullptr,                    /* tp_setattr */
    nullptr,                    /* tp_reserved */
    nullptr,                    /* tp_repr */
    nullptr,                    /* tp_as_number */
    nullptr,                    /* tp_as_sequence */
    nullptr,                    /* tp_as_mapping */
    nullptr,                    /* tp_hash  */
    nullptr,                    /* tp_call */
    nullptr,                    /* tp_str */
    nullptr,                    /* tp_getattro */
    nullptr,                    /* tp_setattro */
    nullptr,                    /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,   /* tp_flags */
    "PyStdout object",    /* tp_doc */
    nullptr,                    /* tp_traverse */
    nullptr,                    /* tp_clear */
    nullptr,                    /* tp_richcompare */
    0,                    /* tp_weaklistoffset */
    nullptr,                    /* tp_iter */
    nullptr,                    /* tp_iternext */
    PyStdout_methods,     /* tp_methods */
    nullptr,                    /* tp_members */
    nullptr,                    /* tp_getset */
    nullptr,                    /* tp_base */
    nullptr,                    /* tp_dict */
    nullptr,                    /* tp_descr_get */
    nullptr,                    /* tp_descr_set */
    0,                    /* tp_dictoffset */
    nullptr,                    /* tp_init */
    nullptr,                    /* tp_alloc */
    PyType_GenericNew     /* tp_new */
};

PyModuleDef consoleRedirectModule =
{
    PyModuleDef_HEAD_INIT,
    "ConsoleRedirect", nullptr, -1, nullptr,
};

PyMODINIT_FUNC PyInit_ConsoleRedirect(void)
{

    if (PyType_Ready(&PyStdoutType) < 0)
        return nullptr;

    PyObject* m = PyModule_Create(&consoleRedirectModule);
    if (m)
    {
        Py_INCREF(&PyStdoutType);
        PyModule_AddObject(m, "Stdout", reinterpret_cast<PyObject*>(&PyStdoutType));
    }

    // install the stdout and stderr
    // this cannot be undone, which is kind of the point.
    auto stdTypeObj = PyStdoutType.tp_new(&PyStdoutType, nullptr, nullptr);
    auto stdObj = reinterpret_cast<PyStdout*>(stdTypeObj);

    stdObj->writeFunc = [](const char *txt, ssize_t strLen)
    {
        std::string_view sv(txt, strLen);
        std::cout << sv << std::flush;
    };

    auto stdErrTypeObj = PyStdoutType.tp_new(&PyStdoutType, nullptr, nullptr);
    auto stdErrObj = reinterpret_cast<PyStdout*>(stdErrTypeObj);

    stdErrObj->writeFunc = [](const char *txt, ssize_t strLen)
    {
        std::string_view sv(txt, strLen);
        std::cerr << sv << std::flush;
    };

    PySys_SetObject("stdout", (PyObject*)stdObj);
    PySys_SetObject("stderr", (PyObject*)stdErrObj);

    return m;
};

};
