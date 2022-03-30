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
