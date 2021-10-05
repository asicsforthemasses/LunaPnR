
#include "hatchbutton.h"
#include "../core/common/logging.h"
#include "hatchdialog.h"
#include <QColorDialog>
#include <QPainter>

using namespace GUI;

SelectHatchButton::SelectHatchButton(QWidget *parent)
{
    setFixedSize(QSize{80,80});
}

#if 0
void SelectHatchButton::changeHatch()
{
    HatchLibrary hatchLib;
    HatchDialog dialog(hatchLib, this);

    auto retval = dialog.exec();
    if (retval == QDialog::Accepted)
    {
        auto index = dialog.getHatchIndex();
        if (index >= 0)
        {
            setHatch(hatchLib.m_hatches.at(index));
            emit onHatchChanged();
        }        
    }
}
#endif

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
    painter.setBrush(Qt::white);
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
