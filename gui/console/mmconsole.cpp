// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include <string_view>
#include <memory>
#include <array>


#include <QDebug>
#include <QFont>
#include <QSettings>
#include <QScreen>
#include <QApplication>

#include "mmconsole.h"

using namespace GUI;

MMConsole::MMConsole(QWidget *parent) : QFrame(parent)
{
    auto vlayout = new QVBoxLayout();

    vlayout->setContentsMargins(0,0,0,0);
    vlayout->setMargin(0);
    vlayout->setSpacing(0);

    m_textDisplay = new QTextEdit();
    m_textDisplay->setAcceptRichText(false);
    m_textDisplay->setUndoRedoEnabled(false);
    m_textDisplay->setReadOnly(true);
    m_textDisplay->document()->setMaximumBlockCount(1000);
    
    m_commandLine = new SingleLineEdit();
    
    vlayout->addWidget(m_textDisplay, 1);
    vlayout->addWidget(m_commandLine, 0);

    reset();

    // default colours
    setColours(QColor("#1d1f21"), QColor("#c5c8c6"), QColor("#a54242"));

    m_mtStringBuffer = std::make_unique<MTStringBuffer>(this);

    setLayout(vlayout);

    connect(m_commandLine, &SingleLineEdit::executeCommand, this, &MMConsole::executeCommand);
}

void MMConsole::setColours(const QColor &bkCol, const QColor &promptCol, const QColor &errorCol) noexcept
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, bkCol);
    pal.setColor(QPalette::Base, bkCol);
    pal.setColor(QPalette::WindowText, promptCol);

    setPalette(pal);
    m_commandLine->setPalette(pal);
    m_textDisplay->setPalette(pal);

    //m_commandLine->setAutoFillBackground(true);
    //m_commandLine->setTextColor(promptCol);
    //m_textDisplay->setAutoFillBackground(true);
    
    m_colours.m_bkCol     = bkCol;
    m_colours.m_promptCol = promptCol;
    m_colours.m_errorCol  = errorCol;
}

bool MMConsole::event(QEvent *event)
{
    if (event->type() == QEvent::User)
    {
        if (m_mtStringBuffer)
        {
            while(m_mtStringBuffer->containsString())
            {
                print(m_mtStringBuffer->pop());
            }
            return true;
        }
        return false;        
    }
    return QFrame::event(event);
}

MMConsole::ConsoleColours MMConsole::getColours() const noexcept
{
    return m_colours;
}

void MMConsole::clear()
{
    m_textDisplay->clear();
    m_commandLine->clear();
}

void MMConsole::reset()
{
    m_commandLine->reset();
}

void MMConsole::appendWithoutNewline(const QString &txt)
{
    // moving the cursor to ::END make the document
    // forget the textColor, so we re-apply it.
    auto col = m_textDisplay->textColor();
    m_textDisplay->moveCursor(QTextCursor::End);
    m_textDisplay->setTextColor(col);

    m_textDisplay->insertPlainText(txt);

    m_textDisplay->moveCursor(QTextCursor::End);
}

void MMConsole::print(const QString &txt)
{
    m_textDisplay->setTextColor(m_colours.m_promptCol);
    appendWithoutNewline(txt);

    if (!txt.endsWith("\n"))
    {
        m_textDisplay->append("");
    }
}

void MMConsole::mtPrint(const std::string &txt)
{
    if (m_mtStringBuffer)
    {
        m_mtStringBuffer->print(txt);
    }
}

void MMConsole::mtPrint(const std::string_view &txt)
{
    if (m_mtStringBuffer)
    {
        m_mtStringBuffer->print(txt);
    }
}

void MMConsole::setPrompt(const QString &prompt)
{
    //m_prompt = prompt;
}

void MMConsole::displayPrompt()
{
#if 0    
    setUndoRedoEnabled(false);

    setTextColor(m_colours.m_promptCol);
    QTextCursor cur = textCursor();
    cur.insertText(m_prompt);
    cur.movePosition(QTextCursor::EndOfLine);
    setTextCursor(cur);

    m_promptBlock = cur.blockNumber();

    setUndoRedoEnabled(true);
#else
    //m_commandLine->setTextColor(m_colours.m_promptCol);
    m_commandLine->setEnabled(true);
#endif
}

void MMConsole::print(const std::string &txt)
{
    print(QString::fromStdString(txt));
}

void MMConsole::print(const std::string_view txt)
{
    print(QString::fromUtf8(&txt.at(0), txt.size()));
}

void MMConsole::print(const std::stringstream &ss)
{
    print(QString::fromStdString(ss.str()));
}

void MMConsole::print(const char *txt)
{
    print(QString::fromUtf8(txt));
}

