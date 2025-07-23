// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "flattilebase.h"

using namespace GUI;

FlatTileBase::FlatTileBase(const QString &text, const QString &iconUrl,
    const QString &actionName, QWidget *parent) : QFrame(parent), m_actionName(actionName)
{
    m_statusIndicator = new FlatImage("://images/status_empty.png");
    m_textLabel = new QLabel(text);
    m_icon = new FlatImage(iconUrl);

    m_hlayout = new QHBoxLayout();
    m_hlayout->setContentsMargins(2,2,2,2);

    m_hlayout->addWidget(m_statusIndicator);
    m_hlayout->addWidget(m_icon);
    m_hlayout->addWidget(m_textLabel, 1);
    m_hlayout->setAlignment(Qt::AlignVCenter);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    setLayout(m_hlayout);
}

void FlatTileBase::setText(const QString &text) noexcept
{
    m_textLabel->setText(text);
}

void FlatTileBase::setIcon(const QString &iconUrl) noexcept
{
    m_icon->setPixmap(iconUrl);
}

void FlatTileBase::setStatus(Status s) noexcept
{
    m_status = s;

    assert(m_statusIndicator != nullptr);

    switch(s)
    {
    case Status::NONE:
        m_statusIndicator->setPixmap("://images/status_empty.png");
        break;
    case Status::RUNNING:
        m_statusIndicator->setPixmap("://images/status_running.png");
        break;
    case Status::ERROR:
        m_statusIndicator->setPixmap("://images/status_error.png");
        break;
    case Status::OK:
        m_statusIndicator->setPixmap("://images/status_ok.png");
        break;
    default:
        break;
    }

    m_statusIndicator->update();
}

/** get action name -- used for action events */
QString FlatTileBase::actionName() const
{
    return m_actionName;
}

/** set action name -- used for action events */
void FlatTileBase::setActionName(const QString &actionName)
{
    m_actionName = actionName;
}
