// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "version.h"
#include "aboutdialog.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QTextEdit>

#ifdef USE_PYTHON
#include <patchlevel.h>     // from Python
#endif

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
    info += "<tt><h2>" + QString(LUNAVERSIONSTRING) + "</h2>";
    info += "<h3>License: GPL v3</h3>";
    info += "Compiled on " __DATE__ "<br>using <small>" + QString(COMPILER_VERSION) + "</small><br>";
    info += "<tt><br>\n";
    info += "GIT:<br>\n";
    info += "  Rev    : " + QString(GIT_REV) + "<br>\n";
    info += "  Branch : '" + QString(GIT_BRANCH) + "'<br>\n";

    QString gitTag{GIT_TAG};
    if (!gitTag.isEmpty())
    {
        info += info += "  Tag    : " + gitTag;
    }

    info.replace(" " , "&nbsp;");

    info += "</tt><br>\n";
    info += "Additional libraries:";
    info += "<ul>";
    info += "<li>Qt version     " + QString(QT_VERSION_STR) + "</li>";

#ifdef USE_PYTHON
    info += "<li>Python version " + QString(PY_VERSION) + "</li>";
#endif

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
