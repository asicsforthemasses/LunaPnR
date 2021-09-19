#pragma once

#include <vector>
#include <QPixmap>

namespace GUI
{

struct HatchLibrary
{
    HatchLibrary();
    std::vector<QPixmap> m_hatches;
};

};
