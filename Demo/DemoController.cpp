#include "DemoController.h"
#include "AsyncImageLoader.h"
#include <QDir>
#include <QStandardPaths>
#include <QPainter>
#include <QtConcurrent>
#include <QFuture>
#include <QApplication>
#include <QDebug>

namespace Glance::Demo {

DemoController::DemoController(QObject *parent)
    : QObject(parent)
    , m_autoDemoTimer(new QTimer(this))
    , m_currentDemoStep(0)
    , m_totalDemoSteps(8)
    , m_autoDemoRunning(false)
{
    // Initialize components
    m_imageProcessor = std::make_unique<Core::ImageProcessor>();
    m_faceDetector = std::make_unique<Core::FaceDetector>();
    m_asyncLoader = std::make_unique<IO::AsyncImageLoader>();

    // Setup demo data directory - use actual demo images
    m_demoDataDir = QApplication::applicationDirPath() + "/Demo/data";
    if (!QDir(m_demoDataDir).exists()) {
        // Fallback to development path
        m_demoDataDir = "../Demo/data";
    }
    qDebug() << "Demo data directory:" << m_demoDataDir;

    // Setup auto demo timer
    m_autoDemoTimer->setInterval(3000); // 3 seconds per step
    connect(m_autoDemoTimer, &QTimer::timeout, this, [this]() { onAutoDemoTimer(); });

    updateStatus("Demo controller initialized");
}

DemoController::~DemoController()
{
    // Cleanup demo data
    QDir demoDir(m_demoDataDir);
    if (demoDir.exists()) {
        demoDir.removeRecursively();
    }
}

void DemoController::initializeDemo()
{
    updateStatus("Initializing demo...");
    createDemoImages();
    loadDemoImages();
    m_currentDemoStep = 0;
    demoStepChanged(m_currentDemoStep, m_totalDemoSteps);
    updateStatus("Demo initialized successfully");
}

void DemoController::startAutoDemo()
{
    if (m_autoDemoRunning) {
        return;
    }

    m_autoDemoRunning = true;
    m_currentDemoStep = 0;
    updateStatus("Starting automatic demo...");
    m_autoDemoTimer->start();
}

void DemoController::stopAutoDemo()
{
    m_autoDemoRunning = false;
    m_autoDemoTimer->stop();
    updateStatus("Demo stopped");
}

void DemoController::resetDemo()
{
    stopAutoDemo();
    m_processingResults.clear();
    m_currentDemoStep = 0;
    demoStepChanged(m_currentDemoStep, m_totalDemoSteps);
    updateStatus("Demo reset");
}

void DemoController::loadDemoImages()
{
    updateStatus("Loading demo images...");
    
    // Load images asynchronously
    for (auto& demoImage : m_demoImages) {
        if (!demoImage.path.isEmpty()) {
            QFuture<Glance::IO::LoadResult> future = m_asyncLoader->loadImageAsync(demoImage.path);
            // In a real implementation, we'd track these futures
            // For demo purposes, we'll load synchronously
            Glance::IO::LoadResult result = future.result();
            if (result.success && !result.image.isNull()) {
                demoImage.thumbnail = result.image.scaled(200, 200, Qt::KeepAspectRatio);
                demoImage.isLoaded = true;
            }
        }
    }

    demoImagesLoaded();
    updateStatus(QString("Loaded %1 demo images").arg(m_demoImages.size()));
}

void DemoController::processImageWithAllFilters()
{
    if (m_demoImages.empty()) {
        updateStatus("No demo images available");
        return;
    }

    updateStatus("Demonstrating image processing filters...");
    
    QImage testImage = m_demoImages[0].thumbnail;
    if (testImage.isNull()) {
        testImage = QImage(400, 300, QImage::Format_RGB32);
        testImage.fill(QColor(128, 128, 128));
    }

    // Test exposure adjustment
    m_benchmarkTimer.start();
    QImage exposureImage = m_imageProcessor->applyExposure(testImage, 1.5f);
    qint64 exposureTime = m_benchmarkTimer.elapsed();
    addProcessingResult(testImage, exposureImage, "Exposure (+1.5)");
    benchmarkCompleted("Exposure Adjustment", exposureTime);

    // Test contrast adjustment
    m_benchmarkTimer.start();
    QImage contrastImage = m_imageProcessor->applyContrast(testImage, 1.5f);
    qint64 contrastTime = m_benchmarkTimer.elapsed();
    addProcessingResult(testImage, contrastImage, "Contrast (1.5x)");
    benchmarkCompleted("Contrast Adjustment", contrastTime);

    // Test gamma correction
    m_benchmarkTimer.start();
    QImage gammaImage = m_imageProcessor->applyGamma(testImage, 0.7f);
    qint64 gammaTime = m_benchmarkTimer.elapsed();
    addProcessingResult(testImage, gammaImage, "Gamma (0.7)");
    benchmarkCompleted("Gamma Correction", gammaTime);

    // Test grayscale conversion
    m_benchmarkTimer.start();
    QImage grayImage = m_imageProcessor->convertToGrayscale(testImage);
    qint64 grayTime = m_benchmarkTimer.elapsed();
    addProcessingResult(testImage, grayImage, "Grayscale");
    benchmarkCompleted("Grayscale Conversion", grayTime);

    // Test combined processing
    Core::ProcessingParams params;
    params.exposure = 0.5f;
    params.contrast = 1.2f;
    params.gamma = 0.8f;
    params.grayscale = false;

    m_benchmarkTimer.start();
    QImage combinedImage = m_imageProcessor->processImage(testImage, params);
    qint64 combinedTime = m_benchmarkTimer.elapsed();
    addProcessingResult(testImage, combinedImage, "Combined Processing");
    benchmarkCompleted("Combined Processing", combinedTime);

    updateStatus("Image processing demonstration completed");
}

void DemoController::demonstrateFaceDetection()
{
    if (m_demoImages.empty()) {
        updateStatus("No demo images available for face detection");
        return;
    }

    updateStatus("Demonstrating face detection...");

    if (!Core::FaceDetector::isSupported()) {
        updateStatus("Face detection not available (dlib not installed)");
        return;
    }

    // Create a face-like test image
    QImage faceImage(300, 400, QImage::Format_RGB32);
    faceImage.fill(QColor(255, 220, 177)); // Skin tone

    // Draw face features
    QPainter painter(&faceImage);
    painter.setBrush(QBrush(QColor(50, 50, 50)));
    painter.setPen(Qt::NoPen);

    // Eyes
    painter.drawEllipse(100, 150, 30, 20);
    painter.drawEllipse(170, 150, 30, 20);

    // Nose
    painter.drawEllipse(140, 190, 20, 30);

    // Mouth
    painter.drawEllipse(120, 230, 60, 15);
    painter.end();

    m_benchmarkTimer.start();
    auto result = m_faceDetector->detectFaces(faceImage);
    qint64 detectionTime = m_benchmarkTimer.elapsed();

    if (result && result->success) {
        updateStatus(QString("Face detection completed: %1 faces found in %2ms")
                    .arg(result->faces.size()).arg(detectionTime));
        
        // Draw rectangles around detected faces
        QImage resultImage = faceImage.copy();
        QPainter resultPainter(&resultImage);
        resultPainter.setPen(QPen(Qt::red, 3));
        for (const QRect& face : result->faces) {
            resultPainter.drawRect(face);
        }
        resultPainter.end();

        addProcessingResult(faceImage, resultImage, "Face Detection");
    } else {
        updateStatus(QString("Face detection failed: %1").arg(result ? result->errorMessage : "Unknown error"));
    }

    benchmarkCompleted("Face Detection", detectionTime);
}

void DemoController::demonstrateAsyncLoading()
{
    updateStatus("Demonstrating asynchronous image loading...");

    // Create test images
    QStringList testPaths;
    for (int i = 0; i < 5; ++i) {
        QString path = m_demoDataDir + QString("/async_test_%1.png").arg(i);
        QImage img(200 + i * 50, 200 + i * 50, QImage::Format_RGB32);
        img.fill(QColor(i * 50, 100 + i * 30, 200 - i * 40));
        
        QPainter painter(&img);
        painter.setPen(QPen(Qt::white, 2));
        painter.drawText(10, 20, QString("Image %1").arg(i + 1));
        painter.end();

        img.save(path, "PNG");
        testPaths << path;
    }

    // Load all images asynchronously
    m_benchmarkTimer.start();
    QList<QFuture<Glance::IO::LoadResult>> futures;
    
    for (const QString& path : testPaths) {
        futures << m_asyncLoader->loadImageAsync(path);
    }

    // Wait for all to complete
    int loadedCount = 0;
    for (auto& future : futures) {
        future.waitForFinished();
        Glance::IO::LoadResult result = future.result();
        if (result.success && !result.image.isNull()) {
            loadedCount++;
        }
    }

    qint64 totalTime = m_benchmarkTimer.elapsed();
    updateStatus(QString("Async loading completed: %1/%2 images loaded in %3ms")
                .arg(loadedCount).arg(testPaths.size()).arg(totalTime));

    benchmarkCompleted("Async Loading", totalTime);

    // Cleanup
    for (const QString& path : testPaths) {
        QFile::remove(path);
    }
}

void DemoController::demonstrateHistogramCalculation()
{
    if (m_demoImages.empty()) {
        updateStatus("No demo images available for histogram calculation");
        return;
    }

    updateStatus("Demonstrating histogram calculation...");

    QImage testImage = m_demoImages[0].thumbnail;
    if (testImage.isNull()) {
        testImage = QImage(400, 300, QImage::Format_RGB32);
        testImage.fill(QColor(128, 128, 128));
    }

    m_benchmarkTimer.start();
    
    // Calculate luminance histogram
    std::vector<int> luminanceHist = m_imageProcessor->calculateHistogram(testImage, -1);
    
    // Calculate RGB histograms
    std::vector<int> redHist = m_imageProcessor->calculateHistogram(testImage, 0);
    std::vector<int> greenHist = m_imageProcessor->calculateHistogram(testImage, 1);
    std::vector<int> blueHist = m_imageProcessor->calculateHistogram(testImage, 2);

    qint64 histogramTime = m_benchmarkTimer.elapsed();

    // Verify histogram data
    bool validHistograms = true;
    validHistograms &= (luminanceHist.size() == 256);
    validHistograms &= (redHist.size() == 256);
    validHistograms &= (greenHist.size() == 256);
    validHistograms &= (blueHist.size() == 256);

    if (validHistograms) {
        updateStatus(QString("Histogram calculation completed in %1ms").arg(histogramTime));
    } else {
        updateStatus("Histogram calculation failed");
    }

    benchmarkCompleted("Histogram Calculation", histogramTime);
}

void DemoController::runPerformanceBenchmarks()
{
    updateStatus("Running performance benchmarks...");
    
    benchmarkImageProcessing();
    benchmarkAsyncLoading();
    benchmarkFaceDetection();
    demonstrateHistogramCalculation();
    
    updateStatus("Performance benchmarks completed");
}

void DemoController::benchmarkImageProcessing()
{
    updateStatus("Benchmarking image processing...");
    processImageWithAllFilters();
}

void DemoController::benchmarkAsyncLoading()
{
    updateStatus("Benchmarking async loading...");
    demonstrateAsyncLoading();
}

void DemoController::benchmarkFaceDetection()
{
    updateStatus("Benchmarking face detection...");
    demonstrateFaceDetection();
}

void DemoController::onAutoDemoTimer()
{
    if (!m_autoDemoRunning) {
        return;
    }

    nextDemoStep();
}

void DemoController::onImageLoadCompleted()
{
    // Handle image load completion
}

void DemoController::onProcessingCompleted()
{
    // Handle processing completion
}

void DemoController::createDemoImages()
{
    m_demoImages.clear();
    
    // Load real images from demo data directory
    QDir dataDir(m_demoDataDir);
    if (!dataDir.exists()) {
        updateStatus("Demo data directory not found: " + m_demoDataDir);
        return;
    }
    
    // Find all jpg files
    QStringList imageFilters;
    imageFilters << "*.jpg" << "*.jpeg" << "*.png";
    QStringList imageFiles = dataDir.entryList(imageFilters, QDir::Files);
    
    updateStatus(QString("Found %1 image files in demo directory").arg(imageFiles.size()));
    
    for (const QString& fileName : imageFiles) {
        // Skip JSON files
        if (fileName.startsWith("bb_")) continue;
        
        DemoImage demoImage;
        demoImage.name = fileName;
        demoImage.path = dataDir.absoluteFilePath(fileName);
        demoImage.description = QString("Real test image: %1").arg(fileName);
        
        // Try to load thumbnail
        QImage image(demoImage.path);
        if (!image.isNull()) {
            demoImage.thumbnail = image.scaled(200, 200, Qt::KeepAspectRatio);
            demoImage.isLoaded = true;
        }
        
        m_demoImages.push_back(demoImage);
        qDebug() << "Added demo image:" << fileName << "from" << demoImage.path;
    }
    
    updateStatus(QString("Loaded %1 real demo images").arg(m_demoImages.size()));
}

void DemoController::updateStatus(const QString& status)
{
    m_statusMessage = status;
    statusChanged(status);
    qDebug() << "Demo Status:" << status;
}

void DemoController::nextDemoStep()
{
    m_currentDemoStep++;
    
    if (m_currentDemoStep > m_totalDemoSteps) {
        m_autoDemoRunning = false;
        m_autoDemoTimer->stop();
        demoCompleted();
        updateStatus("Demo completed");
        return;
    }

    demoStepChanged(m_currentDemoStep, m_totalDemoSteps);

    switch (m_currentDemoStep) {
    case 1:
        updateStatus("Step 1: Loading demo images...");
        loadDemoImages();
        break;
    case 2:
        updateStatus("Step 2: Demonstrating image processing...");
        processImageWithAllFilters();
        break;
    case 3:
        updateStatus("Step 3: Demonstrating face detection...");
        demonstrateFaceDetection();
        break;
    case 4:
        updateStatus("Step 4: Demonstrating async loading...");
        demonstrateAsyncLoading();
        break;
    case 5:
        updateStatus("Step 5: Demonstrating histogram calculation...");
        demonstrateHistogramCalculation();
        break;
    case 6:
        updateStatus("Step 6: Running performance benchmarks...");
        runPerformanceBenchmarks();
        break;
    case 7:
        updateStatus("Step 7: Processing results...");
        // Additional processing if needed
        break;
    case 8:
        updateStatus("Step 8: Finalizing demo...");
        // Final cleanup
        break;
    }
}

void DemoController::addProcessingResult(const QImage& original, const QImage& processed, const QString& operation)
{
    ProcessingResult result;
    result.originalImage = original;
    result.processedImage = processed;
    result.operationName = operation;
    result.processingTimeMs = m_benchmarkTimer.elapsed();
    
    m_processingResults.push_back(result);
    
    imageProcessed(original, processed, operation);
}

} // namespace Glance::Demo
