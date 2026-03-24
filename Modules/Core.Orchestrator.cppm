/**
 * @file Core.Orchestrator.cppm
 * @brief Central coordinator for face detection, image processing, and plugins
 *
 * The Orchestrator manages the lifecycle of all core subsystems:
 * face detection (via FaceDetector and plugin-based detectors),
 * image processing (via ImageProcessor and plugin-based processors),
 * background segmentation (via Segmenter), and plugin configuration.
 * Provides both synchronous and async (QFuture) APIs for all operations.
 */
module;

#include <memory>
#include <optional>
#include <QImage>
#include <QFuture>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include "Plugins/PluginManager.h"
#include "Plugins/IImageProcessorPlugin.h"
#include "Plugins/IFaceDetectorPlugin.h"

export module Core.Orchestrator;

import Core.FaceDetector;
import Core.ImageProcessor;
import Core.Segmenter;

namespace Glance::Core {

/** @brief Central facade coordinating detection, processing, and plugins */
export class Orchestrator {
public:
    Orchestrator() = default;
    ~Orchestrator() = default;

    bool initialize(const QString& pluginDirectory = "plugins");
    void shutdown();
    bool isInitialized() const;

    bool loadPlugins(const QString& pluginDirectory = "plugins");
    void unloadAllPlugins();

    QStringList availablePlugins() const;
    QStringList loadedPlugins() const;

    QImage processImage(const QImage& source, const ProcessingParams& params);
    QFuture<QImage> processImageAsync(const QImage& source, const ProcessingParams& params);

    std::optional<FaceDetectionResult> detectFaces(const QImage& image);
    QFuture<FaceDetectionResult> detectFacesAsync(const QImage& image);

    void setConfidenceThreshold(double threshold);
    void setMinFaceSize(int minSize);

    std::optional<SegmentationResult> removeBackground(const QImage& image);
    QFuture<std::optional<SegmentationResult>> removeBackgroundAsync(const QImage& image);
    void setSegmenterModelPath(const QString& path);

    bool configureImageProcessor(const QString& pluginName, const QJsonObject& config);
    bool configureFaceDetector(const QString& pluginName, const QJsonObject& config);

    void saveConfiguration(const QString& configFile = "config.json");
    void loadConfiguration(const QString& configFile = "config.json");

    bool setActiveImageProcessor(const QString& pluginName);
    bool setActiveFaceDetector(const QString& pluginName);

    QString getActiveImageProcessor() const;
    QString getActiveFaceDetector() const;

    QStringList getAvailableImageProcessors() const;
    QStringList getAvailableFaceDetectors() const;

    QJsonObject getPluginInfo(const QString& pluginName) const;

    QStringList getSupportedImageOperations() const;
    QStringList getSupportedDetectionTypes() const;

    bool supportsAsyncImageProcessing() const;
    bool supportsAsyncFaceDetection() const;

    void startPerformanceMonitoring();
    void stopPerformanceMonitoring();
    QJsonObject getPerformanceMetrics() const;

    QString getLastError() const;
    bool hasError() const;
    void clearError();

private:
    Orchestrator(const Orchestrator&) = delete;
    Orchestrator& operator=(const Orchestrator&) = delete;

    Plugins::PluginManager m_pluginManager;
    Plugins::IImageProcessorPluginPtr m_activeImageProcessor;
    Plugins::IFaceDetectorPluginPtr m_activeFaceDetector;
    std::shared_ptr<FaceDetector> m_fallbackFaceDetector;
    std::shared_ptr<ImageProcessor> m_fallbackImageProcessor;

    bool m_initialized = false;
    QString m_lastError;
    QString m_segmenterModelPath;

    std::unique_ptr<Segmenter> m_segmenter;

    bool initializeDefaultPlugins();
    bool selectDefaultPlugins();
    void updateLastError(const QString& error);
    void clearPluginReferences();
};

} // namespace Glance::Core
