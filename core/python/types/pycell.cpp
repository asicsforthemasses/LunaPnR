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
#include "pypininfolist.h"

/** container for LunaCore::Cell */
struct PyCell : public Python::TypeTemplate<ChipDB::Cell>
{
    static PyObject* getName(PyCell *self, void *closure)
    {
        //std::cout << "PyCell::getName\n";
        if (self->ok())
        {
            return Python::toPython(self->obj()->name());
        }
        
        return nullptr;
    };

    static PyObject* getLeakagePower(PyCell *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->m_leakagePower);
        }
        
        return nullptr;        
    };

    static PyObject* getArea(PyCell *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->m_area);
        }
        
        return nullptr;        
    };

    static PyObject* getSize(PyCell *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->m_size);
        }
        
        return nullptr;        
    };

    static PyObject* getOffset(PyCell *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->m_offset);
        }
        
        return nullptr;        
    };

    static PyObject* getSite(PyCell *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->m_site);
        }
        
        return nullptr;        
    };

    static PyObject* getClass(PyCell *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->m_class);
        }
        
        return nullptr;        
    };

    static PyObject* getSubClass(PyCell *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->m_subclass);
        }
        
        return nullptr;        
    };

    static PyObject* getSymmetry(PyCell *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->m_symmetry);
        }
        
        return nullptr;
    };


    static PyObject* getPins(PyCell *self, void *closure)
    {
        if (self->ok())
        {
            auto pinInfoList = Python::toPython( &(self->obj()->m_pins));
            return pinInfoList;
        }
        
        return nullptr;
    }

    /** set internal values of PyCell */
    static int pyInit(PyCell *self, PyObject *args, PyObject *kwds)
    {
        //std::cout << "pyInit\n";

        // do not use ok() here, as it checks for
        // m_holder to be != nullptr.
        if (self->m_holder != nullptr)
        {
            //std::cout << "  Shared pointer created\n";
            //self->m_holder->reset(new MyCell());
            //self->obj()->name = "Niels";
            //self->obj()->m_number = 123;
        }
        else
        {
            self->m_holder = nullptr;
        }

        return 0;   /* success */
    };

    /** set internal values of PyCell */
    static PyObject* pyStr(PyObject *self)
    {
        //std::cout << "pyStr\n";
        return Python::toPython(PyCell::PythonObjectName);
    };

    static constexpr const char *PythonObjectName = "Cell";
    static constexpr const char *PythonObjectDoc  = "Cell object";
};

// cppcheck-suppress "suppressed_error_id"
static PyMemberDef PyCellMembers[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}  /* Sentinel */
};

static PyGetSetDef PyCellGetSet[] =     // NOLINT(modernize-avoid-c-arrays)
{
    {"name", (getter)PyCell::getName, nullptr, "", nullptr /* closure */},
    {"leakagePower", (getter)PyCell::getLeakagePower, nullptr, "cell leakage power in Watts", nullptr /* closure */},
    {"area", (getter)PyCell::getArea, nullptr, "cell area in um^2", nullptr /* closure */},
    {"size", (getter)PyCell::getSize, nullptr, "cell size in nm", nullptr /* closure */},
    {"offset", (getter)PyCell::getOffset, nullptr, "cell offset in nm", nullptr /* closure */},
    {"site", (getter)PyCell::getSite, nullptr, "cell site name", nullptr /* closure */},
    {"cellClass", (getter)PyCell::getClass, nullptr, "cell class", nullptr /* closure */},
    {"cellSubClass", (getter)PyCell::getSubClass, nullptr, "cell subclass", nullptr /* closure */},
    {"symmetry", (getter)PyCell::getSymmetry, nullptr, "cell symmetry flags", nullptr /* closure */},
    {"pins", (getter)PyCell::getPins, nullptr, "cell pin list", nullptr /* closure */},
    {nullptr}
};


static PyMethodDef PyCellMethods[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}  /* Sentinel */
};

PyTypeObject PyCellType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    PyCell::PythonObjectName,       /* tp_name */
    sizeof(PyCell),                 /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)PyCell::pyDeAlloc,  /* tp_dealloc */
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
    PyCell::pyStr,                  /* tp_str */
    nullptr,                        /* tp_getattro */
    nullptr,                        /* tp_setattro */
    nullptr,                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,        /* tp_flags */
    PyCell::PythonObjectDoc,        /* tp_doc */
    nullptr,                        /* tp_traverse */
    nullptr,                        /* tp_clear */
    nullptr,                        /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    nullptr,                        /* tp_iter */
    nullptr,                        /* tp_iternext */
    PyCellMethods,                  /* tp_methods */
    PyCellMembers,                  /* tp_members */
    PyCellGetSet,                   /* tp_getset */
    nullptr,                        /* tp_base */
    nullptr,                        /* tp_dict */
    nullptr,                        /* tp_descr_get */
    nullptr,                        /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)PyCell::pyInit,       /* tp_init */
    nullptr,                        /* tp_alloc */
    PyCell::pyNewCall
};

PyObject* Python::toPython(std::shared_ptr<ChipDB::Cell> cellPtr)
{
    // create a new PyCell oject
    auto cellObject = reinterpret_cast<PyCell*>(PyObject_CallObject((PyObject*)&PyCellType, nullptr));
    if (cellObject->m_holder != nullptr)
    {
        *cellObject->m_holder = cellPtr;
        return (PyObject*)cellObject;
    }
    return nullptr;
};
