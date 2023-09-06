#include "pdktile.h"
#include <QGridLayout>
#include <QPixmap>
#include "widgets/flatimage.h"

namespace GUI
{

PDKTile::PDKTile(const PDKInfo &info, QWidget *parent) : QFrame(parent)
{
    m_defaultLineWidth = lineWidth();   // save if for later
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    setFrameStyle(QFrame::Box);

    auto layout = new QGridLayout();
    m_title = new QLabel(QString::fromStdString(info.m_title));
    m_version = new QLabel(QString::fromStdString(info.m_version));
    m_date = new QLabel(QString::fromStdString(info.m_date));

    // try and load the "_icon.png"
    QString iconFile = QString::fromStdString((info.m_path / "_icon.png").string());
    auto icon = new FlatImage(iconFile);
    layout->addWidget(icon,0,0,3,1, Qt::AlignCenter);
    layout->setColumnMinimumWidth(0, 100);  // icons are 64x64 so we make the column 100 pixels

    layout->addWidget(new QLabel("Title"),0,1);
    layout->addWidget(m_title,0,2,Qt::AlignLeft);
    layout->addWidget(new QLabel("Version"),1,1);
    layout->addWidget(m_version,1,2, Qt::AlignLeft);
    layout->addWidget(new QLabel("Date"),2,1);
    layout->addWidget(m_date,2, 2, Qt::AlignLeft);
    layout->setColumnStretch(2,1);  // make sure the last column can stretch

    setLayout(layout);
};

void PDKTile::mousePressEvent(QMouseEvent *e)
{
    std::cout << "clicked\n";
    emit clicked(m_id);
}

void PDKTile::setSelected(bool selected)
{
    if (selected)
    {
        setLineWidth(3);
    }
    else
    {
        setLineWidth(m_defaultLineWidth);
    }
}

};
