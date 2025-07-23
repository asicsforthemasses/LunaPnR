// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

/*

    String list model that adapts a vector
    of strings to a QAbstractListModel

*/

#pragma once

#include <QAbstractListModel>
#include <QAbstractTableModel>
#include <string>
#include <vector>

#include "lunacore.h"

namespace GUI
{

/** String list model that adapts a vector of strings to a QAbstractListModel */
class CellLibListModel : public QAbstractListModel
{
public:
    CellLibListModel(const std::shared_ptr<ChipDB::CellLib> m_cellLib);

    void setCellLib(const std::shared_ptr<ChipDB::CellLib> m_cellLib);

    /** return the number of rows in the list */
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /** returns various information about enabled/disabled items etc */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /** query the data a 'index' position */
    virtual QVariant data(const QModelIndex &index, int role) const override;

    /** query the view/list header information */
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    std::shared_ptr<ChipDB::Cell> getCell(int row) const;

protected:
    QColor m_lightColor;
    QColor m_darkColor;
    std::shared_ptr<ChipDB::CellLib> m_cellLib;
};


/** String list model that adapts a vector of strings to a QAbstractListModel */
class CellLibTableModel : public QAbstractTableModel, ChipDB::INamedStorageListener
{
public:
    CellLibTableModel(std::shared_ptr<ChipDB::CellLib> m_cellLib);

    virtual ~CellLibTableModel();

    void setCellLib(std::shared_ptr<ChipDB::CellLib> m_cellLib);

    /** return the number of rows in the table */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /** return the number of columns in the table */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /** returns various information about enabled/disabled items etc */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /** query the data a 'index' position */
    QVariant data(const QModelIndex &index, int role) const override;

    /** set the data of a specific item */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    /** query the view/list header information */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    std::shared_ptr<ChipDB::Cell> getCell(int row) const;

    void notify(ChipDB::ObjectKey index, ChipDB::INamedStorageListener::NotificationType t) override;

protected:

    static constexpr const int32_t c_subclassColumn = 2;

    QColor m_lightColor;
    QColor m_darkColor;
    std::shared_ptr<ChipDB::CellLib> m_cellLib;
};


}; // namespace

