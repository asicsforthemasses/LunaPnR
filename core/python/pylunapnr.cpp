#include <fstream>
#include <sstream>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "consoleredirect.h"
#include "types/pypininfo.h"
#include "types/pypininfolist.h"
#include "types/pycell.h"
#include "types/pycelllib.h"
#include "pylunapnr.h"

#include "import/lef/lefreader.h"
#include "import/liberty/libreader.h"

static PyObject* pyLoadLEF(PyObject *self, PyObject *args)
{
    const char *LEFFileName = nullptr;
    if (PyArg_ParseTuple(args, "s", &LEFFileName))
    {
        std::ifstream LEFfile(LEFFileName);
        if (!LEFfile.good())
        {
            PyErr_Format(PyExc_FileNotFoundError, "Cannot open file %s", LEFFileName);
            return nullptr;
        }

        auto designPtr = reinterpret_cast<ChipDB::Design*>(PyCapsule_Import("Luna.DesignPtr", 0));
        if (designPtr == nullptr)
        {
            PyErr_Format(PyExc_RuntimeError, "Unable to access design database");
            return nullptr;
        }

        if (ChipDB::LEF::Reader::load(*designPtr, LEFfile))
        {
            // Success!            
            Py_INCREF(Py_None);
            return Py_None;
        }
        
        // TODO: set exception / error.
        // load error
        PyErr_Format(PyExc_RuntimeError, "Unable parse/load LEF file %s", LEFFileName);
        return nullptr;
    }    

    PyErr_Format(PyExc_RuntimeError, "loadLef requires a filename argument");
    return nullptr;
}

static PyObject* pyLoadLIB(PyObject *self, PyObject *args)
{
    const char *LIBFileName = nullptr;
    if (PyArg_ParseTuple(args, "s", &LIBFileName))
    {
        std::ifstream LIBfile(LIBFileName);
        if (!LIBfile.good())
        {
            PyErr_Format(PyExc_FileNotFoundError, "Cannot open file %s", LIBFileName);
            return nullptr;
        }

        auto designPtr = reinterpret_cast<ChipDB::Design*>(PyCapsule_Import("Luna.DesignPtr", 0));
        if (designPtr == nullptr)
        {
            PyErr_Format(PyExc_RuntimeError, "Unable to access design database");
            return nullptr;
        }

        if (ChipDB::Liberty::Reader::load(*designPtr, LIBfile))
        {
            // Success!
            Py_INCREF(Py_None);
            return Py_None;
        }
        
        // load error
        PyErr_Format(PyExc_RuntimeError, "Unable parse/load LIB file %s", LIBFileName);
        return nullptr;
    }    

    PyErr_Format(PyExc_RuntimeError, "loadLib requires a filename argument");
    return nullptr;
}

static PyMethodDef LunaMethods[] =  // NOLINT(modernize-avoid-c-arrays)
{
    {"loadLef", pyLoadLEF, METH_VARARGS, "load LEF file"},
    {"loadLib", pyLoadLIB, METH_VARARGS, "load LIB file"},
    {nullptr}
};

static PyModuleDef LunaModule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "Luna",
    .m_doc = "Interfaces with LunaPNR core datastructures",
    .m_size = -1,
    .m_methods = LunaMethods
};

static PyObject* PyInit_Luna()
{
    std::cout << "Luna module init called\n";

    if (PyType_Ready(&PyCellType) < 0)
        return nullptr;

    if (PyType_Ready(&PyCellLibType) < 0)
        return nullptr;

    if (PyType_Ready(&PyPinInfoType) < 0)
        return nullptr;

    if (PyType_Ready(&PyPinInfoListType) < 0)
        return nullptr;

    auto m = PyModule_Create(&LunaModule);
    if (m == nullptr)
        return nullptr;

    Py_INCREF(&PyPinInfoType);
    if (PyModule_AddObject(m, PyPinInfoType.tp_name, (PyObject *) &PyPinInfoType) < 0) 
    {
        Py_DECREF(&PyPinInfoType);
        Py_DECREF(m);
        return nullptr;
    }

    Py_INCREF(&PyPinInfoListType);
    if (PyModule_AddObject(m, PyPinInfoListType.tp_name, (PyObject *) &PyPinInfoListType) < 0) 
    {
        Py_DECREF(&PyPinInfoListType);
        Py_DECREF(m);
        return nullptr;
    }

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

    auto capsule  = PyCapsule_New(design->m_cellLib.get(), "Luna.CellLibraryPtr", nullptr);
    
    if (PyModule_AddObject(lunaModule, "CellLibraryPtr", capsule) < 0)
    {
        std::cout << "PyModule_AddObject failed!\n";
        Py_XDECREF(capsule);
    }

    auto capsule2 = PyCapsule_New(design, "Luna.DesignPtr", nullptr);

    if (PyModule_AddObject(lunaModule, "DesignPtr", capsule2) < 0)
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

