#include "pdktile.h"
#include <QGridLayout>

namespace GUI
{

PDKTile::PDKTile(const PDKInfo &info, QWidget *parent) : QFrame(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

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

    layout->addWidget(new QLabel("Title"),1,1);
    layout->addWidget(m_title,1,2);
    layout->addWidget(new QLabel("Version"),2,1);
    layout->addWidget(m_version,2,2);
    layout->addWidget(new QLabel("Date"),3,1);
    layout->addWidget(m_date,3,2);

    setLayout(layout);
};

};
