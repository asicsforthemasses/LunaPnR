/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/


#include "modelnodebase.h"

using namespace GUI;

// ********************************************************************************
//    NodeBase
// ********************************************************************************

NodeBase::NodeBase(const QString &valueName, QVariant value) :
    m_valueName(valueName), m_value(value), m_parent(nullptr)
{
}

NodeBase::~NodeBase()
{
    for(auto c : m_children)
    {
        delete c;
    }
}

QVariant NodeBase::data(int column) const
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

NodeBase *NodeBase::getChild(int index) const
{
    if ((index < 0) || (index >= m_children.size()))
    {
        return nullptr;
    }

    return m_children.at(index);
}

void NodeBase::addChild(NodeBase *node)
{
    m_children.push_back(node);
    node->m_parent = this;
}

int NodeBase::findIndexOfChild(NodeBase *child) const
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

size_t NodeBase::numberOfChildren() const
{
    return m_children.size();
}

size_t NodeBase::getNumberOfColumns() const
{
    return 2;
}

NodeBase *NodeBase::getParent() const
{
    return m_parent;
}

QColor NodeBase::getBackgroundColor(int column) const
{
    if ((column>=0) && (column<m_bkColors.size()))
    {
        return m_bkColors.at(column);
    }

    return Qt::white;
}

void NodeBase::setBackgroundColor(int column, QColor col)
{
    if ((column>=0) && (column<m_bkColors.size()))
    {
        m_bkColors.at(column) = col;
    }    
}

const QPixmap& NodeBase::getIcon() const
{
    return m_icon;
}

void NodeBase::setIcon(QPixmap icon)
{
    m_icon = icon;
}

void NodeBase::setValue(QVariant value)
{
    m_value = value;
}

void NodeBase::setValueName(QString valueName)
{
    m_valueName = valueName;
}

// ********************************************************************************
//    ModelBase
// ********************************************************************************

ModelBase::ModelBase() : m_rootNode(nullptr)
{
}

ModelBase::~ModelBase()
{
}

QModelIndex ModelBase::index(int row, int column,
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

NodeBase* ModelBase::nodeFromIndex(const QModelIndex &index) const
{
    if (index.isValid())
    {
        return static_cast<NodeBase*>(index.internalPointer());
    }
    else
    {
        return m_rootNode.get();
    }
}

int ModelBase::rowCount(const QModelIndex &parent) const
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

int ModelBase::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QModelIndex ModelBase::parent(const QModelIndex &child) const
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

QVariant ModelBase::data(const QModelIndex &index, int role) const
{
    auto node = nodeFromIndex(index);
    if (node == nullptr)
    {
        return QVariant();
    }

    switch(role)
    {
    case Qt::BackgroundColorRole:
        return node->getBackgroundColor(index.column());
    case Qt::DisplayRole:
        return node->data(index.column());
    case Qt::DecorationRole:
        if (index.column() == 0)
        {
            return node->getIcon();
        }
        break;
    }

    return QVariant();
}


QVariant ModelBase::headerData(int section, Qt::Orientation orientation,
        int role) const
{
#if 0    
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("Node");
        } else if (section == 1) {
            return tr("Value");
        }
    }
#endif
    return QVariant();    
}

Qt::ItemFlags ModelBase::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled; // item is not selectable or editable.
}
