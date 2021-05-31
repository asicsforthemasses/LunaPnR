#include "cellbrowser.h"

using namespace GUI;

CellBrowser::CellBrowser(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_cellTableView = new QTableView(parent);
    m_cellTableView->setSelectionBehavior(QTableView::SelectRows);
    m_cellTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_cellTableView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_cellLayoutView = new CellLayoutView(parent);

    m_cellModel.reset(new CellLibTableModel(nullptr));
    m_cellTableView->setModel(m_cellModel.get());

    m_pinListView = new QListView(parent);

    m_pinModel.reset(new CellPinListModel(nullptr));
    m_pinListView->setModel(m_pinModel.get());

    m_layout = new QHBoxLayout(this);
    m_layout->addWidget(m_cellTableView,0);
    m_layout->addWidget(m_cellLayoutView,1);
    m_layout->addWidget(m_pinListView,0);

    auto ptr = m_cellTableView->selectionModel();

    setLayout(m_layout);

    connect(m_cellTableView->selectionModel(), 
        SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
        this,
        SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
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
    m_pinModel->setCell(cellPtr);
}

void CellBrowser::onSelectionChanged(const QItemSelection &cur, const QItemSelection &prev)
{
    QModelIndex index = m_cellTableView->currentIndex();

    if (index.isValid())
    {        
        auto cell = m_cellModel->getCell(index.row());
        if (cell != nullptr)
        {
            m_cellLayoutView->setCell(cell);
            m_pinModel->setCell(cell);
            update();
            doLog(LOG_VERBOSE, "Selected cell %s\n", cell->m_name.c_str());
        }
    }
}
