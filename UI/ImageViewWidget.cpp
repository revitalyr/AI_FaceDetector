module;
#include <QPainter>
#include <QPainterPath>
#include <QOpenGLContext>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QFont>
module UI.ImageViewWidget;
import Core.Constants;
import UI.FaceRenderingUtils;
import Core.ImageProcessor;

namespace Glance::UI {

ImageViewWidget::ImageViewWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , m_vertexBuffer(QOpenGLBuffer::VertexBuffer)
    , m_indexBuffer(QOpenGLBuffer::IndexBuffer)
    , m_processingTimer(new QTimer(this))
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    
    // Setup processing timer for smooth updates
    m_processingTimer->setSingleShot(true);
    m_processingTimer->setInterval(16); // ~60 FPS
    connect(m_processingTimer, &QTimer::timeout, this, [this]() { onProcessingTimerTimeout(); });
}

ImageViewWidget::~ImageViewWidget()
{
    qDebug() << "ImageViewWidget destructor called";
    
    // Safely cleanup OpenGL resources — destroy ALL GL-owned members
    // while the context is still current, then release it.
    // This prevents the base-class QOpenGLFunctions destructor from firing
    // an assertion about non-zero refs.
    try {
        if (context() && context()->isValid()) {
            qDebug() << "ImageViewWidget destructor: Valid OpenGL context, cleaning up resources.";
            makeCurrent();
            m_imageTexture.reset();
            m_lutBaseTexture.reset();
            m_shaderProgram.reset();
            m_vertexBuffer.destroy();
            m_indexBuffer.destroy();
            doneCurrent();
            qDebug() << "OpenGL resources cleaned up safely";
        } else {
            qDebug() << "No valid OpenGL context, skipping cleanup";
        }
    } catch (const std::exception& e) {
        qCritical() << "Exception in ImageViewWidget destructor:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception in ImageViewWidget destructor";
    }
}

void ImageViewWidget::setImage(const QImage& image)
{
    qDebug() << "setImage called with image size:" << image.size() << "format:" << image.format();
    
    if (image.isNull()) {
        qDebug() << "setImage: image is null, clearing";
        clearImage();
        return;
    }
    
    // Convert to ARGB32 for QOpenGLTexture compatibility
    QImage convertedImage = image;
    qDebug() << "setImage: Original image format:" << image.format();
    if (image.format() == QImage::Format_ARGB32 ||
        image.format() == QImage::Format_ARGB32_Premultiplied) {
        // Keep ARGB32 formats as-is (preserves alpha)
        convertedImage = image;
        qDebug() << "Keeping ARGB32 format (alpha preserved)";
    } else {
        convertedImage = image.convertToFormat(QImage::Format_ARGB32);
        qDebug() << "Converted to ARGB32 format";
    }
    qDebug() << "setImage: Converted image size:" << convertedImage.size() << "format:" << convertedImage.format() << "isNull:" << convertedImage.isNull();
    
    m_currentImage = convertedImage;
    m_originalImageSize = convertedImage.size(); // Store original size for scaling
    m_needsUpdate = true;
    
    // Reset view to fit the new image
    fitToWindow();
    
    // Update texture
    makeCurrent();
    if (!context()->isValid()) {
        qCritical() << "setImage: OpenGL context is not valid after makeCurrent()!";
        doneCurrent();
        return;
    }

    try {
        qDebug() << "setImage: Updating texture - convertedImage size:" << convertedImage.size() 
                 << "format:" << convertedImage.format() 
                 << "null:" << convertedImage.isNull();
        
        QImage texImage;
        if (convertedImage.format() == QImage::Format_ARGB32 ||
            convertedImage.format() == QImage::Format_ARGB32_Premultiplied) {
            texImage = convertedImage.convertToFormat(QImage::Format_RGBA8888);
        } else {
            texImage = convertedImage;
        }
        
        QImage flippedImage = texImage.flipped(Qt::Vertical);
        
        if (!m_imageTexture) {
            qDebug() << "setImage: Creating new texture...";
        } else {
            qDebug() << "setImage: Recreating texture (was ID:" << m_imageTexture->textureId() << ")";
        }
        m_imageTexture = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);
        m_imageTexture->setSize(texImage.width(), texImage.height());
        m_imageTexture->setFormat(QOpenGLTexture::RGBA8_UNorm);
        m_imageTexture->allocateStorage();
        m_imageTexture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8,
                                flippedImage.constBits());
        m_imageTexture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
        m_imageTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
        qDebug() << "setImage: Texture created with ID:" << m_imageTexture->textureId();
    } catch (const std::exception& e) {
        qCritical() << "setImage: Exception during OpenGL texture operations:" << e.what();
    } catch (...) {
        qCritical() << "setImage: Unknown exception during OpenGL texture operations.";
    }
    doneCurrent();
    
    update();
}

void ImageViewWidget::clearImage()
{
    m_currentImage = QImage();
    m_lutBaseImage = QImage();
    m_faceRects.clear();
    
    makeCurrent();
    m_imageTexture.reset();
    m_lutBaseTexture.reset();
    doneCurrent();
    
    update();
}

void ImageViewWidget::setLUTBaseImage(const QImage& image)
{
    m_lutBaseImage = image;
    makeCurrent();
    if (!image.isNull()) {
        QImage converted = image.convertToFormat(QImage::Format_RGB32);
        m_lutBaseTexture = std::make_unique<QOpenGLTexture>(converted.flipped(Qt::Vertical));
        m_lutBaseTexture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
        m_lutBaseTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
    } else {
        m_lutBaseTexture.reset();
    }
    doneCurrent();
    update();
}

void ImageViewWidget::clearLUTBaseImage()
{
    m_lutBaseImage = QImage();
    makeCurrent();
    m_lutBaseTexture.reset();
    doneCurrent();
    update();
}

bool ImageViewWidget::hasImage() const
{
    return !m_currentImage.isNull();
}

QRect ImageViewWidget::calculateImageRect()
{
    try {
        if (m_currentImage.isNull()) {
            return rect();
        }

        // Calculate the rectangle to display the image centered and scaled
        qDebug() << "calculateImageRect: Widget size:" << size() << "Image size:" << m_currentImage.size();
        QSize widgetSize = size();
        QSize imageSize = m_currentImage.size();

        // Scale image to fit widget while maintaining aspect ratio
        QSize scaledSize = imageSize.scaled(widgetSize, Qt::KeepAspectRatio);

        // Center the scaled image
        int x = (widgetSize.width() - scaledSize.width()) / 2;
        int y = (widgetSize.height() - scaledSize.height()) / 2;

        return QRect(x, y, scaledSize.width(), scaledSize.height());
    } catch (const std::exception& e) {
        qCritical() << "calculateImageRect exception:" << e.what();
        return rect();
    } catch (...) {
        qCritical() << "calculateImageRect unknown exception";
        return rect();
    }
}

QImage ImageViewWidget::getCurrentImage() const
{
    qDebug() << "getCurrentImage: Returning m_currentImage (size:" << m_currentImage.size() << ")";
    return m_currentImage;
}

QImage ImageViewWidget::getProcessedImage() const
{
    qDebug() << "getProcessedImage called - applying processing";
    
    // Ensure m_currentImage is valid before processing
    if (m_currentImage.isNull()) {
        qWarning() << "getProcessedImage: m_currentImage is null, returning empty QImage.";
        return QImage();
    }
    try {
        // Always return original if processing fails
        QImage result = m_currentImage;
        
        // If no processing is applied, return original
        if (m_processingParams.getExposure() == 0.0 && 
            m_processingParams.getContrast() == 1.0 && 
            m_processingParams.getGamma() == 1.0 && 
            !m_processingParams.grayscale &&
            m_processingParams.getTemperature() == 0.0 &&
            m_processingParams.getTint() == 0.0 &&
            m_processingParams.getSaturation() == 0.0 &&
            m_processingParams.getVibrance() == 0.0 &&
            m_processingParams.getHighlights() == 0.0 &&
            m_processingParams.getShadows() == 0.0) {
            qDebug() << "No processing needed, returning original";
            return result;
        }
        
        qDebug() << "Applying processing - exposure:" << m_processingParams.getExposure() 
                 << "contrast:" << m_processingParams.getContrast() 
                 << "gamma:" << m_processingParams.getGamma() 
                 << "grayscale:" << m_processingParams.grayscale
                 << "temperature:" << m_processingParams.getTemperature()
                 << "tint:" << m_processingParams.getTint()
                 << "saturation:" << m_processingParams.getSaturation()
                 << "vibrance:" << m_processingParams.getVibrance()
                 << "highlights:" << m_processingParams.getHighlights()
                 << "shadows:" << m_processingParams.getShadows();
        
        // Apply processing using ImageProcessor
        QImage processed = Core::ImageProcessor().processImage(m_currentImage, m_processingParams);
        
        return processed;
    } catch (const std::exception& e) {
        qCritical() << "getProcessedImage failed:" << e.what();
        return m_currentImage; // Return original on error
    } catch (...) {
        qCritical() << "getProcessedImage failed with unknown error";
        return m_currentImage; // Return original on error
    }
}

void ImageViewWidget::setProcessingParams(const Core::ProcessingParams& params)
{
    qDebug() << "setProcessingParams called - exposure:" << params.getExposure() 
             << "contrast:" << params.getContrast() 
             << "gamma:" << params.getGamma() 
             << "grayscale:" << params.grayscale;
    m_processingParams = params;
    m_needsUpdate = true;
    
    // Trigger delayed update for smooth interaction
    m_processingTimer->start();
}

Core::ProcessingParams ImageViewWidget::getProcessingParams() const
{
    return m_processingParams;
}

void ImageViewWidget::resetView()
{
    m_viewParams.zoom = 1.0f;
    m_viewParams.offset = QPointF();
    update();
}

void ImageViewWidget::fitToWindow()
{
    if (!hasImage()) {
        return;
    }
    
    QSizeF imageSize = m_currentImage.size();
    QSizeF widgetSize = size();
    
    float scaleX = widgetSize.width() / imageSize.width();
    float scaleY = widgetSize.height() / imageSize.height();
    float scale = qMin(scaleX, scaleY);
    
    m_viewParams.zoom = scale;
    m_viewParams.offset = QPointF();
    
    update();
}

void ImageViewWidget::zoomIn()
{
    setZoom(m_viewParams.zoom * Constants::ZOOM_FACTOR);
}

void ImageViewWidget::zoomOut()
{
    setZoom(m_viewParams.zoom / Constants::ZOOM_FACTOR);
}

void ImageViewWidget::setZoom(float zoom)
{
    zoom = qBound(Constants::MIN_ZOOM, zoom, Constants::MAX_ZOOM);
    if (!qFuzzyCompare(m_viewParams.zoom, zoom)) {
        m_viewParams.zoom = zoom;
        update();
    }
}

float ImageViewWidget::getZoom() const
{
    return m_viewParams.zoom;
}

void ImageViewWidget::setFaceRects(const std::vector<Core::DetectedFace>& faces)
{
    m_faceRects = faces;
    update();
}

void ImageViewWidget::clearFaceRects()
{
    m_faceRects.clear();
    update();
}

void ImageViewWidget::setShowFaceRects(bool show)
{
    m_showFaceRects = show;
    update();
}

void ImageViewWidget::setShowOriginal(bool show)
{
    m_viewParams.showOriginal = show;
    m_needsUpdate = true;
    update();
}

void ImageViewWidget::setShowPixelInfo(bool show)
{
    m_viewParams.showPixelInfo = show;
    update();
}

void ImageViewWidget::setShowHistogram(bool show)
{
    if (m_viewParams.showHistogram != show) {
        m_viewParams.showHistogram = show;
        m_needsUpdate = true;
        update();
    }
}

QImage ImageViewWidget::grabFramebufferImage()
{
    return grabFramebuffer();
}

void ImageViewWidget::saveCurrentView(const QString& filePath)
{
    QImage screenshot = grabFramebuffer();
    screenshot.save(filePath);
}

void ImageViewWidget::initializeGL()
{
    initializeOpenGLFunctions();
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    setupShaders();
    setupGeometry();
}

void ImageViewWidget::paintGL()
{
    qDebug() << "paintGL: Entry. hasImage:" << hasImage() << "shaderProgram:" << (m_shaderProgram != nullptr) << "texture:" << (m_imageTexture != nullptr);
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (!hasImage()) {
        // Draw placeholder text
        QPainter painter(this);
        painter.setPen(QPen(Qt::gray));
        painter.setFont(QFont("Arial", 14));
        painter.drawText(rect(), Qt::AlignCenter, 
            "No image loaded\n\nDrop an image here or use File > Open");
        return;
    }
    
    // ── Use QPainter with checkerboard for images with alpha ─────────
    if (m_currentImage.hasAlphaChannel()) {
        qDebug() << "paintGL: image has alpha — using QPainter with checkerboard";
        QPainter painter(this);
        // Draw checkerboard background
        static QPixmap s_checkerboard;
        if (s_checkerboard.isNull()) {
            s_checkerboard = QPixmap(16, 16);
            QPainter cp(&s_checkerboard);
            cp.fillRect(0, 0, 8, 8, QColor(180, 180, 180));
            cp.fillRect(8, 0, 8, 8, QColor(220, 220, 220));
            cp.fillRect(0, 8, 8, 8, QColor(220, 220, 220));
            cp.fillRect(8, 8, 8, 8, QColor(180, 180, 180));
        }
        painter.drawTiledPixmap(rect(), s_checkerboard);
        // Draw processed image on top
        QImage processedImage = getProcessedImage();
        painter.drawImage(calculateImageRect(), processedImage);
        return;
    }
    
    // Try simple QPainter rendering first as fallback
    if (!m_shaderProgram || !m_shaderProgram->isLinked() || !m_imageTexture || !m_imageTexture->isCreated()) {
        qDebug() << "paintGL: Falling back to QPainter. Shader linked:" << (m_shaderProgram ? m_shaderProgram->isLinked() : false) << "Texture created:" << (m_imageTexture ? m_imageTexture->isCreated() : false);
        try {
            QPainter painter(this);
            
            qDebug() << "paintGL: QPainter fallback - getting processed image.";
            // Use processed image instead of original
            QImage processedImage = getProcessedImage();
            painter.drawImage(rect(), processedImage);
            
            // Draw face rectangles
            if (!m_faceRects.empty()) {
                qDebug() << "Drawing" << m_faceRects.size() << "face rectangles with QPainter";
                painter.setPen(QPen(QColor(0, 255, 0), 2)); // Green rectangles
                painter.setBrush(QBrush(QColor(0, 255, 0, 50))); // Semi-transparent fill
                
                // Calculate scaling to fit image in widget
                QRect imageRect = calculateImageRect();
                for (const auto& face : m_faceRects) {
                    const QRect& faceRect = face.rect;
                    
                    // Scale face rectangle to fit the image
                    double scaleX = (double)imageRect.width() / processedImage.width();
                    double scaleY = (double)imageRect.height() / processedImage.height();
                    
                    QRect scaledRect(
                        imageRect.left() + faceRect.x() * scaleX,
                        imageRect.top() + faceRect.y() * scaleY,
                        faceRect.width() * scaleX,
                        faceRect.height() * scaleY
                    );
                    
                    painter.drawRect(scaledRect);
                    
                    // Add text label
                    painter.setPen(Qt::white);
                    QString label = (face.type == Core::FaceType::Frontal) ? "Frontal" : "Profile";
                    
                    if (face.confidence > 0.0) {
                        label += QString(" %1%").arg(qRound(face.confidence * 100));
                    }

                    painter.drawText(scaledRect.topLeft() + QPoint(0, -5), label);
                }
            }
            
            qDebug() << "Using QPainter fallback for image rendering";
            return;
        } catch (const std::exception& e) {
            qCritical() << "QPainter fallback failed:" << e.what();
            return;
        } catch (...) {
            qCritical() << "QPainter fallback failed with unknown error";
            return;
        }
    }
    
    qDebug() << "Calling renderImage()";
    renderImage();
    
    if (m_showFaceRects) {
        renderFaceRects();
    }
    
    if (m_viewParams.showHistogram) {
        renderHistogram();
    }

    if (m_viewParams.showPixelInfo) {
        renderPixelInfo();
    }
}

void ImageViewWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void ImageViewWidget::wheelEvent(QWheelEvent* event)
{
    if (!hasImage()) {
        return;
    }
    
    QPoint delta = event->angleDelta();
    float factor = delta.y() > 0 ? Constants::ZOOM_FACTOR : 1.0f / Constants::ZOOM_FACTOR;
    zoomAtPoint(event->position().toPoint(), factor);
    
    event->accept();
}

void ImageViewWidget::mousePressEvent(QMouseEvent* event)
{
    if (!hasImage()) {
        return;
    }
    
    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        // Check if clicking on a face rect
        if (m_showFaceRects && isPointInFaceRect(event->position().toPoint())) {
            event->accept();
            return;
        }
        
        // Start panning
        m_isPanning = true;
        m_panButton = event->button();
        m_lastPanPosition = event->position().toPoint();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
}

void ImageViewWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!hasImage()) {
        return;
    }
    
    QPoint currentPos = event->position().toPoint();
    m_viewParams.mousePosition = currentPos;
    
    if (m_isPanning) {
        updatePan(currentPos);
    } else {
        // Update cursor based on hover state
        if (m_showFaceRects && isPointInFaceRect(currentPos)) {
            setCursor(Qt::PointingHandCursor);
        } else {
            setCursor(Qt::CrossCursor);
        }
        
        updatePixelInfo();
    }
    
    event->accept();
}

void ImageViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_isPanning && event->button() == m_panButton) {
        endPan();
        event->accept();
    } else if (!m_isPanning && event->button() == Qt::LeftButton) {
        event->accept();
    }
}

void ImageViewWidget::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
        case Qt::Key_Plus:
        case Qt::Key_Equal:
            zoomIn();
            event->accept();
            break;
            
        case Qt::Key_Minus:
            zoomOut();
            event->accept();
            break;
            
        case Qt::Key_0:
            resetView();
            event->accept();
            break;
            
        case Qt::Key_F:
            fitToWindow();
            event->accept();
            break;
            
        case Qt::Key_Space:
            m_viewParams.showOriginal = !m_viewParams.showOriginal;
            m_needsUpdate = true;
            update();
            event->accept();
            break;
            
        default:
            QOpenGLWidget::keyPressEvent(event);
            break;
    }
}

void ImageViewWidget::setupShaders()
{
    qDebug() << "Setting up shaders...";
    m_shaderProgram = std::make_unique<QOpenGLShaderProgram>();
    
    // Load and compile shaders
    if (!m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, 
        R"(#version 330 core
        layout (location = 0) in vec2 aPosition;
        layout (location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        uniform vec2 uResolution;
        uniform float uZoom;
        uniform vec2 uOffset;
        void main() {
            TexCoord = aTexCoord;
            vec2 position = aPosition * uZoom + uOffset;
            vec2 ndc = (position / uResolution) * 2.0 - 1.0;
            ndc.y *= -1.0;
            gl_Position = vec4(ndc, 0.0, 1.0);
        })")) {
        qCritical() << "Failed to compile vertex shader:" << m_shaderProgram->log();
        return;
    }
    
    if (!m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,
        R"(#version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D uTexture;
        uniform float uExposure;
        uniform float uContrast;
        uniform float uGamma;
        uniform bool uGrayscale;
        uniform float uTemperature; // Add temperature uniform
        uniform bool uShowOriginal;
        void main() {
            vec4 color = texture(uTexture, TexCoord);
            if (!uShowOriginal) {
                vec3 rgb = color.rgb;
                rgb = rgb * pow(2.0, uExposure);
                rgb = (rgb - 0.5) * uContrast + 0.5;
                rgb = pow(rgb, vec3(1.0 / uGamma));

                // Apply temperature adjustment (simplified for shader)
                // Blue to yellow/orange. Positive tempStrength adds yellow, reduces blue.
                float tempStrength = uTemperature / 100.0;
                rgb.b -= tempStrength * 0.3;
                rgb.r += tempStrength * 0.1;
                rgb.g += tempStrength * 0.05;
                if (uGrayscale) {
                    rgb = vec3(dot(rgb, vec3(0.299, 0.587, 0.114)));
                }
                color = vec4(rgb, color.a);
            }
            FragColor = color;
        })")) {
        qCritical() << "Failed to compile fragment shader:" << m_shaderProgram->log();
        return;
    }
    
    if (!m_shaderProgram->link()) {
        qCritical() << "Failed to link shader program:" << m_shaderProgram->log();
        return;
    }
    
    qDebug() << "Shaders compiled and linked successfully";
    
    // Get uniform locations
    uResolutionLocation = m_shaderProgram->uniformLocation("uResolution");
    uZoomLocation = m_shaderProgram->uniformLocation("uZoom");
    uOffsetLocation = m_shaderProgram->uniformLocation("uOffset");
    uTextureLocation = m_shaderProgram->uniformLocation("uTexture");
    uExposureLocation = m_shaderProgram->uniformLocation("uExposure");
    uContrastLocation = m_shaderProgram->uniformLocation("uContrast");
    uGammaLocation = m_shaderProgram->uniformLocation("uGamma");
    uGrayscaleLocation = m_shaderProgram->uniformLocation("uGrayscale");
    uTemperatureLocation = m_shaderProgram->uniformLocation("uTemperature"); // Get uniform location
    uShowOriginalLocation = m_shaderProgram->uniformLocation("uShowOriginal");
}

void ImageViewWidget::setupGeometry()
{
    // Setup vertex data for a full-screen quad
    float vertices[] = {
        // Position  // TexCoord
        0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 0.0f
    };
    
    unsigned int indices[] = {
        0, 1, 2,
        1, 3, 2
    };
    
    m_vertexBuffer.create();
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(vertices, sizeof(vertices));
    
    m_indexBuffer.create();
    m_indexBuffer.bind();
    m_indexBuffer.allocate(indices, sizeof(indices));
}

void ImageViewWidget::renderImage()
{
    try {
        if (!m_shaderProgram || !m_imageTexture) {
            qDebug() << "renderImage: missing shader program or texture";
            return;
        }
        
        qDebug() << "renderImage: shaderProgram:" << (m_shaderProgram != nullptr) 
                 << "texture:" << (m_imageTexture != nullptr) 
                 << "textureId:" << (m_imageTexture ? m_imageTexture->textureId() : 0)
                 << "image size:" << m_currentImage.size();
        
        if (!m_shaderProgram->bind()) {
            qDebug() << "renderImage: failed to bind shader program";
            return;
        }
        
        // Set uniforms
        qDebug() << "Setting shader uniforms - exposure:" << m_processingParams.getExposure() 
                 << "contrast:" << m_processingParams.getContrast() 
                 << "gamma:" << m_processingParams.getGamma() 
                 << "grayscale:" << m_processingParams.grayscale;
        qDebug() << "Setting shader uniforms - temperature:" << m_processingParams.getTemperature();
        
        if (uResolutionLocation >= 0)
            m_shaderProgram->setUniformValue(uResolutionLocation, QVector2D(width(), height()));
        if (uZoomLocation >= 0)
            m_shaderProgram->setUniformValue(uZoomLocation, m_viewParams.zoom);
        if (uOffsetLocation >= 0)
            m_shaderProgram->setUniformValue(uOffsetLocation, QVector2D(m_viewParams.offset));
        if (uExposureLocation >= 0)
            m_shaderProgram->setUniformValue(uExposureLocation, m_processingParams.getExposure());
        if (uContrastLocation >= 0)
            m_shaderProgram->setUniformValue(uContrastLocation, m_processingParams.getContrast());
        if (uGammaLocation >= 0)
            m_shaderProgram->setUniformValue(uGammaLocation, m_processingParams.getGamma());
        if (uGrayscaleLocation >= 0)
            m_shaderProgram->setUniformValue(uGrayscaleLocation, m_processingParams.grayscale);
        if (uTemperatureLocation >= 0) // Set temperature uniform
            m_shaderProgram->setUniformValue(uTemperatureLocation, m_processingParams.getTemperature());
        if (uShowOriginalLocation >= 0)
            m_shaderProgram->setUniformValue(uShowOriginalLocation, m_viewParams.showOriginal);
        
        // Bind texture (use LUT base if available and not showing original)
        glActiveTexture(GL_TEXTURE0);
        if (m_lutBaseTexture && !m_viewParams.showOriginal) {
            m_lutBaseTexture->bind();
        } else {
            m_imageTexture->bind();
        }
        if (uTextureLocation >= 0)
            m_shaderProgram->setUniformValue(uTextureLocation, 0);
        
        // Setup vertex attributes
        m_vertexBuffer.bind();
        m_indexBuffer.bind();
        
        m_shaderProgram->enableAttributeArray(0);
        m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2, 4 * sizeof(float));
        
        m_shaderProgram->enableAttributeArray(1);
        m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(float), 2, 4 * sizeof(float));
        
        // Draw
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        
        m_shaderProgram->disableAttributeArray(0);
        m_shaderProgram->disableAttributeArray(1);
        
        m_shaderProgram->release();
    } catch (const std::exception& e) {
        qCritical() << "Exception in renderImage:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception in renderImage";
    }
}

void ImageViewWidget::renderFaceRects()
{
    QPainter painter(this);
    
    // Calculate image rectangle for proper face scaling
    QRect imageRect = calculateImageRect();
    
    FaceRenderingUtils::drawFaceRects(
        painter, 
        m_faceRects, 
        imageRect, 
        m_currentImage.size(), 
        m_hoveredFaceIndex, 
        m_viewParams.zoom
    );
}

void ImageViewWidget::renderPixelInfo()
{
    if (m_currentImage.isNull()) return;

    QPointF imagePos = screenToImage(m_viewParams.mousePosition);
    if (!m_currentImage.rect().contains(imagePos.toPoint())) return;

    Core::ImageProcessor processor;
    Core::PixelInfo info = processor.getPixelInfo(m_currentImage, imagePos.toPoint());

    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing);
    
    QString text = QString("X: %1 Y: %2 | R: %3 G: %4 B: %5")
        .arg(qRound(imagePos.x())).arg(qRound(imagePos.y()))
        .arg(info.r).arg(info.g).arg(info.b);

    QFont font("Consolas", 10);
    painter.setFont(font);
    
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(text).adjusted(-5, -2, 5, 2);
    textRect.moveTo(10, height() - textRect.height() - 10);

    painter.setBrush(QColor(0, 0, 0, 180));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(textRect, 3, 3);

    painter.setPen(Qt::white);
    painter.drawText(textRect, Qt::AlignCenter, text);
}

void ImageViewWidget::renderHistogram()
{
    if (m_currentImage.isNull()) return;

    Core::ImageProcessor processor;
    std::vector<int> hist = processor.calculateHistogram(m_currentImage, -1); // Luminance
    if (hist.empty()) return;

    int maxVal = *std::max_element(hist.begin(), hist.end());
    if (maxVal == 0) return;

    QPainter painter(this);
    int hWidth = 256;
    int hHeight = 100;
    QRect hRect(width() - hWidth - 20, 20, hWidth, hHeight);

    // Background
    painter.setBrush(QColor(0, 0, 0, 150));
    painter.setPen(QPen(Qt::gray, 1));
    painter.drawRect(hRect.adjusted(-2, -2, 2, 2));

    // Grid
    painter.setPen(QPen(QColor(255, 255, 255, 50), 1, Qt::DashLine));
    painter.drawLine(hRect.left(), hRect.center().y(), hRect.right(), hRect.center().y());
    painter.drawLine(hRect.center().x(), hRect.top(), hRect.center().x(), hRect.bottom());

    // Histogram path
    QPainterPath path;
    path.moveTo(hRect.bottomLeft());
    for (int i = 0; i < 256; ++i) {
        float val = static_cast<float>(hist[i]) / maxVal;
        int x = hRect.left() + i;
        int y = hRect.bottom() - (val * hHeight);
        path.lineTo(x, y);
    }
    path.lineTo(hRect.bottomRight());

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::white, 1));
    painter.setBrush(QColor(255, 255, 255, 80));
    painter.drawPath(path);
}

QPointF ImageViewWidget::screenToImage(const QPoint& screenPos) const
{
    if (!hasImage()) {
        return QPointF();
    }
    
    QPointF imagePos = (QPointF(screenPos) - m_viewParams.offset) / m_viewParams.zoom;
    return imagePos;
}

QPoint ImageViewWidget::imageToScreen(const QPointF& imagePos) const
{
    QPointF screenPos = imagePos * m_viewParams.zoom + m_viewParams.offset;
    return screenPos.toPoint();
}

QRectF ImageViewWidget::getVisibleImageRect() const
{
    if (!hasImage()) {
        return QRectF();
    }
    
    QPointF topLeft = screenToImage(QPoint(0, 0));
    QPointF bottomRight = screenToImage(QPoint(width(), height()));
    
    return QRectF(topLeft, bottomRight).intersected(m_currentImage.rect());
}

void ImageViewWidget::startPan(const QPoint& position)
{
    m_isPanning = true;
    m_lastPanPosition = position;
    setCursor(Qt::ClosedHandCursor);
}

void ImageViewWidget::updatePan(const QPoint& position)
{
    if (!m_isPanning) {
        return;
    }
    
    QPoint delta = position - m_lastPanPosition;
    m_viewParams.offset += delta;
    m_lastPanPosition = position;
    update();
}

void ImageViewWidget::endPan()
{
    m_isPanning = false;
    setCursor(Qt::CrossCursor);
}

void ImageViewWidget::zoomAtPoint(const QPoint& center, float factor)
{
    QPointF imagePos = screenToImage(center);
    setZoom(m_viewParams.zoom * factor);
    QPointF newScreenPos = imagePos * m_viewParams.zoom + m_viewParams.offset;
    m_viewParams.offset += QPointF(center) - newScreenPos;
    update();
}

bool ImageViewWidget::isPointInFaceRect(const QPoint& point) const
{
    return getFaceRectAtPoint(point).isValid();
}

QRect ImageViewWidget::getFaceRectAtPoint(const QPoint& point) const
{
    for (const auto& face : m_faceRects) {
        QRect screenRect(imageToScreen(face.rect.topLeft()), 
                        QSize(face.rect.width() * m_viewParams.zoom, 
                             face.rect.height() * m_viewParams.zoom));
        if (screenRect.contains(point)) {
            return face.rect;
        }
    }
    return QRect();
}

void ImageViewWidget::updatePixelInfo()
{
    if (!hasImage()) {
        return;
    }
    
    QPointF imagePos = screenToImage(m_viewParams.mousePosition);
    if (m_currentImage.rect().contains(imagePos.toPoint())) {
        Core::ImageProcessor processor;
    }
}

void ImageViewWidget::onProcessingTimerTimeout()
{
    if (m_needsUpdate) {
        m_needsUpdate = false;
        update();
    }
}

} // namespace Glance::UI
