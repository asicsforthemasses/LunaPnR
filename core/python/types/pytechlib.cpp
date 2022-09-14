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
#include "pylayerinfo.h"
#include "techlib/techlib.h"

struct PyTechLibLayerIterator
{
    ChipDB::TechLib* m_techLib; // we do not manage the memory for this
    ChipDB::NamedStorage<ChipDB::LayerInfo>::iterator m_iter;

    auto end()
    {
        return m_techLib->layers().end();
    }

    auto begin()
    {
        return m_techLib->layers().begin();
    }    
};

/** container for LunaCore::LayerInfo */
struct PyTechLibLayers : public Python::TypeTemplate<PyTechLibLayerIterator>
{
    /** set internal values of PyCell */
    static int pyInit(PyTechLibLayers *self, PyObject *args, PyObject *kwds)
    {
        std::cout << "PyTechLibLayers::Init\n";

        // do not use ok() here, as it checks for
        // m_holder to be != nullptr.
        if (self->m_holder != nullptr)
        {
            //std::cout << "  Shared pointer created\n";

            self->m_holder->reset(new PyTechLibLayerIterator());

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

    static PyObject* layerCount(PyTechLibLayers *self, PyObject *args)
    {
        if (self->ok())
        {
            if (!self->obj()->m_techLib)
            {
                PyErr_Format(PyExc_RuntimeError, "TechLib is uninitialized");
                return nullptr;
            }
            return Py_BuildValue("l", self->obj()->m_techLib->getNumberOfLayers());
        }

        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");        
        return nullptr;              
    }

    static PyObject* getLayer(PyTechLibLayers *self, PyObject *args)
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
                auto layer = self->obj()->m_techLib->lookupLayer(key);
                if (!layer)
                {
                    PyErr_Format(PyExc_ValueError, "Layer with key %ld not found", key);
                    return nullptr;                    
                }                
                return Python::toPython(layer);
            }

            // clear previous PyArg_ParseTuple exception
            PyErr_Clear();

            const char *layerName = nullptr;
            if (PyArg_ParseTuple(args,"s", &layerName))
            {
                auto layer = self->obj()->m_techLib->lookupLayer(layerName);
                if (!layer.isValid())
                {
                    PyErr_Format(PyExc_ValueError, "Layer %s not found", layerName);
                    return nullptr;
                }

                return Python::toPython(layer.ptr());
            }

            PyErr_Format(PyExc_ValueError, "getLayer requires a key or name as argument");
            return nullptr;
        }
        
        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");        
        return nullptr;                 
    }

    static PyObject* pyIter(PyTechLibLayers *self)
    {
        std::cout << "PyTechLib::Iter\n";

        if (self->ok())
        {
            self->obj()->m_iter = self->obj()->begin();
            Py_INCREF(self);
            return (PyObject*)self;
        }

        return nullptr;
    };

    static PyObject* pyIterNext(PyTechLibLayers *self)
    {
        std::cout << "PyTechLibLayers::IterNext\n";

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
        return Python::toPython(PyTechLibLayers::PythonObjectName);
    };

    static constexpr const char *PythonObjectName = "TechLibLayers";
    static constexpr const char *PythonObjectDoc  = "Technology library layers object";
};

// cppcheck-suppress "suppressed_error_id"
static PyMemberDef PyTechLibLayersMembers[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}  /* Sentinel */
};

static PyGetSetDef PyTechLibLayersGetSet[] =     // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}
};

static PyMethodDef PyTechLibLayersMethods[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {"getLayer", (PyCFunction)PyTechLibLayers::getLayer, METH_VARARGS, "Lookup and return a technology layer (by name or by key)"},
    {"layerCount", (PyCFunction)PyTechLibLayers::layerCount, METH_VARARGS, "Returns the number of layers"},
    {nullptr}  /* Sentinel */
};

PyTypeObject PyTechLibLayersType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    PyTechLibLayers::PythonObjectName,       /* tp_name */
    sizeof(PyTechLibLayers),                 /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)PyTechLibLayers::pyDeAlloc,  /* tp_dealloc */
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
    PyTechLibLayers::pyStr,                  /* tp_str */
    nullptr,                        /* tp_getattro */
    nullptr,                        /* tp_setattro */
    nullptr,                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,        /* tp_flags */
    PyTechLibLayers::PythonObjectDoc,        /* tp_doc */
    nullptr,                        /* tp_traverse */
    nullptr,                        /* tp_clear */
    nullptr,                        /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    (getiterfunc)PyTechLibLayers::pyIter,         /* tp_iter */
    (iternextfunc)PyTechLibLayers::pyIterNext,    /* tp_iternext */
    PyTechLibLayersMethods,                  /* tp_methods */
    PyTechLibLayersMembers,                  /* tp_members */
    PyTechLibLayersGetSet,                   /* tp_getset */
    nullptr,                        /* tp_base */
    nullptr,                        /* tp_dict */
    nullptr,                        /* tp_descr_get */
    nullptr,                        /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)PyTechLibLayers::pyInit,       /* tp_init */
    nullptr,                        /* tp_alloc */
    PyTechLibLayers::pyNewCall,
};
