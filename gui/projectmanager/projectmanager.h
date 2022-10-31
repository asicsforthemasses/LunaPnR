// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QEvent>
#include <QWidget>
#include <QBoxLayout>
#include <vector>
#include "../common/database.h"
#include "../widgets/flatactiontile.h"
#include "filesetupmanager.h"

namespace GUI
{

class ProjectManager : public QWidget
{
    Q_OBJECT
public:
    ProjectManager(Database &db, QWidget *parent = nullptr);

    void repopulate();

signals:
    void onAction(QString actionName);
    
protected slots:
    void onFloorplanSetup(QString actionName);
    void onWriteToDef(QString actionName);
    void onCTSSetup(QString actionName);

protected:
    bool event(QEvent * event) override;
    void create();    
    
    Database &m_db;
    ProjectSetup &m_projectSetup;

    QVBoxLayout *m_managerLayout = nullptr;
    
    GUI::FileSetupManager *m_fileSetupManager = nullptr;
    std::vector<FlatTileBase*> m_tiles;
};

}