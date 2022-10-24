#pragma once
#include <string>
#include <array>
#include <memory>

#include "common/dbtypes.h"
#include "netlist/net.h"
#include "design/design.h"

namespace LunaCore::CTS
{

struct ClockTreeNode
{
public:
    ClockTreeNode(ClockTreeNode *parent);
    virtual ~ClockTreeNode();

    ClockTreeNode(const ClockTreeNode &) = delete;
    ClockTreeNode& operator=(const ClockTreeNode&) = delete;

    void addCell(ChipDB::ObjectKey key)
    {
        m_cells.push_back(key);
    }

    auto const& cells() const
    {
        return m_cells;
    }

    auto const& children() const
    {
        return m_children;
    }

    void setChild(int index, ClockTreeNode *child)
    {
        m_children.at(index) = child;
    }

protected:
    ClockTreeNode* m_parent{nullptr};
    std::vector<ChipDB::ObjectKey>  m_cells;
    std::array<ClockTreeNode*,2>    m_children{nullptr, nullptr};
};

std::unique_ptr<ClockTreeNode> doStuff(const std::string &clockNetName, ChipDB::Netlist &netlist);

};