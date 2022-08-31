// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QTreeView>
#include <QAction>
#include <QMenu>
#include "../models/filesetupmodel.h"

namespace GUI
{
class FileSetupManager : public QTreeView
{
    Q_OBJECT
public:
    FileSetupManager(QWidget *parent = nullptr);

    void addCategory(const QString &name, const QString &fileExt,  
        std::vector<std::string > *data);

    void addCategoryItem(size_t categoryIndex, const QString &name);

    void repopulate();

protected:
    FileSetupModel m_model;
    void onContextMenuRequested(const QPoint &point);
    void onAddResource();
    void onRemoveResource();

    QModelIndex m_itemModelIndex;

    QAction *m_addResourceAction;
    QAction *m_removeResourceAction;

    QMenu *m_categoryContextMenu;
    QMenu *m_itemContextMenu;
};

};
