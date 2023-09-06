#include "pdktile.h"
#include <QGridLayout>
#include <QPixmap>
#include "widgets/flatimage.h"

namespace GUI
{

PDKTile::PDKTile(const PDKInfo &info, QWidget *parent) : QFrame(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setFrameStyle(QFrame::Box);

    auto layout = new QGridLayout();
    layout->setSizeConstraint(QLayout::SetFixedSize);

    m_title = new QLineEdit(QString::fromStdString(info.m_title));
    m_title->setReadOnly(true);
    m_title->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_version = new QLineEdit(QString::fromStdString(info.m_version));
    m_version->setReadOnly(true);
    m_version->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_date = new QLineEdit(QString::fromStdString(info.m_date));
    m_date->setReadOnly(true);
    m_date->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    // try and load the "_icon.png"
    QString iconFile = QString::fromStdString((info.m_path / "_icon.png").string());
    auto icon = new FlatImage(iconFile);
    layout->addWidget(icon,1,1,3,1, Qt::AlignCenter);
    layout->setColumnMinimumWidth(1, 100);  // icons are 64x64 so we make the column 100 pixels

    layout->addWidget(new QLabel("Title"),1,2);
    layout->addWidget(m_title,1,3);
    layout->addWidget(new QLabel("Version"),2,2);
    layout->addWidget(m_version,2,3);
    layout->addWidget(new QLabel("Date"),3,2);
    layout->addWidget(m_date,3, 3);

    setLayout(layout);
};

};
