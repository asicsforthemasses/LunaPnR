// inspiration: https://github.com/jupyter/qtconsole/blob/master/qtconsole/console_widget.py

#pragma once

#include <string>
#include <vector>
#include <sstream>

#include <QWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QByteArray>
#include <QKeyEvent>
#include <QHBoxLayout>

#include "../common/msvcfix.h"

namespace GUI
{

class MMConsole : public QTextEdit
{
    Q_OBJECT
public:
    MMConsole(QWidget *parent = nullptr);

    void setPrompt(const QString &prompt);
    void clear();

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

signals:
    void executeCommand(const QString &command);

protected:
    virtual void keyPressEvent(QKeyEvent *e) override;

    void    displayPrompt();
    QString getCurrentCommand();
    void    replaceCurrentCommand(const QString &cmd);

    ssize_t m_historyWriteIdx;
    ssize_t m_historyReadIdx;
    std::vector<std::string> m_history;

    QString m_prompt;
    int     m_promptBlock;
};

};
