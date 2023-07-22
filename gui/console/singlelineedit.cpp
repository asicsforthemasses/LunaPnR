// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "singlelineedit.h"
#include <QKeyEvent>
#include <QBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QDialog>
#include <QScrollBar>
#include <QTextBlock>

#include "commandcompletion.inc"

using namespace GUI;

SingleLineEdit::SingleLineEdit(QWidget *parent) : QLineEdit(parent)
{
    m_overlay = new TxtOverlay(this);
    m_overlay->hide();
    reset();
}

bool SingleLineEdit::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *k = static_cast<QKeyEvent*>(event);
        if (k->key() == Qt::Key_Tab)
        {
            handleTabKeypress();
            updateHelpOverlay();
            event->accept();
            return true;
        }
    }
    return QLineEdit::event(event);
}

void SingleLineEdit::keyPressEvent(QKeyEvent *e)
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
        {
            m_historyReadIdx = 0;    
        }

        replaceCurrentCommand(QString(m_history[m_historyReadIdx].c_str()));
        e->accept();
        return;
    case Qt::Key_Tab:
        handleTabKeypress();
        updateHelpOverlay();
        e->accept();
        return;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        {            
            auto cmd = getCurrentCommand();

            // add to the command history
            if (m_historyWriteIdx >= m_history.size())
                m_historyWriteIdx = 0;

            QLineEdit::keyPressEvent(e);

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
            
            clear();
            m_overlay->hide();
            setFocus();
            e->accept();
        }
        return;
    case Qt::Key_Backspace:
        {
#if 0            
            QTextCursor cur = textCursor();
            auto column = cur.columnNumber();
            auto block  = cur.blockNumber();
            if ((block == m_promptBlock) && (column <= m_prompt.length()))
            {
                return;
            }
            else
#endif            
            {
                QLineEdit::keyPressEvent(e);
                updateHelpOverlay();                
            }
        }
        return;
    };

    // check if the cursor is on the last line
    // if not, move it there

#if 0
    QTextCursor cur = textCursor();
    auto block  = cur.blockNumber();        
    if (block != m_promptBlock)
    {
        cur.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        setTextCursor(cur);
    }
#endif

    QLineEdit::keyPressEvent(e);

    updateHelpOverlay();    
}

void SingleLineEdit::reset()
{
    m_historyReadIdx  = 0;
    m_historyWriteIdx = 0;
    m_history.clear();
    m_history.resize(32);
    m_promptEnabled = true;
}

QString SingleLineEdit::getCurrentCommand() const
{
    return text();
}

QRect SingleLineEdit::getCursorRect() const
{
    return cursorRect();
}

void SingleLineEdit::replaceCurrentCommand(const QString &cmd)
{
    setText(cmd);
}

void SingleLineEdit::handleTabKeypress()
{
    auto command = getCurrentCommand();

    auto cmdList = suggestCommand(command);
    if (cmdList.count() == 0)
    {
        //textCursor().insertText("\t");
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

void SingleLineEdit::updateHelpOverlay()
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

bool SingleLineEdit::canShowHelp(const QString &cmd) const
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

QString SingleLineEdit::getHelpString(const QString &cmd) const
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

QStringList SingleLineEdit::suggestCommand(QString partialCommand)
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


PopupCompleter::PopupCompleter(const QStringList& sl, QWidget *parent)
    : QDialog(parent, Qt::Popup)
{
    setModal(true);

    m_listWidget = new PopupListWidget();
    m_listWidget->setMaximumHeight(200);

    //qDebug() << "sizeHint(): " << m_listWidget->sizeHint();
    Q_FOREACH(QString str, sl) {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(str);
        m_listWidget->addItem(item);
    }
    
    //qDebug() << "sizeHint(): " << m_listWidget->sizeHint();
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

int PopupCompleter::exec(SingleLineEdit *parent)
{
    QSize popupSizeHint = this->sizeHint();
    auto cursorRect = parent->getCursorRect();
    QPoint globalPt = parent->mapToGlobal(cursorRect.bottomRight());

    auto currentScreen = QGuiApplication::primaryScreen();
    QRect screenGeom = currentScreen->availableGeometry();
    
    if (globalPt.y() + popupSizeHint.height() > screenGeom.height()) {
        globalPt = parent->mapToGlobal(cursorRect.topRight());
        globalPt.setY(globalPt.y() - popupSizeHint.height());
    }
    
    move(globalPt);
    setFocus();
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
