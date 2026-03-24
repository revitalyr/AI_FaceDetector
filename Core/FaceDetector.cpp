module;
#include <type_traits>
#include <memory>
#include <mutex>
#include <algorithm>
#include <cmath>
#include <stack>
#include <functional>
#include <typeinfo>
#include "TraceScope.h"
#include "PostProcessingStrategy.h"
#include <QTimer>
#include <QDebug>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <QMetaObject>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/objdetect.hpp>
#include <opencv4/opencv2/dnn.hpp>
#include <opencv4/opencv2/core/utils/logger.hpp>
module Core.FaceDetector;

import Core.Constants;

namespace Glance::Core {

class FaceDetector::Impl {
public:
    cv::Ptr<cv::FaceDetectorYN> dnn_detector;
    std::mutex opencv_dnn_mutex;
    Bool initialized = false;
    std::unique_ptr<PostProcessingStrategy> postProcessingStrategy;
    Bool postProcessingEnabled = true;

    Impl() : postProcessingStrategy(std::make_unique<DefaultPostProcessingStrategy>()) {
        cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);
        
        QString appDir = QCoreApplication::applicationDirPath();
        QString projectDir = QDir(appDir).absoluteFilePath("../../"); // Project root from build/debug
        
        QStringList searchPaths = {
            QDir(projectDir).filePath("models/face_detection_yunet_2023mar.onnx"),
            QDir(appDir).filePath("models/face_detection_yunet_2023mar.onnx"),
            QDir::current().filePath("models/face_detection_yunet_2023mar.onnx"),
            QDir(appDir).filePath("data/models/face_detection_yunet_2023mar.onnx")
        };

        QString dnnPath;
        for (const auto& p : searchPaths) {
            if (QFile::exists(p)) {
                dnnPath = p;
                break;
            }
        }

        if (!dnnPath.isEmpty()) {
            try {
                dnn_detector = cv::FaceDetectorYN::create(dnnPath.toStdString(), "", cv::Size(320, 320));
                initialized = true;
                qDebug() << "[FaceDetector] YuNet model loaded from:" << dnnPath;
            } catch (const std::exception& e) {
                qWarning() << "[FaceDetector] Failed to init YuNet:" << e.what();
            }
        } else {
            qCritical() << "[FaceDetector] CRITICAL: YuNet model file not found in any search path!";
        }
    }
};

FaceDetector::FaceDetector() : m_impl(std::make_unique<Impl>()) {}
FaceDetector::~FaceDetector() = default;

std::optional<FaceDetectionResult> FaceDetector::detectFaces(const QImage& image)
{
    if (image.isNull() || !m_impl->initialized) {
        return std::nullopt;
    }

    FaceDetectionResult result;
    result.success = false;

    QElapsedTimer timer;
    timer.start();

    // 1. DNN detection (YuNet / SSD) — primary and only method
    std::vector<DetectedFace> candidates = detectWithDNN(image);
    QSize imgSize = image.size();

    qDebug() << "[FaceDetector] Found DNN candidates:" << candidates.size();

    // Filter results
    for (const auto& face : candidates) {
        if (face.confidence < static_cast<ConfidenceValue>(m_confidenceThreshold)) {
            continue;
        }

        // Basic sanity check for DNN detections (can be refined if needed)
        if (face.rect.width() <= 0 || face.rect.height() <= 0) {
            continue;
        }

        double aspect = static_cast<double>(face.rect.height()) / face.rect.width();
        if (aspect >= 0.3 && aspect <= 3.0) {
            result.faces.push_back(face);
        }
    }

    qDebug() << "[FaceDetector] After confidence/ratio filter:" << result.faces.size();

    // Non-Maximum Suppression — remove overlapping boxes
    std::vector<DetectedFace> uniqueFaces;
    for (const auto& f : result.faces) {
        bool duplicate = false;
        for (auto& uf : uniqueFaces) {
            QRect intersection = f.rect.intersected(uf.rect);
            double intersectionArea = intersection.width() * intersection.height();
            double unionArea = (f.rect.width() * f.rect.height()) + (uf.rect.width() * uf.rect.height()) - intersectionArea;
            double ioU = intersectionArea / unionArea;

            if (ioU > MagicNumbers::IOU_THRESHOLD) {
                duplicate = true;
                // Keep the detection with higher confidence
                if (f.confidence > uf.confidence) { 
                    uf = f; 
                }
                break;
            }
        }
        if (!duplicate) {
            uniqueFaces.push_back(f);
        }
    }

    result.faces = uniqueFaces;
    result.success = !result.faces.empty();
    result.processingTimeMs = timer.elapsed();

    // Apply post-processing strategy if enabled
    if (m_impl->postProcessingEnabled && m_impl->postProcessingStrategy) {
        result.faces = m_impl->postProcessingStrategy->process(result.faces, image);
    }

    qDebug() << "[FaceDetector] Final faces after post-processing:" << result.faces.size();
    return result;
}

// Validators (kept for compatibility, limits relaxed)
Bool FaceDetector::isLikelyLargeFace(const QRect& rect, const QSize& imageSize, FaceType type) {
    Q_UNUSED(type);
    int maxAllowedSize = std::max(imageSize.width(), imageSize.height()) * 0.95;
    if (rect.width() > maxAllowedSize || rect.height() > maxAllowedSize) return false; 
    Float64 aspectRatio = static_cast<Float64>(rect.height()) / rect.width();
    if (aspectRatio < 0.5 || aspectRatio > 2.0) return false;
    return Bool(true);
}

Bool FaceDetector::isLikelyAngledFace(const QRect& rect, const QSize& imageSize, FaceType type) {
    Q_UNUSED(imageSize); Q_UNUSED(type);
    Float64 aspectRatio = static_cast<Float64>(rect.height()) / rect.width();
    if (aspectRatio < 0.5 || aspectRatio > 2.0) return false;
    return Bool(true);
}

Bool FaceDetector::isLikelyBottomFace(const QRect& rect, const QSize& imageSize, FaceType type) {
    Q_UNUSED(type);
    int centerY = rect.y() + rect.height() / 2;
    if (centerY < imageSize.height() * 0.2) return false;
    
    int minSize = std::min(imageSize.width(), imageSize.height()) / 25;
    int maxSize = std::min(imageSize.width(), imageSize.height()) / 1.5;
    if (rect.width() < minSize || rect.height() < minSize || rect.width() > maxSize || rect.height() > maxSize) return false;
    
    Float64 aspectRatio = static_cast<Float64>(rect.height()) / rect.width();
    if (aspectRatio < 0.5 || aspectRatio > 2.2) return false;
    return Bool(true);
}

Bool FaceDetector::isLikelyExtremeCaseFace(const QRect& rect, const QSize& imageSize, FaceType type) {
    Q_UNUSED(type);
    int minSize = std::min(imageSize.width(), imageSize.height()) / 40;
    int maxSize = std::min(imageSize.width(), imageSize.height()) / 1.05;
    if (rect.width() < minSize || rect.height() < minSize || rect.width() > maxSize || rect.height() > maxSize) return false;
    return Bool(true);
}

// Conversion helpers
static cv::Mat copyQImageToMat(const QImage& img, int cvType, int channels) {
    cv::Mat dst(img.height(), img.width(), cvType);
    const uchar* src = img.constBits();
    size_t srcStep = static_cast<size_t>(img.bytesPerLine());
    size_t dstStep = dst.step;
    size_t rowBytes = static_cast<size_t>(img.width()) * channels;
    if (srcStep == dstStep) {
        std::memcpy(dst.data, src, dstStep * static_cast<size_t>(img.height()));
    } else {
        for (int y = 0; y < img.height(); ++y)
            std::memcpy(dst.ptr(y), src + y * srcStep, rowBytes);
    }
    return dst;
}

cv::Mat FaceDetector::qimageToCvMat(const QImage& image) {
    if (image.isNull()) return cv::Mat();
    switch (image.format()) {
        case QImage::Format_RGB888: {
            cv::Mat src = copyQImageToMat(image, CV_8UC3, 3);
            cv::Mat result;
            cv::cvtColor(src, result, cv::COLOR_RGB2BGR);
            return result;
        }
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied: {
            return copyQImageToMat(image, CV_8UC4, 4);
        }
        default: {
            QImage converted = image.convertToFormat(QImage::Format_RGB888);
            cv::Mat src = copyQImageToMat(converted, CV_8UC3, 3);
            cv::Mat result;
            cv::cvtColor(src, result, cv::COLOR_RGB2BGR);
            return result;
        }
    }
}

QRect FaceDetector::cvRectToQRect(const cv::Rect& rect) {
    return QRect(rect.x, rect.y, rect.width, rect.height);
}

ConfidenceValue FaceDetector::calculateDetectionConfidence(const QImage& image, const QRect& detection) {
    Q_UNUSED(image);
    double area = detection.width() * detection.height();
    if (area <= 0) return 0.0;
    return ConfidenceValue(std::clamp(0.45 + (area / 70000.0), 0.1, 1.0));
}

void FaceDetector::setMinFaceSize(FaceSizeMin minSize) { m_minFaceSize = minSize; }
void FaceDetector::setMaxFaceSize(FaceSizeMax maxSize) { m_maxFaceSize = maxSize; }
void FaceDetector::setConfidenceThreshold(ConfidenceThreshold threshold) { m_confidenceThreshold = threshold; }

void FaceDetector::setPostProcessingEnabled(Bool enabled) { 
    m_impl->postProcessingEnabled = enabled; 
    if (m_impl->postProcessingStrategy) {
        m_impl->postProcessingStrategy->setEnabled(enabled);
    }
}

bool FaceDetector::isPostProcessingEnabled() const { 
    return m_impl->postProcessingEnabled; 
}

Bool FaceDetector::isSupported() { return Bool(true); }
QString FaceDetector::getBackendInfo() { return "OpenCV DNN (YuNet)"; }

QFuture<FaceDetectionResult> FaceDetector::detectFacesAsync(const QImage& image) {
    auto imagePtr = std::make_shared<QImage>(image);
    std::weak_ptr<FaceDetector> weakThis = shared_from_this();
    return QtConcurrent::run([weakThis, imagePtr]() {
        auto detector = weakThis.lock();
        if (!detector) {
            return FaceDetectionResult{};
        }
        auto res = detector->detectFaces(*imagePtr);
        return res.value_or(FaceDetectionResult());
    });
}

// Stubs for extended strategy methods
Glance::Core::ImageCharacteristics FaceDetector::analyzeImageCharacteristics(const QImage& image) { Q_UNUSED(image); return Glance::Core::ImageCharacteristics(); }
std::vector<Glance::Core::ConfidenceThreshold> FaceDetector::getAdaptiveThresholds(const Glance::Core::ImageCharacteristics& c) { Q_UNUSED(c); return {}; }
std::vector<DetectedFace> FaceDetector::detectWithEnhancedCLAHE(const QImage& img) { Q_UNUSED(img); return {}; }
std::vector<DetectedFace> FaceDetector::detectWithForcedUpscale(const QImage& img) { Q_UNUSED(img); return {}; }
std::vector<DetectedFace> FaceDetector::detectWithOriginalSize(const QImage& img) { Q_UNUSED(img); return {}; }
std::vector<DetectedFace> FaceDetector::detectWithExtendedRotation(const QImage& img) { Q_UNUSED(img); return {}; }

std::vector<DetectedFace> FaceDetector::detectWithDNN(const QImage& img) {
    if (img.isNull() || !m_impl->dnn_detector) {
        return {};
    }

    try {
        cv::Mat mat = qimageToCvMat(img);
        if (mat.empty()) return {};

        std::vector<DetectedFace> dnnResults;
        // FaceDetectorYN requires a 3-channel BGR image.
        if (mat.channels() == 4) {
            cv::Mat bgr;
            cv::cvtColor(mat, bgr, cv::COLOR_BGRA2BGR);
            mat = bgr;
        }

        std::lock_guard<std::mutex> lock(m_impl->opencv_dnn_mutex);
        
        // Update detector parameters for the current frame
        m_impl->dnn_detector->setScoreThreshold(static_cast<float>(m_confidenceThreshold));
        m_impl->dnn_detector->setInputSize(mat.size());

        cv::Mat detections;
        m_impl->dnn_detector->detect(mat, detections);

        if (detections.cols < 4 || detections.rows == 0) return dnnResults;

        const int cols = detections.cols;

        for (int i = 0; i < detections.rows; i++) {
            float x = detections.at<float>(i, 0);
            float y = detections.at<float>(i, 1);
            float w = detections.at<float>(i, 2);
            float h = detections.at<float>(i, 3);

            float confidence = (cols > 14) ? detections.at<float>(i, 14) : 0.0f;

            DetectedFace face;
            face.rect = QRect(static_cast<int>(x), static_cast<int>(y), 
                              static_cast<int>(w), static_cast<int>(h));
            face.type = FaceType::Unknown;
            face.confidence = static_cast<ConfidenceValue>(confidence);

            // Extract up to 5 landmarks: right eye, left eye, nose tip, mouth right, mouth left
            int numLandmarks = (std::min)((cols - 4) / 2, 5);
            for (int n = 0; n < numLandmarks; ++n) {
                float lx = detections.at<float>(i, 4 + n * 2);
                float ly = detections.at<float>(i, 5 + n * 2);
                face.landmarks.push_back(QPointF(lx, ly));
            }

            dnnResults.push_back(face);
        }
        return dnnResults;
    } catch (const std::exception& e) {
        qWarning() << "DNN detection failure:" << e.what();
        return {};
    }
}

} // namespace Glance::Core