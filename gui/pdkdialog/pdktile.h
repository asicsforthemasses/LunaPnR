// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QVBoxLayout>
#include "common/pdkinfo.h"

namespace GUI
{

class PDKTile : public QWidget
{
    Q_OBJECT
public:
    PDKTile(const PDKInfo &info, QWidget *parent = nullptr);

    constexpr void setID(int id) noexcept
    {
        m_id = id;
    }

    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void setSelected(bool selected = true);

signals:
    void clicked(int id);

protected:
    bool   m_selected{false};
    int    m_id{-1};
    QLabel *m_title{nullptr};
    QLabel *m_date{nullptr};
    QLabel *m_version{nullptr};
};

class PDKTileList : public QWidget
{
    Q_OBJECT
public:
    PDKTileList(QWidget *parent = nullptr);

    PDKTile* createTile(const PDKInfo &info);

    PDKTile* at(int index)
    {
        return m_pdkTiles.at(index);
    }

    std::size_t size() const noexcept
    {
        return m_pdkTiles.size();
    }

    constexpr int selected() const noexcept
    {
        return m_selected;
    }

signals:
    void selectionChanged(int id);

private slots:
    void onTileClicked(int id);

protected:
    int m_selected{-1};
    QVBoxLayout *m_layout{nullptr};
    std::vector<PDKTile*>   m_pdkTiles;
};

};
