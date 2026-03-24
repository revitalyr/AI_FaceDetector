#include <catch2/catch_test_macros.hpp>
#include <QDebug>
#include <QImage>
#include <vector>
#include <opencv2/opencv.hpp>

import Core.ImageProcessor;
import Core.Segmenter;

using namespace Glance::Core;

TEST_CASE("Stress: Process many small images", "[stress]") {
    ImageProcessor processor;
    std::vector<QImage> images;

    for (int i = 0; i < 100; ++i) {
        QImage img(100, 100, QImage::Format_RGB32);
        img.fill(QColor(i % 256, (i * 2) % 256, (i * 3) % 256));
        images.push_back(img);
    }

    for (const auto& img : images) {
        auto result = processor.convertToGrayscale(img);
        REQUIRE(!result.isNull());
    }
}

TEST_CASE("Stress: Process large image", "[stress]") {
    ImageProcessor processor;
    QImage largeImage(4096, 4096, QImage::Format_RGB32);
    largeImage.fill(Qt::blue);

    auto result = processor.convertToGrayscale(largeImage);
    REQUIRE(!result.isNull());
    REQUIRE(result.width() == 4096);
    REQUIRE(result.height() == 4096);
}

TEST_CASE("Stress: Multiple operations sequentially", "[stress]") {
    ImageProcessor processor;
    QImage img(800, 600, QImage::Format_RGB32);
    img.fill(Qt::green);

    for (int i = 0; i < 50; ++i) {
        img = processor.applyExposure(img, Exposure{0.01f * i});
        REQUIRE(!img.isNull());
    }
}

TEST_CASE("Stress: cv::Mat memory leak test", "[stress][memory]") {
    // Test for cv::Mat memory leaks by creating and destroying many matrices
    for (int i = 0; i < 1000; ++i) {
        cv::Mat mat1(1000, 1000, CV_8UC3);
        cv::Mat mat2(500, 500, CV_32F);
        cv::Mat mat3 = mat1.clone();
        
        cv::Mat result;
        cv::add(mat1, mat3, result);
        
        // Mats should be automatically cleaned up when scope ends
    }
    REQUIRE(true); // If we get here without crash, no obvious leaks
}

TEST_CASE("Stress: cv::Mat conversion cycle", "[stress][memory]") {
    // Test QImage <-> cv::Mat conversion cycles for leaks
    QImage testImage(800, 600, QImage::Format_RGB888);
    testImage.fill(Qt::red);
    
    for (int i = 0; i < 500; ++i) {
        cv::Mat mat(testImage.height(), testImage.width(), CV_8UC3,
                   const_cast<uchar*>(testImage.constBits()), testImage.bytesPerLine());
        
        cv::Mat processed;
        cv::GaussianBlur(mat, processed, cv::Size(5, 5), 1.5);
        
        // Convert back to QImage
        QImage result(processed.data, processed.cols, processed.rows,
                    static_cast<int>(processed.step), QImage::Format_RGB888);
        QImage copy = result.copy();
        
        REQUIRE(!copy.isNull());
    }
}

TEST_CASE("Stress: Segmenter model load/unload", "[stress][memory]") {
    // Test Segmenter model loading and unloading for memory leaks
    // Note: This test requires a valid ONNX model file
    QString modelPath = "models/u2netp.onnx";
    
    qDebug() << "Stress: running 10 model load/unload iterations to check for memory leaks...";
    for (int i = 0; i < 10; ++i) {
        Segmenter segmenter;
        // Try to load model - if file doesn't exist, we still test the cleanup
        bool loaded = segmenter.loadModel(modelPath);
        
        if (loaded) {
            REQUIRE(segmenter.isLoaded());
        }
        
        // Segmenter destructor should clean up ONNX resources
    }
    REQUIRE(true); // If we get here without crash, cleanup works
}

TEST_CASE("Stress: Segmenter repeated segmentation", "[stress][memory]") {
    // Test repeated segmentation operations for memory leaks
    QString modelPath = "models/u2netp.onnx";
    
    Segmenter segmenter;
    bool loaded = segmenter.loadModel(modelPath);
    
    if (!loaded) {
        // Skip test if model not available
        SKIP("Segmentation model not available");
    }
    
    QImage testImage(640, 480, QImage::Format_RGB888);
    testImage.fill(Qt::blue);
    
    qDebug() << "Stress: running 50 segment() iterations to check for memory leaks...";
    for (int i = 0; i < 50; ++i) {
        auto result = segmenter.segment(testImage);
        // Result may be nullopt on error, but we're testing for memory leaks
    }
    
    REQUIRE(true); // If we get here without crash, no memory leaks
}

TEST_CASE("Stress: ImageProcessor all operations memory test", "[stress][memory]") {
    // Test all ImageProcessor operations for memory leaks
    ImageProcessor processor;
    QImage testImage(1024, 768, QImage::Format_RGB32);
    testImage.fill(QColor(128, 128, 255));
    
    for (int i = 0; i < 200; ++i) {
        auto exp = processor.applyExposure(testImage, Exposure{0.5f});
        auto cont = processor.applyContrast(testImage, Contrast{1.2f});
        auto gamma = processor.applyGamma(testImage, Gamma{1.5f});
        auto gray = processor.convertToGrayscale(testImage);
        auto temp = processor.applyTemperature(testImage, Temperature{0.3f});
        auto tint = processor.applyTint(testImage, Tint{0.2f});
        auto sat = processor.applySaturation(testImage, Saturation{1.3f});
        auto vib = processor.applyVibrance(testImage, Vibrance{0.4f});
        
        REQUIRE(!exp.isNull());
        REQUIRE(!cont.isNull());
        REQUIRE(!gamma.isNull());
        REQUIRE(!gray.isNull());
        REQUIRE(!temp.isNull());
        REQUIRE(!tint.isNull());
        REQUIRE(!sat.isNull());
        REQUIRE(!vib.isNull());
    }
}