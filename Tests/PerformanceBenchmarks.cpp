module;
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <QImage>
#include <QElapsedTimer>
#include "../Core/ImageProcessingTypes.h"
module Core.ImageProcessor;
import Core.FaceDetector;

using namespace Glance::Core;

struct PerformanceFixture {
    QImage testImage;
    std::unique_ptr<ImageProcessor> processor;

    PerformanceFixture() {
        testImage = QImage(1920, 1080, QImage::Format_RGB32);
        testImage.fill(Qt::blue);
        processor = std::make_unique<ImageProcessor>();
    }
};

TEST_CASE_METHOD(PerformanceFixture, "Benchmark grayscale conversion", "[performance]") {
    BENCHMARK("grayscale") {
        return processor->convertToGrayscale(testImage);
    };
}

TEST_CASE_METHOD(PerformanceFixture, "Benchmark exposure adjustment", "[performance]") {
    Exposure exp{0.5f};
    BENCHMARK("exposure") {
        return processor->applyExposure(testImage, exp);
    };
}

TEST_CASE_METHOD(PerformanceFixture, "Benchmark contrast adjustment", "[performance]") {
    Contrast contrast{1.2f};
    BENCHMARK("contrast") {
        return processor->applyContrast(testImage, contrast);
    };
}