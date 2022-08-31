// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <vector>
#include <string>

#include <QObject>
#include <QString>
#include <QLineEdit>
#include <QListWidget>
#include <QDialog>
#include <QStringList>

#include "widgets/txtoverlay.h"

namespace GUI
{

class SingleLineEdit;   //pre-declaration

/** A list widget that is used for code completion */
class PopupListWidget : public QListWidget
{
    Q_OBJECT

public:
    PopupListWidget(QWidget *parent = 0): QListWidget(parent) 
    {
        setUniformItemSizes(true);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    virtual ~PopupListWidget() {}

protected:
    virtual QSize sizeHint() const;
    virtual void keyPressEvent(QKeyEvent *e);
};

/** a QDialog that shows the code completion list */
class PopupCompleter : public QDialog
{
    Q_OBJECT

public:
    PopupCompleter(const QStringList&, QWidget *parent = 0);
    virtual ~PopupCompleter();

public:
    QString selected() 
    { 
        return m_selected; 
    }

    int exec(SingleLineEdit*);

protected:
    virtual void showEvent(QShowEvent*);

private slots:
    void onItemActivated(QListWidgetItem*);

public:
    QListWidget *m_listWidget;
    QString m_selected;
};

class SingleLineEdit : public QLineEdit
{   
    Q_OBJECT
public:
    SingleLineEdit(QWidget *parent = nullptr);

    void    reset();
    QString getCurrentCommand() const;        
    QRect   getCursorRect() const;

signals:
    void executeCommand(const QString &command);

protected:
    bool event(QEvent*) override;

    QStringList suggestCommand(QString partialCommand);
    void handleTabKeypress();
    QString getHelpString(const QString &cmd) const;
    void replaceCurrentCommand(const QString &cmd);
    bool canShowHelp(const QString &cmd) const;
    void updateHelpOverlay();
    void keyPressEvent(QKeyEvent *e) override;

    bool    m_promptEnabled;

    ssize_t m_historyWriteIdx;
    ssize_t m_historyReadIdx;
    std::vector<std::string> m_history;

    TxtOverlay *m_overlay;
};

};
