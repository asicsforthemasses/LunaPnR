#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include <memory>
#include <array>

#include "../converters.h"
#include "celllib/cell.h"

/** container for LunaCore::Cell */
struct PyCell
{
    std::shared_ptr<ChipDB::Cell> m_cell;

    static PyObject* name(PyCell *self)
    {
        return Python::toPython(self->m_cell->name());
    };

    /** allocate memory for the PyCell */
    static PyObject* pyNewCall(PyTypeObject *type, PyObject *args, PyObject *kwds)
    {
        auto *self = (PyCell*)type->tp_alloc(type, 0);
        if (self != nullptr)
        {
            // placement new - does not allocate
            self = new (self) PyCell();
        }

        return (PyObject*)self;
    }

    static void pyDeAlloc(PyCell *self)
    {
        /* add additional memory free operations here, if necessary */
        
        self->m_cell.reset();
        Py_TYPE(self)->tp_free((PyObject*)self);
    }

    /** set internal values of PyCell */
    static int pyInit(PyCell *self)
    {
        return 0;   /* success */
    };

    static constexpr const char *PythonObjectName = "Luna.Cell";
    static constexpr const char *PythonObjectDoc  = "Luna.Cell object";
};

// cppcheck-suppress "suppressed_error_id"
static PyMemberDef PyCellMembers[] = 
{/*
    {"first", T_OBJECT_EX, offsetof(Noddy, first), 0,
    "first name"},
    {"last", T_OBJECT_EX, offsetof(Noddy, last), 0,
    "last name"},
    {"number", T_INT, offsetof(Noddy, number), 0,
    "noddy number"},
*/
    {nullptr}  /* Sentinel */
};

static PyMethodDef PyCellMethods[] = 
{
    {"name", (PyCFunction)PyCell::name, METH_NOARGS, "Return the cell name"},
    {nullptr}  /* Sentinel */
};

PyTypeObject PyCellType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    PyCell::PythonObjectName,  /* tp_name */
    sizeof(PyCell),            /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)PyCell::pyDeAlloc, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,   /* tp_flags */
    PyCell::PythonObjectDoc,   /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyCellMethods,             /* tp_methods */
    PyCellMembers,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyCell::pyInit,  /* tp_init */
    0,                         /* tp_alloc */
    PyCell::pyNewCall,         /* tp_new */
};
