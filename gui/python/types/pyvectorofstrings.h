// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include <memory>
#include <array>
#include <iostream>

#include "../../../core/python/converters.h"
#include "../../../core/python/types/typetemplate.h"

struct PyVectorOfStringsIterator
{
    using iterator   = std::vector<std::string>::iterator;
    using value_type = typename iterator::value_type;

    std::vector<std::string> *m_vector = nullptr;
    iterator m_iter;

    iterator end()
    {
        if (!m_vector)
        {
            return iterator();
        }
        return m_vector->end();
    }

    iterator begin()
    {
        if (!m_vector)
        {
            return iterator();
        }        
        return m_vector->begin();
    }
};

/** read-only container for std::vector<std::string> */
struct PyVectorOfStrings : public Python::TypeTemplate<PyVectorOfStringsIterator>
{
    static PyObject* getString(PyVectorOfStrings *self, PyObject *args)
    {
        if (self->ok())
        {
            if (!self->obj()->m_vector)
            {
                PyErr_Format(PyExc_RuntimeError, "PyVectorOfStrings has no vector");
                return nullptr;
            }

            int index = 0;
            if (PyArg_ParseTuple(args,"i", &index))
            {   
                auto vecPtr = self->obj()->m_vector;
                if ((vecPtr == nullptr) || (vecPtr->size() <= index))
                {
                    PyErr_Format(PyExc_ValueError, "Index %ld out of range", index);
                    return nullptr;
                }
                return Python::toPython(vecPtr->at(index));
            }

            PyErr_Format(PyExc_ValueError, "at requires an index as argument");
            return nullptr;
        }
        
        PyErr_Format(PyExc_RuntimeError, "Self is uninitialized");
        return nullptr;         
    }

    static PyObject* pyIter(PyVectorOfStrings *self)
    {
        if (self->ok())
        {
            self->obj()->m_iter = self->obj()->begin();
            Py_INCREF(self);
            return (PyObject*)self;
        }

        return nullptr;
    };

    static PyObject* pyIterNext(PyVectorOfStrings *self)
    {
        if (self->ok())
        {
            if (self->obj()->m_iter == self->obj()->end())
            {
                return nullptr; // no more object, stop iteration
            }

            auto const& myString = *self->obj()->m_iter;
            auto result = Python::toPython(myString);

            self->obj()->m_iter++;
            return result;
        }

        return nullptr;
    };
};

