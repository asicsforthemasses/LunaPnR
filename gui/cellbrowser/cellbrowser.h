// SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <memory>
#include <optional>
#include <QWidget>
#include <QListView>
#include <QTableView>
#include <QTreeView>
#include <QGroupBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QItemDelegate>

#include "../common/layerrenderinfo.h"
#include "../models/celllibmodel.h"
#include "../models/cellinfomodel.h"
#include "../models/layerappearancetablemodel.h"
#include "../propertyview/propertyview.h"
#include "celllayoutview.h"

namespace GUI
{

class SubclassDelegate : public QItemDelegate
{
public:
    SubclassDelegate(QObject *parent = 0);
    
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};

class CellBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit CellBrowser(QWidget *parent = nullptr);
    virtual ~CellBrowser();

    void setDatabase(std::shared_ptr<Database> db);
    void refreshDatabase();

    QSize sizeHint() const;

public slots:
    void onCellSelectionChanged(const QItemSelection &cur, const QItemSelection &prev);

protected:
    std::unique_ptr<CellLibTableModel>  m_cellModel;
    std::unique_ptr<CellInfoModel>      m_cellInfoModel;
    std::unique_ptr<LayerAppearanceTableModel> m_layerTableModel;

    QHBoxLayout     *m_layout;
    QVBoxLayout     *m_layout2;
    QTableView      *m_cellTableView;
    CellLayoutView  *m_cellLayoutView;

    QTreeView       *m_cellTreeView;
    QTableView      *m_layerView;
    
    SubclassDelegate m_subclassDelegate;

    std::shared_ptr<Database> m_db;
};

};  // namespace