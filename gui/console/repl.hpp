#pragma once
#include <QPlainTextEdit>

namespace GUI
{

class ReplWidget : public QPlainTextEdit
{
    Q_OBJECT
public:
    ReplWidget(QWidget *parent = 0);

    QString prompt() const;
    void setPrompt(const QString &prompt);

protected:
    void handleLeft(QKeyEvent *event);
    void handleEnter();
    void handleHistoryUp();
    void handleHistoryDown();
    void handleHome();

};

};  // namespace