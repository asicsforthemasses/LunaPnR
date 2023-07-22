// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <QVBoxLayout>
#include <QPushButton>
#include "hatchdialog.h"
#include "hatchdisplay.h"
#include "../common/logging.h"

using namespace GUI;

HatchDialog::HatchDialog(HatchLibrary &hatchLibrary, QWidget *parent, Qt::WindowFlags f) 
    : QDialog(parent, f), m_hatchIndex(-1)
{
    auto topLayout = new QVBoxLayout();

    auto display = new HatchDisplay(hatchLibrary, this);
    topLayout->addWidget(display, 1);

    auto buttonLayout = new QHBoxLayout();

    auto okButton     = new QPushButton(tr("Ok"));
    auto cancelButton = new QPushButton(tr("Cancel"));

    buttonLayout->addWidget(cancelButton, 0);
    buttonLayout->addWidget(okButton, 0);

    topLayout->addLayout(buttonLayout);

    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()) );
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()) );
    connect(display, SIGNAL(clicked(int)), this, SLOT(onHatchClicked(int)));
    
    setLayout(topLayout);
    setWindowTitle(tr("Choose new layer hatch pattern"));
}


void HatchDialog::onHatchClicked(int index)
{
    m_hatchIndex = index;
    Logging::doLog(Logging::LogType::VERBOSE, "onHatchClicked index = %d\n", index);
    accept();
}
