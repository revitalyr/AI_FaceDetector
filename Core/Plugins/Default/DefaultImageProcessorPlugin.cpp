module;
#include "DefaultImageProcessorPlugin.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtConcurrent>
#include <QDebug>
module Core.ImageProcessor;

namespace Glance::Plugins {

DefaultImageProcessorPlugin::DefaultImageProcessorPlugin() 
    : m_imageProcessor(std::make_unique<Core::ImageProcessor>()) {
}

DefaultImageProcessorPlugin::~DefaultImageProcessorPlugin() = default;

// IPlugin interface implementation

QString DefaultImageProcessorPlugin::name() const {
    return "DefaultImageProcessor";
}

QString DefaultImageProcessorPlugin::version() const {
    return "1.0.0";
}

QString DefaultImageProcessorPlugin::description() const {
    return "Default image processing plugin with basic and advanced operations";
}

QString DefaultImageProcessorPlugin::author() const {
    return "Glance Team";
}

QStringList DefaultImageProcessorPlugin::dependencies() const {
    return QStringList(); // No dependencies
}

bool DefaultImageProcessorPlugin::initialize() {
    if (m_initialized) {
        return true;
    }
    
    try {
        // Initialize image processor
        if (!m_imageProcessor) {
            m_imageProcessor = std::make_unique<Core::ImageProcessor>();
        }
        
        m_initialized = true;
        m_lastError.clear();
        
        qDebug() << "DefaultImageProcessorPlugin initialized successfully";
        return true;
        
    } catch (const std::exception& e) {
        updateLastError(QString("Initialization failed: %1").arg(e.what()));
        return false;
    }
}

void DefaultImageProcessorPlugin::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    m_imageProcessor.reset();
    m_initialized = false;
    
    qDebug() << "DefaultImageProcessorPlugin shut down";
}

bool DefaultImageProcessorPlugin::isInitialized() const {
    return m_initialized;
}

QJsonObject DefaultImageProcessorPlugin::defaultConfiguration() const {
    return createDefaultConfiguration();
}

bool DefaultImageProcessorPlugin::configure(const QJsonObject& config) {
    if (!m_initialized) {
        updateLastError("Plugin not initialized");
        return false;
    }
    
    try {
        m_configuration = config;

        // Apply configuration to image processor if needed
        // Configuration-specific logic not implemented - using default configuration

        m_lastError.clear();
        return true;
        
    } catch (const std::exception& e) {
        updateLastError(QString("Configuration failed: %1").arg(e.what()));
        return false;
    }
}

QJsonObject DefaultImageProcessorPlugin::currentConfiguration() const {
    return m_configuration;
}

QString DefaultImageProcessorPlugin::lastError() const {
    return m_lastError;
}

bool DefaultImageProcessorPlugin::hasError() const {
    return !m_lastError.isEmpty();
}

// IImageProcessorPlugin interface implementation

QImage DefaultImageProcessorPlugin::processImage(const QImage& source, const Core::ProcessingParams& params) {
    if (!m_initialized || !m_imageProcessor) {
        updateLastError("Plugin not initialized");
        return QImage();
    }
    
    m_performanceTimer.start();
    
    try {
        QImage result = m_imageProcessor->processImage(source, params);
        updatePerformanceMetrics(m_performanceTimer.elapsed());
        return result;
        
    } catch (const std::exception& e) {
        updateLastError(QString("Image processing failed: %1").arg(e.what()));
        return QImage();
    }
}

QFuture<QImage> DefaultImageProcessorPlugin::processImageAsync(const QImage& source, const Core::ProcessingParams& params) {
    if (!m_initialized || !m_imageProcessor) {
        updateLastError("Plugin not initialized");
        return QFuture<QImage>();
    }
    
    auto sourcePtr = std::make_shared<QImage>(source);
    QPointer<DefaultImageProcessorPlugin> guard(this);
    return QtConcurrent::run([guard, sourcePtr, params]() {
        if (!guard) {
            return QImage();
        }
        return guard->processImage(*sourcePtr, params);
    });
}

QImage DefaultImageProcessorPlugin::applyExposure(const QImage& source, const Core::Exposure& exposure) {
    if (!m_initialized || !m_imageProcessor) {
        updateLastError("Plugin not initialized");
        return QImage();
    }
    
    try {
        return m_imageProcessor->applyExposure(source, exposure);
    } catch (const std::exception& e) {
        updateLastError(QString("Exposure adjustment failed: %1").arg(e.what()));
        return QImage();
    }
}

QImage DefaultImageProcessorPlugin::applyContrast(const QImage& source, const Core::Contrast& contrast) {
    if (!m_initialized || !m_imageProcessor) {
        updateLastError("Plugin not initialized");
        return QImage();
    }
    
    try {
        return m_imageProcessor->applyContrast(source, contrast);
    } catch (const std::exception& e) {
        updateLastError(QString("Contrast adjustment failed: %1").arg(e.what()));
        return QImage();
    }
}

QImage DefaultImageProcessorPlugin::applyGamma(const QImage& source, const Core::Gamma& gamma) {
    if (!m_initialized || !m_imageProcessor) {
        updateLastError("Plugin not initialized");
        return QImage();
    }
    
    try {
        return m_imageProcessor->applyGamma(source, gamma);
    } catch (const std::exception& e) {
        updateLastError(QString("Gamma correction failed: %1").arg(e.what()));
        return QImage();
    }
}

QImage DefaultImageProcessorPlugin::applyTemperature(const QImage& source, const Core::Temperature& temperature) {
    if (!m_initialized || !m_imageProcessor) {
        updateLastError("Plugin not initialized");
        return QImage();
    }
    
    try {
        return m_imageProcessor->applyTemperature(source, temperature);
    } catch (const std::exception& e) {
        updateLastError(QString("Temperature adjustment failed: %1").arg(e.what()));
        return QImage();
    }
}

QImage DefaultImageProcessorPlugin::applyTint(const QImage& source, const Core::Tint& tint) {
    if (!m_initialized || !m_imageProcessor) {
        updateLastError("Plugin not initialized");
        return QImage();
    }
    
    try {
        return m_imageProcessor->applyTint(source, tint);
    } catch (const std::exception& e) {
        updateLastError(QString("Tint adjustment failed: %1").arg(e.what()));
        return QImage();
    }
}

QImage DefaultImageProcessorPlugin::applySaturation(const QImage& source, const Core::Saturation& saturation) {
    if (!m_initialized || !m_imageProcessor) {
        updateLastError("Plugin not initialized");
        return QImage();
    }
    
    try {
        return m_imageProcessor->applySaturation(source, saturation);
    } catch (const std::exception& e) {
        updateLastError(QString("Saturation adjustment failed: %1").arg(e.what()));
        return QImage();
    }
}

QImage DefaultImageProcessorPlugin::applyVibrance(const QImage& source, const Core::Vibrance& vibrance) {
    if (!m_initialized || !m_imageProcessor) {
        updateLastError("Plugin not initialized");
        return QImage();
    }
    
    try {
        return m_imageProcessor->applyVibrance(source, vibrance);
    } catch (const std::exception& e) {
        updateLastError(QString("Vibrance adjustment failed: %1").arg(e.what()));
        return QImage();
    }
}

QImage DefaultImageProcessorPlugin::applyHighlights(const QImage& source, const Core::Highlights& highlights) {
    if (!m_initialized || !m_imageProcessor) {
        updateLastError("Plugin not initialized");
        return QImage();
    }
    
    try {
        // Use applyHighlightsShadows with default shadows value
        Core::Shadows defaultShadows(0.0);
        return m_imageProcessor->applyHighlightsShadows(source, highlights, defaultShadows);
    } catch (const std::exception& e) {
        updateLastError(QString("Highlights adjustment failed: %1").arg(e.what()));
        return QImage();
    }
}

QImage DefaultImageProcessorPlugin::applyShadows(const QImage& source, const Core::Shadows& shadows) {
    if (!m_initialized || !m_imageProcessor) {
        updateLastError("Plugin not initialized");
        return QImage();
    }
    
    try {
        // Use applyHighlightsShadows with default highlights value
        Core::Highlights defaultHighlights(0.0);
        return m_imageProcessor->applyHighlightsShadows(source, defaultHighlights, shadows);
    } catch (const std::exception& e) {
        updateLastError(QString("Shadows adjustment failed: %1").arg(e.what()));
        return QImage();
    }
}

QImage DefaultImageProcessorPlugin::convertToGrayscale(const QImage& source) {
    if (!m_initialized || !m_imageProcessor) {
        updateLastError("Plugin not initialized");
        return QImage();
    }
    
    try {
        return m_imageProcessor->convertToGrayscale(source);
    } catch (const std::exception& e) {
        updateLastError(QString("Grayscale conversion failed: %1").arg(e.what()));
        return QImage();
    }
}

std::vector<int> DefaultImageProcessorPlugin::calculateHistogram(const QImage& source, int channel) {
    if (!m_initialized || !m_imageProcessor) {
        updateLastError("Plugin not initialized");
        return std::vector<int>();
    }
    
    try {
        return m_imageProcessor->calculateHistogram(source, channel);
    } catch (const std::exception& e) {
        updateLastError(QString("Histogram calculation failed: %1").arg(e.what()));
        return std::vector<int>();
    }
}

QStringList DefaultImageProcessorPlugin::supportedOperations() const {
    return QStringList{
        "processImage",
        "applyExposure",
        "applyContrast", 
        "applyGamma",
        "applyTemperature",
        "applyTint",
        "applySaturation",
        "applyVibrance",
        "applyHighlights",
        "applyShadows",
        "convertToGrayscale",
        "calculateHistogram"
    };
}

bool DefaultImageProcessorPlugin::supportsAsync() const {
    return true;
}

bool DefaultImageProcessorPlugin::supportsParallel() const {
    return true;
}

// Private methods implementation

void DefaultImageProcessorPlugin::updateLastError(const QString& error) {
    m_lastError = error;
    qWarning() << "DefaultImageProcessorPlugin error:" << error;
}

void DefaultImageProcessorPlugin::updatePerformanceMetrics(qint64 processingTime) const {
    m_totalProcessingTime += processingTime;
    m_processingCount++;
}

QJsonObject DefaultImageProcessorPlugin::createDefaultConfiguration() const {
    QJsonObject config;
    
    // Performance settings
    QJsonObject performance;
    performance["enableAsync"] = true;
    performance["enableParallel"] = true;
    performance["maxThreads"] = -1; // Use all available threads
    config["performance"] = performance;
    
    // Quality settings
    QJsonObject quality;
    quality["highQuality"] = true;
    quality["antialiasing"] = true;
    config["quality"] = quality;
    
    // Memory settings
    QJsonObject memory;
    memory["maxCacheSize"] = 100; // MB
    memory["enableCaching"] = true;
    config["memory"] = memory;
    
    return config;
}

} // namespace Glance::Plugins
