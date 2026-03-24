/**
 * @file UI.SplitScreenWidget.cppm
 * @brief Split-screen comparison widget for original vs processed images
 *
 * Displays two images side-by-side (or top-bottom) with a draggable
 * splitter divider. Supports face rectangle overlays on both halves,
 * configurable splitter appearance, and labels.
 */
module;
#include <QWidget>
#include <QImage>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QCursor>
#include <QTimer>
#include <QRect>
#include <QColor>
#include <QPainter>
#include <QWheelEvent>
#include <vector>

export module UI.SplitScreenWidget;

import Core.Constants;
import Core.FaceDetector;
import UI.FaceRenderingUtils;

namespace Glance::UI {

/** @brief Side-by-side before/after viewer with draggable splitter */
export class SplitScreenWidget : public QWidget {
public:
    explicit SplitScreenWidget(QWidget* parent = nullptr);
    
    void setOriginalImage(const QImage& image);
    void setProcessedImage(const QImage& image);
    void clearImages();
    
    QImage getOriginalImage() const;
    QImage getProcessedImage() const;
    
    void setFaceRects(const std::vector<Core::DetectedFace>& faceRects);
    
    void setSplitterPosition(double position);
    double splitterPosition() const;
    void setSplitterMode(bool horizontal);
    
    void setSplitterColor(const QColor& color);
    void setSplitterWidth(int width);
    void setShowLabels(bool show);
    void setLabelColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void drawImages(QPainter& painter);
    void drawSplitter(QPainter& painter);
    void drawLabels(QPainter& painter);
    
    QRect getOriginalRect() const;
    QRect getProcessedImageRect() const;
    QRect getSplitterRect() const;
    
    bool isNearSplitter(const QPoint& position) const;
    void updateCursor();
    QPoint mapToOriginal(const QPoint& position) const;
    QPoint mapToProcessed(const QPoint& position) const;

    bool isPointInFaceRect(const QPoint& point) const;
    QRect getFaceRectAtPoint(const QPoint& point) const;

private:
    QImage m_originalImage;
    QImage m_processedImage;
    std::vector<Core::DetectedFace> m_faceRects;
    int m_hoveredFaceIndex = -1;
    
    double m_splitterPosition = 0.5;
    bool m_horizontalSplit = true;
    bool m_isDragging = false;
    int m_dragOffset = 0;
    
    QColor m_splitterColor = QColor(100, 100, 100);
    int m_splitterWidth = 4;
    
    bool m_showLabels = true;
    QColor m_labelColor = QColor(255, 255, 255);
    
};

} // namespace Glance::UI
