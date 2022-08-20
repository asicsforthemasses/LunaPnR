#include <sstream>
#include <QHeaderView>

#include "floorplandialog.h"
#include "common/guihelpers.h"
#include "widgets/rectdelegate.h"
#include "widgets/flatimagebutton.h"

using namespace GUI;

FloorplanDialog::FloorplanDialog(Database &db, QWidget *parent) : QDialog(parent), m_db(db)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setWindowTitle(tr("Floorplan Setup"));

    auto layout = new QGridLayout();
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    m_regionTable = new QTableWidget();    
    m_regionTable->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    m_regionTable->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);

    auto addRowButton = new FlatImageButton("://images/add.png");

    layout->addWidget(addRowButton,0,0,Qt::AlignRight);
    layout->addWidget(m_regionTable,1,0);

    QStringList headerLabels = {"Region name", "Site name", "Size (nm)", "Halo (nm)"};

    m_regionTable->setRowCount(db.m_projectSetup.m_regions.size());
    m_regionTable->setColumnCount(4);
    m_regionTable->setHorizontalHeaderLabels(headerLabels);
    
    // dunno why this fails:
    // the dialog will be almost infinitely wide.. ?
    //m_regionTable->horizontalHeader()->setStretchLastSection(true);

    auto rectDelegate = new RectDelegate();
    m_regionTable->setItemDelegateForColumn(2, rectDelegate);

    size_t row = 0;
    for(auto const& region : db.m_projectSetup.m_regions)
    {
        createTableRow(row, region);
        row++;
    }

    setLayout(layout);

    connect(addRowButton, &FlatImageButton::clicked, this, &FloorplanDialog::onAddRegionRow);
}

void FloorplanDialog::createTableRow(size_t row, const RegionSetup &region)
{
    auto newItem = new QTableWidgetItem(toQString(region.m_regionName));
    m_regionTable->setItem(row, 0, newItem);

    newItem = new QTableWidgetItem(toQString(region.m_site));
    m_regionTable->setItem(row, 1, newItem);

    newItem = new QTableWidgetItem(toQString(region.m_regionSize));
    m_regionTable->setItem(row, 2, newItem);

    newItem = new QTableWidgetItem(toQString(region.m_regionHalo));
    m_regionTable->setItem(row, 3, newItem);
}

void FloorplanDialog::onAddRegionRow()
{
    std::cout << "addRow called\n";
    m_db.m_projectSetup.m_regions.emplace_back();

    auto newRowNumber = m_regionTable->rowCount();

    m_regionTable->insertRow(newRowNumber);
    createTableRow(newRowNumber, m_db.m_projectSetup.m_regions.back());
}
