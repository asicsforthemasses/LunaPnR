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

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    auto mainLayout = new QVBoxLayout();    
    m_tileList = new QScrollArea();
    m_tileList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    
    auto tileLayout = new QVBoxLayout();
    tileLayout->setSizeConstraint(QLayout::SetFixedSize);

    int id = 0;
    for(auto const &pdk : pdks)
    {
        auto tile = new PDKTile(pdk);
        m_pdkTiles.push_back(tile);        
        tile->setID(id);

        connect(tile, &PDKTile::clicked, this, &PDKDialog::onTileClicked);
        tileLayout->addWidget(tile);
        id++;
    }

    m_tileList->setLayout(tileLayout);
    mainLayout->addWidget(m_tileList);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(m_buttonBox);
    
    setLayout(mainLayout);
}

void PDKDialog::onTileClicked(int id)
{
    // de-select previous
    if (m_selected >= 0)
    {
        m_pdkTiles.at(m_selected)->setSelected(false);
    }

    if (id >= 0)
    {
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        m_pdkTiles.at(id)->setSelected(true);
    }

    m_selected = id;
}

};
