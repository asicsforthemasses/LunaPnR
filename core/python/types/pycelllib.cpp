// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include <memory>
#include <array>
#include <iostream>

#include "../converters.h"
#include "typetemplate.h"
#include "pycell.h"
#include "celllib/celllib.h"

struct PyCellLibIterator
{
    ChipDB::CellLib* m_cellLib; // we do not manage the memory for this
    ChipDB::NamedStorage<ChipDB::Cell>::iterator m_iter;

    auto end()
    {
        return m_cellLib->end();
    }

    auto begin()
    {
        return m_cellLib->begin();
    }    
};

/** container for LunaCore::Cell */
struct PyCellLib : public Python::TypeTemplate<PyCellLibIterator>
{
    /** set internal values of PyCell */
    static int pyInit(PyCellLib *self, PyObject *args, PyObject *kwds)
    {
        //std::cout << "PyCellLib::Init\n";

        // do not use ok() here, as it checks for
        // m_holder to be != nullptr.
        if (self->m_holder != nullptr)
        {
            //std::cout << "  Shared pointer created\n";

            self->m_holder->reset(new PyCellLibIterator());
            self->obj()->m_iter = self->obj()->end();

            // get a pointer to 
            auto cellLibPtr = reinterpret_cast<ChipDB::CellLib*>(PyCapsule_Import("Luna.CellLibraryPtr", 0));
            if (cellLibPtr == nullptr)
            {
                std::cout << "UGH Capsule is nullptr!\n";
            }

            self->obj()->m_cellLib = cellLibPtr;
        }
        else
        {
            self->m_holder = nullptr;
        }

        return 0;   /* success */
    };

    static PyObject* getCell(PyCellLib *self, PyObject *args)
    {
        if (self->ok())
        {
            if (!self->obj()->m_cellLib)
            {
                PyErr_Format(PyExc_RuntimeError, "CellLib is uninitialized");
                return nullptr;
            }

            ChipDB::ObjectKey key = -1;
            if (PyArg_ParseTuple(args,"l", &key))
            {
                auto ins = self->obj()->m_cellLib->lookupCell(key);
                if (!ins)
                {
                    PyErr_Format(PyExc_ValueError, "Cell with key %ld not found", key);
                    return nullptr;                    
                }                
                return Python::toPython(ins);
            }

            // clear previous PyArg_ParseTuple exception
            PyErr_Clear();

            const char *cellName = nullptr;
            if (PyArg_ParseTuple(args,"s", &cellName))
            {
                auto ins = self->obj()->m_cellLib->lookupCell(cellName);
                if (!ins.isValid())
                {
                    PyErr_Format(PyExc_ValueError, "Cell %s not found", cellName);
                    return nullptr;
                }

                return Python::toPython(ins.ptr());
            }

            PyErr_Format(PyExc_ValueError, "getCell requires a key or name as argument");
            return nullptr;
        }
        
        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");        
        return nullptr;                 
    }

    static PyObject* pyIter(PyCellLib *self)
    {
        //std::cout << "PyCellLib::Iter\n";

        if (self->ok())
        {
            self->obj()->m_iter = self->obj()->begin();
            Py_INCREF(self);
            return (PyObject*)self;
        }

        return nullptr;
    };

    static PyObject* pyIterNext(PyCellLib *self)
    {
        //std::cout << "PyCellLib::IterNext\n";

        if (self->ok())
        {
            if (self->obj()->m_iter == self->obj()->end())
            {
                return nullptr; // no more object, stop iteration
            }

            auto kvpair = *self->obj()->m_iter;
            PyObject *cellObject = Python::toPython(kvpair.ptr());
            //Py_INCREF(cellObject);

            self->obj()->m_iter++;
            return (PyObject*)cellObject;
        }

        return nullptr;
    };

    /** set internal values of PyCell */
    static PyObject* pyStr(PyObject *self)
    {
        return Python::toPython(PyCellLib::PythonObjectName);
    };

    static constexpr const char *PythonObjectName = "CellLib";
    static constexpr const char *PythonObjectDoc  = "Cell library object";
};

// cppcheck-suppress "suppressed_error_id"
static PyMemberDef PyCellMembers[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}  /* Sentinel */
};

static PyGetSetDef PyCellGetSet[] =     // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}
};

static PyMethodDef PyCellMethods[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {"getCell", (PyCFunction)PyCellLib::getCell, METH_VARARGS, "Lookup and return a cell (by name or by key)"},
    {nullptr}  /* Sentinel */
};

PyTypeObject PyCellLibType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    PyCellLib::PythonObjectName,       /* tp_name */
    sizeof(PyCellLib),                 /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)PyCellLib::pyDeAlloc,  /* tp_dealloc */
    0,                              /* tp_print */
    nullptr,                        /* tp_getattr */
    nullptr,                        /* tp_setattr */
    nullptr,                        /* tp_reserved */
    nullptr,                        /* tp_repr */
    nullptr,                        /* tp_as_number */
    nullptr,                        /* tp_as_sequence */
    nullptr,                        /* tp_as_mapping */
    nullptr,                        /* tp_hash  */
    nullptr,                        /* tp_call */
    PyCellLib::pyStr,                  /* tp_str */
    nullptr,                        /* tp_getattro */
    nullptr,                        /* tp_setattro */
    nullptr,                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,        /* tp_flags */
    PyCellLib::PythonObjectDoc,        /* tp_doc */
    nullptr,                        /* tp_traverse */
    nullptr,                        /* tp_clear */
    nullptr,                        /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    (getiterfunc)PyCellLib::pyIter,         /* tp_iter */
    (iternextfunc)PyCellLib::pyIterNext,    /* tp_iternext */
    PyCellMethods,                  /* tp_methods */
    PyCellMembers,                  /* tp_members */
    PyCellGetSet,                   /* tp_getset */
    nullptr,                        /* tp_base */
    nullptr,                        /* tp_dict */
    nullptr,                        /* tp_descr_get */
    nullptr,                        /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)PyCellLib::pyInit,       /* tp_init */
    nullptr,                        /* tp_alloc */
    PyCellLib::pyNewCall,
};
