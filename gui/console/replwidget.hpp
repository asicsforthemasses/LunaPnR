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

protected:
    void keyPressEvent(QKeyEvent *e);

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

public slots:
    void cmdReply(const QString &text, bool done = false);
};

};  // namespace
