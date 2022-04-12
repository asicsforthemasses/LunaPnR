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

#include "common/logging.h"
#include "cellplacer/qlaplacer.h"
#include "import/verilog/verilogreader.h"
#include "import/lef/lefreader.h"
#include "import/liberty/libreader.h"

static inline ChipDB::Design* getDesign()
{
    return (ChipDB::Design*)PyCapsule_Import("Luna.DesignPtr", 0);
}

///> loadLef(filename : string)
static PyObject* pyLoadLEF(PyObject *self, PyObject *args)
{
    const char *LEFFileName = nullptr;
    if (PyArg_ParseTuple(args, "s", &LEFFileName))
    {
        std::ifstream LEFfile(LEFFileName);
        if (!LEFfile.good())
        {
            return PyErr_Format(PyExc_FileNotFoundError, "Cannot open file %s", LEFFileName);
        }

        auto designPtr = getDesign();
        if (designPtr == nullptr)
        {
            return PyErr_Format(PyExc_RuntimeError, "Unable to access design database");
        }

        if (ChipDB::LEF::Reader::load(*designPtr, LEFfile))
        {
            // Success!
            Py_RETURN_NONE;
        }
        
        // TODO: set exception / error.
        // load error
        return PyErr_Format(PyExc_RuntimeError, "Unable parse/load LEF file %s", LEFFileName);
    }    

    return PyErr_Format(PyExc_RuntimeError, "loadLef requires a filename argument");
}

///> loadLib(filename : string)
static PyObject* pyLoadLIB(PyObject *self, PyObject *args)
{
    const char *LIBFileName = nullptr;
    if (PyArg_ParseTuple(args, "s", &LIBFileName))
    {
        std::ifstream LIBfile(LIBFileName);
        if (!LIBfile.good())
        {            
            return PyErr_Format(PyExc_FileNotFoundError, "Cannot open file %s", LIBFileName);
        }

        auto designPtr = getDesign();
        if (designPtr == nullptr)
        {
            return PyErr_Format(PyExc_RuntimeError, "Unable to access design database");
        }

        if (ChipDB::Liberty::Reader::load(*designPtr, LIBfile))
        {
            // Success!
            Py_RETURN_NONE;
        }
        
        // load error
        return PyErr_Format(PyExc_RuntimeError, "Unable parse/load LIB file %s", LIBFileName);
    }    

    return PyErr_Format(PyExc_RuntimeError, "loadLib requires a filename argument");
}

///> loadVerilog(filename : string)
static PyObject* pyLoadVerilog(PyObject *self, PyObject *args)
{
    const char *VerilogFileName = nullptr;
    if (PyArg_ParseTuple(args, "s", &VerilogFileName))
    {
        std::ifstream Verilogfile(VerilogFileName);
        if (!Verilogfile.good())
        {
            return PyErr_Format(PyExc_FileNotFoundError, "Cannot open file %s", VerilogFileName);
        }

        auto designPtr = getDesign();
        if (designPtr == nullptr)
        {
            return PyErr_Format(PyExc_RuntimeError, "Unable to access design database");
        }

        if (ChipDB::Verilog::Reader::load(*designPtr, Verilogfile))
        {
            // Success!
            Py_RETURN_NONE;
        }
        
        // load error
        return PyErr_Format(PyExc_RuntimeError, "Unable parse/load verilog netlist file %s", VerilogFileName);
    }    

    return PyErr_Format(PyExc_RuntimeError, "loadVerilog requires a filename argument");
}

///> setTopModule(module name : string)
static PyObject* pySetTopModule(PyObject *self, PyObject *args)
{
    const char *topModuleName = nullptr;
    if (PyArg_ParseTuple(args, "s", &topModuleName))
    {

        auto designPtr = getDesign();
        if (designPtr == nullptr)
        {
            return PyErr_Format(PyExc_RuntimeError, "Unable to access design database");
        }

        if (!designPtr->setTopModule(topModuleName))
        {
            return PyErr_Format(PyExc_RuntimeError, "cannot set top module to %s", topModuleName);
        }

        // Success!
        Py_RETURN_NONE;
    }

    return PyErr_Format(PyExc_RuntimeError, "setTopModule requires a module name argument");
}

static PyObject* pyClear(PyObject *self, PyObject *args)
{
    auto designPtr = getDesign();
    if (designPtr == nullptr)
    {
        return PyErr_Format(PyExc_RuntimeError, "Unable to access design database");
    }

    designPtr->clear();

    // Success!
    Py_RETURN_NONE;
}

///> createRegion(regionname : string, x : integer, y : integer, width : integer, height : integer)
static PyObject* pyCreateRegion(PyObject *self, PyObject *args)
{
    auto designPtr = getDesign();
    if (designPtr == nullptr)
    {
        return PyErr_Format(PyExc_RuntimeError, "Unable to access design database");
    }

    if (!designPtr->m_floorplan)
    {
        return PyErr_Format(PyExc_RuntimeError, "Unable to access floorplan");
    }

    // check if the long int has the same range as ChipDB::CoordType
    // if this fails, we have to change the PyArg_ParseTuple format string
    static_assert(sizeof(long int) == sizeof(ChipDB::CoordType));

    long int x,y,width,height;
    const char *regionName;
    if (PyArg_ParseTuple(args, "sllll", &regionName, &x, &y, &width, &height))
    {        
        auto regionKeyObjPair = designPtr->m_floorplan->createRegion(regionName);
        if (!regionKeyObjPair.isValid())
        {
            return PyErr_Format(PyExc_RuntimeError, "Region with name %s already exists!", regionName);
        }

        regionKeyObjPair->m_rect = ChipDB::Rect64(ChipDB::Coord64{x,y}, ChipDB::Coord64{x+width,y+height});

        // Success!
        Py_RETURN_NONE;        
    }

    return PyErr_Format(PyExc_RuntimeError, "Wrong number or type of arguments");
}

///> createRows(regionname : string, startY : integer, rowHeight : integer, numberOfRows : integer)
static PyObject* pyCreateRows(PyObject *self, PyObject *args)
{
    auto designPtr = getDesign();
    if (designPtr == nullptr)
    {
        return PyErr_Format(PyExc_RuntimeError, "Unable to access design database");
    }

    if (!designPtr->m_floorplan)
    {
        return PyErr_Format(PyExc_RuntimeError, "Unable to access floorplan");
    }

    // check if the long int has the same range as ChipDB::CoordType
    // if this fails, we have to change the PyArg_ParseTuple format string
    static_assert(sizeof(long int) == sizeof(ChipDB::CoordType));

    long int starty,rowHeight,numRows;
    const char *regionName;
    if (PyArg_ParseTuple(args, "slll", &regionName, &starty, &rowHeight, &numRows))
    {        
        auto region = designPtr->m_floorplan->lookupRegion(regionName);
        if (!region.isValid())
        {
            return PyErr_Format(PyExc_RuntimeError, "Region with name %s does not exists!", regionName);
        }

        auto placeRect = region->getPlacementRect();

        ChipDB::Coord64 ll = placeRect.m_ll + ChipDB::Coord64{0,starty};
        ChipDB::Coord64 ur = ll + ChipDB::Coord64{placeRect.width(), rowHeight};
        size_t skippedRows = 0;
        for(int i=0; i<numRows; i++)
        {
            if (placeRect.contains(ll) && placeRect.contains(ur))
            {
                region->m_rows.emplace_back();
                auto& row = region->m_rows.back();

                row.m_region = region.ptr();
                row.m_rect = ChipDB::Rect64(ll,ur);
            }
            else
            {
                skippedRows++;
            };

            ll += ChipDB::Coord64{0, rowHeight};
            ur += ChipDB::Coord64{0, rowHeight};
        }

        designPtr->m_floorplan->contentsChanged();

        // Success!
        Py_RETURN_NONE;        
    }

    return PyErr_Format(PyExc_RuntimeError, "Wrong number or type of arguments");
}

///> place_module(modulename : string, regionname : string)
static PyObject* pyPlaceModule(PyObject *self, PyObject *args)
{
    auto designPtr = getDesign();
    if (designPtr == nullptr)
    {
        return PyErr_Format(PyExc_RuntimeError, "Unable to access design database");
    }

    if (!designPtr->m_floorplan)
    {
        return PyErr_Format(PyExc_RuntimeError, "Unable to access floorplan");
    }

    if (!designPtr->m_moduleLib)
    {
        return PyErr_Format(PyExc_RuntimeError, "Unable to access module library");
    }

    const char *moduleName;
    const char *regionName;
    if (PyArg_ParseTuple(args, "ss", &moduleName, &regionName))
    {        
        auto region = designPtr->m_floorplan->lookupRegion(regionName);
        if (!region.isValid())
        {
            return PyErr_Format(PyExc_RuntimeError, "Region with name %s does not exists!", regionName);
        }

        auto mod = designPtr->m_moduleLib->lookupModule(moduleName);
        if (!mod.isValid())
        {
            return PyErr_Format(PyExc_RuntimeError, "Could not find module with name %s!", moduleName);
        }

        auto ll = getLogLevel();
        setLogLevel(LOG_VERBOSE);

        if (!LunaCore::QLAPlacer::place(*region, *mod->m_netlist.get(), nullptr))
        {
            return PyErr_Format(PyExc_RuntimeError, "Placement failed!");
        }

        setLogLevel(ll);

        designPtr->m_floorplan->contentsChanged();

        // Success!
        Py_RETURN_NONE;        
    }

    return PyErr_Format(PyExc_RuntimeError, "Wrong number or type of arguments");
}

///> placeInstance(insname : string, module : string, x : integer, y : integer)
static PyObject* pyPlaceInstance(PyObject *self, PyObject *args)
{
    auto designPtr = getDesign();
    if (designPtr == nullptr)
    {
        return PyErr_Format(PyExc_RuntimeError, "Unable to access design database");
    }

    if (!designPtr->m_floorplan)
    {
        return PyErr_Format(PyExc_RuntimeError, "Unable to access floorplan");
    }

    if (!designPtr->m_moduleLib)
    {
        return PyErr_Format(PyExc_RuntimeError, "Unable to access module library");
    }

    // check if the long int has the same range as ChipDB::CoordType
    // if this fails, we have to change the PyArg_ParseTuple format string
    static_assert(sizeof(long int) == sizeof(ChipDB::CoordType));

    const char *insName;
    const char *modName;
    long int x,y;
    if (PyArg_ParseTuple(args, "ssll", &insName, &modName, &x, &y))
    {        
        auto mod = designPtr->m_moduleLib->lookupModule(modName);
        if (!mod.isValid())
        {
            return PyErr_Format(PyExc_RuntimeError, "Could not find module with name %s!", modName);
        }

        if (!mod->m_netlist)
        {
            return PyErr_Format(PyExc_RuntimeError, "Module %s has no instances!", modName);
        }

        auto ins = mod->m_netlist->lookupInstance(insName);
        if (!ins.isValid())
        {
            return PyErr_Format(PyExc_RuntimeError, "Could not find instance with name %s!", insName);
        }

        ins->m_pos.m_x = x;
        ins->m_pos.m_y = y;
        ins->m_placementInfo = ChipDB::PlacementInfo::PLACEDANDFIXED;

        designPtr->m_floorplan->contentsChanged();

        // Success!
        Py_RETURN_NONE;
    }

    return PyErr_Format(PyExc_RuntimeError, "Wrong number or type of arguments");
}

static PyMethodDef LunaMethods[] =  // NOLINT(modernize-avoid-c-arrays)
{
    {"clear", pyClear, METH_NOARGS, "clear the design database"},
    {"loadLef", pyLoadLEF, METH_VARARGS, "load LEF file"},
    {"loadLib", pyLoadLIB, METH_VARARGS, "load Liberty file"},
    {"loadVerilog", pyLoadVerilog, METH_VARARGS, "load verlog netlist file"},
    {"setTopModule", pySetTopModule, METH_VARARGS, "set the top level module"},
    {"createRegion", pyCreateRegion, METH_VARARGS, "create a region on the floorplan"},
    {"createRows", pyCreateRows, METH_VARARGS, "create a cell rows in a region"},
    {"placeModule", pyPlaceModule, METH_VARARGS, "place a module in a region"},
    {"placeInstance", pyPlaceInstance, METH_VARARGS, "place / set the position of an instance"},
    {nullptr}
};

static PyModuleDef LunaModule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "Luna",               // NOLINT(clang-diagnostic-c99-designator)
    .m_doc = "Interfaces with LunaPNR core datastructures",
    .m_size = -1,
    .m_methods = LunaMethods
};

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

static PyObject* PyInit_Luna()
{
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

void Scripting::Python::init()
{
    PyImport_AppendInittab("ConsoleRedirect", &PyInit_ConsoleRedirect);
    PyImport_AppendInittab("Luna", &PyInit_Luna);

    preInitHook();

    Py_Initialize();

    PyImport_ImportModule("ConsoleRedirect");
    auto lunaModule = PyImport_ImportModule("Luna");

    if (lunaModule == nullptr)
    {
        std::cout << "Luna module init failed!\n";
    }

    auto capsule  = PyCapsule_New(m_design->m_cellLib.get(), "Luna.CellLibraryPtr", nullptr);
    
    if (PyModule_AddObject(lunaModule, "CellLibraryPtr", capsule) < 0)
    {
        std::cout << "PyModule_AddObject failed!\n";
        Py_XDECREF(capsule);
    }

    auto capsule2 = PyCapsule_New(m_design, "Luna.DesignPtr", nullptr);

    if (PyModule_AddObject(lunaModule, "DesignPtr", capsule2) < 0)
    {
        std::cout << "PyModule_AddObject failed!\n";
        Py_XDECREF(capsule);
    }

    postInitHook();
}

Scripting::Python::Python(ChipDB::Design *design) : m_design(design)
{
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

#if 0
void Scripting::Python::addModule(const char *moduleName, 
    ModuleInitFunctionType initFunction)
{
    PyImport_AddModule(moduleName);
    PyObject* pyModule = initFunction();
    PyObject* sys_modules = PyImport_GetModuleDict();
    PyDict_SetItemString(sys_modules, moduleName, pyModule);
    Py_DECREF(pyModule);
}    

bool Scripting::Python::createCapsule(PyModuleDef *moduleDef,
    const char *extendedName,
    const char *capsuleName,
    void *ptr)
{
    auto pyModule = PyState_FindModule(moduleDef);
    if (pyModule == nullptr)
    {
        return false;
    }

    auto capsule = PyCapsule_New(ptr, extendedName, nullptr);

    if (PyModule_AddObject(pyModule, capsuleName, capsule) < 0)
    {
        Py_XDECREF(capsule);
        return false;
    }

    return true;
}
#endif
