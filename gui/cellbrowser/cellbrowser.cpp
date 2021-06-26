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

    // pin group box
    m_pinGroup = new QFrame();
    //m_pinGroup->setFrameStyle(QFrame::Plain);
    m_pinGroup->setFrameStyle(QFrame::StyledPanel);
    
    // pin list view
    m_pinListView = new QListView();
    m_pinListView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pinListView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    m_pinModel.reset(new CellPinListModel(nullptr));
    m_pinListView->setModel(m_pinModel.get());

    // pin info table view
#if 0
    m_pinInfoTable = new QTableView();
    m_pinInfoTable->horizontalHeader()->setStretchLastSection(true);
    m_pinInfoTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pinInfoTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    m_pinInfoModel.reset(new PinInfoTableModel(nullptr));
    m_pinInfoTable->setModel(m_pinInfoModel.get());
#else
    m_pinInfoView = new PropertyView();

#endif
    m_layout2 = new QVBoxLayout();
    m_layout2->addWidget(new QLabel("Pins"),0);
    m_layout2->addWidget(m_pinListView,1);
    //m_layout2->addWidget(m_pinInfoTable,1);
    m_layout2->addWidget(m_pinInfoView,1);

    m_pinGroup->setLayout(m_layout2);

    m_layout = new QHBoxLayout();
    m_layout->addWidget(m_cellTableView,1);
    m_layout->addWidget(m_cellLayoutView,2);
    m_layout->addWidget(m_pinGroup,1);

    setLayout(m_layout);

    connect(m_cellTableView->selectionModel(), 
        SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
        this,
        SLOT(onCellSelectionChanged(const QItemSelection&, const QItemSelection&)));

    connect(m_pinListView->selectionModel(), 
        SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), 
        this,
        SLOT(onPinSelectionChanged(const QItemSelection&, const QItemSelection&)));
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

void CellBrowser::onCellSelectionChanged(const QItemSelection &cur, const QItemSelection &prev)
{
    QModelIndex index = m_cellTableView->currentIndex();

    if (index.isValid())
    {        
        auto cell = m_cellModel->getCell(index.row());
        if (cell != nullptr)
        {
            m_cellLayoutView->setCell(cell);
            m_pinModel->setCell(cell);
            m_pinListView->setCurrentIndex(m_pinModel->index(0));
            update();
            doLog(LOG_VERBOSE, "Selected cell %s\n", cell->m_name.c_str());
        }
    }
}

void CellBrowser::onPinSelectionChanged(const QItemSelection &cur, const QItemSelection &prev)
{
    QModelIndex index = m_pinListView->currentIndex();

    if (index.isValid())
    {        
        auto pinInfo = m_pinModel->getPinInfo(index.row());
        if ((pinInfo != nullptr) && m_pinInfoModel)
        {
            m_pinInfoModel->setPinInfo(pinInfo);
            update();
            doLog(LOG_VERBOSE, "Selected pin %s\n", pinInfo->m_name.c_str());
        }
    }
}
