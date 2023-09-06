#pragma once
#include <string>
#include <optional>
#include <QImage>

namespace GUI
{

std::optional<QImage> makePDKIcon(const std::string &imageFilename);

};