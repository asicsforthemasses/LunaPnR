// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "pdktile.h"
#include <QGridLayout>
#include <QPixmap>
#include <QPainter>
#include "widgets/flatimage.h"

namespace GUI
{

PDKTile::PDKTile(const PDKInfo &info, QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    auto layout = new QGridLayout();
    m_title = new QLabel(QString::fromStdString(info.m_title));
    m_version = new QLabel(QString::fromStdString(info.m_version));
    m_date = new QLabel(QString::fromStdString(info.m_date));

    // try and load the "_icon.png"
    QString iconFile = QString::fromStdString((info.m_path / "_icon.png").string());
    auto icon = new FlatImage(iconFile);
    layout->addWidget(icon,0,0,3,1, Qt::AlignCenter);
    layout->setColumnMinimumWidth(0, 100);  // icons are 64x64 so we make the column 100 pixels

    layout->addWidget(new QLabel("Title"),0,1);
    layout->addWidget(m_title,0,2,Qt::AlignLeft);
    layout->addWidget(new QLabel("Version"),1,1);
    layout->addWidget(m_version,1,2, Qt::AlignLeft);
    layout->addWidget(new QLabel("Date"),2,1);
    layout->addWidget(m_date,2, 2, Qt::AlignLeft);
    layout->setColumnStretch(2,1);  // make sure the last column can stretch
    setLayout(layout);
};

void PDKTile::mousePressEvent(QMouseEvent *e)
{
    emit clicked(m_id);
}

void PDKTile::setSelected(bool selected)
{
    if (m_selected != selected)
    {
        m_selected = selected;
        update();
    }
}

void PDKTile::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter painter(this);
    if (m_selected)
    {
        painter.setPen(QPen{Qt::black, 3.0});
    }
    else
    {
        painter.setPen(QPen{Qt::gray, 1.0});
    }
    
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect().adjusted(1,1,-2,-2));
}


// ********************************************************************************
//  PDKTileList
// ********************************************************************************

PDKTileList::PDKTileList(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_layout = new QVBoxLayout();
    m_layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);

    setLayout(m_layout);
}

PDKTile* PDKTileList::createTile(const PDKInfo &info)
{
    auto tile = new PDKTile(info);
    tile->setID(m_pdkTiles.size());

    m_layout->addWidget(tile);
    m_pdkTiles.push_back(tile);

    connect(tile, &PDKTile::clicked, this, &PDKTileList::onTileClicked);

    return tile;
}

void PDKTileList::onTileClicked(int id)
{
    
    if (m_selected != id)
    {
        // de-select previous
        if (m_selected >= 0)
        {
            m_pdkTiles.at(m_selected)->setSelected(false);
        }

        m_pdkTiles.at(id)->setSelected(true);
        m_selected = id;
        emit selectionChanged(id);
    }
}

};
