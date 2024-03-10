// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QWidget>
#include <QLabel>
#include <QString>
#include "flattilebase.h"
#include "flatimagebutton.h"

namespace GUI
{

class FlatActionTile : public FlatTileBase
{
    Q_OBJECT
public:
    FlatActionTile(const QString &actionTitle,
        const QString &iconUrl,
        const QString &actionIconUrl,
        const QString &actionName,
        QWidget *parent = nullptr);

    virtual ~FlatActionTile() = default;

    void setActionIcon(const QString &iconUrl);

signals:
    void onAction(QString actionName);

private slots:
    void onActionPrivate();

protected:
    QString          m_actionName;
    FlatImageButton *m_actionButton = nullptr;
};

};
