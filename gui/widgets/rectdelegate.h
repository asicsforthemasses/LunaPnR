// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

// https://doc.qt.io/qt-6/qtwidgets-itemviews-stardelegate-example.html

#include <QStyledItemDelegate>

namespace GUI
{

class RectDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

private slots:
    void commitAndCloseEditor();
};

};
