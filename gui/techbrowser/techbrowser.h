// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <memory>
#include <QWidget>
#include <QListView>
#include <QTableView>
#include <QTreeView>
#include <QGroupBox>
#include <QFrame>
#include <QBoxLayout>

#include "../common/database.h"
#include "../models/techlibinfomodel.h"
#include "../models/techlibsitemodel.h"
#include "../widgets/colorbutton.h"
#include "../widgets/hatchbutton.h"
#include "../widgets/layerwidget.h"

namespace GUI
{

class TechBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit TechBrowser(QWidget *parent = nullptr);
    virtual ~TechBrowser();

    void setDatabase(std::shared_ptr<Database> db);
    void refreshDatabase();

    QSize sizeHint() const;

public slots:
    void onLayerSelectionChanged(const QItemSelection &cur, const QItemSelection &prev);
    void onSiteSelectionChanged(const QItemSelection &cur, const QItemSelection &prev);
    void onLayerColorChanged();
    void onChangeHatch();
    void onLayerObsColorChanged();
    void onChangeObsHatch();

protected:
    std::unique_ptr<LayerInfoModel>     m_layerInfoModel;
    std::unique_ptr<LayerTableModel>    m_layerTableModel;
    std::unique_ptr<SiteInfoModel>      m_siteInfoModel;
    std::unique_ptr<SiteTableModel>     m_siteTableModel;

    QVBoxLayout     *m_mainLayout;

    QGroupBox       *m_layerGroupBox;
    QGroupBox       *m_siteGroupBox;

    QHBoxLayout     *m_layerLayout;
    QHBoxLayout     *m_siteLayout;

    QTableView      *m_siteTableView;
    QTreeView       *m_siteTreeView;

    LayerWidget         *m_layerTableView;
    QTreeView           *m_layerTreeView;
    SelectColorButton   *m_colorButton;
    SelectHatchButton   *m_hatchButton;
    SelectColorButton   *m_colorObsButton;
    SelectHatchButton   *m_hatchObsButton;

    std::shared_ptr<Database> m_db;
};

};  // namespace