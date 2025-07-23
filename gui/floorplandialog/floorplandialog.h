// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QDialog>
#include <QGridLayout>
#include <QTableWidget>
#include "common/database.h"
#include "lunacore.h"

namespace GUI
{

class FloorplanDialog : public QDialog
{
    Q_OBJECT
public:
    FloorplanDialog(Database &db, QWidget *parent = nullptr);
    virtual ~FloorplanDialog() = default;

protected slots:
    void onAddRegionRow();
    void accept() override;

protected:
    void onOpenScriptLocationOpen();
    void createTableRow(size_t row, const RegionSetup &region);

    Database &m_db;
    QTableWidget *m_regionTable;
    QLineEdit    *m_floorplanScriptEdit;
};

};