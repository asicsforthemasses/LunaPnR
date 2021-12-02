
#include <iostream>
#include <string_view>
#include "mmconsole.h"

#include <QFont>
#include <QTextBlock>

using namespace GUI;

MMConsoleEdit::MMConsoleEdit(QWidget *parent) : QPlainTextEdit("", parent) 
{
    m_historyReadIdx = 0;
    m_historyWriteIdx = 0;
    m_history.resize(32);

    //FIXME: get font from setup
    QFont newFont("Consolas", 10);
    setFont(newFont);
}

void MMConsoleEdit::keyPressEvent(QKeyEvent *e)
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
            QTextCursor cursor(document());
            cursor.movePosition(QTextCursor::End);
            cursor.movePosition(QTextCursor::StartOfLine);
            cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();            
            cursor.insertText(QString(m_history[m_historyReadIdx].c_str()));
        }        
        return;
    case Qt::Key_Down:
        ++m_historyReadIdx;
        if (m_historyReadIdx >= m_history.size())
            m_historyReadIdx = 0;    
        {
            QTextCursor cursor(document());
            cursor.movePosition(QTextCursor::End);
            cursor.movePosition(QTextCursor::StartOfLine);
            cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
            cursor.insertText(QString(m_history[m_historyReadIdx].c_str()));
        }        
        return;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        {            
            QTextCursor cursor(document());
            cursor.movePosition(QTextCursor::End);

            // emit the last line
            std::string txt(document()->lastBlock().text().toStdString());
            QPlainTextEdit::keyPressEvent(e);

            // add to the command history
            if (m_historyWriteIdx >= m_history.size())
                m_historyWriteIdx = 0;

            m_history[m_historyWriteIdx] = txt;
            ++m_historyWriteIdx;
            m_historyReadIdx = m_historyWriteIdx;            
            emit newCommand(txt.c_str());
        }
        break;
    default:
        QPlainTextEdit::keyPressEvent(e);
        break;
    }
}


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
