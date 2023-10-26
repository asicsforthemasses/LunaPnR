// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "pdkdialog.h"
#include <QBoxLayout>
#include <QPushButton>
#include "pdktile.h"

namespace GUI
{

PDKDialog::PDKDialog(std::vector<PDKInfo> &pdks) : m_pdks(pdks)
{
    setWindowTitle("Select PDK");
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    auto mainLayout = new QVBoxLayout();
    m_tileList = new PDKTileList();

    for(auto const &pdk : pdks)
    {
        m_tileList->createTile(pdk);
    }

    connect(m_tileList, &PDKTileList::selectionChanged, 
        this, &PDKDialog::onSelectionChanged);

    m_tileListScroll = new QScrollArea();
    m_tileListScroll->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    m_tileListScroll->setWidget(m_tileList);
    m_tileListScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_tileListScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    mainLayout->addWidget(m_tileListScroll,1);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(m_buttonBox, 0, Qt::AlignRight);
    
    setLayout(mainLayout);
}

void PDKDialog::onSelectionChanged(int id)
{
    if (id >= 0)
    {
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

};
