// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <QString>
#include <QPlainTextEdit>
#include <deque>
#include <memory>
#include <list>

namespace GUI
{

class ReplWidget : public QPlainTextEdit
{
    Q_OBJECT
public:
    ReplWidget(QWidget *parent = 0);

    /** command completer interface */
    struct ICompleter
    {
        /** try complete should return one string if there is one option.
            if there are two or more options, the first entry in the list
            must be the longest common prefix of the options, followed
            by all the options.
        */
        virtual std::list<QString> tryComplete(const QString &str) = 0;
    };

    /** retrieve the user prompt */
    [[nodiscard]] QString prompt() const noexcept {return m_promptStr; }

    /** set the user prompt */
    void setPrompt(const QString &prompt) noexcept
    {
        m_promptStr = prompt;
        clearLine();
    }

    void cmdReply(const QString &text);

    /** lock the console so the user cannot input anything */
    void lock()
    {
        m_locked = true;
    }

    /** Prints the user prompt and unlock the console so the user
        can input keystrokes.
    */
    void unlock()
    {
        m_locked = false;
        insertPlainText(m_promptStr);
    }

    /** ReplWidget takes ownership of the ICompleter object */
    void installCompleter(ICompleter *completer);

protected:
    void keyPressEvent(QKeyEvent *e);

    // Do not handle other events
    // FIXME: allow copy/paste with mouse.
    void mousePressEvent(QMouseEvent *)
    {
    }

    void mouseDoubleClickEvent(QMouseEvent *)
    {
    }

    void mouseMoveEvent(QMouseEvent *)
    {
    }

    void mouseReleaseEvent(QMouseEvent *)
    {
    }

    void handleLeft(QKeyEvent *e);
    void handleEnter();
    void handleHistoryUp();
    void handleHistoryDown();
    void handleHome();
    void handleCompleter();
    void moveToEndOfLine();
    void clearLine();

    struct LastToken
    {
        QString     m_str;
        std::size_t m_offset{0};
        std::size_t m_len{0};
    };

    LastToken strGetLastToken(const QString &str) const;

    QString getCommand() const;
    void setCommand(QString &cmd);
    int getIndex(const QTextCursor &textCursor);

    QString m_promptStr{"> "};
    std::deque<QString> m_historyUp;
    std::deque<QString> m_historyDown;

    bool m_locked{false};
    bool m_historySkip{false};

    std::unique_ptr<ICompleter> m_completer;

signals:
    void command(const QString &command);
};

};  // namespace
