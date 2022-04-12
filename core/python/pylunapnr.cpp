#include <fstream>
#include <sstream>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "consoleredirect.h"
#include "types/pypininfo.h"
#include "types/pypininfolist.h"
#include "types/pypin.h"
#include "types/pycell.h"
#include "types/pycelllib.h"
#include "types/pyinstance.h"
#include "types/pyinstances.h"
#include "types/pynet.h"
#include "types/pynets.h"
#include "pylunapnr.h"

#include "import/verilog/verilogreader.h"
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

static PyObject* pyLoadVerilog(PyObject *self, PyObject *args)
{
    const char *VerilogFileName = nullptr;
    if (PyArg_ParseTuple(args, "s", &VerilogFileName))
    {
        std::ifstream Verilogfile(VerilogFileName);
        if (!Verilogfile.good())
        {
            PyErr_Format(PyExc_FileNotFoundError, "Cannot open file %s", VerilogFileName);
            return nullptr;
        }

        auto designPtr = reinterpret_cast<ChipDB::Design*>(PyCapsule_Import("Luna.DesignPtr", 0));
        if (designPtr == nullptr)
        {
            PyErr_Format(PyExc_RuntimeError, "Unable to access design database");
            return nullptr;
        }

        if (ChipDB::Verilog::Reader::load(*designPtr, Verilogfile))
        {
            // Success!
            Py_INCREF(Py_None);
            return Py_None;
        }
        
        // load error
        PyErr_Format(PyExc_RuntimeError, "Unable parse/load verilog netlist file %s", VerilogFileName);
        return nullptr;
    }    

    PyErr_Format(PyExc_RuntimeError, "loadVerilog requires a filename argument");
    return nullptr;
}

static PyObject* pySetTopModule(PyObject *self, PyObject *args)
{
    const char *topModuleName = nullptr;
    if (PyArg_ParseTuple(args, "s", &topModuleName))
    {

        auto designPtr = reinterpret_cast<ChipDB::Design*>(PyCapsule_Import("Luna.DesignPtr", 0));
        if (designPtr == nullptr)
        {
            PyErr_Format(PyExc_RuntimeError, "Unable to access design database");
            return nullptr;
        }

        if (!designPtr->setTopModule(topModuleName))
        {
            PyErr_Format(PyExc_RuntimeError, "cannot set top module to %s", topModuleName);
            return nullptr;
        }

        // Success!
        Py_INCREF(Py_None);
        return Py_None;        
    }

    PyErr_Format(PyExc_RuntimeError, "setTopModule requires a module name argument");
    return nullptr;    
}

static PyObject* pyClear(PyObject *slef, PyObject *args)
{
    auto designPtr = reinterpret_cast<ChipDB::Design*>(PyCapsule_Import("Luna.DesignPtr", 0));
    if (designPtr == nullptr)
    {
        PyErr_Format(PyExc_RuntimeError, "Unable to access design database");
        return nullptr;
    }

    designPtr->clear();
    // Success!
    Py_INCREF(Py_None);
    return Py_None;      
}

static PyMethodDef LunaMethods[] =  // NOLINT(modernize-avoid-c-arrays)
{
    {"clear", pyClear, METH_NOARGS, "clear the design database"},
    {"loadLef", pyLoadLEF, METH_VARARGS, "load LEF file"},
    {"loadLib", pyLoadLIB, METH_VARARGS, "load LIB file"},
    {"loadVerilog", pyLoadVerilog, METH_VARARGS, "load verlog netlist file"},
    {"setTopModule", pySetTopModule, METH_VARARGS, "set the top level module"},
    {nullptr}
};

static PyModuleDef LunaModule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "Luna",
    .m_doc = "Interfaces with LunaPNR core datastructures",
    .m_size = -1,
    .m_methods = LunaMethods
};

/** function to add a type to the Python interpreter */
bool incRefAndAddObject(PyObject *module, PyTypeObject *typeObj)
{
    if (typeObj == nullptr)
    {
        return false;
    }

    Py_INCREF(typeObj);
    if (PyModule_AddObject(module, typeObj->tp_name, (PyObject *)typeObj) < 0)
    {
        Py_DECREF(&PyPinInfoType);
        return false;        
    }

    return true;
}

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

    if (PyType_Ready(&PyInstanceType) < 0)
        return nullptr;

    if (PyType_Ready(&PyInstancesType) < 0)
        return nullptr;

    if (PyType_Ready(&PyPinType) < 0)
        return nullptr;

    if (PyType_Ready(&PyNetType) < 0)
        return nullptr;

    if (PyType_Ready(&PyNetsType) < 0)
        return nullptr;

    auto m = PyModule_Create(&LunaModule);
    if (m == nullptr)
        return nullptr;

    incRefAndAddObject(m, &PyPinType);
    incRefAndAddObject(m, &PyPinInfoType);
    incRefAndAddObject(m, &PyPinInfoListType);
    incRefAndAddObject(m, &PyCellType);
    incRefAndAddObject(m, &PyCellLibType);
    incRefAndAddObject(m, &PyInstanceType);
    incRefAndAddObject(m, &PyInstancesType);
    incRefAndAddObject(m, &PyNetType);
    incRefAndAddObject(m, &PyNetsType);
    
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

void Scripting::Python::setConsoleRedirect(std::function<void(const char *, ssize_t)> stdoutFunc,
    std::function<void(const char *, ssize_t)> stderrFunc)
{
    auto pyStdout = reinterpret_cast<Scripting::PyConsoleRedirect::PyStdout*>(PySys_GetObject("stdout"));
    if (pyStdout != nullptr)
    {
        pyStdout->writeFunc = stdoutFunc;
    }    

    auto pyStderr = reinterpret_cast<Scripting::PyConsoleRedirect::PyStdout*>(PySys_GetObject("stderr"));
    if (pyStderr != nullptr)
    {
        pyStderr->writeFunc = stderrFunc;
    }        
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

