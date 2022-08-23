// SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "configurationdialog.h"

#include <iostream>
#include <QLabel>
#include <QString>
#include <QPushButton>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QDialogButtonBox>
#include <QGridLayout>

using namespace GUI;

ConfigurationDialog::ConfigurationDialog(Database &db, QWidget *parent)
    : QDialog(parent), m_db(db)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QFileIconProvider iconProvider;
    setWindowTitle("Luna Configuration");

    auto layout = new QGridLayout();
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    m_openSTALocationEdit = new QLineEdit(QString::fromStdString(db.m_projectSetup.m_openSTALocation));
    m_openSTALocationEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    auto openSTALocationButton = new QPushButton(iconProvider.icon(QFileIconProvider::Folder),"");

    layout->addWidget(new QLabel("OpenSTA location"),0,0);
    layout->addWidget(m_openSTALocationEdit,0,1);
    layout->addWidget(openSTALocationButton,0,2);

    connect(openSTALocationButton, &QPushButton::clicked, this, &ConfigurationDialog::onOpenSTALocationOpen);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(buttonBox, 1, 0, 1, 3);
    setLayout(layout);
}

void ConfigurationDialog::onOpenSTALocationOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("OpenSTA location"),
                                                m_openSTALocationEdit->text(),
                                                tr("OpenSTA"));
    if (!fileName.isEmpty())
    {
        m_openSTALocationEdit->setText(fileName);
    }
}

void ConfigurationDialog::accept()
{
    //std::cout << "ConfigurationDialog::accept\n";
    m_db.m_projectSetup.m_openSTALocation = m_openSTALocationEdit->text().toStdString();
    QDialog::accept();
}
