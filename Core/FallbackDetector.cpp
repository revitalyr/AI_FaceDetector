#include "FallbackDetector.h"
#include "Core/FaceDetector.h"
#include <QDir>
#include <QFile>
#include <QCoreApplication>

namespace Glance {
namespace Core {

FallbackDetector* FallbackDetector::s_instance = nullptr;

FallbackDetector& FallbackDetector::instance() {
    if (!s_instance) {
        s_instance = new FallbackDetector();
    }
    return *s_instance;
}

std::shared_ptr<FaceDetector> FallbackDetector::getFaceDetector() {
    if (!m_faceDetector) {
        m_faceDetector = std::make_shared<FaceDetector>();
    }
    return m_faceDetector;
}

QString FallbackDetector::resolveYuNetModelPath() {
    QString appDir = QCoreApplication::applicationDirPath();
    QString projectDir = QDir(appDir).absoluteFilePath("../../");
    
    QStringList searchPaths = {
        QDir(projectDir).filePath("models/face_detection_yunet_2023mar.onnx"),
        QDir(appDir).filePath("models/face_detection_yunet_2023mar.onnx"),
        QDir::current().filePath("models/face_detection_yunet_2023mar.onnx"),
        QDir(appDir).filePath("data/models/face_detection_yunet_2023mar.onnx")
    };

    for (const auto& path : searchPaths) {
        if (QFile::exists(path)) {
            return path;
        }
    }
    
    return QString();
}

QString FallbackDetector::resolveSegmentationModelPath() {
    QStringList candidates = {
        QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../../models/u2netp.onnx"),
        QCoreApplication::applicationDirPath() + "/../models/u2netp.onnx",
        QCoreApplication::applicationDirPath() + "/../../models/u2netp.onnx",
        QDir::currentPath() + "/models/u2netp.onnx",
        QDir::currentPath() + "/../../models/u2netp.onnx",
    };
    
    for (const auto& path : candidates) {
        if (QFile::exists(path)) {
            return path;
        }
    }
    
    return QString();
}

void FallbackDetector::reset() {
    delete s_instance;
    s_instance = nullptr;
}

} // namespace Core
} // namespace Glance
