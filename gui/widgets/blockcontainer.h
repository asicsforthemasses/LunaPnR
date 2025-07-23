// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QWidget>
#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>

#include "flattogglebutton.h"
#include "clickablelabel.h"

namespace GUI
{

class BlockContainer;   // pre-declaration

class BlockContainerHeader : public QFrame
{
    Q_OBJECT
public:
    BlockContainerHeader(BlockContainer *owner, QWidget *parent = 0);
    BlockContainerHeader(const QString &name, BlockContainer *owner, QWidget *parent = 0);

    void setName(const QString &name);

    void addWidget(QWidget *widget);

    bool isFolded() const;

protected slots:
    void onFoldButtonToggled();
    void onLabelClicked();

protected:
    BlockContainer      *m_owner = nullptr;
    QHBoxLayout         *m_hlayout = nullptr;
    FlatToggleButton    *m_foldButton = nullptr;
    ClickableLabel      *m_name = nullptr;
};

class BlockContainer : public QWidget
{
    Q_OBJECT
public:
    BlockContainer(QWidget *parent = 0);

    void addWidget(QWidget *widget, int stretch = 0);
    void setBlockName(const QString &name);

    BlockContainerHeader* header()
    {
        return m_header;
    }

    bool isFolded() const;
    void setFolded(bool state = true);

protected slots:
    void onHeaderFoldToggled(bool state);

protected:
    bool m_folded = false;
    std::vector<QWidget*> m_contents;
    QVBoxLayout *m_vlayout = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    BlockContainerHeader *m_header = nullptr;
    QFrame *m_contentFrame = nullptr;
};

};