#include <fstream>
#include <sstream>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "pyluna_extra.h"
#include "common/guihelpers.h"

static GUI::Database* getDatabase()
{
    return reinterpret_cast<GUI::Database*>(PyCapsule_Import("LunaExtra.DatabasePtr", 0));
}

///> loadLayers(filename : string)
static PyObject* pyLoadLayers(PyObject *self, PyObject *args)
{
    const char *layerFileName = nullptr;
    if (PyArg_ParseTuple(args, "s", &layerFileName))
    {
        std::ifstream layerFile(layerFileName);
        if (!layerFile.good())
        {
            PyErr_Format(PyExc_FileNotFoundError, "Cannot open file %s", layerFileName);
            return nullptr;
        }

        auto databasePtr = getDatabase();
        if (databasePtr == nullptr)
        {
            return PyErr_Format(PyExc_RuntimeError, "Unable to access GUI database");
        }

        std::stringstream buffer;
        buffer << layerFile.rdbuf();

        if (!databasePtr->m_layerRenderInfoDB.readJson(buffer.str()))
        {
            return PyErr_Format(PyExc_RuntimeError, "Error parsing Layers file '%s'", layerFileName);
        }
        
        Py_RETURN_NONE;
    }
    
    return PyErr_Format(PyExc_RuntimeError, "loadLayers requires a filename argument");
}

///> addHatch(width : integer, height : integer, pattern : string)
static PyObject* pyAddHatch(PyObject *self, PyObject *args)
{
    auto databasePtr = getDatabase();
    if (databasePtr == nullptr)
    {
        PyErr_Format(PyExc_RuntimeError, "Unable to access GUI database");
        return nullptr;
    }

    int width, height;
    const char *str;
    if (PyArg_ParseTuple(args, "iis", &width, &height, &str))
    {
        auto hatchPixmap = GUI::createPixmapFromString(std::string(str), width, height);
        if (hatchPixmap.has_value())
        {
            databasePtr->m_hatchLib.m_hatches.push_back(*hatchPixmap);
        }        
        Py_RETURN_NONE;
    }

    return PyErr_Format(PyExc_RuntimeError, "Invalid arguments");
}

/** function to add a type to the Python interpreter */
static bool incRefAndAddObject(PyObject *module, PyTypeObject *typeObj)
{
    if (typeObj == nullptr)
    {
        return false;
    }

    Py_INCREF(typeObj);
    if (PyModule_AddObject(module, typeObj->tp_name, (PyObject *)typeObj) < 0)
    {
        Py_DECREF(typeObj);
        return false;        
    }

    return true;
}

static PyMethodDef LunaExtraMethods[] =  // NOLINT(modernize-avoid-c-arrays)
{
    {"loadLayers", pyLoadLayers, METH_VARARGS, "load the layer definitions for the GUI"},
    {"addHatch", pyAddHatch, METH_VARARGS, "add hatch to the hatch library"},
    {nullptr}
};

static PyModuleDef LunaExtraModule = {  
    PyModuleDef_HEAD_INIT,              
    .m_name = "LunaExtra",                  // NOLINT(clang-diagnostic-c99-designator)
    .m_doc = "Interfaces with LunaPNR GUI",
    .m_size = -1,
    .m_methods = LunaExtraMethods
};

static PyObject* PyInit_LunaExtra()
{
    std::cout << "PyInit_LunaExtra called\n";
    auto m = PyModule_Create(&LunaExtraModule);
    if (m == nullptr)
    {
        return nullptr;
    }

    return m;
}

GUI::Python::Python(GUI::Database *db) : Scripting::Python(&db->design()), m_db(db)
{
}

GUI::Python::~Python()
{
}

bool GUI::Python::preInitHook()
{
    PyImport_AppendInittab("LunaExtra", &PyInit_LunaExtra);
    return true;
}

bool GUI::Python::postInitHook()
{
    auto lunaExtraModule = PyImport_ImportModule("LunaExtra");

    if (lunaExtraModule == nullptr)
    {
        return false;
    }

    auto capsule = PyCapsule_New(m_db, "LunaExtra.DatabasePtr", nullptr);
    
    if (PyModule_AddObject(lunaExtraModule, "DatabasePtr", capsule) < 0)
    {
        std::cout << "PyModule_AddObject failed!\n";
        Py_XDECREF(capsule);
    }    

    return true;
}
