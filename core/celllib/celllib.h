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

    size_t size() const
    {
        return m_cells.size();
    };

    KeyObjPair<Cell> createCell(const std::string &name);
    
    const std::shared_ptr<Cell> lookupCell(const std::string &name) const;
    std::shared_ptr<Cell> lookupCell(const std::string &name);

    const std::shared_ptr<Cell> lookupCell(ObjectKey key) const;
    std::shared_ptr<Cell> lookupCell(ObjectKey key);
    
    bool removeCell(ObjectKey key);
    bool removeCell(const std::string &name);

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

    std::shared_ptr<Module> lookupModule(const std::string &name);
    const std::shared_ptr<Module> lookupModule(const std::string &name) const;
    
    std::shared_ptr<Module> lookupModule(ObjectKey key);
    const std::shared_ptr<Module> lookupModule(ObjectKey key) const;

    KeyObjPair<Module> createModule(const std::string &name);

    bool removeModule(ObjectKey key);
    bool removeModule(const std::string &name);

protected:
    NamedStorage<Module> m_modules;
};

};