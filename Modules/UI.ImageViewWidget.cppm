/**
 * @file UI.ImageViewWidget.cppm
 * @brief OpenGL-accelerated image viewer with zoom, pan, and face overlay
 *
 * ImageViewWidget renders images using OpenGL 3.3 Core profile with
 * real-time zoom/pan/fit-to-window, GPU-accelerated processing via
 * shader uniforms, face rectangle overlays, pixel info and histogram
 * overlays, LUT base image preview, and keyboard/mouse interaction.
 */
module;
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QImage>
#include <QRect>
#include <QVector2D>
#include <QPointF>
#include <QSizeF>
#include <QColor>
#include <QFont>
#include <QPainter>
#include <memory>
#include <vector>
#include <algorithm>

export module UI.ImageViewWidget;

import Core.Constants;
import Core.ImageProcessor;
import Core.FaceDetector;
import UI.FaceRenderingUtils;

namespace Glance::UI {

/** @brief Current view state (zoom, offset, overlay toggles) */
export struct ViewParameters {
    float zoom = 1.0f;
    QPointF offset;
    bool showOriginal = false;
    bool showPixelInfo = true;
    bool showHistogram = true;
    QPoint mousePosition;
};

/** @brief OpenGL 3.3 image viewer with shader processing and face overlays */
export class ImageViewWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
public:
    explicit ImageViewWidget(QWidget* parent = nullptr);
    ~ImageViewWidget();

    void setImage(const QImage& image);
    void clearImage();
    bool hasImage() const;
    QImage getCurrentImage() const;
    QImage getProcessedImage() const;
    QRect calculateImageRect();

    void setLUTBaseImage(const QImage& image);
    void clearLUTBaseImage();
    
    void setProcessingParams(const Core::ProcessingParams& params);
    Core::ProcessingParams getProcessingParams() const;
    
    void resetView();
    void fitToWindow();
    void zoomIn();
    void zoomOut();
    void setZoom(float zoom);
    float getZoom() const;
    
    void setFaceRects(const std::vector<Core::DetectedFace>& faces);
    void clearFaceRects();
    void setShowFaceRects(bool show);
    
    void setShowOriginal(bool show);
    void setShowPixelInfo(bool show);
    void setShowHistogram(bool show);
    
    QImage grabFramebufferImage();
    void saveCurrentView(const QString& filePath);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void updatePixelInfo();
    void onProcessingTimerTimeout();

private:
    void setupShaders();
    void setupGeometry();
    void setupTextures();
    
    void renderImage();
    void renderFaceRects();
    void renderPixelInfo();
    void renderHistogram();
    
    QPointF screenToImage(const QPoint& screenPos) const;
    QPoint imageToScreen(const QPointF& imagePos) const;
    QRectF getVisibleImageRect() const;
    
    void startPan(const QPoint& position);
    void updatePan(const QPoint& position);
    void endPan();
    void zoomAtPoint(const QPoint& center, float factor);
    
    bool isPointInFaceRect(const QPoint& point) const;
    QRect getFaceRectAtPoint(const QPoint& point) const;

private:
    std::unique_ptr<QOpenGLShaderProgram> m_shaderProgram;
    std::unique_ptr<QOpenGLTexture> m_imageTexture;
    std::unique_ptr<QOpenGLTexture> m_lutBaseTexture;
    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_indexBuffer;
    
    QImage m_currentImage;
    QImage m_lutBaseImage;
    QSize m_originalImageSize;
    Core::ProcessingParams m_processingParams;
    ViewParameters m_viewParams;
    
    std::vector<Core::DetectedFace> m_faceRects;
    bool m_showFaceRects = true;
    int m_hoveredFaceIndex = -1;
    
    bool m_isPanning = false;
    QPoint m_lastPanPosition;
    Qt::MouseButton m_panButton = Qt::LeftButton;
    
    QTimer* m_processingTimer;
    bool m_needsUpdate = true;
    
    int uResolutionLocation = -1;
    int uZoomLocation = -1;
    int uOffsetLocation = -1;
    int uTextureLocation = -1;
    int uExposureLocation = -1;
    int uContrastLocation = -1;
    int uGammaLocation = -1;
    int uGrayscaleLocation = -1;
    int uTemperatureLocation = -1;
    int uShowOriginalLocation = -1;
    int uMousePosLocation = -1;
    int uShowPixelInfoLocation = -1;
};

} // namespace Glance::UI
