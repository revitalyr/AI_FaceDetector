module;
#include "TestConstants.h"
#include <catch2/catch_test_macros.hpp>
#include <QImage>
#include <QPainter>
#include "../Core/ImageProcessingTypes.h"
module Core.ImageProcessor;

using namespace Glance::Core;
using namespace Glance::Tests::Constants;

struct ImageProcessorFixture {
    std::unique_ptr<ImageProcessor> processor;
    QImage colorImage;
    QImage grayImage;

    ImageProcessorFixture() {
        processor = std::make_unique<ImageProcessor>();
        colorImage = QImage(400, 400, QImage::Format_RGB32);
        colorImage.fill(QColor(100, 150, 200).rgb());
        grayImage = QImage(400, 400, QImage::Format_Grayscale8);
        grayImage.fill(128);
    }
};

TEST_CASE_METHOD(ImageProcessorFixture, "ImageProcessor creation", "[imageprocessor]") {
    REQUIRE(processor != nullptr);
}

TEST_CASE_METHOD(ImageProcessorFixture, "Convert to grayscale", "[imageprocessor]") {
    auto result = processor->convertToGrayscale(colorImage);
    REQUIRE(!result.isNull());
    REQUIRE(result.width() == colorImage.width());
    REQUIRE(result.height() == colorImage.height());
    // Note: implementation may return RGB32 or Grayscale8, both are valid
}

TEST_CASE_METHOD(ImageProcessorFixture, "Apply exposure", "[imageprocessor]") {
    Exposure exp{0.5f};
    auto result = processor->applyExposure(colorImage, exp);
    REQUIRE(!result.isNull());
    REQUIRE(result.size() == colorImage.size());
}

TEST_CASE_METHOD(ImageProcessorFixture, "Apply contrast", "[imageprocessor]") {
    Contrast contrast{1.2f};
    auto result = processor->applyContrast(colorImage, contrast);
    REQUIRE(!result.isNull());
    REQUIRE(result.size() == colorImage.size());
}

TEST_CASE_METHOD(ImageProcessorFixture, "Apply gamma", "[imageprocessor]") {
    Gamma gamma{1.5f};
    auto result = processor->applyGamma(colorImage, gamma);
    REQUIRE(!result.isNull());
    REQUIRE(result.size() == colorImage.size());
}

TEST_CASE_METHOD(ImageProcessorFixture, "Apply saturation", "[imageprocessor]") {
    Saturation sat{1.3f};
    auto result = processor->applySaturation(colorImage, sat);
    REQUIRE(!result.isNull());
    REQUIRE(result.size() == colorImage.size());
}

TEST_CASE_METHOD(ImageProcessorFixture, "Process image with params", "[imageprocessor]") {
    ProcessingParams params;
    params.exposure = Exposure{0.2f};
    params.contrast = Contrast{1.1f};
    auto result = processor->processImage(colorImage, params);
    REQUIRE(!result.isNull());
    REQUIRE(result.size() == colorImage.size());
}

TEST_CASE_METHOD(ImageProcessorFixture, "Calculate histogram", "[imageprocessor]") {
    auto hist = processor->calculateHistogram(grayImage, 0);
    REQUIRE(!hist.empty());
}

TEST_CASE_METHOD(ImageProcessorFixture, "Get pixel info", "[imageprocessor]") {
    auto info = processor->getPixelInfo(colorImage, QPoint(200, 200));
    REQUIRE(info.position == QPoint(200, 200));
}

TEST_CASE_METHOD(ImageProcessorFixture, "Apply brightness", "[imageprocessor]") {
    Brightness brightness{0.3f};
    auto result = processor->applyBrightness(colorImage, brightness);
    REQUIRE(!result.isNull());
    REQUIRE(result.size() == colorImage.size());
}

TEST_CASE_METHOD(ImageProcessorFixture, "Apply temperature", "[imageprocessor]") {
    Temperature temperature{0.5f};
    auto result = processor->applyTemperature(colorImage, temperature);
    REQUIRE(!result.isNull());
    REQUIRE(result.size() == colorImage.size());
}

TEST_CASE_METHOD(ImageProcessorFixture, "Apply tint", "[imageprocessor]") {
    Tint tint{0.2f};
    auto result = processor->applyTint(colorImage, tint);
    REQUIRE(!result.isNull());
    REQUIRE(result.size() == colorImage.size());
}

TEST_CASE_METHOD(ImageProcessorFixture, "Apply vibrance", "[imageprocessor]") {
    Vibrance vibrance{0.4f};
    auto result = processor->applyVibrance(colorImage, vibrance);
    REQUIRE(!result.isNull());
    REQUIRE(result.size() == colorImage.size());
}

TEST_CASE_METHOD(ImageProcessorFixture, "Apply highlights and shadows", "[imageprocessor]") {
    Highlights highlights{0.3f};
    Shadows shadows{0.2f};
    auto result = processor->applyHighlightsShadows(colorImage, highlights, shadows);
    REQUIRE(!result.isNull());
    REQUIRE(result.size() == colorImage.size());
}

TEST_CASE_METHOD(ImageProcessorFixture, "Apply details enhancement", "[imageprocessor]") {
    QImage dstImage(colorImage.size(), colorImage.format());
    Details details{0.5f};
    processor->applyDetailsEnhancement(colorImage, dstImage, details);
    REQUIRE(!dstImage.isNull());
    REQUIRE(dstImage.size() == colorImage.size());
}

TEST_CASE_METHOD(ImageProcessorFixture, "Apply LUT", "[imageprocessor]") {
    QString lutPath = QString::asprintf("%s%s", TEST_DATA_DIR, "/demo_lut.cube");
    auto result = processor->applyLUT(colorImage, lutPath);
    REQUIRE(!result.isNull());
    REQUIRE(result.size() == colorImage.size());
}