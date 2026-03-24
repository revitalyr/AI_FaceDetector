#pragma once

#include <QObject>
#include <QImage>
#include <QTimer>
#include <QElapsedTimer>
#include <memory>
#include <vector>

#include "../Core/ImageProcessor.h"
// FaceDetector.h removed — types are now in module Core.FaceDetector.
// Use the bridge header for legacy .h compatibility:
#include "../Core/Plugins/CoreFaceDetectorFwd.h"
#include "../IO/AsyncImageLoader.h"

namespace Glance::Demo {

struct DemoImage {
    QString name;
    QString path;
    QString description;
    QImage thumbnail;
    bool isLoaded = false;
};

struct ProcessingResult {
    QImage originalImage;
    QImage processedImage;
    qint64 processingTimeMs;
    QString operationName;
};

class DemoController : public QObject
{
public:
    explicit DemoController(QObject *parent = nullptr);
    ~DemoController();

    void initializeDemo();
    void startAutoDemo();
    void stopAutoDemo();
    void resetDemo();

    void loadDemoImages();
    void processImageWithAllFilters();
    void demonstrateFaceDetection();
    void demonstrateAsyncLoading();
    void demonstrateHistogramCalculation();

    void runPerformanceBenchmarks();
    void benchmarkImageProcessing();
    void benchmarkAsyncLoading();
    void benchmarkFaceDetection();

    const std::vector<DemoImage>& getDemoImages() const { return m_demoImages; }
    const std::vector<ProcessingResult>& getProcessingResults() const { return m_processingResults; }
    QString getStatusMessage() const { return m_statusMessage; }
    int getCurrentDemoStep() const { return m_currentDemoStep; }
    int getTotalDemoSteps() const { return m_totalDemoSteps; }

private:
    void onAutoDemoTimer();
    void onImageLoadCompleted();
    void onProcessingCompleted();

private:
    void createDemoImages();
    void updateStatus(const QString& status);
    void nextDemoStep();
    void addProcessingResult(const QImage& original, const QImage& processed, const QString& operation);

    // Demo components
    std::unique_ptr<Core::ImageProcessor> m_imageProcessor;
    std::unique_ptr<Core::FaceDetector> m_faceDetector;
    std::unique_ptr<IO::AsyncImageLoader> m_asyncLoader;

    // Demo data
    std::vector<DemoImage> m_demoImages;
    std::vector<ProcessingResult> m_processingResults;

    // Demo state
    QTimer* m_autoDemoTimer;
    QElapsedTimer m_benchmarkTimer;
    QString m_statusMessage;
    int m_currentDemoStep;
    int m_totalDemoSteps;
    bool m_autoDemoRunning;

    // Demo image paths
    QString m_demoDataDir;
};

} // namespace Glance::Demo
