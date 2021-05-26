#pragma once

#include <string>
#include <vector>
#include "celllib/celllib.h"
#include "netlist/net.h"
#include "common/visitor.h"

namespace ChipDB
{

class Net;  // pre-declaration

class Instance
{
public:
    Instance(Cell *cell) : m_cell(cell) 
    {
        m_pinToNet.resize(cell->m_pins.size());
    }

    IMPLEMENT_ACCEPT;

    std::string m_name; ///< name of instance

    /** get access to the cell/module */
    const Cell* cell() const
    {
        return m_cell;
    }

    /** return the cell/module name */
    virtual std::string getArchetypeName() const;

    bool isModule() const;
    
    struct Pin
    {
        Pin() : m_pinIndex(-1), m_connection(nullptr), m_info(nullptr) {}

        bool isValid() const
        {
            return m_pinIndex >= 0;
        }

        ssize_t          m_pinIndex;
        const Net       *m_connection;
        const PinInfo   *m_info;
    };

    const Pin pin(ssize_t pinIndex) const;
    const Pin pin(const std::string &pinName) const;

    bool connect(ssize_t pinIndex, Net *net);
    bool connect(const std::string &pinName, Net *net);


    class PinConstIterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Pin;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        PinConstIterator(const std::vector<Net*> &s1, const PinInfoList &s2, size_t index) 
        : m_s1(s1), m_s2(s2), m_index(index)
        {
        }

        PinConstIterator(const PinConstIterator &iter) :
            m_s1(iter.m_s1), m_s2(iter.m_s2), m_index(iter.m_index)
        {
        } 

        PinConstIterator operator++(int)
        {
            auto temp(*this);
            if (m_index < m_s1.size())
                ++m_index;

            return temp;
        }

        PinConstIterator& operator++()
        {
            if (m_index < m_s1.size())
                ++m_index;

            return (*this);
        }

        bool operator==(const PinConstIterator& iter) const {return (m_index == iter.m_index);}
        bool operator!=(const PinConstIterator& iter) const {return (m_index != iter.m_index);}

        const value_type operator*()
        {
            Pin p;
            p.m_pinIndex   = m_index;
            p.m_connection = m_s1[m_index];
            p.m_info       = m_s2[m_index];

            return p;
        }

        const value_type operator->()
        {
            Pin p;
            p.m_pinIndex   = m_index;
            p.m_connection = m_s1[m_index];
            p.m_info       = m_s2[m_index];

            return p;
        }

    protected:
        const std::vector<Net*>&    m_s1;
        const PinInfoList&          m_s2;
        ssize_t m_index;
    };

    PinConstIterator begin() const
    {
        return PinConstIterator(m_pinToNet, m_cell->m_pins, 0);
    }

    PinConstIterator end() const
    {
        return PinConstIterator(m_pinToNet, m_cell->m_pins, m_cell->m_pins.size());
    }

protected:
    std::vector<Net*>   m_pinToNet;  ///< connections from pin to net
    Cell* m_cell;
};

};
