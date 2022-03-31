#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "consoleredirect.h"
#include "types/pycell.h"
#include "types/pycelllib.h"

#include "pylunapnr.h"

static PyModuleDef LunaModule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "Luna",
    .m_doc = "Interfaces with LunaPNR core datastructures",
    .m_size = -1
};

static PyObject* PyInit_Luna()
{
    std::cout << "Luna module init called\n";

    if (PyType_Ready(&PyCellType) < 0)
        return nullptr;

    if (PyType_Ready(&PyCellLibType) < 0)
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

    Py_INCREF(&PyCellLibType);
    if (PyModule_AddObject(m, PyCellLibType.tp_name, (PyObject *) &PyCellLibType) < 0) 
    {
        Py_DECREF(&PyCellLibType);
        Py_DECREF(m);
        return nullptr;
    }

    return m;
}

Scripting::Python::Python(ChipDB::Design *design) : m_design(design)
{
    PyImport_AppendInittab("ConsoleRedirect", &PyInit_ConsoleRedirect);
    PyImport_AppendInittab("Luna", &PyInit_Luna);
    Py_Initialize();

    PyImport_ImportModule("ConsoleRedirect");
    auto lunaModule = PyImport_ImportModule("Luna");

    if (lunaModule == nullptr)
    {
        std::cout << "Luna module init failed!\n";
    }

    auto capsule = PyCapsule_New(design->m_cellLib.get(), "Luna.CellLibraryPtr", nullptr);

    if (PyModule_AddObject(lunaModule, "CellLibraryPtr", capsule) < 0)
    {
        std::cout << "PyModule_AddObject failed!\n";
        Py_XDECREF(capsule);
    }
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

