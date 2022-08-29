// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "aboutdialog.h"
#include <QBoxLayout>
#include <QMessageBox>
#include "widgets/flatimage.h"

using namespace GUI;

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
    m_tabWidget = new QTabWidget;
    m_tabWidget->addTab(new AboutGeneralTab(this), tr("General"));
    m_tabWidget->addTab(new AboutQtTab(this), tr("Qt"));

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);    

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_tabWidget);
    mainLayout->addWidget(m_buttonBox);
    setLayout(mainLayout);    

    setWindowTitle(tr("About Dialog"));
}

AboutGeneralTab::AboutGeneralTab(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    auto logo = new FlatImage("://images/lunapnrlogo.png");

    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(logo);
    setLayout(mainLayout);
}

AboutQtTab::AboutQtTab(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    auto logo = new FlatImage(":/qt-project.org/qmessagebox/images/qtlogo-64.png");

    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(logo);

#if 0
    auto logo = new FlatImage("://images/lunapnrlogo.png");

    mainLayout->contentsMargins(0,0,0,0);
    mainLayout->addWidget(logo);
#endif

    setLayout(mainLayout);
}
