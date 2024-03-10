// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <QBoxLayout>
#include "flatactiontile.h"

using namespace GUI;

FlatActionTile::FlatActionTile(const QString &actionTitle,
    const QString &iconUrl,
    const QString &actionIconUrl,
    const QString &actionName,
    QWidget *parent) : FlatTileBase(actionTitle, iconUrl, actionName, parent), m_actionName(actionName)
{
    m_actionButton = new FlatImageButton(actionIconUrl);

    m_hlayout->addWidget(m_actionButton);

    connect(m_actionButton, &FlatImageButton::pressed, this, &FlatActionTile::onActionPrivate);

    setLayout(m_hlayout);
}

void FlatActionTile::onActionPrivate()
{
    emit onAction(m_actionName);
}

void FlatActionTile::setActionIcon(const QString &iconUrl)
{
    m_actionButton->setPixmap(iconUrl);
}
