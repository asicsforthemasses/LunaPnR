// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QDialog>
#include <QGridLayout>
#include <QComboBox>
#include "common/database.h"
#include "lunacore.h"

namespace GUI
{

class CTSDialog : public QDialog
{
    Q_OBJECT
public:
    CTSDialog(Database &db, QWidget *parent = nullptr);
    virtual ~CTSDialog() = default;

protected slots:
    void accept() override;

protected:
    void populateBufferList();

    Database &m_db;
    QComboBox   *m_ctsBufferCombo{nullptr};
    QLineEdit   *m_ctsMaxCap{nullptr};
};

};
