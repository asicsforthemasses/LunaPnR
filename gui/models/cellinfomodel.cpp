#include <array>
#include <QString>
#include <QColor>
#include <QVariant>
#include "cellinfomodel.h"

using namespace GUI;

// ********************************************************************************
//    CellInfoNode
// ********************************************************************************

CellInfoNode::CellInfoNode(const QString &valueName, QVariant value) 
    : m_valueName(valueName), m_value(value), m_parent(nullptr)
{

}

CellInfoNode::~CellInfoNode()
{
    for(auto child : m_children)
    {
        delete child;
    }
}

CellInfoNode* CellInfoNode::getChild(int index) const
{
    if (index < 0)
    {
        return nullptr;
    }

    if (index >= m_children.size())
    {
        return nullptr;
    }

    return m_children.at(index);
}

int CellInfoNode::findIndexOfChild(CellInfoNode *child) const
{
    int index = 0;
    for(auto c : m_children)
    {
        if (c == child)
        {
            return index;
        }
        index++;
    }
    return -1;  // not found
}

QVariant CellInfoNode::data(int column) const
{
    switch(column)
    {
    case 0:
        return m_valueName;
    case 1:
        return m_value;
    }

    return QVariant();
}

size_t CellInfoNode::numberOfChildren() const
{
    return m_children.size();
}

CellInfoNode* CellInfoNode::getParent() const
{
    return m_parent;
}

void CellInfoNode::addChild(CellInfoNode *node)
{
    m_children.push_back(node);
    node->m_parent = this;
}

// ********************************************************************************
//    CellInfoModel
// ********************************************************************************

CellInfoModel::CellInfoModel() : m_rootNode(nullptr)
{

}

CellInfoModel::~CellInfoModel()
{

}

void CellInfoModel::setCell(const ChipDB::Cell *cell)
{
    beginResetModel();
    m_rootNode.reset(new CellInfoNode("Cell", ""));

    // generic cell information
    auto areaNode = new CellInfoNode("Area (um²)", cell->m_area);
    m_rootNode->addChild(areaNode);

    auto classNode = new CellInfoNode("Class", QString::fromStdString(toString(cell->m_class)));
    m_rootNode->addChild(classNode);

    auto subclassNode = new CellInfoNode("Subclass", QString::fromStdString(toString(cell->m_subclass)));
    m_rootNode->addChild(subclassNode);

    auto siteNode = new CellInfoNode("Site", QString::fromStdString(cell->m_site));
    m_rootNode->addChild(siteNode);

    auto leakPowerNode = new CellInfoNode("Leakage pwr (nW)", cell->m_leakagePower * 1e9f);
    m_rootNode->addChild(leakPowerNode);

    auto offsetNode = new CellInfoNode("Offset ", QString::asprintf("%ld,%ld", cell->m_offset.m_x, cell->m_offset.m_y));
    m_rootNode->addChild(offsetNode);

    // expose all the pins
    for(auto const& pin : cell->m_pins)
    {
        auto pinNode = new CellInfoNode("Pin", QString::fromStdString(pin.m_name));
        m_rootNode->addChild(pinNode);

        // add input/output for each pin
        pinNode->addChild(new CellInfoNode("Type", QString::fromStdString(toString(pin.m_iotype))));

        if (!pin.m_function.empty())
        {
            pinNode->addChild(new CellInfoNode("Function", QString::fromStdString(pin.m_function)));
        }

        if (pin.m_cap > 0.0)
        {
            pinNode->addChild(new CellInfoNode("Capacitance (pF)", pin.m_cap * 1.e12f));
        }
    }

    endResetModel();
}

QModelIndex CellInfoModel::index(int row, int column,
    const QModelIndex &parent) const
{
    if ((!m_rootNode) || (row < 0) || (column <0))
    {
        return QModelIndex();
    }

    auto parentNode = nodeFromIndex(parent);
    auto childNode = parentNode->getChild(row);

    if (childNode == nullptr)
    {
        return QModelIndex();
    }

    return createIndex(row, column, childNode);
}

CellInfoNode* CellInfoModel::nodeFromIndex(const QModelIndex &index) const
{
    if (index.isValid())
    {
        return static_cast<CellInfoNode*>(index.internalPointer());
    }
    else
    {
        return m_rootNode.get();
    }
}

int CellInfoModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
    {
        return 0;
    }

    auto parentNode = nodeFromIndex(parent);
    if (parentNode == nullptr)
    {
        return 0;
    }

    return parentNode->numberOfChildren();
}

int CellInfoModel::columnCount(const QModelIndex &parent) const
{
    // TODO: return info from the parent node.
    return 2;
}

QModelIndex CellInfoModel::parent(const QModelIndex &child) const
{
    auto node = nodeFromIndex(child);
    if (node == nullptr)
    {
        return QModelIndex();
    }

    auto parentNode = node->getParent();
    if (parentNode == nullptr)
    {
        return QModelIndex();
    }

    auto grandparentNode = parentNode->getParent();
    if (grandparentNode == nullptr)
    {
        return QModelIndex();
    }

    int row = grandparentNode->findIndexOfChild(parentNode);
    return createIndex(row, 0, parentNode);
}

QVariant CellInfoModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    auto node = nodeFromIndex(index);
    if (node == nullptr)
    {
        return QVariant();
    }

    if (index.column() < 2)
    {
        return node->data(index.column());
    }

    return QVariant();
}


QVariant CellInfoModel::headerData(int section, Qt::Orientation orientation,
        int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("Node");
        } else if (section == 1) {
            return tr("Value");
        }
    }
    return QVariant();    
}

#if 0
class CellInfoModel : public QAbstractItemModel
{
public:
    CellInfoModel();
    virtual ~CellInfoModel();

    void setRootNode(CellInfoNode *rootNode);

    QModelIndex index(int row, int column, 
        const QModelIndex &parent) const;

    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
        int role) const;

protected:
    CellInfoNode *nodeFromIndex(const QModelIndex &index) const;

    CellInfoNode *m_rootNode;
};

#endif