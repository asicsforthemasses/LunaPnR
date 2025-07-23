// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
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
#include "pysiteinfo.h"
#include "database/database.h"

struct PyTechLibSiteIterator
{
    ChipDB::TechLib* m_techLib; // we do not manage the memory for this
    ChipDB::NamedStorage<ChipDB::SiteInfo>::iterator m_iter;

    auto end()
    {
        return m_techLib->sites().end();
    }

    auto begin()
    {
        return m_techLib->sites().begin();
    }
};

/** container for LunaCore::LayerInfo */
struct PyTechLibSites : public Python::TypeTemplate<PyTechLibSiteIterator>
{
    /** set internal values of PyCell */
    static int pyInit(PyTechLibSites *self, PyObject *args, PyObject *kwds)
    {
        //std::cout << "PyTechLibSites::Init\n";

        // do not use ok() here, as it checks for
        // m_holder to be != nullptr.
        if (self->m_holder != nullptr)
        {
            //std::cout << "  Shared pointer created\n";

            self->m_holder->reset(new PyTechLibSiteIterator());

            // get a pointer to
            auto techLibPtr = reinterpret_cast<ChipDB::TechLib*>(PyCapsule_Import("Luna.TechLibraryPtr", 0));
            if (techLibPtr == nullptr)
            {
                std::cout << "UGH Capsule is nullptr!\n";
            }

            self->obj()->m_techLib = techLibPtr;
            self->obj()->m_iter = self->obj()->end();
        }
        else
        {
            self->m_holder = nullptr;
        }

        return 0;   /* success */
    };

    static PyObject* siteCount(PyTechLibSites *self, PyObject *args)
    {
        if (self->ok())
        {
            if (!self->obj()->m_techLib)
            {
                PyErr_Format(PyExc_RuntimeError, "TechLib is uninitialized");
                return nullptr;
            }
            return Py_BuildValue("l", self->obj()->m_techLib->getNumberOfSites());
        }

        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return nullptr;
    }

    static PyObject* getSite(PyTechLibSites *self, PyObject *args)
    {
        if (self->ok())
        {
            if (!self->obj()->m_techLib)
            {
                PyErr_Format(PyExc_RuntimeError, "TechLib is uninitialized");
                return nullptr;
            }

            ChipDB::ObjectKey key = -1;
            if (PyArg_ParseTuple(args,"l", &key))
            {
                auto siteinfo = self->obj()->m_techLib->lookupSiteInfo(key);
                if (!siteinfo)
                {
                    PyErr_Format(PyExc_ValueError, "Site with key %ld not found", key);
                    return nullptr;
                }
                return Python::toPython(siteinfo);
            }

            // clear previous PyArg_ParseTuple exception
            PyErr_Clear();

            const char *siteName = nullptr;
            if (PyArg_ParseTuple(args,"s", &siteName))
            {
                auto siteinfo = self->obj()->m_techLib->lookupSiteInfo(siteName);
                if (!siteinfo.isValid())
                {
                    PyErr_Format(PyExc_ValueError, "Site %s not found", siteName);
                    return nullptr;
                }

                return Python::toPython(siteinfo.ptr());
            }

            PyErr_Format(PyExc_ValueError, "getLayer requires a key or name as argument");
            return nullptr;
        }

        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return nullptr;
    }

    static PyObject* pyIter(PyTechLibSites *self)
    {
        //std::cout << "PyTechLib::Iter\n";

        if (self->ok())
        {
            self->obj()->m_iter = self->obj()->begin();
            Py_INCREF(self);
            return (PyObject*)self;
        }

        return nullptr;
    };

    static PyObject* pyIterNext(PyTechLibSites *self)
    {
        //std::cout << "PyTechLibSites::IterNext\n";

        if (self->ok())
        {
            if (self->obj()->m_iter == self->obj()->end())
            {
                return nullptr; // no more object, stop iteration
            }

            auto kvpair = *self->obj()->m_iter;
            PyObject *layerObject = Python::toPython(kvpair.ptr());
            //Py_INCREF(cellObject);

            self->obj()->m_iter++;
            return (PyObject*)layerObject;
        }

        return nullptr;
    };

    /** set internal values of PyCell */
    static PyObject* pyStr(PyObject *self)
    {
        return Python::toPython(PyTechLibSites::PythonObjectName);
    };

    static constexpr const char *PythonObjectName = "TechLibSites";
    static constexpr const char *PythonObjectDoc  = "Technology library sites object";
};

// cppcheck-suppress "suppressed_error_id"
static PyMemberDef PyTechLibSitesMembers[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}  /* Sentinel */
};

static PyGetSetDef PyTechLibSitesGetSet[] =     // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}
};

static PyMethodDef PyTechLibSitesMethods[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {"getSite", (PyCFunction)PyTechLibSites::getSite, METH_VARARGS, "Lookup and return a technology site (by name or by key)"},
    {"siteCount", (PyCFunction)PyTechLibSites::siteCount, METH_VARARGS, "Returns the number of sites"},
    {nullptr}  /* Sentinel */
};

PyTypeObject PyTechLibSitesType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    PyTechLibSites::PythonObjectName,       /* tp_name */
    sizeof(PyTechLibSites),                 /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)PyTechLibSites::pyDeAlloc,  /* tp_dealloc */
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
    PyTechLibSites::pyStr,                  /* tp_str */
    nullptr,                        /* tp_getattro */
    nullptr,                        /* tp_setattro */
    nullptr,                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,        /* tp_flags */
    PyTechLibSites::PythonObjectDoc,        /* tp_doc */
    nullptr,                        /* tp_traverse */
    nullptr,                        /* tp_clear */
    nullptr,                        /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    (getiterfunc)PyTechLibSites::pyIter,         /* tp_iter */
    (iternextfunc)PyTechLibSites::pyIterNext,    /* tp_iternext */
    PyTechLibSitesMethods,                  /* tp_methods */
    PyTechLibSitesMembers,                  /* tp_members */
    PyTechLibSitesGetSet,                   /* tp_getset */
    nullptr,                        /* tp_base */
    nullptr,                        /* tp_dict */
    nullptr,                        /* tp_descr_get */
    nullptr,                        /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)PyTechLibSites::pyInit,       /* tp_init */
    nullptr,                        /* tp_alloc */
    PyTechLibSites::pyNewCall,
};
