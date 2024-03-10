// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <array>
#include <memory>
#include <vector>

#include <QObject>
#include <QString>
#include <QVariant>
#include <QPixmap>
#include <QAbstractItemModel>

namespace GUI
{

class NodeBase : public QObject
{
    Q_OBJECT

public:
    NodeBase(const QString &valueName, QVariant value);
    virtual ~NodeBase();

    /** user overrides this method to expose data to a Qt view */
    virtual QVariant data(int column) const;

    NodeBase *getChild(int index) const;
    void addChild(NodeBase *node);

    int findIndexOfChild(NodeBase *child) const;
    size_t numberOfChildren() const;

    size_t getNumberOfColumns() const;

    NodeBase *getParent() const;

    QColor getBackgroundColor(int column) const;
    void   setBackgroundColor(int column, QColor col);

    const QPixmap& getIcon() const;
    void setIcon(QPixmap icon);

    void setValue(QVariant value);
    void setValueName(QString valueName);

protected:
    NodeBase *m_parent;

    QPixmap  m_icon;
    QVariant m_value;
    QString  m_valueName;

    std::array<QColor,2>   m_bkColors;
    std::vector<NodeBase*> m_children;
};


class ModelBase : public QAbstractItemModel
{
    Q_OBJECT
public:
    ModelBase();
    virtual ~ModelBase();

    QModelIndex index(int row, int column,
        const QModelIndex &parent) const override;

    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
        int role) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
    NodeBase *nodeFromIndex(const QModelIndex &index) const;

    std::unique_ptr<NodeBase> m_rootNode;
};

}
