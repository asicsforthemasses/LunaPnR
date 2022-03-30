#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "consoleredirect.h"
#include "types/pycell.h"

#include "pylunapnr.h"

static PyModuleDef LunaModule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "Luna",
    .m_doc = "Interfaces with LunaPNR core datastructures",
    .m_size = -1
};

static PyObject* PyInit_Luna()
{
    if (PyType_Ready(&PyCellType) < 0)
        return nullptr;

    auto m = PyModule_Create(&LunaModule);
    if (m == nullptr)
        return nullptr;

    Py_INCREF(&PyCellType);
    if (PyModule_AddObject(m, PyCellType.tp_name, (PyObject *) &PyCellType) < 0) 
    {
        Py_DECREF(&PyCellType);
        Py_DECREF(m);
        return nullptr;
    }

#if 0
    g_vector.emplace_back(std::make_shared<MyCppStruct>());
    g_vector.emplace_back(std::make_shared<MyCppStruct>());
    g_vector.emplace_back(std::make_shared<MyCppStruct>());
    g_vector.emplace_back(std::make_shared<MyCppStruct>());

    g_vector.at(0)->m_name = "Vlaai";
    g_vector.at(1)->m_name = "Susanne";
    g_vector.at(2)->m_name = "Niels";
    g_vector.at(3)->m_name = "Joyce";
#endif

    return m;
}

Scripting::Python::Python()
{
    PyImport_AppendInittab("ConsoleRedirect", &PyInit_ConsoleRedirect);
    PyImport_AppendInittab("Luna", &PyInit_Luna);
    Py_Initialize();    
    PyImport_ImportModule("ConsoleRedirect");
}

Scripting::Python::~Python()
{
    Py_FinalizeEx();
}

bool Scripting::Python::executeScript(const std::string &code)
{
    const int PyResultOK = 0;
    if (PyRun_SimpleString(code.c_str()) == PyResultOK)
    {
        return true;
    }

    return false;
}
