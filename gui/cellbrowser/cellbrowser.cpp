#include "cellbrowser.h"
#include <QHeaderView>
#include <QLabel>
#include <QComboBox>
#include <QItemDelegate>
#include <QSortFilterProxyModel>

using namespace GUI;

SubclassDelegate::SubclassDelegate(QObject *parent) : QItemDelegate(parent)
{
}

QWidget* SubclassDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 2) 
    {
        auto classIndex = index.siblingAtColumn(1);
        if (index.model()->data(classIndex, Qt::EditRole) == "CORE")
        {
            auto box = new QComboBox(parent);
            box->addItem("NONE", static_cast<int>(ChipDB::CellSubclass::NONE));
            box->addItem("SPACER", static_cast<int>(ChipDB::CellSubclass::SPACER));
            box->addItem("FEEDTHRU", static_cast<int>(ChipDB::CellSubclass::FEEDTHRU));
            box->addItem("TIEHIGH", static_cast<int>(ChipDB::CellSubclass::TIEHIGH));
            box->addItem("TIELOW", static_cast<int>(ChipDB::CellSubclass::TIELOW));
            box->addItem("WELLTAP", static_cast<int>(ChipDB::CellSubclass::WELLTAP));
            box->addItem("ANTENNACELL", static_cast<int>(ChipDB::CellSubclass::ANTENNACELL));
            return box;
        }
        else
        {
            return nullptr;
        }
    } 
    else 
    {
        return QItemDelegate::createEditor(parent, option, index);
    }    
}

void SubclassDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto currentSubclassID = index.model()->data(index, Qt::UserRole).toInt();

    auto comboBox = static_cast<QComboBox*>(editor);
    if (comboBox != nullptr)
    {
        auto const itemCount = comboBox->count();
        for(int index=0; index<itemCount; index++)
        {
            bool ok = false;
            int subclassID = comboBox->itemData(index, Qt::UserRole).toInt(&ok);
            if ((subclassID == currentSubclassID) && ok)
            {
                comboBox->setCurrentIndex(index);
                return;
            }
        }
        comboBox->setCurrentIndex(0);
    }
}

void SubclassDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
    const QModelIndex &index) const
{
    auto comboBox = static_cast<QComboBox*>(editor);
    if (comboBox != nullptr)
    {
        auto subclassID = comboBox->currentData(Qt::UserRole);
        model->setData(index, subclassID, Qt::UserRole);
    }
}

CellBrowser::CellBrowser(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    // cell table view
    m_cellTableView = new QTableView(parent);
    m_cellTableView->setSelectionBehavior(QTableView::SelectRows);
    m_cellTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_cellTableView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_cellTableView->horizontalHeader()->setStretchLastSection(true);

    m_cellTableView->setItemDelegateForColumn(2, &m_subclassDelegate);

    m_cellLayoutView = new CellLayoutView(parent);

    m_cellModel.reset(new CellLibTableModel(nullptr));
    m_cellTableView->setModel(m_cellModel.get());
    
    // pin list view
    m_cellTreeView = new QTreeView();
    m_cellTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers); // make read-only
    //m_cellTreeView->setHeaderHidden(true);

    // layer appearance
    m_layerTableModel.reset(new LayerAppearanceTableModel());
    m_layerView = new QTableView();
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(m_layerTableModel.get());
    proxyModel->sort(0, Qt::AscendingOrder);
    m_layerView->setModel(proxyModel);

    m_cellInfoModel.reset(new CellInfoModel());
    m_cellTreeView->setModel(m_cellInfoModel.get());

    m_layout2 = new QVBoxLayout();
    m_layout2->addWidget(new QLabel("Cell information"),0);
    m_layout2->addWidget(m_cellTreeView,1);

    m_layout = new QHBoxLayout();
    m_layout->addWidget(m_cellTableView,2);
    m_layout->addWidget(m_cellLayoutView,4);
    m_layout->addWidget(m_layerView,1);
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

void CellBrowser::setDatabase(Database *db)
{
    m_db = db;
    
    m_cellLayoutView->setDatabase(db);
    m_cellModel->setCellLib(&db->cellLib());
    m_layerTableModel->setLayers(&db->m_layerRenderInfoDB);

    auto cellPtr = db->cellLib().m_cells.at(0);
    
    m_cellLayoutView->setCell(cellPtr);    
    m_cellInfoModel->setCell(cellPtr);

    // make sure all columns can expand
    for(size_t c=0; c < m_cellTreeView->header()->count(); c++)
    {
        m_cellTreeView->header()->setSectionResizeMode(
            c, QHeaderView::Stretch);
    }    

    m_layerView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_cellTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
}

void CellBrowser::refreshDatabase()
{
    setDatabase(m_db);
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
            m_layerView->update();   // ?? 
            update();
            doLog(LOG_VERBOSE, "Selected cell %s\n", cell->name().c_str());
        }
    }
}

