// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include <QFileInfo>
#include "filesetupmodel.h"

using namespace GUI;

// ******************************************************************************************
//   FileSetupModel
// ******************************************************************************************

FileSetupModel::FileSetupModel(QObject *parent) : QAbstractItemModel(parent)
{
    m_folderIcon = QPixmap("://images/folder.png");
    m_fileIcon = QPixmap("://images/file.png");
}

FileSetupModel::~FileSetupModel()
{
}

void FileSetupModel::repopulate()
{
    beginResetModel();
    endResetModel();
}

void FileSetupModel::addCategory(const QString &name, const QStringList &fileExt, std::vector<std::string > *data)
{
    beginInsertRows(QModelIndex(), m_categories.size(), m_categories.size());
    m_categories.emplace_back();
    m_categories.back().m_name = name;
    m_categories.back().m_extension = fileExt;
    m_categories.back().m_data = data;
    endInsertRows();
}

void FileSetupModel::addCategoryItem(size_t categoryIndex, const QString &data)
{
    if (categoryIndex < m_categories.size())
    {
        auto catDataPtr = m_categories.at(categoryIndex).m_data;
        
        if (catDataPtr == nullptr)
        {
            return;
        }

        const auto newRowIndex = catDataPtr->size();        

        auto parent = createIndex(categoryIndex, 0, 0xFFFFFFFF);
        if (insertRow(newRowIndex, parent))
        {
            catDataPtr->back() = data.toStdString();
        }
    }
}

bool FileSetupModel::isCategory(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return false;
    }

    return (index.internalId() == 0xFFFFFFFF);
}

QModelIndex FileSetupModel::index(int row, int column,
    const QModelIndex &parent) const
{
    //std::cout << "  FileSetupModel::index " << row << " " << column << "\n";
    if ((row<0) || (column != 0))
    {
        return QModelIndex();
    }

    if (isCategory(parent))
    {
        // make an item node where the internalId is set to 
        // the parent row. This allows finding the parent
        // easily.
        return createIndex(row, column, parent.row());
    }
    else
    {
        // return a category node
        if (row < m_categories.size())
        {
            return createIndex(row, column, 0xFFFFFFFF);
        }        
    }

    return QModelIndex();
}

QModelIndex FileSetupModel::parent(const QModelIndex &child) const
{
    //std::cout << "  FileSetupModel::parent " << child.row() << " " << child.column() << "\n";
    if (!child.isValid())
    {
        return QModelIndex();
    }

    if (isCategory(child))
    {
        return QModelIndex();   // return the root node index, which is invalid
    }

    // return a category node
    return createIndex(child.internalId(), 0, 0xFFFFFFFF);
}

int FileSetupModel::rowCount(const QModelIndex &parent) const
{   
    //std::cout << "  FileSetupModel::rowCount " << parent.row() << " " << parent.column() << "\n";
    if (!parent.isValid())
    {
        // this is the root node
        return m_categories.size();
    }
    else
    {
        if (isCategory(parent))
        {
            return m_categories.at(parent.row()).m_data->size();
        }
    }
    return 0;
}

int FileSetupModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

bool FileSetupModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!isCategory(parent))
    {
        return false;
    }

    auto categoryIndex = parent.row();
    if (categoryIndex >= m_categories.size())
    {
        return false;
    }
    
    auto categoryData = m_categories.at(categoryIndex).m_data;
    if (categoryData == nullptr)
    {
        return false;
    }

    const size_t lastErasedItemIndex = row + count - 1;

    if (categoryData->size() <= lastErasedItemIndex)
    {
        return false;   // not enough items to erase
    }

    beginRemoveRows(parent, row, lastErasedItemIndex);
    categoryData->erase(categoryData->begin() + row, categoryData->begin() + row + count);
    endRemoveRows();

    return true;
}

bool FileSetupModel::insertRows(int row, int count, const QModelIndex &parent) 
{
    if (parent.isValid() && isCategory(parent))
    {
        beginInsertRows(parent, row, row + count - 1);
        auto categoryIndex = parent.row();

        if (categoryIndex >= m_categories.size())
        {
            return false;
        }

        auto catDataPtr = m_categories.at(categoryIndex).m_data;

        if (catDataPtr == nullptr)
        {
            return false;
        }

        catDataPtr->push_back("UNDEFINED");

        endInsertRows();
        return true;
    }

    return false;
}

QVariant FileSetupModel::data(const QModelIndex &index, int role) const
{
    //std::cout << "  FileSetupModel::data " << index.row() << " " << index.column() << "\n";
    if (!index.isValid())
        return QVariant();

    if (isCategory(index))
    {
        return categoryData(m_categories.at(index.row()), role);
    }
    else
    {
        auto categoryIndex = index.internalId();
        return categoryItemData(m_categories.at(categoryIndex), index.row(), role);
    }
}

bool FileSetupModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
    {
        return false;
    }

    if (role != Qt::EditRole)
    {
        return false;
    }

    if (isCategory(index))
    {
        return false;
    }

    auto catIndex = index.internalId();
    if (catIndex >= m_categories.size())
    {
        return false;
    }

    auto itemIndex = index.row();
    if (itemIndex >= m_categories.at(catIndex).m_data->size())
    {
        return false;
    }

    m_categories.at(catIndex).m_data->at(itemIndex) = value.toString().toStdString();
    emit dataChanged(index,index);

    return true;
}

Qt::ItemFlags FileSetupModel::flags(const QModelIndex &index) const
{
    if (index.isValid())
    {
        if (!isCategory(index))
        {
            return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
        }

        return Qt::ItemIsEnabled;
    }

    return Qt::NoItemFlags;
}


QVariant FileSetupModel::categoryData(const Category &cat, int role) const
{
    switch(role)
    {
    case Qt::DisplayRole:
        return cat.m_name;
    case Qt::DecorationRole:
        return m_folderIcon;
    }
    return QVariant();
}

QVariant FileSetupModel::categoryItemData(const Category &cat, size_t index, int role) const
{
    switch(role)
    {
    case Qt::DisplayRole:
        return shortenFilename(QString::fromStdString(cat.m_data->at(index)));
    case Qt::DecorationRole:
        return m_fileIcon;
    case Qt::ToolTipRole:
        return QString::fromStdString(cat.m_data->at(index));
    case Qt::EditRole:
        return QString::fromStdString(cat.m_data->at(index));        
    }
    return QVariant();
}

QString FileSetupModel::shortenFilename(const QString &fullname) const
{
    QFileInfo info(fullname);

    // if the full name contains an environment variable,
    // always show the full names
    if (fullname.contains("$"))
    {
        return fullname;
    }

    // shorten the file name if it contains a path
    QString filename = info.fileName();
    if (filename != fullname)
    {
        return QString("... ") + filename;
    }
    return fullname;
}
