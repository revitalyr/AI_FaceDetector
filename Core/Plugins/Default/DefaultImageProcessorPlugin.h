#pragma once

namespace Glance::Core { class ImageProcessor; }

#include "../IImageProcessorPlugin.h"

#include <QJsonObject>
#include <QElapsedTimer>

namespace Glance::Plugins {

/**
 * @brief Default image processor plugin implementation
 * 
 * This plugin provides the standard image processing capabilities
 * using the original ImageProcessor implementation.
 */
class DefaultImageProcessorPlugin : public Core::Plugins::IImageProcessorPlugin {
public:
    DefaultImageProcessorPlugin();
    ~DefaultImageProcessorPlugin() override;

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

    // IImageProcessorPlugin interface
    QImage processImage(const QImage& source, const Core::ProcessingParams& params) override;
    QFuture<QImage> processImageAsync(const QImage& source, const Core::ProcessingParams& params) override;
    
    QImage applyExposure(const QImage& source, const Core::Exposure& exposure) override;
    QImage applyContrast(const QImage& source, const Core::Contrast& contrast) override;
    QImage applyGamma(const QImage& source, const Core::Gamma& gamma) override;
    QImage applyTemperature(const QImage& source, const Core::Temperature& temperature) override;
    QImage applyTint(const QImage& source, const Core::Tint& tint) override;
    QImage applySaturation(const QImage& source, const Core::Saturation& saturation) override;
    QImage applyVibrance(const QImage& source, const Core::Vibrance& vibrance) override;
    QImage applyHighlights(const QImage& source, const Core::Highlights& highlights) override;
    QImage applyShadows(const QImage& source, const Core::Shadows& shadows) override;
    QImage convertToGrayscale(const QImage& source) override;
    
    std::vector<int> calculateHistogram(const QImage& source, int channel = -1) override;
    
    QStringList supportedOperations() const override;
    bool supportsAsync() const override;
    bool supportsParallel() const override;

private:
    std::unique_ptr<Core::ImageProcessor> m_imageProcessor;
    bool m_initialized = false;
    QString m_lastError;
    QJsonObject m_configuration;
    
    // Performance monitoring
    mutable QElapsedTimer m_performanceTimer;
    mutable qint64 m_totalProcessingTime = 0;
    mutable qint64 m_processingCount = 0;
    
    // Internal methods
    void updateLastError(const QString& error);
    void updatePerformanceMetrics(qint64 processingTime) const;
    QJsonObject createDefaultConfiguration() const;
};

} // namespace Glance::Plugins
