
#include <iostream>
#include <string_view>
#include "mmconsole.h"

#include <QFont>
#include <QTextBlock>

using namespace GUI;

MMConsole::MMConsole(QWidget *parent) : QTextEdit("", parent)
{
    setAcceptRichText(false);
    setUndoRedoEnabled(false);

    reset();

    //FIXME: get font from setup
    QStringList substitutes;
    substitutes << "Droid Sans Mono" << "monospace";
    
    QFont::insertSubstitutions("Consolas", substitutes);
    
    QFont newFont("Consolas", 11);
    newFont.setStyleStrategy(QFont::PreferQuality);

#if 0
    std::stringstream ss;
    ss << newFont.family().toStdString() << " ";

    std::cout << ss.str() << "\n";
#endif

    setFont(newFont);
    setColours(QColor("#1d1f21"), QColor("#c5c8c6"), QColor("#a54242"));

    m_prompt = "> ";
}

void MMConsole::setColours(QColor bkCol, QColor promptCol, QColor errorCol)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Background, bkCol);
    pal.setColor(QPalette::Base, bkCol);
    setPalette(pal);
    setAutoFillBackground(true);

    m_bkCol = bkCol;
    m_promptCol = promptCol;
    m_errorCol = errorCol;
}

void MMConsole::clear()
{
    QTextEdit::clear();
    m_promptBlock = -1;
}

void MMConsole::reset()
{
    m_historyReadIdx  = 0;
    m_historyWriteIdx = 0;
    m_history.clear();
    m_history.resize(32);
}

void MMConsole::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    //case Qt::Key_Left:
    //case Qt::Key_Right:
    case Qt::Key_Up:
        --m_historyReadIdx;
        if (m_historyReadIdx < 0)
            m_historyReadIdx = m_history.size() - 1;

        {
            replaceCurrentCommand(QString(m_history[m_historyReadIdx].c_str()));
        }        
        return;
    case Qt::Key_Down:
        ++m_historyReadIdx;
        if (m_historyReadIdx >= m_history.size())
            m_historyReadIdx = 0;    
        {
            replaceCurrentCommand(QString(m_history[m_historyReadIdx].c_str()));
        }        
        return;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        {            
            auto cmd = getCurrentCommand();

            // add to the command history
            if (m_historyWriteIdx >= m_history.size())
                m_historyWriteIdx = 0;

            m_history[m_historyWriteIdx] = cmd.toStdString();
            ++m_historyWriteIdx;
            m_historyReadIdx = m_historyWriteIdx;
            emit executeCommand(cmd);
        }
        break;
    case Qt::Key_Backspace:
        {
            QTextCursor cur = textCursor();
            auto column = cur.columnNumber();
            auto block  = cur.blockNumber();
            if ((block == m_promptBlock) && (column <= m_prompt.length()))
            {
                return;
            }
            else
            {
                QTextEdit::keyPressEvent(e);
            }
        }
        break;
    default:
        QTextEdit::keyPressEvent(e);
        break;
    }
}

QString MMConsole::getCurrentCommand()
{
    QTextCursor cur = textCursor();
    cur.movePosition(QTextCursor::StartOfBlock);
    cur.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, m_prompt.length());
    cur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    QString cmd = cur.selectedText();
    cur.clearSelection();
    return cmd;
}

void MMConsole::replaceCurrentCommand(const QString &cmd)
{
    QTextCursor cur = textCursor();
    cur.movePosition(QTextCursor::StartOfLine);
    cur.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, m_prompt.length());
    cur.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    cur.insertText(cmd);
}

void MMConsole::print(const QString &txt, PrintType pt)
{
    if (pt == PrintType::Error)
    {
        setTextColor(m_errorCol);
    }
    else
    {
        setTextColor(m_promptCol);
    }

    append(txt);

    if ((pt == PrintType::Complete) || (pt == PrintType::Error))
    {
        if (!txt.endsWith("\n"))
        {
            append("\n");
        }

        displayPrompt();
    }

    moveCursor(QTextCursor::End);
}

void MMConsole::setPrompt(const QString &prompt)
{
    m_prompt = prompt;
}

void MMConsole::displayPrompt()
{
    setUndoRedoEnabled(false);

    setTextColor(m_promptCol);
    QTextCursor cur = textCursor();
    cur.insertText(m_prompt);
    cur.movePosition(QTextCursor::EndOfLine);
    setTextCursor(cur);

    m_promptBlock = cur.blockNumber();

    setUndoRedoEnabled(true);
}

void MMConsole::print(const std::string &txt, PrintType pt)
{
    print(QString::fromStdString(txt), pt);
}

void MMConsole::print(const std::stringstream &ss, PrintType pt)
{
    print(QString::fromStdString(ss.str()), pt);
}

void MMConsole::print(const char *txt, PrintType pt)
{
    print(QString::fromUtf8(txt), pt);
}

