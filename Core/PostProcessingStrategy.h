#pragma once

#include <vector>
#include <memory>
#include <QImage>
#include <QSize>
#include "TypeAliases.h"

namespace Glance {

namespace Core {

class DetectedFace;

/**
 * @brief Strategy interface for post-processing face detection results
 * 
 * This interface defines a contract for different post-processing strategies
 * that can be applied to face detection results. Implementations can:
 * - Filter detections based on various criteria
 * - Adjust bounding boxes
 * - Apply heuristic corrections
 * - Merge overlapping detections
 * 
 * This allows for pluggable post-processing logic without modifying the
 * core FaceDetector implementation.
 */
class PostProcessingStrategy {
public:
    virtual ~PostProcessingStrategy() = default;

    /**
     * @brief Apply post-processing to detected faces
     * @param detections Original detected faces from the detector
     * @param image The source image (may be used for context-aware processing)
     * @return Processed list of detected faces
     */
    virtual std::vector<DetectedFace> process(
        const std::vector<DetectedFace>& detections,
        const QImage& image
    ) = 0;

    /**
     * @brief Get a human-readable description of this strategy
     * @return Strategy description
     */
    virtual QString description() const = 0;

    /**
     * @brief Check if this strategy is enabled
     * @return True if enabled, false otherwise
     */
    virtual Bool isEnabled() const = 0;

    /**
     * @brief Enable or disable this strategy
     * @param enabled True to enable, false to disable
     */
    virtual void setEnabled(Bool enabled) = 0;
};

/**
 * @brief Default post-processing strategy with heuristic corrections
 * 
 * This strategy applies common heuristic corrections to face detections:
 * - Expands bounding boxes slightly to include more of the face
 * - Applies aspect ratio correction (faces are typically taller than wide)
 * - Filters by size (removes too small or too large detections)
 * - Filters by position (faces are usually in upper half of image)
 */
class DefaultPostProcessingStrategy : public PostProcessingStrategy {
public:
    DefaultPostProcessingStrategy();
    ~DefaultPostProcessingStrategy() override = default;

    std::vector<DetectedFace> process(
        const std::vector<DetectedFace>& detections,
        const QImage& image
    ) override;

    QString description() const override;
    Bool isEnabled() const override;
    void setEnabled(Bool enabled) override;

    /**
     * @brief Set the expansion amount for bounding boxes (in pixels)
     * @param pixels Number of pixels to expand on each side
     */
    void setBoxExpansion(Int32 pixels);

    /**
     * @brief Set the minimum face size as a fraction of image size
     * @param fraction Fraction (e.g., 0.1 for 10% of min dimension)
     */
    void setMinFaceSizeFraction(Float64 fraction);

    /**
     * @brief Set the maximum face size as a fraction of image size
     * @param fraction Fraction (e.g., 0.5 for 50% of min dimension)
     */
    void setMaxFaceSizeFraction(Float64 fraction);

    /**
     * @brief Set the maximum Y position as a fraction of image height
     * @param fraction Fraction (e.g., 0.8 for 80% of image height)
     */
    void setMaxYPositionFraction(Float64 fraction);

private:
    std::vector<DetectedFace> expandBoundingBoxes(
        const std::vector<DetectedFace>& detections,
        const QImage& image
    );

    std::vector<DetectedFace> correctAspectRatio(
        const std::vector<DetectedFace>& detections,
        const QImage& image
    );

    std::vector<DetectedFace> filterBySize(
        const std::vector<DetectedFace>& detections,
        const QSize& imageSize
    );

    std::vector<DetectedFace> filterByPosition(
        const std::vector<DetectedFace>& detections,
        const QSize& imageSize
    );

    Bool m_enabled = true;
    Int32 m_boxExpansion = 10;
    Float64 m_minFaceSizeFraction = 0.05;
    Float64 m_maxFaceSizeFraction = 0.8;
    Float64 m_maxYPositionFraction = 1.0;
};

/**
 * @brief No-op strategy that passes through detections unchanged
 */
class NoPostProcessingStrategy : public PostProcessingStrategy {
public:
    std::vector<DetectedFace> process(
        const std::vector<DetectedFace>& detections,
        const QImage& image
    ) override;

    QString description() const override;
    Bool isEnabled() const override;
    void setEnabled(Bool enabled) override;
};

} // namespace Core
} // namespace Glance
