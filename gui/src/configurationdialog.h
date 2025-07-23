// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QDialog>
#include <QLineEdit>
#include "common/database.h"

namespace GUI
{

class ConfigurationDialog : public QDialog
{
    Q_OBJECT
public:
    ConfigurationDialog(Database &db, QWidget *parent = nullptr);

    QString getOpenSTALocation() const noexcept;

protected slots:
    void accept() override;

protected:
    void onOpenSTALocationOpen();
    QLineEdit   *m_openSTALocationEdit;
    Database    &m_db;
};

};
