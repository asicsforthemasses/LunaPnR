// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "hatchdisplay.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QVariant>

using namespace GUI;

HatchDisplay::HatchDisplay(HatchLibrary &hatchLibrary, QWidget *parent) 
    : QScrollArea(parent), m_hatchLibrary(hatchLibrary)
{
    setWidgetResizable(true);

    QWidget *widget = new QWidget();

    auto mainLayout = new QVBoxLayout();
    widget->setLayout(mainLayout);

    setWidget(widget);

    int hatchIndex = 0;
    for(auto const& hatchPixmap : m_hatchLibrary.m_hatches)
    {
        auto hatchButton = new QPushButton();
        hatchButton->setFlat(true);
        hatchButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        hatchButton->setIcon(QIcon(hatchPixmap));
        hatchButton->setIconSize(hatchPixmap.rect().size());
        hatchButton->setProperty("hatchindex", QVariant(hatchIndex));

        hatchIndex++;

        mainLayout->addWidget(hatchButton);

        connect(hatchButton, SIGNAL(clicked()), this, SLOT(onHatchClick()));
    }
}

void HatchDisplay::onHatchClick()
{
    QPushButton* buttonSender = qobject_cast<QPushButton*>(sender());
    if (buttonSender != nullptr)
    {
        auto index = buttonSender->property("hatchindex").toInt();
        emit clicked(index);
    }
}
