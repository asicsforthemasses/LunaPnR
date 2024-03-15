#pragma once
#include <QString>
#include <QPlainTextEdit>
#include <deque>

namespace GUI
{

class ReplWidget : public QPlainTextEdit
{
    Q_OBJECT
public:
    ReplWidget(QWidget *parent = 0);

    [[nodiscard]] QString prompt() const noexcept {return m_promptStr; }

    void setPrompt(const QString &prompt) noexcept
    {
        m_promptStr = prompt;
        clearLine();
    }

    void cmdReply(const QString &text);

    void lock()
    {
        m_locked = true;
    }

    void unlock()
    {
        m_locked = false;
        insertPlainText(m_promptStr);
    }

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
    void moveToEndOfLine();
    void clearLine();

    QString getCommand() const;
    int getIndex(const QTextCursor &textCursor);

    QString m_promptStr{"> "};
    std::deque<QString> m_historyUp;
    std::deque<QString> m_historyDown;

    bool m_locked{false};
    bool m_historySkip{false};

signals:
    void command(const QString &command);
};

};  // namespace
