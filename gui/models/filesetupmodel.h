// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <vector>
#include <string>

#include <QVariant>
#include <QPixmap>
#include <QModelIndex>
#include <QAbstractItemModel>

namespace GUI
{

class FileSetupModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    FileSetupModel(QObject *parent = nullptr);
    virtual ~FileSetupModel();

    QModelIndex index(int row, int column,
        const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;

    void addCategory(const QString &name, const QStringList &fileExt, std::vector<std::string > *data);
    void addCategoryItem(size_t categoryIndex, const QString &data);

    bool isCategory(const QModelIndex &index) const;

    auto const& categories() const
    {
        return m_categories;
    }

    /** reset the model to update the views */
    void repopulate();

protected:

    struct Category
    {
        QString     m_name;         ///< name of the category
        QStringList m_extension;    ///< file extensions for this category
        std::vector<std::string > *m_data = nullptr;
    };

    QVariant categoryData(const Category&, int role) const;
    QVariant categoryItemData(const Category&, size_t index, int role) const;

    QPixmap m_folderIcon;
    QPixmap m_fileIcon;

    QString shortenFilename(const QString &fullname) const;

    std::vector<Category> m_categories;
};

};