#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "common/dbtypes.h"
#include "common/namedstorage.h"
#include "cell.h"
#include "module.h"

namespace ChipDB
{

class CellLib
{
public:
    CellLib() 
    {
        clear();
    }

    virtual ~CellLib() = default;

    NamedStorage<Cell*, true> m_cells;

    void clear();

    size_t size() const
    {
        return m_cells.size();
    };

    auto begin() const
    {
        return m_cells.begin();
    }

    auto end() const
    {
        return m_cells.end();
    }

    Cell* createCell(const std::string &name);
    Cell* lookup(const std::string &name) const;

protected:
    /** create a special net connection cell so we can connect nets */
    void createNetConCell();
};

class ModuleLib
{
public:
    virtual ~ModuleLib() = default;

    NamedStorage<Module*, true> m_modules;

    void clear();

    size_t size() const
    {
        return m_modules.size();
    };

    auto begin() const
    {
        return m_modules.begin();
    }

    auto end() const
    {
        return m_modules.end();
    }

    Module* at(size_t index);
    const Module* at(size_t index) const;
    Module* lookup(const std::string &name) const;
    Module* createModule(const std::string &name);
};

};