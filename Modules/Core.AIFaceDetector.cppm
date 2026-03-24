module;

#include <memory>
#include <vector>
#include <optional>

export module Core.AIFaceDetector;

import Qt.Wrapper;
export import Core.FaceDetector;

export
{
    using ::QImage;
    using ::QString;
    using ::QSize;
    using ::QRect;
}

export namespace Glance::Core {

/** @brief Thin wrapper around FaceDetector with AI-specific configuration */
class AIFaceDetector {
public:
    AIFaceDetector();

    std::optional<FaceDetectionResult> detectFaces(const QImage& image);

    void setConfidenceThreshold(double threshold);
    void setUsePostProcessing(bool use);

    static QString getBackendInfo();
    static bool isSupported() { return FaceDetector::isSupported(); }

private:
    std::shared_ptr<FaceDetector> m_baseDetector;
    double m_confidenceThreshold;
    bool m_usePostProcessing;
};

}
