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

    m_layerTableView = new QTableView(this);
    m_layerTableView->setSelectionBehavior(QTableView::SelectRows);
    m_layerTableView->setSelectionMode(QAbstractItemView::SingleSelection);

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

    m_mainLayout = new QHBoxLayout();
    m_mainLayout->addWidget(m_layerTableView,1);
    m_mainLayout->addWidget(m_layerTreeView,2);

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

    m_mainLayout->addLayout(renderInfoBox);

    //m_layout->addWidget(m_cellLayoutView,2);
    //m_layout->addLayout(m_layout2,1);

    setLayout(m_mainLayout);

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
}

TechBrowser::~TechBrowser()
{
}

QSize TechBrowser::sizeHint() const
{
    return m_layerTreeView->sizeHint();
}

void TechBrowser::setDatabase(Database *db)
{
    if (m_db != nullptr)
    {
        m_db->techLib().m_layers.removeListener(m_layerInfoModel.get());
    }

    m_db = db;

    if (m_db == nullptr)
    {
        m_layerInfoModel->setLayer(nullptr);
        m_layerTableModel->setTechLib(nullptr);
        return;
    }

    m_db->techLib().m_layers.addListener(m_layerInfoModel.get());

    auto layer = m_db->techLib().m_layers.at(0);
    m_layerInfoModel->setLayer(layer);
    m_layerTableModel->setTechLib(&m_db->techLib());

    // make sure all columns can expand
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
    m_layerTreeView->expandAll();    
    
    m_layerTableView->selectRow(0);
}

void TechBrowser::refreshDatabase()
{
    if (m_db != nullptr)
    {
        // make sure that every layer in the tech library
        // has a corresponding layer in the layer render library

        for(auto techLayer : m_db->techLib().m_layers)
        {   
            m_db->m_layerRenderInfoDB.createLayer(techLayer->m_name);
        }
                
        m_layerTableModel->setTechLib(&m_db->techLib());
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
                if (m_db->m_layerRenderInfoDB.size() != m_db->techLib().m_layers.size())
                {
                    refreshDatabase();
                }

                auto info = m_db->m_layerRenderInfoDB.lookup(layer->m_name);
                if (info != nullptr)
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
            doLog(LOG_VERBOSE, "Selected layer %s\n", layer->m_name.c_str());
        }
    }
}

void TechBrowser::onLayerColorChanged()
{
    QModelIndex index = m_layerTableView->currentIndex();
    auto layer = m_layerTableModel->getLayer(index.row());
    
    if ((layer != nullptr) && (m_db != nullptr))
    {
        auto info = m_db->m_layerRenderInfoDB.lookup(layer->m_name);
        if (info != nullptr)
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
                auto info = m_db->m_layerRenderInfoDB.lookup(layer->m_name);
                if (info != nullptr)
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
        auto info = m_db->m_layerRenderInfoDB.lookup(layer->m_name);
        if (info != nullptr)
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
                auto info = m_db->m_layerRenderInfoDB.lookup(layer->m_name);
                if (info != nullptr)
                {
                    info->obstruction().setTexture(hatch);
                    m_db->m_layerRenderInfoDB.contentsChanged();
                }
            }            
        }        
    } 
}
