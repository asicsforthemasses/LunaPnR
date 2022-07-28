#pragma once

#include <QWidget>
#include <QLabel>
#include <QString>
#include "flatimage.h"
#include "flatimagebutton.h"

namespace GUI
{

class FlatActionTile : public QWidget
{
    Q_OBJECT
public:
    FlatActionTile(const QString &actionTitle, 
        const QString &iconUrl,        
        const QString &actionIconUrl,
        QWidget *parent = nullptr);

    QString actionTitle() const;
    void setActionTitle(const QString &name);

    void setIcon(const QString &iconUrl);
    void setActionIcon(const QString &iconUrl);

signals:
    void onAction();

protected:
    QLabel          *m_actionTitle;
    FlatImage       *m_icon;
    FlatImageButton *m_actionButton;
};

};