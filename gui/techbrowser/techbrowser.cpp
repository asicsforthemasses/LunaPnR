// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "techbrowser.h"
#include "../widgets/hatchdialog.h"
#include <QHeaderView>
#include <QLabel>
#include <QColor>
#include <QGroupBox>

using namespace GUI;

TechBrowser::TechBrowser(QWidget *parent) : QWidget(parent), m_db(nullptr)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    // ************************************************************************
    // create layer and site group boxes and stack them vertically
    // ************************************************************************

    m_mainLayout = new QVBoxLayout();

    m_layerGroupBox = new QGroupBox("Layers");
    m_siteGroupBox  = new QGroupBox("Sites");

    m_mainLayout->addWidget(m_layerGroupBox);
    m_mainLayout->addWidget(m_siteGroupBox);

    setLayout(m_mainLayout);

    // ************************************************************************
    // create contents of layer groupbox
    // ************************************************************************

    m_layerTableView = new LayerWidget();

    m_layerTableModel.reset(new LayerTableModel(nullptr));
    m_layerTableView->setModel(m_layerTableModel.get());

    // layer information tree view
    m_layerTreeView = new QTreeView(this);
    m_layerTreeView->setRootIsDecorated(false);
    m_layerTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers); // make read-only

    m_layerInfoModel.reset(new LayerInfoModel());
    m_layerTreeView->setModel(m_layerInfoModel.get());

    //m_layout2 = new QVBoxLayout();
    //m_layout2->addWidget(new QLabel("Cell information"),0);
    //m_layout2->addWidget(m_cellTreeView,1);

    m_layerLayout = new QHBoxLayout();
    m_layerLayout->addWidget(m_layerTableView,1);
    m_layerLayout->addWidget(m_layerTreeView,2);

    m_colorButton = new SelectColorButton();
    m_hatchButton = new SelectHatchButton();
    m_colorObsButton = new SelectColorButton();
    m_hatchObsButton = new SelectHatchButton();

    auto gbox1 = new QGroupBox(tr("Routing"));
    auto gbox1Layout = new QHBoxLayout();

    gbox1Layout->addWidget(m_hatchButton,0);
    gbox1Layout->addWidget(m_colorButton,0);
    gbox1->setLayout(gbox1Layout);

    auto gbox2 = new QGroupBox(tr("Obstructions"));
    auto gbox2Layout = new QHBoxLayout();

    gbox2Layout->addWidget(m_hatchObsButton,0);
    gbox2Layout->addWidget(m_colorObsButton,0);
    gbox2->setLayout(gbox2Layout);

    auto renderInfoBox = new QVBoxLayout();
    renderInfoBox->addWidget(gbox1);
    renderInfoBox->addWidget(gbox2);

    m_layerLayout->addLayout(renderInfoBox);

    //m_layout->addWidget(m_cellLayoutView,2);
    //m_layout->addLayout(m_layout2,1);

    m_layerGroupBox->setLayout(m_layerLayout);

    // ************************************************************************
    // create contents of layer groupbox
    // ************************************************************************

    m_siteTableView = new QTableView(this);
    m_siteTableView->setSelectionBehavior(QTableView::SelectRows);
    m_siteTableView->setSelectionMode(QAbstractItemView::SingleSelection);

    m_siteTableModel.reset(new SiteTableModel(nullptr));
    m_siteTableView->setModel(m_siteTableModel.get());

    // layer information tree view
    m_siteTreeView = new QTreeView(this);
    m_siteTreeView->setRootIsDecorated(false);
    m_siteTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers); // make read-only

    m_siteInfoModel.reset(new SiteInfoModel());
    m_siteTreeView->setModel(m_siteInfoModel.get());

    m_siteLayout = new QHBoxLayout();
    m_siteLayout->addWidget(m_siteTableView,1);
    m_siteLayout->addWidget(m_siteTreeView,2);

    m_siteGroupBox->setLayout(m_siteLayout);

    // ************************************************************************
    // connect signals/slots
    // ************************************************************************

    connect(m_layerTableView->selectionModel(),
        SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this,
        SLOT(onLayerSelectionChanged(const QItemSelection&, const QItemSelection&)));

    connect(m_colorButton,
        SIGNAL(onColorChanged()),
        this,
        SLOT(onLayerColorChanged()));

    connect(m_hatchButton,
        SIGNAL(clicked()),
        this,
        SLOT(onChangeHatch()) );

    connect(m_colorObsButton,
        SIGNAL(onColorChanged()),
        this,
        SLOT(onLayerObsColorChanged()));

    connect(m_hatchObsButton,
        SIGNAL(clicked()),
        this,
        SLOT(onChangeObsHatch()) );

    connect(m_siteTableView->selectionModel(),
        SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this,
        SLOT(onSiteSelectionChanged(const QItemSelection&, const QItemSelection&)));
}

TechBrowser::~TechBrowser()
{
}

QSize TechBrowser::sizeHint() const
{
    return m_layerTreeView->sizeHint();
}

void TechBrowser::setDatabase(std::shared_ptr<Database> db)
{
    if (m_db)
    {
        m_db->techLib()->removeLayerListener(m_layerInfoModel.get());
        m_db->techLib()->removeSiteListener(m_siteInfoModel.get());
    }

    m_db = db;

    if (!m_db)
    {
        m_layerInfoModel->setLayer(nullptr);
        m_layerTableModel->setTechLib(nullptr);
        return;
    }

    m_db->techLib()->addLayerListener(m_layerInfoModel.get());
    m_db->techLib()->addSiteListener(m_siteInfoModel.get());

    if (m_db->techLib()->getNumberOfLayers() > 0)
    {
        auto layer = m_db->techLib()->layers().at(0);
        m_layerInfoModel->setLayer(layer);
    }
    else
    {
        m_layerInfoModel->setLayer(nullptr);
    }

    if (m_db->techLib()->getNumberOfSites() > 0)
    {
        auto site = m_db->techLib()->sites().at(0);
        m_siteInfoModel->setSite(site);
    }
    else
    {
        m_siteInfoModel->setSite(nullptr);
    }

    m_layerTableModel->setTechLib(m_db->techLib());
    m_siteTableModel->setTechLib(m_db->techLib());

    // make sure all columns of the layer table can expand
    for(size_t c=0; c < m_layerTableView->horizontalHeader()->count(); c++)
    {
        m_layerTableView->horizontalHeader()->setSectionResizeMode(
            c, QHeaderView::Stretch);
    }

    for(size_t c=0; c < m_layerTreeView->header()->count(); c++)
    {
        m_layerTreeView->header()->setSectionResizeMode(
            c, QHeaderView::Stretch);
    }

    // make sure all columns of the site table can expand
    for(size_t c=0; c < m_siteTableView->horizontalHeader()->count(); c++)
    {
        m_siteTableView->horizontalHeader()->setSectionResizeMode(
            c, QHeaderView::Stretch);
    }

    for(size_t c=0; c < m_siteTreeView->header()->count(); c++)
    {
        m_siteTreeView->header()->setSectionResizeMode(
            c, QHeaderView::Stretch);
    }

    m_layerTreeView->expandAll();
    m_layerTableView->selectRow(0);

    m_siteTreeView->expandAll();
    m_siteTableView->selectRow(0);
}

void TechBrowser::refreshDatabase()
{
    if (!m_db)
    {
        // make sure that every layer in the tech library
        // has a corresponding layer in the layer render library

        for(auto techLayer : m_db->techLib()->layers())
        {
            m_db->m_layerRenderInfoDB.createLayer(techLayer->name());
        }

        m_layerTableModel->setTechLib(m_db->techLib());
    }
}

void TechBrowser::onLayerSelectionChanged(const QItemSelection &cur, const QItemSelection &prev)
{
    QModelIndex index = m_layerTableView->currentIndex();

    if (index.isValid())
    {
        auto layer = m_layerTableModel->getLayer(index.row());
        if (layer != nullptr)
        {
            m_layerInfoModel->setLayer(layer);
            m_layerTreeView->expandAll();

            if (m_db != nullptr)
            {
                // make sure the layer render info database stays in sync
                if (m_db->m_layerRenderInfoDB.size() != m_db->techLib()->layers().size())
                {
                    refreshDatabase();
                }

                auto info = m_db->m_layerRenderInfoDB[layer->name()];
                if (info.isValid())
                {
                    m_colorButton->setEnabled(true);
                    m_hatchButton->setEnabled(true);
                    m_colorObsButton->setEnabled(true);
                    m_hatchObsButton->setEnabled(true);
                    m_colorButton->setColor(info->routing().getColor());
                    m_hatchButton->setHatch(info->routing().getTexture());
                    m_colorObsButton->setColor(info->obstruction().getColor());
                    m_hatchObsButton->setHatch(info->obstruction().getTexture());
                }
                else
                {
                    m_colorButton->setDisabled(true);
                    m_hatchButton->setDisabled(true);
                    m_colorObsButton->setDisabled(true);
                    m_hatchObsButton->setDisabled(true);
                }
            }
            else
            {
                m_colorButton->setDisabled(true);
                m_hatchButton->setDisabled(true);
                m_colorObsButton->setDisabled(true);
                m_hatchObsButton->setDisabled(true);
            }

            update();
            Logging::logVerbose("Selected layer %s\n", layer->name().c_str());
        }
    }
}

void TechBrowser::onLayerColorChanged()
{
    QModelIndex index = m_layerTableView->currentIndex();
    auto layer = m_layerTableModel->getLayer(index.row());

    if ((layer != nullptr) && (m_db != nullptr))
    {
        auto info = m_db->m_layerRenderInfoDB[layer->name()];
        if (info.isValid())
        {
            info->routing().setColor(m_colorButton->getColor());
            m_db->m_layerRenderInfoDB.contentsChanged();
        }
    }
}

void TechBrowser::onChangeHatch()
{
    if (m_db == nullptr)
    {
        return;
    }

    HatchDialog dialog(m_db->m_hatchLib, this);

    auto retval = dialog.exec();
    if (retval == QDialog::Accepted)
    {
        auto index = dialog.getHatchIndex();
        if (index >= 0)
        {
            auto hatch = m_db->m_hatchLib.m_hatches.at(index);
            m_hatchButton->setHatch(hatch);

            QModelIndex index = m_layerTableView->currentIndex();
            auto layer = m_layerTableModel->getLayer(index.row());
            if (layer != nullptr)
            {
                auto info = m_db->m_layerRenderInfoDB[layer->name()];
                if (info.isValid())
                {
                    info->routing().setTexture(hatch);
                    m_db->m_layerRenderInfoDB.contentsChanged();
                }
            }
        }
    }
}

void TechBrowser::onLayerObsColorChanged()
{
    QModelIndex index = m_layerTableView->currentIndex();
    auto layer = m_layerTableModel->getLayer(index.row());

    if ((layer != nullptr) && (m_db != nullptr))
    {
        auto info = m_db->m_layerRenderInfoDB[layer->name()];
        if (info.isValid())
        {
            info->obstruction().setColor(m_colorObsButton->getColor());
            m_db->m_layerRenderInfoDB.contentsChanged();
        }
    }
}

void TechBrowser::onChangeObsHatch()
{
    if (m_db == nullptr)
    {
        return;
    }

    HatchDialog dialog(m_db->m_hatchLib, this);

    auto retval = dialog.exec();
    if (retval == QDialog::Accepted)
    {
        auto index = dialog.getHatchIndex();
        if (index >= 0)
        {
            auto hatch = m_db->m_hatchLib.m_hatches.at(index);
            m_hatchObsButton->setHatch(hatch);

            QModelIndex index = m_layerTableView->currentIndex();
            auto layer = m_layerTableModel->getLayer(index.row());
            if (layer != nullptr)
            {
                auto info = m_db->m_layerRenderInfoDB[layer->name()];
                if (info.isValid())
                {
                    info->obstruction().setTexture(hatch);
                    m_db->m_layerRenderInfoDB.contentsChanged();
                }
            }
        }
    }
}

void TechBrowser::onSiteSelectionChanged(const QItemSelection &cur, const QItemSelection &prev)
{
    QModelIndex index = m_siteTableView->currentIndex();

    if (index.isValid())
    {
        auto site = m_siteTableModel->getSite(index.row());
        if (site != nullptr)
        {
            m_siteInfoModel->setSite(site);
            m_siteTreeView->expandAll();

#if 0
            if (m_db != nullptr)
            {
                // make sure the layer render info database stays in sync
                if (m_db->m_layerRenderInfoDB.size() != m_db->techLib()->layers().size())
                {
                    refreshDatabase();
                }

                auto info = m_db->m_layerRenderInfoDB[layer->name()];
                if (info.isValid())
                {
                    m_colorButton->setEnabled(true);
                    m_hatchButton->setEnabled(true);
                    m_colorObsButton->setEnabled(true);
                    m_hatchObsButton->setEnabled(true);
                    m_colorButton->setColor(info->routing().getColor());
                    m_hatchButton->setHatch(info->routing().getTexture());
                    m_colorObsButton->setColor(info->obstruction().getColor());
                    m_hatchObsButton->setHatch(info->obstruction().getTexture());
                }
                else
                {
                    m_colorButton->setDisabled(true);
                    m_hatchButton->setDisabled(true);
                    m_colorObsButton->setDisabled(true);
                    m_hatchObsButton->setDisabled(true);
                }
            }
            else
            {
                m_colorButton->setDisabled(true);
                m_hatchButton->setDisabled(true);
                m_colorObsButton->setDisabled(true);
                m_hatchObsButton->setDisabled(true);
            }
#endif
            update();
            Logging::logVerbose("Selected site %s\n", site->name().c_str());
        }
    }
}