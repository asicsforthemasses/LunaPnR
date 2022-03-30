#include <functional>
#include <string>
#include <iostream>
#include <Python.h>

// inspiration:
// https://github.com/mloskot/workshop/blob/master/python/emb/emb.cpp

namespace Python::ConsoleRedirect
{

struct PyStdout
{
    PyObject_HEAD
    std::function<void(const char *)> writeFunc;

    static PyObject* pyWrite(PyObject *self, PyObject *args)
    {
        auto *pyStdout = reinterpret_cast<PyStdout*>(self);
        if (pyStdout->writeFunc)
        {
            char *str;
            if (!PyArg_ParseTuple(args, "s", &str))
            {
                return nullptr;
            }

            pyStdout->writeFunc(str);
        }

        return Py_BuildValue("");
    };

    static PyObject* pyFlush(PyObject *self, PyObject *args)
    {
        return Py_BuildValue("");
    };

};

PyMethodDef PyStdout_methods[] =
{
    {"write", PyStdout::pyWrite, METH_VARARGS, "sys.stdout.write"},
    {"flush", PyStdout::pyFlush, METH_VARARGS, "sys.stdout.write"},
    {0, 0, 0, 0} // sentinel
};

PyTypeObject PyStdoutType = 
{
    PyVarObject_HEAD_INIT(0, 0)
    "PyStdoutType",       /* tp_name */
    sizeof(PyStdout),     /* tp_basicsize */
    0,                    /* tp_itemsize */
    0,                    /* tp_dealloc */
    0,                    /* tp_print */
    0,                    /* tp_getattr */
    0,                    /* tp_setattr */
    0,                    /* tp_reserved */
    0,                    /* tp_repr */
    0,                    /* tp_as_number */
    0,                    /* tp_as_sequence */
    0,                    /* tp_as_mapping */
    0,                    /* tp_hash  */
    0,                    /* tp_call */
    0,                    /* tp_str */
    0,                    /* tp_getattro */
    0,                    /* tp_setattro */
    0,                    /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,   /* tp_flags */
    "PyStdout object",    /* tp_doc */
    0,                    /* tp_traverse */
    0,                    /* tp_clear */
    0,                    /* tp_richcompare */
    0,                    /* tp_weaklistoffset */
    0,                    /* tp_iter */
    0,                    /* tp_iternext */
    PyStdout_methods,     /* tp_methods */
    0,                    /* tp_members */
    0,                    /* tp_getset */
    0,                    /* tp_base */
    0,                    /* tp_dict */
    0,                    /* tp_descr_get */
    0,                    /* tp_descr_set */
    0,                    /* tp_dictoffset */
    0,                    /* tp_init */
    0,                    /* tp_alloc */
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

    // install the stdout

    auto stdTypeObj = PyStdoutType.tp_new(&PyStdoutType, nullptr, nullptr);
    auto stdObj = reinterpret_cast<PyStdout*>(stdTypeObj);

    stdObj->writeFunc = [](const char *txt)
    {
        std::cout << "Console: " << txt << "\n";
    };

    PySys_SetObject("stdout", (PyObject*)stdObj);

    return m;
};

};
