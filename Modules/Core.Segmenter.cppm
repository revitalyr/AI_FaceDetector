/**
 * @file Core.Segmenter.cppm
 * @brief Background removal using deep learning segmentation
 *
 * Wraps OpenCV DNN-based segmentation models (u2net) to remove
 * backgrounds from images. Produces SegmentationResult containing
 * the binary mask, composited output, and confidence score.
 */
module;

#include <optional>
#include <memory>
#include <opencv2/dnn.hpp>

export module Core.Segmenter;

import Qt.Wrapper;

export
{
    using ::QImage;
    using ::QString;
}

export namespace Glance::Core {

/** @brief Result of a background segmentation operation */
struct SegmentationResult {
    QImage mask;         ///< Binary foreground mask
    QImage composited;   ///< Composited output image
    float confidence = 0.0f;
};

/** @brief Deep-learning-based background removal using ONNX models */
class Segmenter {
public:
    Segmenter();
    ~Segmenter();

    Segmenter(const Segmenter&) = delete;
    Segmenter& operator=(const Segmenter&) = delete;

    bool loadModel(const QString& modelPath);
    bool isLoaded() const;

    std::optional<SegmentationResult> segment(const QImage& image);

private:
    QImage postprocessFromFloat(const float* data, int width, int height, const QImage& original);

    std::unique_ptr<cv::dnn::Net> m_net;
    int m_inputWidth = 320;
    int m_inputHeight = 320;
    bool m_loaded = false;
};

}
