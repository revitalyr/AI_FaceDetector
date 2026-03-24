module;

#include "PostProcessingStrategy.h"
#include <algorithm>
#include <cmath>

module Core.FaceDetector;

namespace Glance {
namespace Core {

DefaultPostProcessingStrategy::DefaultPostProcessingStrategy() = default;

std::vector<DetectedFace> DefaultPostProcessingStrategy::process(
    const std::vector<DetectedFace>& detections,
    const QImage& image
) {
    if (!m_enabled || detections.empty()) {
        return detections;
    }

    std::vector<DetectedFace> result = detections;
    
    result = expandBoundingBoxes(result, image);
    result = correctAspectRatio(result, image);
    result = filterBySize(result, image.size());
    result = filterByPosition(result, image.size());
    
    return result;
}

QString DefaultPostProcessingStrategy::description() const {
    return "Default post-processing: box expansion, aspect ratio correction, size/position filtering";
}

Bool DefaultPostProcessingStrategy::isEnabled() const {
    return m_enabled;
}

void DefaultPostProcessingStrategy::setEnabled(Bool enabled) {
    m_enabled = enabled;
}

void DefaultPostProcessingStrategy::setBoxExpansion(Int32 pixels) {
    m_boxExpansion = pixels < 0 ? 0 : pixels;
}

void DefaultPostProcessingStrategy::setMinFaceSizeFraction(Float64 fraction) {
    m_minFaceSizeFraction = std::clamp(fraction, 0.01, 1.0);
}

void DefaultPostProcessingStrategy::setMaxFaceSizeFraction(Float64 fraction) {
    m_maxFaceSizeFraction = std::clamp(fraction, 0.01, 1.0);
}

void DefaultPostProcessingStrategy::setMaxYPositionFraction(Float64 fraction) {
    m_maxYPositionFraction = std::clamp(fraction, 0.0, 1.0);
}

std::vector<DetectedFace> DefaultPostProcessingStrategy::expandBoundingBoxes(
    const std::vector<DetectedFace>& detections,
    const QImage& image
) {
    std::vector<DetectedFace> expanded;
    
    for (const auto& detection : detections) {
        QRect rect = detection.rect.adjusted(-m_boxExpansion, -m_boxExpansion, 
                                            m_boxExpansion, m_boxExpansion);
        rect = rect.intersected(image.rect());
        
        DetectedFace expandedFace = detection;
        expandedFace.rect = rect;
        expanded.push_back(expandedFace);
    }
    
    return expanded;
}

std::vector<DetectedFace> DefaultPostProcessingStrategy::correctAspectRatio(
    const std::vector<DetectedFace>& detections,
    const QImage& image
) {
    std::vector<DetectedFace> corrected;
    
    for (const auto& detection : detections) {
        QRect rect = detection.rect;
        
        // Apply aspect ratio correction (faces are typically taller than wide)
        int width = rect.width();
        int height = rect.height();
        double aspectRatio = static_cast<double>(height) / width;
        
        if (aspectRatio < 1.2) {
            int newHeight = static_cast<int>(width * 1.3);
            int yDiff = (newHeight - height) / 2;
            rect.adjust(0, -yDiff, 0, newHeight - height + yDiff);
        } else if (aspectRatio > 1.8) {
            int newWidth = static_cast<int>(height / 1.5);
            int xDiff = (newWidth - width) / 2;
            rect.adjust(-xDiff, 0, newWidth - width + xDiff, 0);
        }
        
        rect = rect.intersected(image.rect());
        
        DetectedFace correctedFace = detection;
        correctedFace.rect = rect;
        corrected.push_back(correctedFace);
    }
    
    return corrected;
}

std::vector<DetectedFace> DefaultPostProcessingStrategy::filterBySize(
    const std::vector<DetectedFace>& detections,
    const QSize& imageSize
) {
    std::vector<DetectedFace> filtered;
    
    int minFaceSize = static_cast<int>(std::min(imageSize.width(), imageSize.height()) * m_minFaceSizeFraction);
    int maxFaceSize = static_cast<int>(std::min(imageSize.width(), imageSize.height()) * m_maxFaceSizeFraction);
    
    for (const auto& detection : detections) {
        const QRect& rect = detection.rect;
        
        if (rect.width() < minFaceSize || rect.height() < minFaceSize) {
            continue;
        }
        if (rect.width() > maxFaceSize || rect.height() > maxFaceSize) {
            continue;
        }

        filtered.push_back(detection);
    }

    return filtered;
}

std::vector<DetectedFace> DefaultPostProcessingStrategy::filterByPosition(
    const std::vector<DetectedFace>& detections,
    const QSize& imageSize
) {
    std::vector<DetectedFace> filtered;
    int maxYPosition = static_cast<int>(imageSize.height() * m_maxYPositionFraction);

    for (const auto& detection : detections) {
        const QRect& rect = detection.rect;

        int centerY = rect.y() + rect.height() / 2;
        if (centerY > maxYPosition) {
            continue;
        }

        filtered.push_back(detection);
    }

    return filtered;
}

// ── NoPostProcessingStrategy ─────────────────────────────────────────────────

std::vector<DetectedFace> NoPostProcessingStrategy::process(
    const std::vector<DetectedFace>& detections,
    const QImage& image
) {
    return detections;
}

QString NoPostProcessingStrategy::description() const {
    return "No post-processing (pass-through)";
}

Bool NoPostProcessingStrategy::isEnabled() const {
    return true;
}

void NoPostProcessingStrategy::setEnabled(Bool enabled) {
    (void)enabled;
}

} // namespace Core
} // namespace Glance
