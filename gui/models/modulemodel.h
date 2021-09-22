#pragma once

#include <QColor>
#include <QAbstractTableModel>
#include <QModelIndex>
#include "lunacore.h"

namespace GUI
{

class ModuleTableModel : public QAbstractTableModel
{
public:
    ModuleTableModel(const ChipDB::ModuleLib *moduleLib);

    void setModuleLib(const ChipDB::ModuleLib *moduleLib);

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

    const ChipDB::Module* getModule(int row) const;

protected:
    QColor m_lightColor;
    QColor m_darkColor;
    const ChipDB::ModuleLib *m_moduleLib;
};


/** gives a list of all available models */
class ModuleListModel : public QAbstractListModel
{
public:
    ModuleListModel(const ChipDB::ModuleLib *moduleLib);

    void setModuleLib(const ChipDB::ModuleLib *moduleLib);

    /** returns various information about enabled/disabled items etc */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /** return the number of rows in the list */
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /** query the data a 'index' position */
    virtual QVariant data(const QModelIndex &index, int role) const override;

    /** query the view/list header information */
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

protected:
    QColor m_lightColor;
    QColor m_darkColor;
    const ChipDB::ModuleLib *m_moduleLib;
};

}; // namespace
