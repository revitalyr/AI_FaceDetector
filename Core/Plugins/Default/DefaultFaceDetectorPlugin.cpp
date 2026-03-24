// ============================================================
// DefaultFaceDetectorPlugin.cpp
// Plain (non-module) translation unit.
//
// The #includes must appear in the global-module fragment BEFORE
// any `import` directive.  Since this file is NOT itself a module
// unit we must NOT write a bare `module;` preamble — that syntax
// is only legal immediately before `export module X;` or
// `module X;`.  We therefore use the portable form:
//   1. #include everything we need
//   2. import the module whose types we consume
// ============================================================
#include "DefaultFaceDetectorPlugin.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtConcurrent>
#include <QDebug>

// Types provided via DefaultFaceDetectorPlugin.h → IFaceDetectorPlugin.h → CoreFaceDetectorFwd.h
import Core.FaceDetector;

namespace Glance::Plugins {

DefaultFaceDetectorPlugin::DefaultFaceDetectorPlugin() 
    : m_faceDetector(std::make_shared<Core::FaceDetector>()) {
}

DefaultFaceDetectorPlugin::~DefaultFaceDetectorPlugin() = default;

// IPlugin interface implementation

QString DefaultFaceDetectorPlugin::name() const {
    return "DefaultFaceDetector";
}

QString DefaultFaceDetectorPlugin::version() const {
    return "1.0.0";
}

QString DefaultFaceDetectorPlugin::description() const {
    return "Default face detection plugin with dlib and OpenCV support";
}

QString DefaultFaceDetectorPlugin::author() const {
    return "Glance Team";
}

QStringList DefaultFaceDetectorPlugin::dependencies() const {
    return QStringList(); // No dependencies
}

bool DefaultFaceDetectorPlugin::initialize() {
    if (m_initialized) {
        return true;
    }
    
    try {
        // Initialize face detector
        if (!m_faceDetector) {
            m_faceDetector = std::make_shared<Core::FaceDetector>();
        }
        
        // Check if face detection is supported
        if (!m_faceDetector->isSupported()) {
            updateLastError("Face detection not supported on this system");
            return false;
        }
        
        // Apply initial configuration
        applyConfigurationToDetector();
        
        m_initialized = true;
        m_lastError.clear();
        
        qDebug() << "DefaultFaceDetectorPlugin initialized successfully";
        return true;
        
    } catch (const std::exception& e) {
        updateLastError(QString("Initialization failed: %1").arg(e.what()));
        return false;
    }
}

void DefaultFaceDetectorPlugin::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    m_faceDetector.reset();
    m_initialized = false;
    
    qDebug() << "DefaultFaceDetectorPlugin shut down";
}

bool DefaultFaceDetectorPlugin::isInitialized() const {
    return m_initialized;
}

QJsonObject DefaultFaceDetectorPlugin::defaultConfiguration() const {
    return createDefaultConfiguration();
}

bool DefaultFaceDetectorPlugin::configure(const QJsonObject& config) {
    if (!m_initialized) {
        updateLastError("Plugin not initialized");
        return false;
    }
    
    try {
        m_configuration = config;
        
        // Extract configuration values
        if (config.contains("detection")) {
            QJsonObject detection = config["detection"].toObject();
            
            if (detection.contains("minFaceSize")) {
                m_minFaceSize = detection["minFaceSize"].toInt(20);
            }
            
            if (detection.contains("maxFaceSize")) {
                m_maxFaceSize = detection["maxFaceSize"].toInt(500);
            }
            
            if (detection.contains("confidenceThreshold")) {
                m_confidenceThreshold = detection["confidenceThreshold"].toDouble(0.5);
            }
            
            if (detection.contains("types")) {
                QJsonArray types = detection["types"].toArray();
                m_detectionTypes.clear();
                for (const QJsonValue& type : types) {
                    m_detectionTypes.append(type.toString());
                }
            }
        }
        
        // Apply configuration to detector
        applyConfigurationToDetector();
        
        m_lastError.clear();
        return true;
        
    } catch (const std::exception& e) {
        updateLastError(QString("Configuration failed: %1").arg(e.what()));
        return false;
    }
}

QJsonObject DefaultFaceDetectorPlugin::currentConfiguration() const {
    return m_configuration;
}

QString DefaultFaceDetectorPlugin::lastError() const {
    return m_lastError;
}

bool DefaultFaceDetectorPlugin::hasError() const {
    return !m_lastError.isEmpty();
}

// IFaceDetectorPlugin interface implementation

std::optional<Core::FaceDetectionResult> DefaultFaceDetectorPlugin::detectFaces(const QImage& image) {
    if (!m_initialized || !m_faceDetector) {
        updateLastError("Plugin not initialized");
        return std::nullopt;
    }
    
    m_performanceTimer.start();
    
    try {
        auto result = m_faceDetector->detectFaces(image);
        updatePerformanceMetrics(m_performanceTimer.elapsed());
        return result;
        
    } catch (const std::exception& e) {
        updateLastError(QString("Face detection failed: %1").arg(e.what()));
        return std::nullopt;
    }
}

QFuture<Core::FaceDetectionResult> DefaultFaceDetectorPlugin::detectFacesAsync(const QImage& image) {
    if (!m_initialized || !m_faceDetector) {
        updateLastError("Plugin not initialized");
        return QFuture<Core::FaceDetectionResult>();
    }
    
    auto imagePtr = std::make_shared<QImage>(image);
    QPointer<DefaultFaceDetectorPlugin> guard(this);
    return QtConcurrent::run([guard, imagePtr]() {
        if (!guard) {
            return Core::FaceDetectionResult{};
        }
        auto result = guard->detectFaces(*imagePtr);
        if (result) {
            return *result;
        }
        return Core::FaceDetectionResult{};
    });
}

void DefaultFaceDetectorPlugin::setMinFaceSize(int minSize) {
    m_minFaceSize = minSize;
    if (m_initialized && m_faceDetector) {
        m_faceDetector->setMinFaceSize(minSize);
    }
}

void DefaultFaceDetectorPlugin::setMaxFaceSize(int maxSize) {
    m_maxFaceSize = maxSize;
    if (m_initialized && m_faceDetector) {
        m_faceDetector->setMaxFaceSize(maxSize);
    }
}

void DefaultFaceDetectorPlugin::setConfidenceThreshold(double threshold) {
    m_confidenceThreshold = threshold;
    if (m_initialized && m_faceDetector) {
        m_faceDetector->setConfidenceThreshold(threshold);
    }
}

void DefaultFaceDetectorPlugin::setDetectionTypes(const QStringList& types) {
    m_detectionTypes = types;
    // Note: This would require additional implementation in FaceDetector
    // to support dynamic type switching
}

QStringList DefaultFaceDetectorPlugin::supportedDetectionTypes() const {
    return getAvailableDetectionTypes();
}

bool DefaultFaceDetectorPlugin::supportsAsync() const {
    return true;
}

bool DefaultFaceDetectorPlugin::supportsParallel() const {
    return true;
}

bool DefaultFaceDetectorPlugin::supportsProfileDetection() const {
    return true;
}

bool DefaultFaceDetectorPlugin::supportsAIEnhancement() const {
    return true;
}

bool DefaultFaceDetectorPlugin::isSupported() const {
    return Core::FaceDetector::isSupported();
}

QString DefaultFaceDetectorPlugin::getDetectionModelInfo() const {
    return "dlib HOG + OpenCV Haar cascades with AI enhancements";
}

double DefaultFaceDetectorPlugin::getAverageDetectionTime() const {
    if (m_detectionCount == 0) {
        return 0.0;
    }
    return static_cast<double>(m_totalDetectionTime) / m_detectionCount;
}

int DefaultFaceDetectorPlugin::getTotalDetections() const {
    return m_detectionCount;
}

void DefaultFaceDetectorPlugin::resetMetrics() {
    m_totalDetectionTime = 0;
    m_detectionCount = 0;
}

// Private methods implementation

void DefaultFaceDetectorPlugin::updateLastError(const QString& error) {
    m_lastError = error;
    qWarning() << "DefaultFaceDetectorPlugin error:" << error;
}

void DefaultFaceDetectorPlugin::updatePerformanceMetrics(qint64 detectionTime) const {
    m_totalDetectionTime += detectionTime;
    m_detectionCount++;
}

QJsonObject DefaultFaceDetectorPlugin::createDefaultConfiguration() const {
    QJsonObject config;
    
    // Detection settings
    QJsonObject detection;
    detection["minFaceSize"] = 20;
    detection["maxFaceSize"] = 500;
    detection["confidenceThreshold"] = 0.5;
    
    QJsonArray types;
    types.append("frontal");
    types.append("profile");
    detection["types"] = types;
    
    config["detection"] = detection;
    
    // Performance settings
    QJsonObject performance;
    performance["enableAsync"] = true;
    performance["enableParallel"] = true;
    performance["maxThreads"] = -1; // Use all available threads
    config["performance"] = performance;
    
    // AI enhancement settings
    QJsonObject ai;
    ai["enableProfileDetection"] = true;
    ai["enableAIEnhancement"] = true;
    ai["enableRotationStrategies"] = true;
    config["ai"] = ai;
    
    return config;
}

void DefaultFaceDetectorPlugin::applyConfigurationToDetector() {
    if (!m_initialized || !m_faceDetector) {
        return;
    }
    
    m_faceDetector->setMinFaceSize(m_minFaceSize);
    m_faceDetector->setMaxFaceSize(m_maxFaceSize);
    m_faceDetector->setConfidenceThreshold(m_confidenceThreshold);
}

QStringList DefaultFaceDetectorPlugin::getAvailableDetectionTypes() const {
    return QStringList{
        "frontal",
        "profile",
        "frontal_rotated",
        "profile_rotated",
        "ai_enhanced"
    };
}

} // namespace Glance::Plugins
