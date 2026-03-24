module;
#include <catch2/catch_test_macros.hpp>
#include <QImage>
#include <QJsonObject>
#include <QDir>
#include <QCoreApplication>
#include <memory>
#include "TestConstants.h"
module Core.EditStack;
module Core.LUT;
module Core.Orchestrator;
import Core.ImageProcessor;
import Core.FaceDetector;

using namespace Glance::Core;

// ============================================================================
// EditStack Tests
// ============================================================================

TEST_CASE("EditStack: Basic operations", "[editstack]") {
    EditStack stack;
    
    REQUIRE(stack.size() == 0);
    REQUIRE(!stack.canUndo());
    REQUIRE(!stack.canRedo());
}

TEST_CASE("EditStack: Push and undo", "[editstack]") {
    EditStack stack;
    QImage testImage(400, 300, QImage::Format_RGB32);
    testImage.fill(Qt::blue);
    
    // Create and push an exposure operation
    auto exposureOp = std::make_unique<ExposureOperation>();
    exposureOp->m_exposure = Exposure{0.5f};
    stack.push(std::move(exposureOp));
    
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.canUndo());
    REQUIRE(!stack.canRedo());
    
    stack.undo();
    
    REQUIRE(stack.canRedo());
    REQUIRE(!stack.canUndo());
}

TEST_CASE("EditStack: Multiple operations", "[editstack]") {
    EditStack stack;
    
    auto op1 = std::make_unique<ExposureOperation>();
    op1->m_exposure = Exposure{0.3f};
    stack.push(std::move(op1));
    
    auto op2 = std::make_unique<ExposureOperation>();
    op2->m_exposure = Exposure{0.5f};
    stack.push(std::move(op2));
    
    auto op3 = std::make_unique<ExposureOperation>();
    op3->m_exposure = Exposure{0.7f};
    stack.push(std::move(op3));
    
    REQUIRE(stack.size() == 3);
    REQUIRE(stack.canUndo());
    REQUIRE(!stack.canRedo());
    
    stack.undo();
    REQUIRE(stack.size() == 2);
    REQUIRE(stack.canUndo());
    REQUIRE(stack.canRedo());
    
    stack.redo();
    REQUIRE(stack.size() == 3);
    REQUIRE(stack.canUndo());
    REQUIRE(!stack.canRedo());
}

TEST_CASE("EditStack: Clear", "[editstack]") {
    EditStack stack;
    
    auto op = std::make_unique<ExposureOperation>();
    op->m_exposure = Exposure{0.5f};
    stack.push(std::move(op));
    
    REQUIRE(stack.size() == 1);
    
    stack.clear();
    
    REQUIRE(stack.size() == 0);
    REQUIRE(!stack.canUndo());
    REQUIRE(!stack.canRedo());
}

TEST_CASE("EditStack: Apply all", "[editstack]") {
    EditStack stack;
    QImage testImage(400, 300, QImage::Format_RGB32);
    testImage.fill(QColor(100, 150, 200));
    
    auto op1 = std::make_unique<ExposureOperation>();
    op1->m_exposure = Exposure{0.3f};
    stack.push(std::move(op1));
    
    auto op2 = std::make_unique<ExposureOperation>();
    op2->m_exposure = Exposure{0.5f};
    stack.push(std::move(op2));
    
    QImage result = testImage;
    stack.applyAll(result);
    
    REQUIRE(!result.isNull());
    REQUIRE(result.size() == testImage.size());
}

// ============================================================================
// LUT Tests
// ============================================================================

TEST_CASE("LUT: Parse valid cube file", "[lut]") {
    QString lutPath = TEST_DATA_DIR "/demo_lut.cube";
    
    auto lut = CubeLUT::parse(lutPath);
    
    REQUIRE(lut.has_value());
    REQUIRE(lut->isValid());
    REQUIRE(lut->size > 0);
}

TEST_CASE("LUT: Parse invalid file", "[lut]") {
    QString lutPath = "nonexistent_lut.cube";
    
    auto lut = CubeLUT::parse(lutPath);
    
    REQUIRE(!lut.has_value());
}

TEST_CASE("LUT: Apply to color values", "[lut]") {
    QString lutPath = TEST_DATA_DIR "/demo_lut.cube";
    auto lut = CubeLUT::parse(lutPath);
    
    REQUIRE(lut.has_value());
    
    float r = 0.5f;
    float g = 0.5f;
    float b = 0.5f;
    
    lut->apply(r, g, b);
    
    // Values should be modified (unless LUT is identity)
    REQUIRE(r >= 0.0f);
    REQUIRE(r <= 1.0f);
    REQUIRE(g >= 0.0f);
    REQUIRE(g <= 1.0f);
    REQUIRE(b >= 0.0f);
    REQUIRE(b <= 1.0f);
}

TEST_CASE("LUT: Invalid LUT structure", "[lut]") {
    CubeLUT lut;
    lut.size = 32;
    lut.data.resize(100); // Wrong size
    
    REQUIRE(!lut.isValid());
}

TEST_CASE("LUT: Valid LUT structure", "[lut]") {
    CubeLUT lut;
    lut.size = 32;
    lut.data.resize(32 * 32 * 32 * 3); // Correct size
    
    REQUIRE(lut.isValid());
}

// ============================================================================
// Orchestrator Tests
// ============================================================================

TEST_CASE("Orchestrator: Basic initialization", "[orchestrator]") {
    Orchestrator orchestrator;
    
    REQUIRE(!orchestrator.isInitialized());
    
    // Initialize without plugins (should use fallbacks)
    bool initialized = orchestrator.initialize("nonexistent_plugin_dir");
    
    // Should still initialize with fallback components
    REQUIRE(orchestrator.isInitialized() || !initialized);
}

TEST_CASE("Orchestrator: Process image with fallback", "[orchestrator]") {
    Orchestrator orchestrator;
    
    QImage testImage(400, 300, QImage::Format_RGB32);
    testImage.fill(Qt::red);
    
    ProcessingParams params;
    params.exposure = Exposure{0.3f};
    
    auto result = orchestrator.processImage(testImage, params);
    
    REQUIRE(!result.isNull());
    REQUIRE(result.size() == testImage.size());
}

TEST_CASE("Orchestrator: Detect faces with fallback", "[orchestrator]") {
    Orchestrator orchestrator;

    QImage testImage(400, 300, QImage::Format_RGB32);
    testImage.fill(Qt::blue);

    auto result = orchestrator.detectFaces(testImage);

    // Verify the result is valid (even if no faces detected, should return a result)
    REQUIRE(result.has_value());
    // For a solid blue image, we expect no faces, but the result should be valid
    REQUIRE(result->faces.empty());
}

TEST_CASE("Orchestrator: Error handling", "[orchestrator]") {
    Orchestrator orchestrator;
    
    REQUIRE(!orchestrator.hasError());
    
    // Try to set non-existent plugin
    bool success = orchestrator.setActiveImageProcessor("nonexistent");
    
    REQUIRE(!success);
    REQUIRE(orchestrator.hasError());
    
    orchestrator.clearError();
    REQUIRE(!orchestrator.hasError());
}

TEST_CASE("Orchestrator: Configuration", "[orchestrator]") {
    Orchestrator orchestrator;

    QJsonObject config;
    config["param1"] = 1.0;
    config["param2"] = "test";

    // Configure non-existent plugin (should fail gracefully)
    bool success = orchestrator.configureImageProcessor("nonexistent", config);

    // Verify it failed gracefully
    REQUIRE(!success);
    REQUIRE(orchestrator.hasError());
}

TEST_CASE("Orchestrator: Performance monitoring", "[orchestrator]") {
    Orchestrator orchestrator;

    orchestrator.startPerformanceMonitoring();

    QImage testImage(400, 300, QImage::Format_RGB32);
    testImage.fill(Qt::green);

    ProcessingParams params;
    auto result = orchestrator.processImage(testImage, params);

    auto metrics = orchestrator.getPerformanceMetrics();

    orchestrator.stopPerformanceMonitoring();

    // Verify metrics were collected
    REQUIRE(!result.isNull());
    REQUIRE(metrics.size() > 0);
}

// ============================================================================
// Segmenter Tests
// ============================================================================

TEST_CASE("Segmenter: Basic creation", "[segmenter]") {
    Segmenter segmenter;
    
    REQUIRE(!segmenter.isLoaded());
}

TEST_CASE("Segmenter: Load non-existent model", "[segmenter]") {
    Segmenter segmenter;
    
    bool loaded = segmenter.loadModel("nonexistent_model.onnx");
    
    REQUIRE(!loaded);
    REQUIRE(!segmenter.isLoaded());
}

TEST_CASE("Segmenter: Segment without model", "[segmenter]") {
    Segmenter segmenter;
    
    QImage testImage(400, 300, QImage::Format_RGB888);
    testImage.fill(Qt::blue);
    
    auto result = segmenter.segment(testImage);
    
    REQUIRE(!result.has_value());
}

TEST_CASE("Segmenter: Try to load actual model if available", "[segmenter]") {
    Segmenter segmenter;
    
    // Try common model paths
    QStringList modelPaths = {
        "models/u2netp.onnx",
        "../../models/u2netp.onnx",
        "../models/u2netp.onnx"
    };
    
    bool modelFound = false;
    for (const auto& path : modelPaths) {
        if (QFile::exists(path)) {
            bool loaded = segmenter.loadModel(path);
            if (loaded) {
                modelFound = true;
                break;
            }
        }
    }
    
    if (modelFound) {
        REQUIRE(segmenter.isLoaded());

        QImage testImage(400, 300, QImage::Format_RGB888);
        testImage.fill(Qt::blue);

        auto result = segmenter.segment(testImage);

        // Verify segmentation completed (even if no mask found)
        REQUIRE(result.has_value());
        REQUIRE(!result->composited.isNull());
    } else {
        // Skip if model not available
        SKIP("Segmentation model not available for testing");
    }
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_CASE("Integration: EditStack with ImageProcessor", "[integration][editstack]") {
    EditStack stack;
    ImageProcessor processor;
    
    QImage testImage(400, 300, QImage::Format_RGB32);
    testImage.fill(QColor(128, 128, 255));
    
    auto op1 = std::make_unique<ExposureOperation>();
    op1->m_exposure = Exposure{0.3f};
    stack.push(std::move(op1));
    
    auto op2 = std::make_unique<ExposureOperation>();
    op2->m_exposure = Exposure{0.5f};
    stack.push(std::move(op2));
    
    QImage result = testImage;
    stack.applyAll(result);
    
    REQUIRE(!result.isNull());
    REQUIRE(result.size() == testImage.size());
}

TEST_CASE("Integration: LUT with ImageProcessor", "[integration][lut]") {
    ImageProcessor processor;
    
    QString lutPath = TEST_DATA_DIR "/demo_lut.cube";
    auto lut = CubeLUT::parse(lutPath);
    
    if (lut.has_value()) {
        QImage testImage(400, 300, QImage::Format_RGB32);
        testImage.fill(QColor(200, 150, 100));
        
        auto result = processor.applyLUT(testImage, lutPath);
        
        REQUIRE(!result.isNull());
        REQUIRE(result.size() == testImage.size());
    } else {
        SKIP("LUT file not available");
    }
}

TEST_CASE("Integration: Orchestrator with multiple operations", "[integration][orchestrator]") {
    Orchestrator orchestrator;

    QImage testImage(400, 300, QImage::Format_RGB32);
    testImage.fill(QColor(100, 150, 200));

    ProcessingParams params;
    params.exposure = Exposure{0.3f};
    params.contrast = Contrast{1.2f};
    params.gamma = Gamma{1.5f};

    auto result = orchestrator.processImage(testImage, params);

    REQUIRE(!result.isNull());
    REQUIRE(result.size() == testImage.size());
}

// ============================================================================
// Multithreading Tests
// ============================================================================

TEST_CASE("Multithreading: Concurrent detectFacesAsync calls", "[multithreading][orchestrator]") {
    Orchestrator orchestrator;

    QImage testImage(400, 300, QImage::Format_RGB32);
    testImage.fill(Qt::blue);

    // Launch multiple concurrent detectFacesAsync calls
    std::vector<QFuture<FaceDetectionResult>> futures;
    const int numThreads = 10;

    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(orchestrator.detectFacesAsync(testImage));
    }

    // Wait for all futures to complete
    for (auto& future : futures) {
        REQUIRE(future.isFinished() || future.waitForFinished(5000));
    }

    // Verify all completed without crashes
    for (auto& future : futures) {
        REQUIRE(future.isFinished());
    }
}

TEST_CASE("Multithreading: Concurrent processImageAsync calls", "[multithreading][orchestrator]") {
    Orchestrator orchestrator;

    QImage testImage(400, 300, QImage::Format_RGB32);
    testImage.fill(Qt::green);

    ProcessingParams params;
    params.exposure = Exposure{0.3f};

    // Launch multiple concurrent processImageAsync calls
    std::vector<QFuture<QImage>> futures;
    const int numThreads = 10;

    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(orchestrator.processImageAsync(testImage, params));
    }

    // Wait for all futures to complete
    for (auto& future : futures) {
        REQUIRE(future.isFinished() || future.waitForFinished(5000));
    }

    // Verify all completed without crashes
    for (auto& future : futures) {
        REQUIRE(future.isFinished());
        REQUIRE(!future.result().isNull());
    }
}

TEST_CASE("Multithreading: Mixed concurrent async operations", "[multithreading][orchestrator]") {
    Orchestrator orchestrator;

    QImage testImage(400, 300, QImage::Format_RGB32);
    testImage.fill(Qt::red);

    ProcessingParams params;
    params.exposure = Exposure{0.3f};

    // Launch mixed concurrent operations
    std::vector<QFuture<QImage>> imageFutures;
    std::vector<QFuture<FaceDetectionResult>> faceFutures;

    for (int i = 0; i < 5; ++i) {
        imageFutures.push_back(orchestrator.processImageAsync(testImage, params));
        faceFutures.push_back(orchestrator.detectFacesAsync(testImage));
    }

    // Wait for all futures to complete
    for (auto& future : imageFutures) {
        REQUIRE(future.isFinished() || future.waitForFinished(5000));
    }
    for (auto& future : faceFutures) {
        REQUIRE(future.isFinished() || future.waitForFinished(5000));
    }

    // Verify all completed without crashes
    for (auto& future : imageFutures) {
        REQUIRE(future.isFinished());
        REQUIRE(!future.result().isNull());
    }
    for (auto& future : faceFutures) {
        REQUIRE(future.isFinished());
    }
}
