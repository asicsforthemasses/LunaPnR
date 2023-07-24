// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QFrame>
#include <QString>
#include <QLabel>
#include <QBoxLayout>

#include "flatimage.h"

namespace GUI
{

class FlatTileBase : public QFrame
{
    Q_OBJECT
public:
    FlatTileBase(const QString &text, const QString &iconUrl, 
        const QString &actionName, QWidget *parent = nullptr);

    virtual ~FlatTileBase() = default;

    [[nodiscard]] QString text() const noexcept;
    void setText(const QString &text) noexcept;

    void setIcon(const QString &iconUrl) noexcept;

    enum class Status
    {
        NONE,
        RUNNING,
        OK,
        ERROR
    };

    void setStatus(Status s) noexcept;

    /** get action name -- used for action events */
    QString actionName() const;

    /** set action name -- used for action events */
    void setActionName(const QString &actionName);

protected:
    QLabel      *m_textLabel = nullptr;
    QString     m_actionName;
    FlatImage   *m_statusIndicator = nullptr;    
    FlatImage   *m_icon = nullptr;    
    QHBoxLayout *m_hlayout = nullptr;
    Status      m_status{Status::NONE};
};

using FlatStatusTile = FlatTileBase;

};
