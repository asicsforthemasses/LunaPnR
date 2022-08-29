// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "aboutdialog.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QTextEdit>
#include <patchlevel.h>     // from Python
#include "widgets/flatimage.h"
#include <Eigen/Core>

using namespace GUI;

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
    m_tabWidget = new QTabWidget;
    m_tabWidget->addTab(new AboutGeneralTab(this), tr("General"));
    m_tabWidget->addTab(new AboutQtTab(this), tr("Qt"));

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    m_buttonBox->addButton(tr("Donate"), QDialogButtonBox::InvalidRole); 

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

    auto textDisplay = new QTextEdit();
    textDisplay->setReadOnly(true);

    QString info;

    const auto eigenVersion = QString::asprintf("%d.%d.%d", 
        EIGEN_WORLD_VERSION,
        EIGEN_MAJOR_VERSION,
        EIGEN_MINOR_VERSION);

    // build information string    
    info += "<h2>" LUNAVERSIONSTRING "</h2>";
    info += "<h3>License: GPL v3</h3>";
    info += "Compiled on " __DATE__ " using " COMPILERVERSIONSTRING "<br>";
    info += "Additional libraries:";
    info += "<ul>";
    info += "<li>Qt version     " QT_VERSION_STR "</li>";
    info += "<li>Python version " PY_VERSION "</li>";
    info += "<li>Eigen3 version " + eigenVersion + "</li>";
    info += "</ul>";

    textDisplay->setHtml(info);

    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(logo);
    mainLayout->addWidget(textDisplay);
    setLayout(mainLayout);
}

AboutQtTab::AboutQtTab(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    auto logo = new FlatImage(":/qt-project.org/qmessagebox/images/qtlogo-64.png");

    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(logo);

    setLayout(mainLayout);
}
