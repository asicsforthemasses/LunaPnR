// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "propertyview.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QFrame>

using namespace GUI;

#if 0
class ElidingLabel : public QWidget
{
public:
    explicit ElidingLabel(const QString &text = QString(),
                          QWidget *parent = nullptr) : QWidget(parent), m_text(text)
        { setContentsMargins(3, 2, 3, 2); }

    void setText(const QString &text) {
        m_text = text;
        updateGeometry();
    }
    void setElidemode(Qt::TextElideMode mode) {
        m_mode = mode;
        updateGeometry();
    }

protected:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent *e) override;

private:
    QString m_text;
    Qt::TextElideMode m_mode = Qt::ElideRight;
};

QSize ElidingLabel::sizeHint() const
{
    QSize size = fontMetrics().boundingRect(m_text).size();
    size += QSize(contentsMargins().left() + contentsMargins().right(),
                  contentsMargins().top() + contentsMargins().bottom());
    return size;
}

void ElidingLabel::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setPen(QColor(0, 0, 0, 60));
    painter.setBrush(QColor(255, 255, 255, 40));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    painter.setPen(palette().windowText().color());
    painter.drawText(contentsRect(), Qt::AlignLeft,
                     fontMetrics().elidedText(m_text, Qt::ElideRight, width(), 0));
}
#endif

PropertyView::PropertyView(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Testing123"));
    //layout->addSpacerItem(new QSpacerItem(0,1));
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);

    // make sure we don't stretch in the vertical direction
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

    // add grid layout to manage elements
    m_gridLayout = new QGridLayout();
    layout->addLayout(m_gridLayout);

    m_gridLayout->setColumnStretch(1,1);
    m_gridLayout->setSpacing(0);
    m_gridLayout->setContentsMargins(QMargins());

    setLayout(layout);

    addPropertyWidget(createTextLabel("test1"), 0,0);
    addPropertyWidget(createTextLabel("test2"), 0,1);
    addPropertyWidget(createTextLabel("test3"), 1,0);
    addPropertyWidget(createTextLabel("test4"), 1,1);
}

PropertyView::~PropertyView()
{

}

QLabel *PropertyView::createTextLabel(const std::string &name)
{
    auto label = new QLabel();
    label->setTextFormat(Qt::PlainText);
    label->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    label->setText(QString::fromStdString(name));
    label->setContentsMargins(QMargins(0,0,0,0));
    return label;
}

void PropertyView::addPropertyWidget(QWidget *widget, int32_t row, int32_t col)
{
    auto frame  = new QFrame();
    auto layout = new QHBoxLayout();
    layout->addWidget(widget);
    layout->setContentsMargins(QMargins(2,0,2,0));

    frame->setLayout(layout);
    frame->setFrameStyle(QFrame::Box);
    frame->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

    if (row == 0)
        frame->setContentsMargins(QMargins((col == 0) ? 1 : 0,1,1,1));
    else
        frame->setContentsMargins(QMargins((col == 0) ? 1 : 0,0,1,1));

    m_gridLayout->addWidget(frame, row, col);
}
