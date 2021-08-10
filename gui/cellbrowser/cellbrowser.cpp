#include "cellbrowser.h"
#include <QHeaderView>
#include <QLabel>

using namespace GUI;

CellBrowser::CellBrowser(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    // cell table view

    m_cellTableView = new QTableView(parent);
    m_cellTableView->setSelectionBehavior(QTableView::SelectRows);
    m_cellTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_cellTableView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_cellTableView->horizontalHeader()->setStretchLastSection(true);
    m_cellLayoutView = new CellLayoutView(parent);

    m_cellModel.reset(new CellLibTableModel(nullptr));
    m_cellTableView->setModel(m_cellModel.get());
    
    // pin list view
    m_cellTreeView = new QTreeView();
    m_cellTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers); // make read-only
    m_cellTreeView->setHeaderHidden(true);

    m_cellInfoModel.reset(new CellInfoModel());
    m_cellTreeView->setModel(m_cellInfoModel.get());

    m_layout2 = new QVBoxLayout();
    m_layout2->addWidget(new QLabel("Cell information"),0);
    m_layout2->addWidget(m_cellTreeView,1);

    m_layout = new QHBoxLayout();
    m_layout->addWidget(m_cellTableView,1);
    m_layout->addWidget(m_cellLayoutView,2);
    m_layout->addLayout(m_layout2,1);

    setLayout(m_layout);

    connect(m_cellTableView->selectionModel(), 
        SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
        this,
        SLOT(onCellSelectionChanged(const QItemSelection&, const QItemSelection&)));
}

CellBrowser::~CellBrowser()
{

}

QSize CellBrowser::sizeHint() const
{
    return m_cellTableView->sizeHint();
}

void CellBrowser::setCellLib(ChipDB::CellLib *cellLib)
{
    m_cellModel->setCellLib(cellLib);

    auto cellPtr = cellLib->m_cells.at(0);

    m_cellLayoutView->setCell(cellPtr);
    m_cellInfoModel->setCell(cellPtr);
}

void CellBrowser::onCellSelectionChanged(const QItemSelection &cur, const QItemSelection &prev)
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

