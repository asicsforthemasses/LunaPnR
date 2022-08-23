// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "rectdelegate.h"
#include <iostream>

using namespace GUI;

QWidget* RectDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
#if 0    
    if (index.data().canConvert<StarRating>()) 
    {
        StarEditor *editor = new StarEditor(parent);
        connect(editor, &StarEditor::editingFinished,
                this, &StarDelegate::commitAndCloseEditor);
        return editor;
    }
#endif
    std::cout << "RectDelegate\n";
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void RectDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}

void RectDelegate::commitAndCloseEditor()
{
    //StarEditor *editor = qobject_cast<StarEditor *>(sender());
    //emit commitData(editor);
    //emit closeEditor(editor);
}

void RectDelegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const
{
    //if (index.data().canConvert<StarRating>()) 
    {
        //StarRating starRating = qvariant_cast<StarRating>(index.data());
        //StarEditor *starEditor = qobject_cast<StarEditor *>(editor);
        //starEditor->setStarRating(starRating);
    } 
    //else 
    {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

QSize RectDelegate::sizeHint(const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    //if (index.data().canConvert<StarRating>()) {
    //    StarRating starRating = qvariant_cast<StarRating>(index.data());
    //    return starRating.sizeHint();
    //}
    return QStyledItemDelegate::sizeHint(option, index);
}
