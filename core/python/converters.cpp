// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <sstream>
#include "converters.h"

PyObject* Python::toPython(const int &t)
{
    return PyLong_FromLong(t);
}

PyObject* Python::toPython(const unsigned int &t)
{
    return PyLong_FromUnsignedLong(t);
}

PyObject* Python::toPython(const float &t)
{
    return PyFloat_FromDouble(t);
}

PyObject* Python::toPython(const double &t)
{
    return PyFloat_FromDouble(t);
}

PyObject* Python::toPython(const size_t &t)
{
    return PyLong_FromSize_t(t);
}

PyObject* Python::toPython(const ssize_t &t)
{
    return PyLong_FromSsize_t(t);
}

PyObject* Python::toPython(const std::string_view &t)
{
    if (t.size() == 0)
    {
        return PyUnicode_FromStringAndSize(nullptr, 0);
    }
    else
    {
        return PyUnicode_FromStringAndSize(&t.at(0), t.size());
    }    
}

PyObject* Python::toPython(const std::string &t)
{
    if (t.size() == 0)
    {
        return PyUnicode_FromStringAndSize(nullptr, 0);
    }
    else
    {
        return PyUnicode_FromStringAndSize(&t.at(0), t.size());
    }    
}

PyObject* Python::toPython(const char *t)
{
    return PyUnicode_FromString(t);
}

PyObject* Python::toPython(const ChipDB::Coord64 &t)
{
    return Py_BuildValue("ll", t.m_x, t.m_y);
}

PyObject* toPython(const ChipDB::Rect64 &t)
{
    return Py_BuildValue("(ll)(ll)", t.getLL().m_x, t.getLL().m_y, 
        t.getUR().m_x, t.getUR().m_y);
}

PyObject* Python::toPython(const ChipDB::CellClass &t)
{
    return Python::toPython(ChipDB::toString(t));
}

PyObject* Python::toPython(const ChipDB::CellSubclass &t)
{
    return Python::toPython(ChipDB::toString(t));    
}

PyObject* Python::toPython(const ChipDB::SymmetryFlags &t)
{
    std::stringstream ss;
    auto flags = static_cast<uint8_t>(t.m_flags);
    if (flags & ChipDB::SymmetryFlags::SYM_X)
    {
        ss << "X ";
    }
    if (flags & ChipDB::SymmetryFlags::SYM_Y)
    {
        ss << "Y ";
    }
    if (flags & ChipDB::SymmetryFlags::SYM_R90)
    {
        ss << "R90 ";
    }

    std::string flagString = ss.str();
    if (flagString.empty())
    {
        flagString = "NONE";
    }

    // make it pretty by removing the superfluous space
    // at the end of the string, if there is one.
    if (flagString.back() == ' ')
    {
        flagString.pop_back();
    }

    return Python::toPython(flagString);
}

// ********************************************************************************
//   fromPython
// ********************************************************************************

bool Python::fromPython(PyObject *obj, int &result)
{
    if (PyLong_Check(obj) == 1)
    {
        result = PyLong_AsLong(obj);
        return true;   // OK result
    }
    return false;  // error
}

bool Python::fromPython(PyObject *obj, std::string &result)
{
    if (PyUnicode_Check(obj) == 1)
    {
        result = PyUnicode_AsUTF8(obj);
        return true;   // OK result
    }
    return false;  // error
}
