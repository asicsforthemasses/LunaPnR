/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/


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

    void clear();

    size_t getNumberOfCells() const
    {
        return m_cells.size();
    }

    /** deprecated */
    size_t size() const
    {
        return m_cells.size();
    };

    /** create a cell of the given name or return an existing one */
    KeyObjPair<Cell> createCell(const std::string &name);
    
    KeyObjPair<Cell> lookupCell(const std::string &name) const;
    KeyObjPair<Cell> lookupCell(const std::string &name);

    const std::shared_ptr<Cell> lookupCell(ObjectKey key) const;
    std::shared_ptr<Cell> lookupCell(ObjectKey key);
    
    bool removeCell(ObjectKey key);
    bool removeCell(const std::string &name);

    auto begin() const noexcept
    {
        return m_cells.begin();
    }

    auto end() const noexcept
    {
        return m_cells.end();
    }

    void addListener(INamedStorageListener *listener);
    void removeListener(INamedStorageListener *listener);

protected:
    /** create a special net connection cell so we can connect nets */
    void createNetConCell();

    NamedStorage<Cell> m_cells;
};

class ModuleLib
{
public:
    virtual ~ModuleLib() = default;

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

    KeyObjPair<Module> lookupModule(const std::string &name);
    KeyObjPair<Module> lookupModule(const std::string &name) const;
    
    std::shared_ptr<Module> lookupModule(ObjectKey key);
    const std::shared_ptr<Module> lookupModule(ObjectKey key) const;

    KeyObjPair<Module> createModule(const std::string &name);

    bool removeModule(ObjectKey key);
    bool removeModule(const std::string &name);

    void addListener(INamedStorageListener *listener);
    void removeListener(INamedStorageListener *listener);

protected:
    NamedStorage<Module> m_modules;
};

};