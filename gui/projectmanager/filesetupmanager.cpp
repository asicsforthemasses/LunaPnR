// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <QMenu>
#include <QFileDialog>
#include "filesetupmanager.h"

using namespace GUI;

FileSetupManager::FileSetupManager(QWidget *parent) : QTreeView(parent)
{
    setModel(&m_model);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeView::customContextMenuRequested, this, &FileSetupManager::onContextMenuRequested);

    m_addResourceAction = new QAction("Add Resource...");
    m_removeResourceAction = new QAction("Remove");

    connect(m_addResourceAction, &QAction::triggered, this, &FileSetupManager::onAddResource);
    connect(m_removeResourceAction, &QAction::triggered, this, &FileSetupManager::onRemoveResource);

    // create the context menus
    m_categoryContextMenu = new QMenu();
    m_categoryContextMenu->addAction(m_addResourceAction);

    m_itemContextMenu = new QMenu();
    m_itemContextMenu->addAction(m_removeResourceAction);
}

void FileSetupManager::addCategory(const QString &name, const QString &fileExt, std::vector<std::string > *data)
{
    m_model.addCategory(name, fileExt, data);
}

void FileSetupManager::addCategoryItem(size_t categoryIndex, const QString &name)
{
    m_model.addCategoryItem(categoryIndex, name);
}

void FileSetupManager::onContextMenuRequested(const QPoint &point)
{
    m_itemModelIndex = indexAt(point);
    if (m_itemModelIndex.isValid())
    {
        if (m_model.isCategory(m_itemModelIndex))
        {
            m_categoryContextMenu->exec(viewport()->mapToGlobal(point));
        }
        else
        {
            m_itemContextMenu->exec(viewport()->mapToGlobal(point));
        }
    }
}

void FileSetupManager::onAddResource()
{
    QString extensions = tr("Files (");

    for(auto const& cat : m_model.categories())
    {
        extensions.append("*");
        extensions.append(cat.m_extension);
        extensions.append(" ");
    }
    extensions.append(")");

    QString fileName = QFileDialog::getOpenFileName(this, tr("Add resource"),
        "", extensions);

    if (!fileName.isEmpty())
    {
        size_t catIndex = 0;
        for(auto const& cat : m_model.categories())
        {
            if (fileName.endsWith(cat.m_extension))
            {
                m_model.addCategoryItem(catIndex, fileName);
                return;
            }
            catIndex++;
        }
    }
}

void FileSetupManager::onRemoveResource()
{
    auto itemRow = m_itemModelIndex.row();
    m_model.removeRow(itemRow, m_itemModelIndex.parent());
}

void FileSetupManager::repopulate()
{
    m_model.repopulate();
}
