#pragma once

namespace Glance::Core { class FaceDetector; }

#include "../IFaceDetectorPlugin.h"

#include <QJsonObject>
#include <QElapsedTimer>

namespace Glance::Plugins {

/**
 * @brief Default face detector plugin implementation
 * 
 * This plugin provides face detection capabilities using dlib
 * and OpenCV with support for frontal and profile faces.
 */
class DefaultFaceDetectorPlugin : public Core::Plugins::IFaceDetectorPlugin {
public:
    DefaultFaceDetectorPlugin();
    ~DefaultFaceDetectorPlugin() override;

    // IPlugin interface
    QString name() const override;
    QString version() const override;
    QString description() const override;
    QString author() const override;
    QStringList dependencies() const override;
    
    bool initialize() override;
    void shutdown() override;
    bool isInitialized() const override;
    
    QJsonObject defaultConfiguration() const override;
    bool configure(const QJsonObject& config) override;
    QJsonObject currentConfiguration() const override;
    
    QString lastError() const override;
    bool hasError() const override;

    // IFaceDetectorPlugin interface
    std::optional<Core::FaceDetectionResult> detectFaces(const QImage& image) override;
    QFuture<Core::FaceDetectionResult> detectFacesAsync(const QImage& image) override;
    
    void setMinFaceSize(int minSize) override;
    void setMaxFaceSize(int maxSize) override;
    void setConfidenceThreshold(double threshold) override;
    void setDetectionTypes(const QStringList& types) override;
    
    QStringList supportedDetectionTypes() const override;
    bool supportsAsync() const override;
    bool supportsParallel() const override;
    bool supportsProfileDetection() const override;
    bool supportsAIEnhancement() const override;
    
    bool isSupported() const override;
    QString getDetectionModelInfo() const override;
    
    double getAverageDetectionTime() const override;
    int getTotalDetections() const override;
    void resetMetrics() override;

private:
    std::shared_ptr<Core::FaceDetector> m_faceDetector;
    bool m_initialized = false;
    QString m_lastError;
    QJsonObject m_configuration;
    
    // Configuration
    int m_minFaceSize = 20;
    int m_maxFaceSize = 500;
    double m_confidenceThreshold = 0.5;
    QStringList m_detectionTypes = {"frontal", "profile"};
    
    // Performance monitoring
    mutable QElapsedTimer m_performanceTimer;
    mutable qint64 m_totalDetectionTime = 0;
    mutable qint64 m_detectionCount = 0;
    
    // Internal methods
    void updateLastError(const QString& error);
    void updatePerformanceMetrics(qint64 detectionTime) const;
    QJsonObject createDefaultConfiguration() const;
    void applyConfigurationToDetector();
    QStringList getAvailableDetectionTypes() const;
};

} // namespace Glance::Plugins
