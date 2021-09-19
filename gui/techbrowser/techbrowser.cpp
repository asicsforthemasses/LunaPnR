#include "techbrowser.h"
#include "../widgets/hatchdialog.h"
#include <QHeaderView>
#include <QLabel>
#include <QColor>

using namespace GUI;

TechBrowser::TechBrowser(QWidget *parent) : QWidget(parent)
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

    m_colorButton = new SelectColorButton(this);
    m_hatchButton = new SelectHatchButton(this);

    m_layout = new QHBoxLayout();
    m_layout->addWidget(m_layerTableView,1);
    m_layout->addWidget(m_layerTreeView,2);
    m_layout->addWidget(m_hatchButton,0);
    m_layout->addWidget(m_colorButton,0);

    //m_layout->addWidget(m_cellLayoutView,2);
    //m_layout->addLayout(m_layout2,1);

    setLayout(m_layout);

    connect(m_layerTableView->selectionModel(), 
        SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
        this,
        SLOT(onLayerSelectionChanged(const QItemSelection&, const QItemSelection&)));

    connect(m_colorButton, 
        SIGNAL(onColorChanged()), 
        this,
        SLOT(onLayerColorChanged()));

    connect(m_hatchButton, 
        SIGNAL(onHatchChanged()), 
        this,
        SLOT(onLayerHatchChanged()));
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
    m_db = db;

    if (m_db == nullptr)
    {
        m_layerInfoModel->setLayer(nullptr);
        m_layerTableModel->setTechLib(nullptr);
        return;
    }

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
                auto info = m_db->m_layerRenderInfoDB.getRenderInfo(layer->m_name);
                if (info.has_value())
                {
                    m_hatchButton->setEnabled(true);
                    m_hatchButton->setEnabled(true);
                    m_colorButton->setColor(info->getBrush().color());
                    m_hatchButton->setHatch(info->getPixmap());
                }
                else
                {
                    m_colorButton->setDisabled(true);    
                    m_hatchButton->setDisabled(true);
                }
            }
            else
            {
                m_colorButton->setDisabled(true);
                m_hatchButton->setDisabled(true);
            }

            update();
            doLog(LOG_VERBOSE, "Selected layer %s\n", layer->m_name.c_str());
        }
    }
}

void TechBrowser::onLayerColorChanged()
{
    std::cout << "onLayerColorChanged called!\n";
    QModelIndex index = m_layerTableView->currentIndex();
    auto layer = m_layerTableModel->getLayer(index.row());
    
    if ((layer != nullptr) && (m_db != nullptr))
    {
        auto info = m_db->m_layerRenderInfoDB.getRenderInfo(layer->m_name);
        if (info)
        {
            info->setColor(m_colorButton->getColor());
            m_db->m_layerRenderInfoDB.setRenderInfo(layer->m_name, *info);
            std::cout << "new color set!\n";
        }        
    }
}

void TechBrowser::onLayerHatchChanged()
{
    std::cout << "onLayerHatchChanged called!\n";
    QModelIndex index = m_layerTableView->currentIndex();
    auto layer = m_layerTableModel->getLayer(index.row());
    
    if ((layer != nullptr) && (m_db != nullptr))
    {
        auto info = m_db->m_layerRenderInfoDB.getRenderInfo(layer->m_name);
        if (info)
        {
            info->setTexture(m_hatchButton->getHatch());
            m_db->m_layerRenderInfoDB.setRenderInfo(layer->m_name, *info);
            std::cout << "new hatch set!\n";
        }
    }
}
