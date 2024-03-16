#include <cassert>

#include "replwidget.hpp"
#include <QTextBlock>

namespace GUI
{

ReplWidget::ReplWidget(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setLineWrapMode(NoWrap);
    insertPlainText(m_promptStr);
}

void ReplWidget::installCompleter(ICompleter *completer)
{
    m_completer.reset(completer);
}

void ReplWidget::keyPressEvent(QKeyEvent *e)
{
    if (m_locked)
    {
        // do not react on keypresses when a command
        // has been submitted but no result has been
        // received yet.
        return;
    }

    switch(e->key())
    {
    case Qt::Key_Return:
        handleEnter();
        break;
    case Qt::Key_Backspace:
        handleLeft(e);
        break;
    case Qt::Key_Up:
        handleHistoryUp();
        break;
    case Qt::Key_Down:
        handleHistoryDown();
        break;
    case Qt::Key_Home:
        handleHome();
        break;
    case Qt::Key_Tab:
        handleCompleter();
        break;
    default:
        QPlainTextEdit::keyPressEvent(e);
        break;
    }
}

void ReplWidget::handleEnter()
{
    QString cmd = getCommand();

    if (!cmd.isEmpty())
    {
        while (!m_historyDown.empty())
        {
            m_historyUp.push_front(m_historyDown.front());
            m_historyDown.pop_front();
        }

        m_historyUp.push_front(cmd);
    }

    moveToEndOfLine();

    if (!cmd.isEmpty())
    {
        lock();
        setFocus();
        insertPlainText("\n");
        emit command(cmd);
    }
    else
    {
        insertPlainText("\n");
        insertPlainText(m_promptStr);
        ensureCursorVisible();
    }
}

void ReplWidget::handleHistoryUp()
{
    if (!m_historyUp.empty())
    {
        QString cmd = m_historyUp.front();
        m_historyUp.pop_front();
        m_historyDown.push_front(cmd);

        clearLine();
        insertPlainText(cmd);
    }

    m_historySkip = true;
}

void ReplWidget::handleHistoryDown()
{
    if (!m_historyDown.empty() && m_historySkip)
    {
        m_historyUp.push_front(m_historyDown.front());
        m_historyDown.pop_front();
        m_historySkip = false;
    }

    if (!m_historyDown.empty())
    {
        QString cmd = m_historyDown.front();
        m_historyDown.pop_front();
        m_historyUp.push_front(cmd);

        clearLine();
        insertPlainText(cmd);
    }
    else
    {
        clearLine();
    }
}

void ReplWidget::clearLine()
{
    QTextCursor c = textCursor();
    c.select(QTextCursor::LineUnderCursor);
    c.removeSelectedText();
    insertPlainText(m_promptStr);
}

QString ReplWidget::getCommand() const
{
    QTextCursor c = textCursor();
    c.select(QTextCursor::LineUnderCursor);

    QString text = c.selectedText();
    text.remove(0, m_promptStr.length());

    return text;
}

void ReplWidget::setCommand(QString &cmd)
{
    QTextCursor c = textCursor();
    c.select(QTextCursor::LineUnderCursor);

    c.removeSelectedText();
    c.insertText(m_promptStr+cmd);
    moveToEndOfLine();
}

void ReplWidget::moveToEndOfLine()
{
    QPlainTextEdit::moveCursor(QTextCursor::EndOfLine);
}

// The text cursor is not allowed to move beyond the
// prompt
void ReplWidget::handleLeft(QKeyEvent *event)
{
    if (getIndex(textCursor()) > m_promptStr.length())
    {
        QPlainTextEdit::keyPressEvent(event);
    }
}

// Home (pos1) key pressed
void ReplWidget::handleHome()
{
    QTextCursor c = textCursor();
    c.movePosition(QTextCursor::StartOfLine);
    c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, m_promptStr.length());
    setTextCursor(c);
}

// Solution for getting x and y position of the cursor. Found
// them in the Qt mailing list
int ReplWidget::getIndex(const QTextCursor &textCursor)
{
    int column = 1;
    auto block = textCursor.block();
    column = textCursor.position() - block.position();
    return column;
}

void ReplWidget::cmdReply(const QString &result)
{
    insertPlainText(result);
    if (!result.endsWith("\n"))
    {
        insertPlainText("\n");
    }
    ensureCursorVisible();
}

constexpr bool isWhitespace(QChar c) noexcept
{
    return (c=='\t') || (c==' ');
}

ReplWidget::LastToken ReplWidget::strGetLastToken(const QString &str) const
{
    LastToken result;
    int startIndex = 0, endIndex = 0;
    for (int i = 0; i <= str.size(); i++)
    {
        // If we reached the end of the word or the end of the input.
        if ((i == str.size()) || isWhitespace(str[i]))
        {
            endIndex = i;
            result.m_len = endIndex - startIndex;
            result.m_offset = startIndex;
            result.m_str = str.mid(startIndex, endIndex - startIndex);

            startIndex = endIndex + 1;
        }
    }
    return result;
}

void ReplWidget::handleCompleter()
{
    if (!m_completer) return;   // no completer installed

    auto result = strGetLastToken(getCommand());
    if (result.m_str.isEmpty()) return; // command/token list is empty

    auto options = m_completer->tryComplete(result.m_str);

    // the options list can never have a length of 2.
    assert(options.size() != 2);

    if (options.size() == 1)
    {
        auto cmd = getCommand().first(result.m_offset);
        cmd.append(options.front());
        setCommand(cmd);
    }
    else
    {
        auto commonPrefix = options.front();
    }
}

};
