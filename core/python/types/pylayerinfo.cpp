// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
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

/** container for LunaCore::Cell */
struct PyLayerInfo : public Python::TypeTemplate<ChipDB::LayerInfo>
{
    static PyObject* getName(PyLayerInfo *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->name());
        }
        
        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return nullptr;
    };

#if 0
    static PyObject* getPosition(PyLayerInfo *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->m_pos);
        }

        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return nullptr;
    }

    static int setPosition(PyInstance *self, PyObject *value, void *closure)
    {
        if (self->ok())
        {
            if (!PyArg_ParseTuple(value, "LL", &self->obj()->m_pos.m_x, &self->obj()->m_pos.m_y))
            {
                PyErr_Format(PyExc_ValueError, "x,y value required");
                return -1;
            }
            return 0; // success
        }

        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return -1;
    }

    static PyObject* getPlacementInfo(PyInstance *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(ChipDB::toString(self->obj()->m_placementInfo));
        }

        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return nullptr;
    }

    static int setPlacementInfo(PyInstance *self, PyObject *value, void *closure)
    {
        if (self->ok())
        {
            const char *placementString = PyUnicode_AsUTF8(value);            

            std::cout << "  " << placementString << "\n";

            if (placementString == nullptr)
            {
                PyErr_Format(PyExc_ValueError, "placementInfo requires a string");
                return -1; /* failure */
            }

            if (!ChipDB::fromString(placementString, self->obj()->m_placementInfo))
            {
                PyErr_Format(PyExc_ValueError, "Placement info: unknown option %s", placementString);
                Py_XDECREF(placementString);
                return -1; /* failure */
            }

            //Py_XDECREF(placementString);
            return 0; /* success */
        }

        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return -1;
    }

    static PyObject* getOrientation(PyInstance *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(ChipDB::toString(self->obj()->m_orientation));
        }

        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return nullptr;
    }

    static PyObject* getSize(PyInstance *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->instanceSize());
        }
        
        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");        
        return nullptr;        
    };

    static PyObject* getArea(PyInstance *self, void *closure)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->getArea());
        }
        
        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");        
        return nullptr;        
    };

    static PyObject* getPinCount(PyInstance *self, PyObject *args)
    {
        if (self->ok())
        {
            return Python::toPython(self->obj()->getNumberOfPins());
        }
        
        return nullptr;                
    }

    static PyObject* getPin(PyInstance *self, PyObject *args)
    {
        if (self->ok())
        {
            ChipDB::PinObjectKey key;
            if (PyArg_ParseTuple(args,"i", &key))
            {
                auto pin = self->obj()->getPin(key);
                //std::cout << "\t\tgetPin: of ins" << self->obj()->name() << " PinName:" << pin.name() << " PinKey:" << pin.m_pinKey << " NetKey:" << pin.m_netKey << "\n";
                return Python::toPython(self->obj()->getPin(key));
            }

            // clear previous PyArg_ParseTuple exception
            PyErr_Clear();

            const char *pinName = nullptr;
            if (PyArg_ParseTuple(args,"s", &pinName))
            {
                auto pin = self->obj()->getPin(pinName);
                //std::cout << "\t\tgetPin: of ins" << self->obj()->name() << " PinName:" << pin.name() << " PinKey:" << pin.m_pinKey << " NetKey:" << pin.m_netKey << "\n";
                return Python::toPython(self->obj()->getPin(pinName));                
            }

            PyErr_Format(PyExc_ValueError, "getPin requires a key or name as argument");
            return nullptr;
        }
        
        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");        
        return nullptr;                
    }

    static PyObject* setPinNet(PyInstance *self, PyObject *args)
    {
        if (self->ok())
        {
            ChipDB::PinObjectKey pinKey = -1;
            ChipDB::NetObjectKey netKey = -1;
            if (PyArg_ParseTuple(args,"ii", &pinKey, &netKey))
            {
                if (self->obj()->setPinNet(pinKey, netKey))
                {
                    Py_RETURN_NONE;
                }
            }
            PyErr_Format(PyExc_ValueError, "setPinNet requires a pin key and net key argument");
            return nullptr;
        }

        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return nullptr;                
    }
#endif

    /** set internal values of PyInstance */
    static int pyInit(PyLayerInfo *self, PyObject *args, PyObject *kwds)
    {
        //std::cout << "Instance init called\n";
        return 0;   /* success */
    };

    /** set internal values of PyInstance */
    static PyObject* pyStr(PyObject *self)
    {
        //std::cout << "pyStr\n";
        return Python::toPython(PyLayerInfo::PythonObjectName);
    };

    static constexpr const char *PythonObjectName = "LayerInfo";
    static constexpr const char *PythonObjectDoc  = "LayerInfo object";
};

// cppcheck-suppress "suppressed_error_id"
static PyMemberDef PyLayerInfoMembers[] =    // NOLINT(modernize-avoid-c-arrays)
{
    {nullptr}  /* Sentinel */
};

static PyGetSetDef PyLayerInfoGetSet[] =     // NOLINT(modernize-avoid-c-arrays)
{
    {"name", (getter)PyLayerInfo::getName, nullptr, "layer name", nullptr /* closure */},
#if 0        
    {"archetype", (getter)PyInstance::getArchetype, nullptr, "archetype name", nullptr /* closure */},
    {"position", (getter)PyInstance::getPosition, (setter)PyInstance::setPosition, "lower left position in nm", nullptr /* closure */},
    {"pos", (getter)PyInstance::getPosition, (setter)PyInstance::setPosition, "lower left position in nm", nullptr /* closure */},
    {"placementInfo", (getter)PyInstance::getPlacementInfo, (setter)PyInstance::setPlacementInfo, "placement status", nullptr /* closure */},
    {"orientation", (getter)PyInstance::getOrientation, nullptr, "orientation of instance", nullptr /* closure */},
    {"size", (getter)PyInstance::getSize, nullptr, "size in nm", nullptr /* closure */},
    {"area", (getter)PyInstance::getArea, nullptr, "area in um^2", nullptr /* closure */},
#endif    
    {nullptr}
};

static PyMethodDef PyLayerInfoMethods[] =    // NOLINT(modernize-avoid-c-arrays)
{
#if 0    
    {"getPin", (PyCFunction)PyInstance::getPin, METH_VARARGS, "get pin by pin key or name"},
    {"getPinCount", (PyCFunction)PyInstance::getPinCount, METH_NOARGS, "get number of pins"},
    {"setPinNet", (PyCFunction)PyInstance::setPinNet, METH_VARARGS, "set the net key of a pin: setPinNet(pinKey, netKey). returns TRUE if successful."},
#endif    
    {nullptr}  /* Sentinel */
};

PyTypeObject PyLayerInfoType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    PyLayerInfo::PythonObjectName,       /* tp_name */
    sizeof(PyLayerInfo),                 /* tp_basicsize */
    0,                              /* tp_itemsize */
    (destructor)PyLayerInfo::pyDeAlloc,  /* tp_dealloc */
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
    PyLayerInfo::pyStr,                  /* tp_str */
    nullptr,                        /* tp_getattro */
    nullptr,                        /* tp_setattro */
    nullptr,                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,        /* tp_flags */
    PyLayerInfo::PythonObjectDoc,        /* tp_doc */
    nullptr,                        /* tp_traverse */
    nullptr,                        /* tp_clear */
    nullptr,                        /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    nullptr,                        /* tp_iter */
    nullptr,                        /* tp_iternext */
    PyLayerInfoMethods,                  /* tp_methods */
    PyLayerInfoMembers,                  /* tp_members */
    PyLayerInfoGetSet,                   /* tp_getset */
    nullptr,                        /* tp_base */
    nullptr,                        /* tp_dict */
    nullptr,                        /* tp_descr_get */
    nullptr,                        /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)PyLayerInfo::pyInit,       /* tp_init */
    nullptr,                        /* tp_alloc */
    PyLayerInfo::pyNewCall
};

PyObject* Python::toPython(std::shared_ptr<ChipDB::LayerInfo> layerInfoPtr)
{
    // create a new PyLayerInfo oject
    auto layerInfoObject = reinterpret_cast<PyLayerInfo*>(PyObject_CallObject((PyObject*)&PyLayerInfoType, nullptr));
    if (layerInfoObject->m_holder != nullptr)
    {
        *layerInfoObject->m_holder = layerInfoPtr;
        return (PyObject*)layerInfoObject;
    }
    return nullptr;
};
