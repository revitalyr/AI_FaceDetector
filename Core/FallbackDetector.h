#pragma once

#include <memory>
#include <QString>

namespace Glance {

class FaceDetector;

namespace Core {

/**
 * @brief Singleton factory for fallback FaceDetector and model path resolution
 * 
 * Provides a single entry point for creating fallback FaceDetector instances
 * and resolving model file paths, eliminating duplication across ProcessingPipeline
 * and Orchestrator.
 */
class FallbackDetector {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static FallbackDetector& instance();

    /**
     * @brief Get or create the shared fallback FaceDetector
     * @return Shared pointer to the FaceDetector instance
     */
    std::shared_ptr<FaceDetector> getFaceDetector();

    /**
     * @brief Resolve the path to the YuNet face detection model
     * @return Path to the model file, or empty string if not found
     */
    static QString resolveYuNetModelPath();

    /**
     * @brief Resolve the path to the u2netp segmentation model
     * @return Path to the model file, or empty string if not found
     */
    static QString resolveSegmentationModelPath();

    /**
     * @brief Reset the singleton (useful for testing)
     */
    static void reset();

private:
    FallbackDetector() = default;
    ~FallbackDetector() = default;

    FallbackDetector(const FallbackDetector&) = delete;
    FallbackDetector& operator=(const FallbackDetector&) = delete;

    std::shared_ptr<FaceDetector> m_faceDetector;
    static FallbackDetector* s_instance;
};

} // namespace Core
} // namespace Glance
