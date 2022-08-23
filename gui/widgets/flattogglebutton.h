// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once 

#include <vector>
#include <QPixmap>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QAbstractButton>

namespace GUI 
{

class FlatToggleButton : public QAbstractButton 
{
    Q_OBJECT
public:
    FlatToggleButton(QWidget *parent = nullptr);

    QSize sizeHint() const;

protected:
    bool eventFilter(QObject * watched, QEvent * event) override;
    void resizeEvent(QResizeEvent *e) override;
    void paintEvent(QPaintEvent* e) override;

    std::unique_ptr<QPixmap> m_onPixmap;
    std::unique_ptr<QPixmap> m_offPixmap;
    bool m_hover = false;
};

};
