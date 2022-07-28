#include "blockframe.h"

using namespace GUI;

BlockFrame::BlockFrame(QWidget *parent) : QFrame(parent)
{
    m_layout = new QVBoxLayout();
    setFrameStyle(QFrame::Box);

    QPalette pal = QPalette();
    pal.setColor(QPalette::Window, Qt::white);

    setAutoFillBackground(true); 
    setPalette(pal);

    setLayout(m_layout);
}

void BlockFrame::addWidget(QWidget *widget, int stetch, Qt::Alignment alignment)
{
    m_layout->addWidget(widget, stetch, alignment);
}
