#include <QBoxLayout>
#include "flatactiontile.h"

using namespace GUI;

FlatActionTile::FlatActionTile(const QString &actionTitle, 
    const QString &iconUrl,     
    const QString &actionIconUrl,     
    QWidget *parent) : QWidget(parent)
{
    auto hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(2,2,2,2);

    m_icon  = new FlatImage(iconUrl);
    m_actionTitle = new QLabel(actionTitle);
    m_actionButton = new FlatImageButton(actionIconUrl);

    hlayout->addWidget(m_icon);
    hlayout->addWidget(m_actionTitle, 1);
    hlayout->addWidget(m_actionButton);
    hlayout->setAlignment(Qt::AlignVCenter);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    connect(m_actionButton, &FlatImageButton::pressed, this, &FlatActionTile::onAction);

    setLayout(hlayout);
}

void FlatActionTile::setIcon(const QString &iconUrl)
{
    m_icon->setPixmap(iconUrl);
}

void FlatActionTile::setActionIcon(const QString &iconUrl)
{
    m_actionButton->setPixmap(iconUrl);
}

QString FlatActionTile::actionTitle() const
{
    return m_actionTitle->text();
}

void FlatActionTile::setActionTitle(const QString &name)
{
    m_actionTitle->setText(name);
}
