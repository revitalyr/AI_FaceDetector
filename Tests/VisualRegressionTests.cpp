module;
#include <catch2/catch_test_macros.hpp>
#include <QImage>
#include <QCryptographicHash>
#include "../Core/ImageProcessingTypes.h"
module Core.ImageProcessor;

using namespace Glance::Core;

static QByteArray imageHash(const QImage& img) {
    if (img.isNull()) return QByteArray();
    QImage copy = img.convertToFormat(QImage::Format_ARGB32);
    QByteArray data(reinterpret_cast<const char*>(copy.bits()), copy.sizeInBytes());
    return QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
}

TEST_CASE("Visual regression: Grayscale is deterministic", "[visual]") {
    ImageProcessor processor;

    QImage testImage(400, 300, QImage::Format_RGB32);
    testImage.fill(QColor(100, 150, 200));

    auto result1 = processor.convertToGrayscale(testImage);
    auto result2 = processor.convertToGrayscale(testImage);

    REQUIRE(!result1.isNull());
    REQUIRE(!result2.isNull());
    REQUIRE(imageHash(result1) == imageHash(result2));
}

TEST_CASE("Visual regression: Exposure is deterministic", "[visual]") {
    ImageProcessor processor;

    QImage testImage(400, 300, QImage::Format_RGB32);
    testImage.fill(Qt::yellow);

    auto result1 = processor.applyExposure(testImage, Exposure{0.5f});
    auto result2 = processor.applyExposure(testImage, Exposure{0.5f});

    REQUIRE(!result1.isNull());
    REQUIRE(!result2.isNull());
    REQUIRE(imageHash(result1) == imageHash(result2));
}