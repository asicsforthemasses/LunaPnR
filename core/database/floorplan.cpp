// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "floorplan.h"

using namespace ChipDB;

void Floorplan::clear()
{
    m_coreSize = Size64();
    m_io2coreMargins = Margins64();
    m_ioMargins = Margins64();
}

Rect64 Floorplan::coreRect() const noexcept
{
    Rect64 core;
    core.setLL(
        Coord64(
            m_io2coreMargins.left() +
            m_ioMargins.left(),
            m_io2coreMargins.bottom() +
            m_ioMargins.bottom()
        )
    );

    core.setSize(m_coreSize);
    return core;
}

Size64 Floorplan::dieSize() const noexcept
{
    // calculate the die size
    auto width = m_coreSize.m_x +
        m_io2coreMargins.left() +
        m_io2coreMargins.right() +
        m_ioMargins.left() +
        m_ioMargins.right();

    auto height = m_coreSize.m_y +
        m_io2coreMargins.top() +
        m_io2coreMargins.bottom() +
        m_ioMargins.top() +
        m_ioMargins.bottom();

    return Size64(width, height);
}

void Floorplan::addListener(INamedStorageListener *listener)
{

}

void Floorplan::removeListener(INamedStorageListener *listener)
{

}

void Floorplan::contentsChanged() const
{

}
