// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QTreeView>
#include <QAction>
#include <QMenu>
#include <QString>
#include <QStringList>
#include "../models/filesetupmodel.h"

namespace GUI
{
class FileSetupManager : public QTreeView
{
    Q_OBJECT
public:
    FileSetupManager(QWidget *parent = nullptr);

    /** add a file category with a single file extension */
    void addCategory(const QString &name, const QString &fileExt,
        std::vector<std::string > *data);

    /** add a file category with multiple file extensions */
    void addCategory(const QString &name, const QStringList &fileExt,  
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
