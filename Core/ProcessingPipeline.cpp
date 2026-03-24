module;
#include <type_traits>
#include <memory>
#include <string>
#include <coroutine>
#include <QElapsedTimer>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QtConcurrent>
module Core.ProcessingPipeline;
import Core.ImageProcessor;
import Core.FaceDetector;

namespace Glance::Core {

ProcessingPipeline::ProcessingPipeline(QObject* parent)
    : QObject(parent)
    , m_imageProcessor(ProcessorFactory::create(ProcessorFactory::ProcessorType::Basic))
    , m_faceDetector(std::make_shared<FaceDetector>())
    , m_imageWatcher(std::make_unique<QFutureWatcher<QImage>>())
    , m_faceWatcher(std::make_unique<QFutureWatcher<FaceDetectionResult>>())
{
    // Connect signals for async operations
    connect(m_imageWatcher.get(), &QFutureWatcher<QImage>::finished, this, [this]() {
        try {
            QImage result = m_imageWatcher->result();
            m_isProcessing.store(false, std::memory_order_release);
            updateState(ProcessingStage::Completed, 100, "Processing completed");
        } catch (const std::exception& e) {
            updateStateError(QString("Processing failed: %1").arg(e.what()));
        }
    });
    
    connect(m_faceWatcher.get(), &QFutureWatcher<FaceDetectionResult>::finished, this, [this]() {
        try {
            m_faceWatcher->result();
        } catch (const std::exception& e) {
            updateStateError(QString("Face detection failed: %1").arg(e.what()));
        }
    });
}

ProcessingPipeline::~ProcessingPipeline() = default;

QFuture<QImage> ProcessingPipeline::processImageAsync(const QString& imagePath, const ProcessingParams& params) {
    if (m_isProcessing.load(std::memory_order_acquire)) {
        updateStateError("Another operation is in progress");
        return QFuture<QImage>();
    }
    
    m_isProcessing.store(true, std::memory_order_release);
    m_currentParams = params;
    updateState(ProcessingStage::Loading, 0, "Starting image processing...");
    
    QPointer<ProcessingPipeline> guard(this);
    return QtConcurrent::run([guard, imagePath, params]() -> QImage {
        if (!guard) {
            return QImage();
        }
        
        try {
            // Stage 1: Load image
            guard->updateState(ProcessingStage::Loading, 10, QString("Loading image from: %1").arg(imagePath));
            QImage sourceImage = guard->loadImage(imagePath);
            if (sourceImage.isNull()) {
                throw std::runtime_error("Failed to load image");
            }
            
            // Stage 2: Face detection (if enabled)
            if (guard->m_faceDetectionEnabled && guard->m_faceDetector) {
                guard->updateState(ProcessingStage::FaceDetection, 30, "Detecting faces...");
                auto faceResult = guard->performFaceDetection(sourceImage);
                // We don't wait for face detection to complete before continuing
            }
            
            // Stage 3: Image processing
            guard->updateState(ProcessingStage::ImageProcessing, 60, "Applying filters...");
            QImage processedImage = guard->applyImageProcessing(sourceImage, params);
            
            guard->updateState(ProcessingStage::Completed, 100, "Processing completed");
            return processedImage;
            
        } catch (const std::exception& e) {
            guard->updateStateError(QString("Processing error: %1").arg(e.what()));
            return QImage();
        }
    });
}

QFuture<FaceDetectionResult> ProcessingPipeline::detectFacesAsync(const QString& imagePath) {
    if (!m_faceDetector) {
        return QFuture<FaceDetectionResult>();
    }
    
    QPointer<ProcessingPipeline> guard(this);
    QFuture<FaceDetectionResult> future = QtConcurrent::run([guard, imagePath]() -> FaceDetectionResult {
        if (!guard) {
            return FaceDetectionResult{ {}, false, "Pipeline destroyed", 0 };
        }
        
        try {
            QImage image = guard->loadImage(imagePath);
            if (image.isNull()) {
                return FaceDetectionResult{ {}, false, "Failed to load image", 0 };
            }
            
            return guard->performFaceDetection(image);
        } catch (const std::exception& e) {
            return FaceDetectionResult{ {}, false, QString("Detection failed: %1").arg(e.what()), 0 };
        }
    });
    
    // Set the future to the watcher for signal emission
    m_faceWatcher->setFuture(future);
    
    return future;
}

void ProcessingPipeline::cancelCurrentOperation() {
    if (m_imageWatcher->isRunning()) {
        m_imageWatcher->cancel();
    }
    
    if (m_faceWatcher->isRunning()) {
        m_faceWatcher->cancel();
    }
    
    m_isProcessing.store(false, std::memory_order_release);
    updateState(ProcessingStage::Error, 0, "Operation cancelled");
}

bool ProcessingPipeline::isProcessing() const {
    return m_isProcessing.load(std::memory_order_acquire);
}

void ProcessingPipeline::setProcessingParams(const ProcessingParams& params) {
    m_currentParams = params;
}

void ProcessingPipeline::setFaceDetectionEnabled(bool enabled) {
    m_faceDetectionEnabled = enabled;
}

PipelineState ProcessingPipeline::getCurrentState() const {
    return m_currentState;
}

QImage ProcessingPipeline::loadImage(const QString& imagePath) {
    if (imagePath.isEmpty() || imagePath.trimmed().isEmpty()) {
        throw std::runtime_error("Image path is empty");
    }

    QString resolved = QDir::cleanPath(imagePath);
    if (resolved != imagePath) {
        qDebug() << "Path normalized:" << imagePath << "->" << resolved;
    }

    QFileInfo fileInfo(resolved);
    if (!fileInfo.exists()) {
        throw std::runtime_error("Image file does not exist");
    }
    if (!fileInfo.isFile()) {
        throw std::runtime_error("Path is not a regular file");
    }
    if (fileInfo.isSymLink()) {
        resolved = fileInfo.symLinkTarget();
        fileInfo = QFileInfo(resolved);
        if (!fileInfo.exists() || !fileInfo.isFile()) {
            throw std::runtime_error("Symlink target is not a valid file");
        }
    }

    QImage image(resolved);
    if (image.isNull()) {
        throw std::runtime_error("Unsupported image format or corrupted file");
    }

    // Preserve original format — Segmenter and ImageProcessor convert internally
    return image;
}

[[nodiscard]] QImage ProcessingPipeline::applyImageProcessing(const QImage& source, const ProcessingParams& params) {
    if (!m_imageProcessor) {
        return source;
    }
    
    return m_imageProcessor->processImage(source, params);
}

FaceDetectionResult ProcessingPipeline::performFaceDetection(const QImage& image) {
    if (!m_faceDetector) {
        return FaceDetectionResult{ {}, false, "Face detector not available", 0 };
    }
    
    auto result = m_faceDetector->detectFaces(image);
    return result ? *result : FaceDetectionResult{ {}, false, "Face detection failed", 0 };
}

void ProcessingPipeline::updateState(ProcessingStage stage, int progress, const QString& message) {
    m_currentState.stage = stage;
    m_currentState.progress = progress;
    m_currentState.message = message;
    m_currentState.error.reset();
    
}

void ProcessingPipeline::updateStateError(const QString& error) {
    m_currentState.stage = ProcessingStage::Error;
    m_currentState.progress = 0;
    m_currentState.message = "Error occurred";
    m_currentState.error = error;
    m_isProcessing.store(false, std::memory_order_release);
    
}


std::generator<PipelineState> AsyncImageProcessor::processWithCoroutines(
    const QString& imagePath, 
    const ProcessingParams& params
) {
    PipelineState state;
    
    co_yield state; // Initial state
    
    // Simulate async loading
    state.stage = ProcessingStage::Loading;
    for (int i = 0; i <= 30; i += 10) {
        state.progress = i;
        co_yield state;
    }
    
    // Simulate processing
    state.stage = ProcessingStage::ImageProcessing;
    for (int i = 30; i <= 100; i += 10) {
        state.progress = i;
        co_yield state;
    }
    
    state.stage = ProcessingStage::Completed;
    co_yield state;
}


} // namespace Glance::Core
