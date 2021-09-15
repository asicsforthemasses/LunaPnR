
#include "hatchbutton.h"
#include "../core/common/logging.h"
#include <QColorDialog>
#include <QPainter>

using namespace GUI;

SelectHatchButton::SelectHatchButton(QWidget *parent)
{
    setFixedSize(QSize{80,80});
    connect(this, SIGNAL(clicked()), this, SLOT(changeHatch()));

    m_pixmap.load(":/hatch1.png", nullptr, Qt::NoFormatConversion);

    std::stringstream ss;
    ss << "Loaded pixmap depth: " << m_pixmap.depth() << "\n";
    doLog(LOG_INFO, ss);    
}

void SelectHatchButton::changeHatch()
{
    setHatch(m_pixmap);
    emit onHatchChanged();
}

void SelectHatchButton::setHatch(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    update();
}

QPixmap SelectHatchButton::getHatch() const
{
    return m_pixmap;
}

void SelectHatchButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    
    auto r = rect().adjusted(2,2,-2,-2);

    painter.setPen(Qt::black);
    if (!m_pixmap.isNull())
    {
        painter.setBrush(m_pixmap);
    }
    else
    {
        painter.setBrush(Qt::black);
    }
    painter.drawRect(r);
}
