/*

*/

#pragma once

//#include <QAbstractListModel>
//#include <QAbstractTableModel>
#include <QAbstractItemModel>
#include <QString>

#include <string>
#include <vector>

#include "lunacore.h"

namespace GUI
{

class CellInfoNode
{
public:
    CellInfoNode(const QString &valueName, QVariant value);
    virtual ~CellInfoNode();

    CellInfoNode *getChild(int index) const;
    void addChild(CellInfoNode *node);

    int findIndexOfChild(CellInfoNode *child) const;
    size_t numberOfChildren() const;

    CellInfoNode* getParent() const;

    virtual QVariant data(int column) const;

protected:
    QString         m_valueName;
    QVariant        m_value;

    CellInfoNode    *m_parent;
    std::vector<CellInfoNode *> m_children;
};

class CellInfoModel : public QAbstractItemModel
{
public:
    CellInfoModel();
    virtual ~CellInfoModel();

    void setCell(const ChipDB::Cell *cell);

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

    std::unique_ptr<CellInfoNode> m_rootNode;
};

};
