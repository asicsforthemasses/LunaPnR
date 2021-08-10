#include "techbrowser.h"
#include <QHeaderView>
#include <QLabel>

using namespace GUI;

TechBrowser::TechBrowser(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    // cell table view

#if 0
    m_layerTreeView = new QTableView(parent);
    m_layerTreeView->setSelectionBehavior(QTableView::SelectRows);
    m_layerTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_layerTreeView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_layerTreeView->horizontalHeader()->setStretchLastSection(true);
    m_layerTreeView = new CellLayoutView(parent);

    m_cellModel.reset(new CellLibTableModel(nullptr));
    m_cellTableView->setModel(m_cellModel.get());
#endif


    // pin list view
    m_layerTreeView = new QTreeView();
    m_layerTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers); // make read-only
    m_layerTreeView->setHeaderHidden(true);

    m_layerInfoModel.reset(new LayerInfoModel());
    m_layerTreeView->setModel(m_layerInfoModel.get());

    //m_layout2 = new QVBoxLayout();
    //m_layout2->addWidget(new QLabel("Cell information"),0);
    //m_layout2->addWidget(m_cellTreeView,1);

    m_layout = new QHBoxLayout();
    m_layout->addWidget(m_layerTreeView,1);
    //m_layout->addWidget(m_cellLayoutView,2);
    //m_layout->addLayout(m_layout2,1);

    setLayout(m_layout);

#if 0
    connect(m_cellTableView->selectionModel(), 
        SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
        this,
        SLOT(onCellSelectionChanged(const QItemSelection&, const QItemSelection&)));
#endif
}

TechBrowser::~TechBrowser()
{

}

QSize TechBrowser::sizeHint() const
{
    return m_layerTreeView->sizeHint();
}

void TechBrowser::setTechLib(ChipDB::TechLib *techLib)
{
    auto layer = techLib->m_layers.at(0);
    m_layerInfoModel->setLayer(layer);
}

#if 0
void TechBrowser::onCellSelectionChanged(const QItemSelection &cur, const QItemSelection &prev)
{
    QModelIndex index = m_cellTableView->currentIndex();

    if (index.isValid())
    {        
        auto cell = m_cellModel->getCell(index.row());
        if (cell != nullptr)
        {
            m_cellLayoutView->setCell(cell);
            m_cellInfoModel->setCell(cell);
            update();
            doLog(LOG_VERBOSE, "Selected cell %s\n", cell->m_name.c_str());
        }
    }
}
#endif