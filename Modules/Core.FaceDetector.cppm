/**
 * @file Core.FaceDetector.cppm
 * @brief Main face detection engine with multi-strategy detection
 *
 * Provides the primary FaceDetector class that orchestrates face
 * detection using multiple strategies (CLAHE-enhanced, DNN, rotation-aware,
 * forced upscale). Supports synchronous and asynchronous (QFuture)
 * detection with configurable confidence thresholds and face size limits.
 */
module;

#include <memory>
#include <type_traits>
#include <functional>
#include <vector>
#include <optional>
#include <opencv4/opencv2/opencv.hpp>
#include "../Core/Plugins/CoreFaceDetectorFwd.h"

export module Core.FaceDetector;

import Qt.Wrapper;
export import Core.TypeAliases;

export
{
    using ::QImage;
    using ::QRect;
    using ::QPointF;
    using ::QString;
    using ::QFuture;
    using ::QSize;
}

export namespace Glance::Core {
using ::Glance::Core::FaceType;
using ::Glance::Core::DetectedFace;
using ::Glance::Core::FaceDetectionResult;

/** @brief Single detection result annotated with score and strategy source */
struct DetectionWithScore {
    DetectedFace face;
    DetectionScore score = 0.0;
    StrategyIndex strategyIndex = 0;
};

/** @brief Multi-strategy face detector with adaptive thresholds and post-processing */
class FaceDetector : public std::enable_shared_from_this<FaceDetector> {
public:
    FaceDetector();
    ~FaceDetector();

    std::optional<FaceDetectionResult> detectFaces(const QImage& image);
    QFuture<FaceDetectionResult> detectFacesAsync(const QImage& image);

    void setMinFaceSize(FaceSizeMin minSize);
    void setMaxFaceSize(FaceSizeMax maxSize);
    void setConfidenceThreshold(ConfidenceThreshold threshold);

    void setPostProcessingEnabled(Bool enabled);
    Bool isPostProcessingEnabled() const;

    static Bool isSupported();
    static QString getBackendInfo();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

    cv::Mat qimageToCvMat(const QImage& image);
    QRect cvRectToQRect(const cv::Rect& rect);
    cv::Rect qRectToCvRect(const QRect& rect);
    QImage rotateImage(const QImage& image, RotationAngle angle);
    QRect rotateBoundingBoxBack(const QRect& rect, const QSize& imageSize, RotationAngle angle);
    QRect scaleBoundingBoxBack(const QRect& rect, ScaleFactor scale);
    QRect fuseBoundingBoxes(const QRect& box1, const QRect& box2, Float64 weight1, Float64 weight2);

    ImageCharacteristics analyzeImageCharacteristics(const QImage& image);
    std::vector<ConfidenceThreshold> getAdaptiveThresholds(const ImageCharacteristics& characteristics);
    std::vector<DetectedFace> detectWithEnhancedCLAHE(const QImage& image);
    std::vector<DetectedFace> detectWithForcedUpscale(const QImage& image);
    std::vector<DetectedFace> detectWithOriginalSize(const QImage& image);
    std::vector<DetectedFace> detectWithExtendedRotation(const QImage& image);
    std::vector<DetectedFace> detectWithDNN(const QImage& image);

    DetectionScore calculateDetectionScore(const QImage& image, const QRect& detection, StrategyIndex strategyIndex);
    ConfidenceValue calculateDetectionConfidence(const QImage& image, const QRect& detection);

    Bool isLikelyLargeFace(const QRect& rect, const QSize& imageSize, FaceType type);
    Bool isLikelyAngledFace(const QRect& rect, const QSize& imageSize, FaceType type);
    Bool isLikelyBottomFace(const QRect& rect, const QSize& imageSize, FaceType type);
    Bool isLikelyExtremeCaseFace(const QRect& rect, const QSize& imageSize, FaceType type);

    FaceSizeMin m_minFaceSize = 20;
    FaceSizeMax m_maxFaceSize = 500;
    ConfidenceThreshold m_confidenceThreshold = 0.4;
};

}
