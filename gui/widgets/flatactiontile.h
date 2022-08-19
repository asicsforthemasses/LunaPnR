#pragma once

#include <QWidget>
#include <QLabel>
#include <QString>
#include "flatimage.h"
#include "flatimagebutton.h"

namespace GUI
{

class FlatActionTile : public QFrame
{
    Q_OBJECT
public:
    FlatActionTile(const QString &actionTitle, 
        const QString &iconUrl,        
        const QString &actionIconUrl,
        const QString &actionName,
        QWidget *parent = nullptr);

    /** get title displayed in the GUI */
    QString actionTitle() const;

    /** set title displayed in the GUI */
    void setActionTitle(const QString &name);

    /** get action name -- used for action events */
    QString actionName() const;

    /** set action name -- used for action events */
    void setActionName(const QString &actionName);

    void setIcon(const QString &iconUrl);
    void setActionIcon(const QString &iconUrl);

    enum class Status
    {
        NONE,
        RUNNING,
        OK,
        ERROR
    };

    void setStatus(Status s);

signals:
    void onAction(QString actionName);

private slots:
    void onActionPrivate();

protected:
    Status           m_status{Status::NONE};
    QString          m_actionName;
    QLabel          *m_actionTitle = nullptr;
    FlatImage       *m_icon = nullptr;
    FlatImage       *m_statusIndicator = nullptr;
    FlatImageButton *m_actionButton = nullptr;
};

};
