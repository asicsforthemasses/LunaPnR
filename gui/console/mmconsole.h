// inspiration: https://github.com/jupyter/qtconsole/blob/master/qtconsole/console_widget.py

#pragma once

#include <string>
#include <vector>
#include <sstream>

#include <QWidget>
#include <QListWidget>
#include <QDialog>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QByteArray>
#include <QKeyEvent>
#include <QHBoxLayout>

#include "../common/msvcfix.h"

namespace GUI
{

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

    int exec(QTextEdit*);

protected:
    virtual void showEvent(QShowEvent*);

private slots:
    void onItemActivated(QListWidgetItem*);

public:
    QListWidget *m_listWidget;
    QString m_selected;
};

class MMConsole : public QTextEdit
{
    Q_OBJECT
public:
    MMConsole(QWidget *parent = nullptr);

    void setPrompt(const QString &prompt);
    void clear();
    void reset();

    enum class PrintType
    {
        Error,
        Partial,
        Complete
    };

    void print(const QString &txt, PrintType pt);
    void print(const std::string &txt, PrintType pt);
    void print(const std::stringstream &ss, PrintType pt);
    void print(const char *txt, PrintType pt);

    void setColours(QColor bkCol, QColor promptCol, QColor errorCol);

    void disablePrompt()
    {
        m_promptEnabled = false;
    }

    void enablePrompt()
    {
        m_promptEnabled = true;
        displayPrompt();
    }

signals:
    void executeCommand(const QString &command);

protected:
    virtual void keyPressEvent(QKeyEvent *e) override;

    void    displayPrompt();
    QString getCurrentCommand();
    void    replaceCurrentCommand(const QString &cmd);
    void    handleTabKeypress();

    void    appendWithoutNewline(const QString &txt);

    QStringList suggestCommand(QString partialCommand);

    ssize_t m_historyWriteIdx;
    ssize_t m_historyReadIdx;
    std::vector<std::string> m_history;

    QString m_prompt;
    int     m_promptBlock;
    bool    m_promptEnabled;

    QColor  m_bkCol;
    QColor  m_promptCol;
    QColor  m_errorCol;
};

};
