module;
#include <QDebug>
#include <QColor>
#include <QtGlobal>
#include <cmath>
module Core.ExposureOperation;

namespace Glance::Core {

ExposureOperation::ExposureOperation(float exposure)
    : m_exposure(exposure)
{
}

void ExposureOperation::apply(QImage& image) const
{
    if (image.format() != QImage::Format_RGB32 && image.format() != QImage::Format_ARGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }

    float factor = std::pow(2.0f, m_exposure);

    for (int y = 0; y < image.height(); ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            QRgb pixel = line[x];
            int r = qBound(0, static_cast<int>(qRed(pixel) * factor), 255);
            int g = qBound(0, static_cast<int>(qGreen(pixel) * factor), 255);
            int b = qBound(0, static_cast<int>(qBlue(pixel) * factor), 255);
            line[x] = qRgba(r, g, b, qAlpha(pixel));
        }
    }
}

void ExposureOperation::undo(QImage& image) const
{
    apply(image); // Exposure is symmetric
}

QString ExposureOperation::description() const
{
    return QString("Exposure: %1").arg(m_exposure);
}

} // namespace Glance::Core
