
#include <iostream>
#include <string_view>
#include "mmconsole.h"

#include <QFont>
#include <QTextBlock>

using namespace GUI;

MMConsole::MMConsole(QWidget *parent) : QTextEdit("", parent)
{
    m_historyReadIdx = 0;
    m_historyWriteIdx = 0;
    m_history.resize(32);

    //FIXME: get font from setup
    QFont newFont("Consolas", 10);
    setFont(newFont);

    m_promptBlock = -1;
    m_prompt = "> ";
}

void MMConsole::clear()
{
    QTextEdit::clear();
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
            //QTextEdit::keyPressEvent(e);

            // add to the command history
            if (m_historyWriteIdx >= m_history.size())
                m_historyWriteIdx = 0;

            m_history[m_historyWriteIdx] = cmd.toStdString();
            ++m_historyWriteIdx;
            m_historyReadIdx = m_historyWriteIdx;
            emit executeCommand(cmd);
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
        setTextColor(Qt::red);
    }
    else
    {
        setTextColor(Qt::black);
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

    setTextColor(Qt::black);
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

#if 0
MMConsole::MMConsole(QWidget *parent) : QWidget(parent)
{
    m_txt    = new MMConsoleEdit(this);
    m_txt->setMaximumBlockCount(100);

    connect(m_txt, &MMConsoleEdit::newCommand, this, &MMConsole::onCommand);

    #if 1
    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::green);
    m_txt->setPalette(p);
    #endif

    m_layout = new QHBoxLayout(this);
    m_layout->addWidget(m_txt,1);
}

QSize MMConsole::sizeHint() const
{
    const size_t xCharCount = 80;
    const size_t yCharCount = 25;

    auto w = xCharCount * getFontWidth();
    auto h = yCharCount * getFontHeight();

    auto margin = m_txt->frameWidth();
    margin += m_txt->document()->documentMargin() * 2;

    return QSize(w+margin,h+margin);
}

int MMConsole::getFontWidth(const QFont *font) const
{
    if (font == nullptr)
        font = &m_txt->font();

    auto metrics = QFontMetrics(*font);
    auto spaceAdvance = metrics.horizontalAdvance(' ');
    if (spaceAdvance != 0)
    {
        return spaceAdvance;
    }
    else
    {
        return metrics.width(' ');
    }
}

int MMConsole::getFontHeight(const QFont *font) const
{
    if (font == nullptr)
        font = &m_txt->font();

    auto metrics = QFontMetrics(*font);
    return metrics.height();
}

void MMConsole::putData(const std::string &txt)
{
    m_txt->insertPlainText(txt.c_str());
    m_txt->ensureCursorVisible();
}

void MMConsole::putData(const char *txt)
{
    m_txt->insertPlainText(txt);
    m_txt->ensureCursorVisible();
}

void MMConsole::putData(const QByteArray &data)
{
    m_txt->insertPlainText(data);
    m_txt->ensureCursorVisible();
}

void MMConsole::onCommand(const char *cmd)
{
    emit newCommand(cmd);
}

void MMConsole::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)
    m_txt->ensureCursorVisible();
}

void MMConsole::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    setFocus();
}
#endif
