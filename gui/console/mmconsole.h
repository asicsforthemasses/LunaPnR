/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/


// inspiration: https://github.com/jupyter/qtconsole/blob/master/qtconsole/console_widget.py

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <list>
#include <sstream>
#include <mutex>

#include <QWidget>
#include <QListWidget>
#include <QDialog>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QByteArray>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QFrame>

#include "mtstringbuffer.h"
#include "singlelineedit.h"
#include "../common/msvcfix.h"

namespace GUI
{


class MMConsole : public QFrame
{
    Q_OBJECT
public:
    MMConsole(QWidget *parent = nullptr);

    void setPrompt(const QString &prompt);
    void clear();
    void reset();

    void print(const QString &txt);
    void print(const std::string &txt);
    void print(const std::string_view txt);
    void print(const std::stringstream &ss);
    void print(const char *txt);

    /** multi-threaded safe print */
    void mtPrint(const std::string &txt);

    /** multi-threaded safe print */
    void mtPrint(const std::string_view &txt);

    struct ConsoleColours
    {
        QColor  m_bkCol;
        QColor  m_promptCol;
        QColor  m_errorCol;
    };

    void setColours(const QColor &bkCol, const QColor &promptCol, const QColor &errorCol) noexcept;
    void setColours(const ConsoleColours &colours) noexcept;

    [[nodiscard]] ConsoleColours getColours() const noexcept;

    void disablePrompt()
    {
        m_commandLine->setEnabled(false);
        //m_promptEnabled = false;
    }

    void enablePrompt()
    {
        m_commandLine->setEnabled(true);
        //m_promptEnabled = true;
        //displayPrompt();
    }

signals:
    void executeCommand(const QString &command);

protected:
    void    displayPrompt();
    void    appendWithoutNewline(const QString &txt);
    
    bool event(QEvent *event) override;
    std::unique_ptr<MTStringBuffer> m_mtStringBuffer;
    
    QTextEdit       *m_textDisplay;   ///< widget to display all console messages
    SingleLineEdit  *m_commandLine;   ///< single line text edit.
    ConsoleColours  m_colours;
};

};
