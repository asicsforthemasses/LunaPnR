// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "version.h"
#include "aboutdialog.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QTextEdit>
#include <patchlevel.h>     // from Python
#include "widgets/flatimage.h"
#include <Eigen/Core>
#include <toml++/toml.h>

using namespace GUI;

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
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

    const auto tomlVersion = QString::asprintf("%d.%d.%d", 
        TOML_LIB_MAJOR,
        TOML_LIB_MINOR,
        TOML_LIB_PATCH);

    // build information string    
    info += "<h2>" + QString(LUNAVERSIONSTRING) + "</h2>";
    info += "<h3>License: GPL v3</h3>";
    info += "Compiled on " __DATE__ " using " + QString(COMPILERVERSIONSTRING) + "<br>";
    info += "Additional libraries:";
    info += "<ul>";
    info += "<li>Qt version     " + QString(QT_VERSION_STR) + "</li>";
    info += "<li>Python version " + QString(PY_VERSION) + "</li>";
    info += "<li>Eigen3 version " + eigenVersion + "</li>";
    info += "<li>Toml++ version " + tomlVersion + "</li>";
    info += "</ul>";

    textDisplay->setHtml(info);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    //mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->addWidget(logo);
    mainLayout->addWidget(textDisplay);
    mainLayout->addWidget(m_buttonBox);
    setLayout(mainLayout);
    
    setWindowTitle(tr("About Dialog"));
}
