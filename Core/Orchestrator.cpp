module;

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QtConcurrent>

module Core.Orchestrator;

namespace Glance::Core {

bool Orchestrator::initialize(const QString& pluginDirectory) {
    if (m_initialized) {
        qWarning() << "Orchestrator already initialized";
        return true;
    }
    qDebug() << "Initializing Core Orchestrator...";
    if (!loadPlugins(pluginDirectory)) {
        updateLastError("Failed to load plugins");
        return false;
    }
    if (!initializeDefaultPlugins()) {
        updateLastError("Failed to initialize default plugins");
        return false;
    }
    if (!selectDefaultPlugins()) {
        updateLastError("Failed to select default plugins");
        return false;
    }
    m_initialized = true;
    qDebug() << "Core Orchestrator initialized successfully";
    return true;
}

void Orchestrator::shutdown() {
    if (!m_initialized) return;
    qDebug() << "Shutting down Core Orchestrator...";
    clearPluginReferences();
    unloadAllPlugins();
    m_initialized = false;
    qDebug() << "Core Orchestrator shut down successfully";
}

bool Orchestrator::loadPlugins(const QString& pluginDirectory) {
    return m_pluginManager.loadPlugins(pluginDirectory);
}

void Orchestrator::unloadAllPlugins() {
    m_pluginManager.unloadAllPlugins();
    clearPluginReferences();
}

QStringList Orchestrator::availablePlugins() const {
    return m_pluginManager.availablePlugins();
}

QStringList Orchestrator::loadedPlugins() const {
    return m_pluginManager.loadedPlugins();
}

QImage Orchestrator::processImage(const QImage& source, const ProcessingParams& params) {
    if (m_initialized && m_activeImageProcessor) {
        try {
            return m_activeImageProcessor->processImage(source, params);
        } catch (const std::exception& e) {
            updateLastError(QString("Plugin image processing failed: %1").arg(e.what()));
        }
    }
    try {
        if (!m_fallbackImageProcessor)
            m_fallbackImageProcessor = std::make_shared<ImageProcessor>();
        return m_fallbackImageProcessor->processImage(source, params);
    } catch (const std::exception& e) {
        updateLastError(QString("Image processing failed: %1").arg(e.what()));
        return QImage();
    }
}

QFuture<QImage> Orchestrator::processImageAsync(const QImage& source, const ProcessingParams& params) {
    if (m_initialized && m_activeImageProcessor) {
        try {
            return m_activeImageProcessor->processImageAsync(source, params);
        } catch (const std::exception& e) {
            updateLastError(QString("Plugin async image processing failed: %1").arg(e.what()));
        }
    }
    try {
        if (!m_fallbackImageProcessor)
            m_fallbackImageProcessor = std::make_shared<ImageProcessor>();
        return m_fallbackImageProcessor->processImageAsync(source, params);
    } catch (const std::exception& e) {
        updateLastError(QString("Async image processing failed: %1").arg(e.what()));
        return QFuture<QImage>();
    }
}

std::optional<FaceDetectionResult> Orchestrator::detectFaces(const QImage& image) {
    if (m_initialized && m_activeFaceDetector) {
        try {
            return m_activeFaceDetector->detectFaces(image);
        } catch (const std::exception& e) {
            updateLastError(QString("Plugin face detection failed: %1").arg(e.what()));
        }
    }
    try {
        if (!m_fallbackFaceDetector)
            m_fallbackFaceDetector = std::make_shared<FaceDetector>();
        return m_fallbackFaceDetector->detectFaces(image);
    } catch (const std::exception& e) {
        updateLastError(QString("Face detection failed: %1").arg(e.what()));
        return std::nullopt;
    }
}

QFuture<FaceDetectionResult> Orchestrator::detectFacesAsync(const QImage& image) {
    if (m_initialized && m_activeFaceDetector) {
        try {
            return m_activeFaceDetector->detectFacesAsync(image);
        } catch (const std::exception& e) {
            updateLastError(QString("Plugin async face detection failed: %1").arg(e.what()));
        }
    }
    try {
        if (!m_fallbackFaceDetector)
            m_fallbackFaceDetector = std::make_shared<FaceDetector>();
        return m_fallbackFaceDetector->detectFacesAsync(image);
    } catch (const std::exception& e) {
        updateLastError(QString("Async face detection failed: %1").arg(e.what()));
        return QFuture<FaceDetectionResult>();
    }
}

void Orchestrator::setConfidenceThreshold(double threshold) {
    if (!m_fallbackFaceDetector)
        m_fallbackFaceDetector = std::make_shared<FaceDetector>();
    m_fallbackFaceDetector->setConfidenceThreshold(threshold);
}

void Orchestrator::setMinFaceSize(int minSize) {
    if (!m_fallbackFaceDetector)
        m_fallbackFaceDetector = std::make_shared<FaceDetector>();
    m_fallbackFaceDetector->setMinFaceSize(minSize);
}

std::optional<SegmentationResult> Orchestrator::removeBackground(const QImage& image) {
    try {
        if (!m_segmenter)
            m_segmenter = std::make_unique<Segmenter>();
        if (!m_segmenter->isLoaded()) {
            if (m_segmenterModelPath.isEmpty()) {
                QString appDir = QCoreApplication::applicationDirPath();
                QString projectDir = QDir(appDir).absoluteFilePath("../../");
                QStringList searchPaths = {
                    QDir(projectDir).filePath("models/u2netp.onnx"),
                    QDir(appDir).filePath("models/u2netp.onnx"),
                    QDir::current().filePath("models/u2netp.onnx"),
                    QDir(appDir).filePath("data/models/u2netp.onnx")
                };
                for (const auto& p : searchPaths) {
                    if (QFile::exists(p)) {
                        m_segmenterModelPath = p;
                        break;
                    }
                }
            }
            if (m_segmenterModelPath.isEmpty() || !m_segmenter->loadModel(m_segmenterModelPath)) {
                updateLastError("Failed to load segmentation model");
                return std::nullopt;
            }
        }
        return m_segmenter->segment(image);
    } catch (const std::exception& e) {
        updateLastError(QString("Background removal failed: %1").arg(e.what()));
        return std::nullopt;
    }
}

QFuture<std::optional<SegmentationResult>> Orchestrator::removeBackgroundAsync(const QImage& image) {
    auto imagePtr = std::make_shared<QImage>(image);
    Orchestrator* self = this;
    return QtConcurrent::run([self, imagePtr]() -> std::optional<SegmentationResult> {
        if (!self) {
            return std::nullopt;
        }
        return self->removeBackground(*imagePtr);
    });
}

void Orchestrator::setSegmenterModelPath(const QString& path) {
    m_segmenterModelPath = path;
}

bool Orchestrator::configureImageProcessor(const QString& pluginName, const QJsonObject& config) {
    bool success = m_pluginManager.configurePlugin(pluginName, config);
    if (!success)
        updateLastError(QString("Failed to configure image processor plugin: %1").arg(pluginName));
    return success;
}

bool Orchestrator::configureFaceDetector(const QString& pluginName, const QJsonObject& config) {
    bool success = m_pluginManager.configurePlugin(pluginName, config);
    if (!success)
        updateLastError(QString("Failed to configure face detector plugin: %1").arg(pluginName));
    return success;
}

void Orchestrator::saveConfiguration(const QString& configFile) {
    m_pluginManager.savePluginConfigurations(configFile);
}

void Orchestrator::loadConfiguration(const QString& configFile) {
    m_pluginManager.loadPluginConfigurations(configFile);
}

bool Orchestrator::setActiveImageProcessor(const QString& pluginName) {
    auto plugin = m_pluginManager.getImageProcessorPlugin(pluginName);
    if (plugin) {
        m_activeImageProcessor = plugin;
        qDebug() << "Set active image processor:" << pluginName;
        return true;
    }
    updateLastError(QString("Image processor plugin not found: %1").arg(pluginName));
    return false;
}

bool Orchestrator::setActiveFaceDetector(const QString& pluginName) {
    auto plugin = m_pluginManager.getFaceDetectorPlugin(pluginName);
    if (plugin) {
        m_activeFaceDetector = plugin;
        qDebug() << "Set active face detector:" << pluginName;
        return true;
    }
    updateLastError(QString("Face detector plugin not found: %1").arg(pluginName));
    return false;
}

QString Orchestrator::getActiveImageProcessor() const {
    if (m_activeImageProcessor) return m_activeImageProcessor->name();
    return QString();
}

QString Orchestrator::getActiveFaceDetector() const {
    if (m_activeFaceDetector) return m_activeFaceDetector->name();
    return QString();
}

QStringList Orchestrator::getAvailableImageProcessors() const {
    return m_pluginManager.pluginsByType("ImageProcessor");
}

QStringList Orchestrator::getAvailableFaceDetectors() const {
    return m_pluginManager.pluginsByType("FaceDetector");
}

QJsonObject Orchestrator::getPluginInfo(const QString& pluginName) const {
    return m_pluginManager.getPluginInfo(pluginName);
}

QStringList Orchestrator::getSupportedImageOperations() const {
    if (m_activeImageProcessor) return m_activeImageProcessor->supportedOperations();
    return QStringList();
}

QStringList Orchestrator::getSupportedDetectionTypes() const {
    if (m_activeFaceDetector) return m_activeFaceDetector->supportedDetectionTypes();
    return QStringList();
}

bool Orchestrator::supportsAsyncImageProcessing() const {
    return m_activeImageProcessor && m_activeImageProcessor->supportsAsync();
}

bool Orchestrator::supportsAsyncFaceDetection() const {
    return m_activeFaceDetector && m_activeFaceDetector->supportsAsync();
}

void Orchestrator::startPerformanceMonitoring() {
    m_pluginManager.startPerformanceMonitoring();
}

void Orchestrator::stopPerformanceMonitoring() {
    m_pluginManager.stopPerformanceMonitoring();
}

QJsonObject Orchestrator::getPerformanceMetrics() const {
    return m_pluginManager.getPerformanceMetrics();
}

QString Orchestrator::getLastError() const {
    return m_lastError;
}

bool Orchestrator::hasError() const {
    return !m_lastError.isEmpty();
}

void Orchestrator::clearError() {
    m_lastError.clear();
}

bool Orchestrator::initializeDefaultPlugins() {
    QStringList loaded = loadedPlugins();
    for (const QString& pluginName : loaded) {
        auto imageProcessor = m_pluginManager.getImageProcessorPlugin(pluginName);
        auto faceDetector = m_pluginManager.getFaceDetectorPlugin(pluginName);
        if (imageProcessor && !imageProcessor->isInitialized()) {
            if (!imageProcessor->initialize())
                qWarning() << "Failed to initialize plugin:" << pluginName;
        }
        if (faceDetector && !faceDetector->isInitialized()) {
            if (!faceDetector->initialize())
                qWarning() << "Failed to initialize plugin:" << pluginName;
        }
    }
    return true;
}

bool Orchestrator::selectDefaultPlugins() {
    auto imageProcessors = getAvailableImageProcessors();
    if (!imageProcessors.isEmpty()) {
        if (!setActiveImageProcessor(imageProcessors.first()))
            return false;
    }
    auto faceDetectors = getAvailableFaceDetectors();
    if (!faceDetectors.isEmpty()) {
        if (!setActiveFaceDetector(faceDetectors.first()))
            return false;
    }
    return true;
}

void Orchestrator::updateLastError(const QString& error) {
    m_lastError = error;
    qWarning() << "Orchestrator error:" << error;
}

void Orchestrator::clearPluginReferences() {
    m_activeImageProcessor.reset();
    m_activeFaceDetector.reset();
}

} // namespace Glance::Core
