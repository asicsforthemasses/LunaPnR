// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <sstream>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QFileIconProvider>
#include <QLineEdit>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>

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

    layout->addWidget(addRowButton,0,0,1,3,Qt::AlignRight);
    layout->addWidget(m_regionTable,1,0,1,3);

    QStringList headerLabels = {"Region name", "Site name", "Size (nm)", "Halo (nm)"};

    m_regionTable->setRowCount(db.m_projectSetup.m_regions.size());
    m_regionTable->setColumnCount(4);
    m_regionTable->setHorizontalHeaderLabels(headerLabels);
    
    // dunno why this fails:
    // the dialog will be almost infinitely wide.. ?
    // m_regionTable->horizontalHeader()->setStretchLastSection(true);

    auto rectDelegate = new RectDelegate();
    m_regionTable->setItemDelegateForColumn(2, rectDelegate);

    size_t row = 0;
    for(auto const& region : db.m_projectSetup.m_regions)
    {
        createTableRow(row, region);
        row++;
    }

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);


    QFileIconProvider iconProvider;
    m_floorplanScriptEdit = new QLineEdit(QString::fromStdString(db.m_projectSetup.m_floorplanScriptLocation));
    m_floorplanScriptEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    auto openScriptLocationButton = new QPushButton(iconProvider.icon(QFileIconProvider::Folder),"");
    connect(openScriptLocationButton, &QPushButton::clicked, this, &FloorplanDialog::onOpenScriptLocationOpen);

    layout->addWidget(new QLabel("Floorplan script location"),2,0);
    layout->addWidget(m_floorplanScriptEdit,2,1);
    layout->addWidget(openScriptLocationButton,2,2);

    // standard dialog box buttons
    layout->addWidget(buttonBox, 5,0,1,3);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &FloorplanDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FloorplanDialog::reject);

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

void FloorplanDialog::onOpenScriptLocationOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Floorplan python script location"),
                                                m_floorplanScriptEdit->text(),
                                                tr("Floorplan script (*.py)"));
    if (!fileName.isEmpty())
    {
        m_floorplanScriptEdit->setText(fileName);
    }    
}

void FloorplanDialog::accept()
{
    //std::cout << "ConfigurationDialog::accept\n";
    m_db.m_projectSetup.m_floorplanScriptLocation = m_floorplanScriptEdit->text().toStdString();
    QDialog::accept();
}