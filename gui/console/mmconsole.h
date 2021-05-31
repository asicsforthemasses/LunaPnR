// inspiration: https://github.com/jupyter/qtconsole/blob/master/qtconsole/console_widget.py

#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QByteArray>
#include <QKeyEvent>
#include <QHBoxLayout>

namespace GUI
{

class MMConsoleEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    MMConsoleEdit(QWidget *parent = nullptr);

    virtual void keyPressEvent(QKeyEvent *e) override;

signals:
    void newCommand(const char *txt);

protected:
    ssize_t m_historyWriteIdx;
    ssize_t m_historyReadIdx;
    std::vector<std::string> m_history;    
};

class MMConsole : public QWidget
{
    Q_OBJECT
    
public:
    MMConsole(QWidget *parent = nullptr);

    virtual QSize sizeHint() const override;

    void putData(const std::string &txt);
    void putData(const char *txt);
    void putData(const QByteArray &data);

    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void resizeEvent(QResizeEvent *e) override;

signals:
    void newCommand(const char *cmd);

protected slots:
    void onCommand(const char *cmd);
    

protected:
    //bool eventFilter(QObject *o, QEvent *e);
    //bool filterKeyEvent(QKeyEvent *evt);

    int getFontWidth(const QFont *font = nullptr) const;
    int getFontHeight(const QFont *font = nullptr) const;

    QHBoxLayout     *m_layout;
    MMConsoleEdit   *m_txt;
};

};
