#include "imagemanip.h"

namespace GUI
{

std::optional<QImage> makePDKIcon(const std::string &imageFilename)
{
    QImage img;
    if (!img.load(QString::fromStdString(imageFilename)))
    {
        return std::nullopt;
    }

    img = img.scaled(QSize(64,64), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return std::move(img);
}

};
