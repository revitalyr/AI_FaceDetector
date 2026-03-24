module;
#include <catch2/catch_test_macros.hpp>
#include <QImage>
#include <QDir>
#include <QCoreApplication>
#include "../Core/ImageProcessingTypes.h"
#include "TestConstants.h"
module Core.ImageProcessor;
import IO.ImageReader;
import IO.AsyncImageLoader;
import Core.FaceDetector;
import Core.ProcessingPipeline;

using namespace Glance::Core;
using namespace Glance::IO;
using namespace Glance::Tests::Constants;

TEST_CASE("Integration: Load real image and process", "[integration]") {
    ImageReader reader;
    ImageProcessor processor;

    // Load actual test image from disk
    QString exePath = QCoreApplication::applicationFilePath();
    QFileInfo exeInfo(exePath);
    QString buildDir = exeInfo.absolutePath();
    QString testDataDir = QDir::cleanPath(QDir(buildDir).absoluteFilePath("../../Tests/TestData"));
    QDir dir(testDataDir);
    QString imagePath = dir.filePath("300.jpg");
    auto loadedImageOpt = reader.read(imagePath);
    REQUIRE(loadedImageOpt.has_value());
    QImage loadedImage = loadedImageOpt.value();
    
    REQUIRE(!loadedImage.isNull());
    REQUIRE(loadedImage.width() > 0);
    REQUIRE(loadedImage.height() > 0);

    // Process with real image
    auto grayscale = processor.convertToGrayscale(loadedImage);
    REQUIRE(!grayscale.isNull());
    REQUIRE(grayscale.size() == loadedImage.size());

    auto withExposure = processor.applyExposure(loadedImage, Exposure{0.3f});
    REQUIRE(!withExposure.isNull());
    REQUIRE(withExposure.size() == loadedImage.size());

    auto withContrast = processor.applyContrast(withExposure, Contrast{1.2f});
    REQUIRE(!withContrast.isNull());
    REQUIRE(withContrast.size() == loadedImage.size());
}

// TEST_CASE("Integration: Async image loader with processor", "[integration]") {
//     AsyncImageLoader loader;
//     ImageProcessor processor;
//
//     QString imagePath = QString(TEST_DATA_DIR) + "/300.jpg";
//
//     // Load image asynchronously
//     auto future = loader.loadImageAsync(imagePath);
//     future.waitForFinished();
//
//     auto loadResult = future.result();
//     REQUIRE(loadResult.success);
//     REQUIRE(!loadResult.image.isNull());
//
//     // Process the loaded image
//     auto processed = processor.applyGamma(loadResult.image, Gamma{1.5f});
//     REQUIRE(!processed.isNull());
//     REQUIRE(processed.size() == loadResult.image.size());
// }

TEST_CASE("Integration: Full pipeline - load, detect faces, process", "[integration]") {
    ImageReader reader;
    ImageProcessor processor;
    FaceDetector detector;

    QString exePath = QCoreApplication::applicationFilePath();
    QFileInfo exeInfo(exePath);
    QString buildDir = exeInfo.absolutePath();
    QString testDataDir = QDir::cleanPath(QDir(buildDir).absoluteFilePath("../../Tests/TestData"));
    QDir dir(testDataDir);
    QString imagePath = dir.filePath("300.jpg");
    auto loadedImageOpt = reader.read(imagePath);
    REQUIRE(loadedImageOpt.has_value());
    QImage loadedImage = loadedImageOpt.value();
    
    REQUIRE(!loadedImage.isNull());

    // Detect faces
    auto faceResult = detector.detectFaces(loadedImage);
    
    // Process image regardless of face detection result
    ProcessingParams params;
    params.exposure = Exposure{0.2f};
    params.contrast = Contrast{1.1f};
    params.saturation = Saturation{1.2f};
    
    auto processed = processor.processImage(loadedImage, params);
    REQUIRE(!processed.isNull());
    REQUIRE(processed.size() == loadedImage.size());
}

TEST_CASE("Integration: Multiple image batch processing", "[integration]") {
    ImageReader reader;
    ImageProcessor processor;

    QString exePath = QCoreApplication::applicationFilePath();
    QFileInfo exeInfo(exePath);
    QString buildDir = exeInfo.absolutePath();
    QString testDataDir = QDir::cleanPath(QDir(buildDir).absoluteFilePath("../../Tests/TestData"));
    QDir dir(testDataDir);
    QStringList imagePaths = {
        dir.filePath("300.jpg"),
        dir.filePath("300_1.jpg"),
        dir.filePath("300_2.jpg")
    };

    for (const auto& path : imagePaths) {
        auto imgOpt = reader.read(path);
        REQUIRE(imgOpt.has_value());
        QImage img = imgOpt.value();
        REQUIRE(!img.isNull());

        auto processed = processor.processImage(img, ProcessingParams{});
        REQUIRE(!processed.isNull());
        REQUIRE(processed.size() == img.size());
    }
}

// TEST_CASE("Integration: ProcessingPipeline with real image", "[integration]") {
//     QString imagePath = QString(TEST_DATA_DIR) + "/300.jpg";
//
//     ProcessingPipeline pipeline;
//     ProcessingParams params;
//     params.exposure = Exposure{0.3f};
//     params.contrast = Contrast{1.2f};
//
//     auto future = pipeline.processImageAsync(imagePath, params);
//     future.waitForFinished();
//
//     QImage result = future.result();
//     REQUIRE(!result.isNull());
//     REQUIRE(result.width() > 0);
//     REQUIRE(result.height() > 0);
// }

TEST_CASE("Integration: ImageReader format detection", "[integration]") {
    ImageReader reader;

    QString exePath = QCoreApplication::applicationFilePath();
    QFileInfo exeInfo(exePath);
    QString buildDir = exeInfo.absolutePath();
    QString testDataDir = QDir::cleanPath(QDir(buildDir).absoluteFilePath("../../Tests/TestData"));
    QDir dir(testDataDir);
    QString imagePath = dir.filePath("300.jpg");
    auto formatInfo = reader.detectFormat(imagePath);

    REQUIRE(formatInfo != Glance::IO::ImageFormat::Unknown);
}

TEST_CASE("Integration: LUT application with real image", "[integration]") {
    ImageReader reader;
    ImageProcessor processor;

    QString exePath = QCoreApplication::applicationFilePath();
    QFileInfo exeInfo(exePath);
    QString buildDir = exeInfo.absolutePath();
    QString testDataDir = QDir::cleanPath(QDir(buildDir).absoluteFilePath("../../Tests/TestData"));
    QDir dir(testDataDir);
    QString imagePath = dir.filePath("300.jpg");
    QString lutPath = dir.filePath("demo_lut.cube");

    auto loadedImageOpt = reader.read(imagePath);
    REQUIRE(loadedImageOpt.has_value());
    QImage loadedImage = loadedImageOpt.value();
    REQUIRE(!loadedImage.isNull());

    auto withLUT = processor.applyLUT(loadedImage, lutPath);
    REQUIRE(!withLUT.isNull());
    REQUIRE(withLUT.size() == loadedImage.size());
}