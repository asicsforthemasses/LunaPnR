// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <sstream>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QDoubleValidator>
#include <QLineEdit>
#include <QLabel>

#include "ctsdialog.h"
#include "common/guihelpers.h"
#include "widgets/rectdelegate.h"
#include "widgets/flatimagebutton.h"

using namespace GUI;

CTSDialog::CTSDialog(Database &db, QWidget *parent) : QDialog(parent), m_db(db)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setWindowTitle(tr("Clock Tree Synthesis Setup"));

    auto layout = new QGridLayout();
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    m_ctsBufferCombo = new QComboBox();
    m_ctsBufferCombo->setEditable(false);

    populateBufferList();

    int comboIndex = m_ctsBufferCombo->findText(QString::fromStdString(db.m_projectSetup.m_ctsBuffer));

    if (comboIndex >= 0)
    {
        m_ctsBufferCombo->setCurrentIndex(comboIndex);
    }
    else
    {
        m_ctsBufferCombo->setCurrentIndex(0);
    }

    m_ctsMaxCap     = new QLineEdit();
    m_ctsMaxCap->setText(QString::asprintf("%e", db.m_projectSetup.m_ctsMaxCap));
    m_ctsMaxCap->setValidator(new QDoubleValidator(10e-15, 1, 3));

    layout->addWidget(new QLabel("Buffer name"), 0,0);
    if (m_ctsBufferCombo->count() > 0) layout->addWidget(m_ctsBufferCombo, 0, 1);
    layout->addWidget(new QLabel("Max capacitance [F]"), 1,0);
    layout->addWidget(m_ctsMaxCap, 1, 1);

    // standard dialog box buttons
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    layout->addWidget(buttonBox, 5,0,1,3);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &CTSDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CTSDialog::reject);

    setLayout(layout);
}

void CTSDialog::populateBufferList()
{
    auto cellLib = m_db.cellLib();
    if (!cellLib) return;

    for(auto cell : *cellLib)
    {
        int inputs{0};
        int outputs{0};
        for(auto pin : cell->m_pins)
        {
            if (pin->isInput()) inputs++;
            else if (pin->isOutput()) outputs++;
        }

        if ((inputs==1) && (outputs==1))
        {
            m_ctsBufferCombo->addItem(QString::fromStdString(cell->name()));
        }
    }
}

void CTSDialog::accept()
{
    try
    {
        m_db.m_projectSetup.m_ctsMaxCap = std::stof(m_ctsMaxCap->text().toStdString());
    }
    catch(std::invalid_argument &e)
    {
        Logging::doLog(Logging::LogType::ERROR, "Invalid CTS max cap value");
        QDialog::reject();
    }

    if (m_ctsBufferCombo->count() > 0)
    {
        auto bufferName = m_ctsBufferCombo->currentText();
        m_db.m_projectSetup.m_ctsBuffer = bufferName.toStdString();
    }

    QDialog::accept();
}