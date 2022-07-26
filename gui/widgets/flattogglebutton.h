/*

    FlowManager Widget Source Code

    SPDX-License-Identifier: GPL-3.0-only
    SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>

*/

#pragma once 

#include <vector>
#include <QPaintEvent>
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
    void paintEvent(QPaintEvent* e) override;

    std::unique_ptr<QPixmap> m_onPixmap;
    std::unique_ptr<QPixmap> m_offPixmap;
};

};
