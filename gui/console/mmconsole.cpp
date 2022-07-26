/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/



#include <iostream>
#include <string_view>
#include <memory>
#include <array>

#include <QDebug>
#include <QFont>
#include <QScrollBar>
#include <QTextBlock>
#include <QDesktopWidget>
#include <QApplication>
#include <QSettings>

#include "mmconsole.h"

using namespace GUI;

#include "commandcompletion.inc"

MMConsole::MMConsole(QWidget *parent) : QTextEdit("", parent)
{
    setAcceptRichText(false);
    setUndoRedoEnabled(false);

    document()->setMaximumBlockCount(1000);

    reset();

    // default colours
    setColours(QColor("#1d1f21"), QColor("#c5c8c6"), QColor("#a54242"));

    m_prompt = "> ";

    m_overlay = new TxtOverlay(this);
    m_overlay->hide();
    m_overlay->setText("load_verilog(filename)");
}

void MMConsole::setColours(const QColor &bkCol, const QColor &promptCol, const QColor &errorCol) noexcept
{
    QPalette pal = palette();
    pal.setColor(QPalette::Window, bkCol);
    pal.setColor(QPalette::Base, bkCol);
    setPalette(pal);
    setAutoFillBackground(true);

    m_colours.m_bkCol     = bkCol;
    m_colours.m_promptCol = promptCol;
    m_colours.m_errorCol  = errorCol;
}

MMConsole::ConsoleColours MMConsole::getColours() const noexcept
{
    return m_colours;
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
    m_promptEnabled = true;    
}

void MMConsole::keyPressEvent(QKeyEvent *e)
{
    if (!m_promptEnabled)
    {
        return;
    }

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
    case Qt::Key_Tab:
        handleTabKeypress();
        updateHelpOverlay();
        return;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        {            
            auto cmd = getCurrentCommand();

            // add to the command history
            if (m_historyWriteIdx >= m_history.size())
                m_historyWriteIdx = 0;

            QTextEdit::keyPressEvent(e);

            m_history[m_historyWriteIdx] = cmd.toStdString();
            ++m_historyWriteIdx;
            m_historyReadIdx = m_historyWriteIdx;

#if 0
            // if the command does not have any parentheses
            // add '()' at the end to allow parameter-less
            // commands to be entered without additional ().

            if (!cmd.contains("("))
            {
                cmd.append("()");
            }
#endif
            emit executeCommand(cmd);
            m_overlay->hide();
        }
        return;
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
                updateHelpOverlay();                
            }
        }
        return;
    };

    // check if the cursor is on the last line
    // if not, move it there

    QTextCursor cur = textCursor();
    auto block  = cur.blockNumber();        
    if (block != m_promptBlock)
    {
        cur.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        setTextCursor(cur);
    }

    QTextEdit::keyPressEvent(e);

    updateHelpOverlay();
}

void MMConsole::updateHelpOverlay()
{
    auto cmd = getCurrentCommand();
    if (canShowHelp(cmd))
    {
        if (!m_overlay->isHidden())
        {
            return;
        }
        
        const int32_t overlayDistance = 4;  // pixels from the cursor y pos
        auto overlayPos = cursorRect().bottomRight();
        overlayPos += QPoint(0, overlayDistance);
        
        m_overlay->setText(getHelpString(cmd));
        m_overlay->move(overlayPos);
        
        // make sure the overlay is always visible,
        // even when the cursor is on the last line.
        // then, we move the overlay 2 lines up
        
        auto overlayBottomPos = m_overlay->height() + m_overlay->pos().y();
        if (overlayBottomPos >= rect().bottom())
        {
            QFontMetrics fm(font());
            auto lineHeight = fm.height();
            overlayPos.ry() -= lineHeight + m_overlay->height() + 2*overlayDistance;
            m_overlay->move(overlayPos);
        }
        
        m_overlay->show();
    }
    else
    {
        m_overlay->hide();
    }
}

bool MMConsole::canShowHelp(const QString &cmd) const
{
    for(auto cmdhelp : gs_helptxt)
    {
        if (cmd.startsWith(cmdhelp.cmdstr))
        {
            return true;
        }
    }

    return false;
}

QString MMConsole::getHelpString(const QString &cmd) const
{
    for(auto cmdhelp : gs_helptxt)
    {
        if (cmd.startsWith(cmdhelp.cmdstr))
        {
            return cmdhelp.helpstr;
        }
    }
    return QString("");
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

void MMConsole::handleTabKeypress()
{
    auto command = getCurrentCommand();

    auto cmdList = suggestCommand(command);
    if (cmdList.count() == 0)
    {
        textCursor().insertText("\t");
    }
    else
    {
        if (cmdList.count() == 1)
        {
            replaceCurrentCommand(cmdList[0]);
        }
        else
        {            
            auto popup = std::make_unique<PopupCompleter>(cmdList);
            if (popup->exec(this) == QDialog::Accepted)
            { 
                replaceCurrentCommand(popup->selected());
            }
        }
    }
}

QStringList MMConsole::suggestCommand(QString partialCommand)
{
    QStringList cmdOptions;
    for(auto const& cmds : gs_helptxt)
    {
        QString cmd = cmds.cmdstr;
        if (cmd.startsWith(partialCommand))
        {
            cmdOptions.append(cmd);
        }
    }

    return cmdOptions;
}

void MMConsole::appendWithoutNewline(const QString &txt)
{
    // moving the cursor to ::END make the document
    // forget the textColor, so we re-apply it.
    auto col = textColor();
    moveCursor(QTextCursor::End);
    setTextColor(col);

    insertPlainText(txt);

    moveCursor(QTextCursor::End);
}

void MMConsole::print(const QString &txt, PrintType pt)
{
    if (pt == PrintType::Error)
    {
        setTextColor(m_colours.m_errorCol);
    }
    else
    {
        setTextColor(m_colours.m_promptCol);
    }

    appendWithoutNewline(txt);

    if ((pt == PrintType::Complete) || (pt == PrintType::Error))
    {
        if (!txt.endsWith("\n"))
        {
            append("");
        }

        if (m_promptEnabled) 
        {
            displayPrompt();
        }
    }
}

void MMConsole::setPrompt(const QString &prompt)
{
    m_prompt = prompt;
}

void MMConsole::displayPrompt()
{
    setUndoRedoEnabled(false);

    setTextColor(m_colours.m_promptCol);
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

void MMConsole::print(const std::string_view txt, PrintType pt)
{
    print(QString::fromUtf8(&txt.at(0), txt.size()), pt);
}

void MMConsole::print(const std::stringstream &ss, PrintType pt)
{
    print(QString::fromStdString(ss.str()), pt);
}

void MMConsole::print(const char *txt, PrintType pt)
{
    print(QString::fromUtf8(txt), pt);
}



PopupCompleter::PopupCompleter(const QStringList& sl, QWidget *parent)
    : QDialog(parent, Qt::Popup)
{
    setModal(true);

    m_listWidget = new PopupListWidget();
    m_listWidget->setMaximumHeight(200);

    qDebug() << "sizeHint(): " << m_listWidget->sizeHint();
    Q_FOREACH(QString str, sl) {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(str);
        m_listWidget->addItem(item);
    }
    
    qDebug() << "sizeHint(): " << m_listWidget->sizeHint();
    m_listWidget->setFixedSize(m_listWidget->sizeHint());

    QLayout *layout = new QVBoxLayout();
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_listWidget);

    setLayout(layout);

    // connect signal
    connect(m_listWidget, &QListWidget::itemActivated, this, &PopupCompleter::onItemActivated);
}

PopupCompleter::~PopupCompleter()
{
}

void PopupCompleter::showEvent(QShowEvent *event)
{
    m_listWidget->setFocus();
}

void PopupCompleter::onItemActivated(QListWidgetItem *event)
{
    m_selected = event->text();
    done(QDialog::Accepted);
}

int PopupCompleter::exec(QTextEdit *parent)
{
    QSize popupSizeHint = this->sizeHint();
    QRect cursorRect = parent->cursorRect();
    QPoint globalPt = parent->mapToGlobal(cursorRect.bottomRight());
    QDesktopWidget *dsk = QApplication::desktop();
    QRect screenGeom = dsk->screenGeometry(dsk->screenNumber(this));
    
    if (globalPt.y() + popupSizeHint.height() > screenGeom.height()) {
        globalPt = parent->mapToGlobal(cursorRect.topRight());
        globalPt.setY(globalPt.y() - popupSizeHint.height());
    }
    this->move(globalPt);
    this->setFocus();
    return QDialog::exec();
}




void PopupListWidget::keyPressEvent(QKeyEvent *e) 
{
    if (e->key() == Qt::Key_Tab || e->key() == Qt::Key_Return)
    {
        emit(itemActivated(currentItem()));
    }
    else
    {
        QListWidget::keyPressEvent(e);
    }
}


QSize PopupListWidget::sizeHint() const
{
    QAbstractItemModel *model = this->model();
    QAbstractItemDelegate *delegate = this->itemDelegate();
    const QStyleOptionViewItem sovi;
    int left, top, right, bottom = 0;

    QMargins margin = this->contentsMargins();

    top = margin.top();
    bottom = margin.bottom();
    left = margin.left();
    right = margin.right();

    const int vOffset = top + bottom;
    const int hOffset = left + right;

    bool vScrollOn = false;
    int height = 0;
    int width = 0;
    for (int i=0; i<this->count(); ++i) {
        QModelIndex index = model->index(i, 0);
        QSize itemSizeHint = delegate->sizeHint(sovi, index);
        if (itemSizeHint.width() > width)
            width = itemSizeHint.width();

        // height
        const int nextHeight = height + itemSizeHint.height();
        if (nextHeight + vOffset < this->maximumHeight())
            height = nextHeight;
        else {
            // early termination
            vScrollOn = true;
            break;
        }
    }

    QSize sizeHint(width + hOffset, 0);
    sizeHint.setHeight(height + vOffset);
    if (vScrollOn) 
    {
        int scrollWidth = verticalScrollBar()->sizeHint().width();
        sizeHint.setWidth(sizeHint.width() + scrollWidth);
    }
    return sizeHint;
}