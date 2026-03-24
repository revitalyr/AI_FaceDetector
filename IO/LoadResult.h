/**
 * @file LoadResult.h
 * @brief Result structure for asynchronous image loading operations
 */
#pragma once

#include <QImage>
#include <QString>
#include <QSize>

namespace Glance::IO {

/** @brief Result of an image loading operation with metadata */
struct LoadResult {
    QImage image;
    QString filePath;
    bool success = false;
    QString errorMessage;
    qint64 loadTimeMs = 0;
    QSize originalSize;
    QString format;
};

} // namespace Glance::IO
