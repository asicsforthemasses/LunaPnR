// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
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
#include "celllib/cell.h"
#include "celllib/pin.h"
#include "pypininfo.h"
#include "pycell.h"
#include "pypininfolist.h"

struct PyPinInfoListIterator
{
    ChipDB::PinInfoList *m_pinInfoList = nullptr;
    ChipDB::PinInfoList::iterator m_iter;

    ChipDB::PinInfoList::iterator end()
    {
        if (m_pinInfoList)
        {
            return m_pinInfoList->end();
        }
        else
        {
            return ChipDB::PinInfoList::iterator();
        }
    }

    ChipDB::PinInfoList::iterator begin()
    {
        if (m_pinInfoList)
        {
            return m_pinInfoList->begin();
        }
        else
        {
            return ChipDB::PinInfoList::iterator();
        }        
    }    
};

/** container for LunaCore::Cell */
struct PyPinInfoList : public Python::TypeTemplate<PyPinInfoListIterator>
{
    /** set internal values of PyCell 
    */
    static int pyInit(PyPinInfoList *self, PyObject *args, PyObject *kwds)
    {
        //std::cout << "PyPinInfoList::Init\n";

        // do not use ok() here, as it checks for
        // m_holder to be != nullptr.
        if (self->m_holder == nullptr)
        {
            //std::cout << "  Shared pointer created\n";
            return -1; /* error */
        }

        self->m_holder->reset(new PyPinInfoListIterator());
        return 0;   /* success */
    };

    static PyObject* pyIter(PyPinInfoList *self)
    {
        //std::cout << "PyPinInfoList::Iter\n";

        if (self->ok())
        {
            self->obj()->m_iter = self->obj()->begin();
            Py_INCREF(self);
            return (PyObject*)self;
        }

        return nullptr;
    };

    static PyObject* pyIterNext(PyPinInfoList *self)
    {
        //std::cout << "PyPinInfoList::IterNext\n";

        if (self->ok())
        {
            if (self->obj()->m_iter == self->obj()->end())
            {
                return nullptr; // no more object, stop iteration
            }

            auto ptr = *self->obj()->m_iter;
            PyObject *cellObject = Python::toPython(ptr);
            //Py_INCREF(cellObject);

            self->obj()->m_iter++;
            return (PyObject*)cellObject;
        }

        return nullptr;
    };

    /** set internal values of PyCell */
    static PyObject* pyStr(PyObject *self)
    {
        return Python::toPython(PyPinInfoList::PythonObjectName);
    };

    static constexpr const char *PythonObjectName = "PinInfoList";
    static constexpr const char *PythonObjectDoc  = "PinInfoList object";
};

// cppcheck-suppress "suppressed_error_id"
static PyMemberDef PyPinInfoListMembers[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}  /* Sentinel */
};

static PyGetSetDef PyPinInfoListGetSet[] =     // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}
};

static PyMethodDef PyPinInfoListMethods[] =    // NOLINT(modernize-avoid-c-arrays)
{
//    {"name", (PyCFunction)PyCell::name, METH_NOARGS, "Return the cell name"},
    {nullptr}  /* Sentinel */
};

PyTypeObject PyPinInfoListType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    PyPinInfoList::PythonObjectName,       /* tp_name */
    sizeof(PyPinInfoList),                 /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)PyPinInfoList::pyDeAlloc,  /* tp_dealloc */
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
    PyPinInfoList::pyStr,                  /* tp_str */
    nullptr,                        /* tp_getattro */
    nullptr,                        /* tp_setattro */
    nullptr,                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,        /* tp_flags */
    PyPinInfoList::PythonObjectDoc,        /* tp_doc */
    nullptr,                        /* tp_traverse */
    nullptr,                        /* tp_clear */
    nullptr,                        /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    (getiterfunc)PyPinInfoList::pyIter,         /* tp_iter */
    (iternextfunc)PyPinInfoList::pyIterNext,    /* tp_iternext */
    PyPinInfoListMethods,                  /* tp_methods */
    PyPinInfoListMembers,                  /* tp_members */
    PyPinInfoListGetSet,                   /* tp_getset */
    nullptr,                        /* tp_base */
    nullptr,                        /* tp_dict */
    nullptr,                        /* tp_descr_get */
    nullptr,                        /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)PyPinInfoList::pyInit,       /* tp_init */
    nullptr,                        /* tp_alloc */
    PyPinInfoList::pyNewCall
};

PyObject* Python::toPython(ChipDB::PinInfoList *pinInfoListPtr)
{
    // create a new PyPinInfoList oject

    auto pinInfoListObject = reinterpret_cast<PyPinInfoList*>(PyObject_CallObject((PyObject*)&PyPinInfoListType, nullptr));
    if (pinInfoListObject->ok())
    {
        pinInfoListObject->obj()->m_pinInfoList = pinInfoListPtr;
        return (PyObject*)pinInfoListObject;
    }
    return nullptr;    
}
