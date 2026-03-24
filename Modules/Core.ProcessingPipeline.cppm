/**
 * @file Core.ProcessingPipeline.cppm
 * @brief Pipeline orchestrating image loading, processing, and face detection
 *
 * ProcessingPipeline connects image loading, face detection, and
 * image processing stages into a single async pipeline with progress
 * reporting. Supports cancellation and state tracking via PipelineState.
 * Also provides the AsyncImageProcessor coroutine-based utility for
 * C++20 coroutine-style pipeline processing.
 */
module;
#include <QImage>
#include <QFuture>
#include <QFutureWatcher>
#include <QObject>
#include <QString>
#include <memory>
#include <variant>
#include <optional>
#include <atomic>

#include <coroutine>
#include <generator>

export module Core.ProcessingPipeline;

import Core.ImageProcessor;
import Core.FaceDetector;

namespace Glance::Core {

/** @brief Stages of the async processing pipeline */
export enum class ProcessingStage {
    Loading,
    FaceDetection,
    ImageProcessing,
    Completed,
    Error
};

export struct PipelineState {
    ProcessingStage stage = ProcessingStage::Loading;
    int progress = 0;
    QString message;
    std::optional<QString> error;
};

export class ProcessingPipeline : public QObject {
public:
    explicit ProcessingPipeline(QObject* parent = nullptr);
    ~ProcessingPipeline() override;

    QFuture<QImage> processImageAsync(const QString& imagePath, const ProcessingParams& params);
    QFuture<FaceDetectionResult> detectFacesAsync(const QString& imagePath);
    
    void cancelCurrentOperation();
    bool isProcessing() const;
    
    void setProcessingParams(const ProcessingParams& params);
    void setFaceDetectionEnabled(bool enabled);
    
    PipelineState getCurrentState() const;

private:
    QImage loadImage(const QString& imagePath);
    QImage applyImageProcessing(const QImage& source, const ProcessingParams& params);
    FaceDetectionResult performFaceDetection(const QImage& image);
    
    void updateState(ProcessingStage stage, int progress = 0, const QString& message = QString());
    void updateStateError(const QString& error);

private:
    std::unique_ptr<ImageProcessor> m_imageProcessor;
    std::shared_ptr<FaceDetector> m_faceDetector;
    
    ProcessingParams m_currentParams;
    PipelineState m_currentState;
    bool m_faceDetectionEnabled = true;
    std::atomic<bool> m_isProcessing = false;
    
    std::unique_ptr<QFutureWatcher<QImage>> m_imageWatcher;
    std::unique_ptr<QFutureWatcher<FaceDetectionResult>> m_faceWatcher;
};

/** @brief C++20 coroutine-based image processing utility */
export class AsyncImageProcessor {
public:
    struct promise_type {
        QImage m_value;
        
        QImage get_return_object() {
            return m_value;
        }
        
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_value(QImage value) { m_value = std::move(value); }
        void unhandled_exception() {}
    };
    
    static std::generator<PipelineState> processWithCoroutines(
        const QString& imagePath, 
        const ProcessingParams& params
    );
};


} // namespace Glance::Core
