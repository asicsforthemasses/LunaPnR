// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QColor>
#include <QAbstractTableModel>
#include <QModelIndex>
#include "lunacore.h"

namespace GUI
{

class ModuleTableModel : public QAbstractTableModel,  ChipDB::INamedStorageListener
{
public:
    ModuleTableModel(std::shared_ptr<ChipDB::ModuleLib> moduleLib);

    void setModuleLib(std::shared_ptr<ChipDB::ModuleLib> moduleLib);

    /** return the number of rows in the table */
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /** return the number of columns in the table */
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /** returns various information about enabled/disabled items etc */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /** query the data a 'index' position */
    virtual QVariant data(const QModelIndex &index, int role) const override;

    /** query the view/list header information */
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    const std::shared_ptr<ChipDB::Module> getModule(int row) const;

    /** called by ChipdB::ModuleLib */
    void notify(ChipDB::ObjectKey index, NotificationType t) override;

protected:
    QColor m_lightColor;
    QColor m_darkColor;
    std::shared_ptr<ChipDB::ModuleLib> m_moduleLib;
};


/** gives a list of all available models */
class ModuleListModel : public QAbstractListModel, ChipDB::INamedStorageListener
{
public:
    ModuleListModel(std::shared_ptr<ChipDB::ModuleLib> moduleLib);

    void setModuleLib(std::shared_ptr<ChipDB::ModuleLib> moduleLib);

    /** returns various information about enabled/disabled items etc */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /** return the number of rows in the list */
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /** query the data a 'index' position */
    virtual QVariant data(const QModelIndex &index, int role) const override;

    /** query the view/list header information */
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /** called by ChipdB::ModuleLib */
    void notify(ChipDB::ObjectKey index, NotificationType t) override;

protected:
    QColor m_lightColor;
    QColor m_darkColor;
    std::shared_ptr<ChipDB::ModuleLib> m_moduleLib;
};

}; // namespace
