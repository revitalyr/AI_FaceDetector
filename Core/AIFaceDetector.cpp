module;
#include <algorithm>
#include <cmath>
#include <type_traits>
#include <memory>
#include <QDebug>
#include <QPainter>
#include <QImage>
#include <QRect>
#include <QElapsedTimer>
module Core.AIFaceDetector;

namespace Glance::Core {

AIFaceDetector::AIFaceDetector()
    : m_baseDetector(std::make_shared<FaceDetector>())
    , m_confidenceThreshold(0.3)
    , m_usePostProcessing(true)
{
    // Enable post-processing in the base detector
    m_baseDetector->setPostProcessingEnabled(m_usePostProcessing);
    qDebug() << "AI Face Detector initialized with post-processing";
}

std::optional<FaceDetectionResult> AIFaceDetector::detectFaces(const QImage& image)
{
    if (!FaceDetector::isSupported()) {
        qWarning() << "Face detection not supported on this platform";
        return std::nullopt;
    }

    QElapsedTimer timer;
    timer.start();

    try {
        // Configure base detector with current settings
        m_baseDetector->setConfidenceThreshold(m_confidenceThreshold);
        m_baseDetector->setPostProcessingEnabled(m_usePostProcessing);

        // Delegate to base detector which now handles post-processing
        auto result = m_baseDetector->detectFaces(image);
        
        if (result) {
            result->processingTimeMs = timer.elapsed();
            qDebug() << QString("AI Face Detection: found %1 faces in %2ms")
                        .arg(result->faces.size())
                        .arg(result->processingTimeMs);
        }

        return result;

    } catch (const std::exception& e) {
        FaceDetectionResult result;
        result.success = false;
        result.errorMessage = QString("AI Face detection error: %1").arg(e.what());
        qWarning() << result.errorMessage;
        return result;
    }
}

void AIFaceDetector::setConfidenceThreshold(ConfidenceThreshold threshold)
{
    m_confidenceThreshold = ConfidenceThreshold(std::clamp(threshold, 0.0, 1.0));
    m_baseDetector->setConfidenceThreshold(m_confidenceThreshold);
}

void AIFaceDetector::setUsePostProcessing(bool use)
{
    m_usePostProcessing = use;
    m_baseDetector->setPostProcessingEnabled(m_usePostProcessing);
}

QString AIFaceDetector::getBackendInfo()
{
    return QString("AI Enhanced Face Detector (Base: %1)")
           .arg(FaceDetector::getBackendInfo());
}

} // namespace Glance::Core
