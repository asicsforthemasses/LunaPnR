// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "blockcontainer.h"

using namespace GUI;

BlockContainerHeader::BlockContainerHeader(BlockContainer *owner, QWidget *parent)
{
    BlockContainerHeader("", owner, parent);
}

BlockContainerHeader::BlockContainerHeader(const QString &name, BlockContainer *owner, QWidget *parent) 
    : QFrame(parent), m_owner(owner)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    m_hlayout = new QHBoxLayout();

    // create fold button
    m_foldButton = new FlatToggleButton();
    m_foldButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_hlayout->addWidget(m_foldButton);

    // create name label
    m_name = new ClickableLabel(name);
    m_hlayout->addWidget(m_name, 1);
    
    setLayout(m_hlayout);

    connect(m_foldButton, &FlatToggleButton::toggled, this, &BlockContainerHeader::onFoldButtonToggled);
    connect(m_name, &ClickableLabel::clicked, this, &BlockContainerHeader::onLabelClicked);
}

void BlockContainerHeader::onFoldButtonToggled()
{
    if (m_owner != nullptr)
    {
        m_owner->setFolded(isFolded());
    }
}

void BlockContainerHeader::onLabelClicked()
{
    m_foldButton->toggle();
}

bool BlockContainerHeader::isFolded() const
{
    return m_foldButton->isChecked();
}

void BlockContainerHeader::setName(const QString &name)
{
    m_name->setText(name);
}

void BlockContainerHeader::addWidget(QWidget *widget)
{
    m_hlayout->addWidget(widget,0, Qt::AlignRight);
}

BlockContainer::BlockContainer(QWidget *parent) : QWidget(parent)
{
    m_vlayout = new QVBoxLayout();
    m_vlayout->setContentsMargins(0,0,0,0);
    m_vlayout->setSpacing(0);
    //m_vlayout->setMargin(0);

    m_header  = new BlockContainerHeader("UNNAMED", this);
    m_contentFrame = new QFrame();    
    m_contentFrame->setFrameStyle(QFrame::Box);
    
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setSpacing(0);
    //m_contentLayout->setMargin(0);
    
    m_contentFrame->setLayout(m_contentLayout);
    m_contentFrame->setContentsMargins(0,0,0,0);

    m_vlayout->addWidget(m_header);
    m_vlayout->addWidget(m_contentFrame, 1);
    setLayout(m_vlayout);

    setFolded(m_header->isFolded());

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
}

void BlockContainer::setBlockName(const QString &name)
{
    m_header->setName(name);
}

void BlockContainer::onHeaderFoldToggled(bool state)
{
    setFolded(m_header->isFolded());
}

void BlockContainer::addWidget(QWidget *widget, int stretch)
{
    m_contentLayout->addWidget(widget, stretch);
}

bool BlockContainer::isFolded() const
{
    return m_folded;
}

void BlockContainer::setFolded(bool state)
{
    m_folded = state;
    if (m_folded)
    {
        m_contentFrame->show();
    }
    else
    {
        m_contentFrame->hide();
    }
}
