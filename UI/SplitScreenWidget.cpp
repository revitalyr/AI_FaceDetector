module;
#include <QPainter>
#include <QDebug>
#include <QStyle>
#include <QApplication>
module UI.SplitScreenWidget;
import Core.Constants;
import UI.FaceRenderingUtils;
import Core.FaceDetector;

namespace Glance::UI {

SplitScreenWidget::SplitScreenWidget(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumSize(200, 200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void SplitScreenWidget::setOriginalImage(const QImage& image)
{
    m_originalImage = image;
    update();
}

void SplitScreenWidget::setProcessedImage(const QImage& image)
{
    qDebug() << "setProcessedImage called - image size:" << image.size() 
             << "null:" << image.isNull() << "format:" << image.format();
    m_processedImage = image;
    update();
}

void SplitScreenWidget::clearImages()
{
    m_originalImage = QImage();
    m_processedImage = QImage();
    m_faceRects.clear();
    update();
}

void SplitScreenWidget::setFaceRects(const std::vector<Core::DetectedFace>& faceRects)
{
    m_faceRects = faceRects;
    qDebug() << "SplitScreenWidget::setFaceRects called with" << faceRects.size() << "faces";
    if (!faceRects.empty()) {
        qDebug() << "First rectangle:" << faceRects[0].rect;
    }
    this->update();
}

void SplitScreenWidget::setSplitterPosition(double position)
{
    position = qBound(0.0, position, 1.0);
    if (!qFuzzyCompare(m_splitterPosition, position)) {
        m_splitterPosition = position;
        update();
    }
}

double SplitScreenWidget::splitterPosition() const
{
    return m_splitterPosition;
}

void SplitScreenWidget::setSplitterMode(bool horizontal)
{
    if (m_horizontalSplit != horizontal) {
        m_horizontalSplit = horizontal;
        update();
    }
}

void SplitScreenWidget::setSplitterColor(const QColor& color)
{
    m_splitterColor = color;
    update();
}

void SplitScreenWidget::setSplitterWidth(int width)
{
    m_splitterWidth = qMax(2, width);
    update();
}

void SplitScreenWidget::setShowLabels(bool show)
{
    if (m_showLabels != show) {
        m_showLabels = show;
        update();
    }
}

void SplitScreenWidget::setLabelColor(const QColor& color)
{
    m_labelColor = color;
    update();
}

void SplitScreenWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Fill background
    painter.fillRect(rect(), QColor(30, 30, 30));
    
    if (m_originalImage.isNull() && m_processedImage.isNull()) {
        painter.setPen(QPen(QColor(100, 100, 100), 1));
        painter.drawText(rect(), Qt::AlignCenter, "No images loaded");
        return;
    }
    
    drawImages(painter);
    drawSplitter(painter);
    
    if (m_showLabels) {
        drawLabels(painter);
    }
}

void SplitScreenWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (isNearSplitter(event->pos())) {
            m_isDragging = true;
            if (m_horizontalSplit) {
                m_dragOffset = event->pos().x() - qRound(m_splitterPosition * width());
            } else {
                m_dragOffset = event->pos().y() - qRound(m_splitterPosition * height());
            }
            setCursor(Qt::SplitHCursor);
        } else {
            // Check which side was clicked
            QRect originalRect = getOriginalRect();
            QRect processedRect = getProcessedImageRect();
            
            if (originalRect.contains(event->pos())) {
                QPoint imagePos = mapToOriginal(event->pos());
            } else if (processedRect.contains(event->pos())) {
                QPoint imagePos = mapToProcessed(event->pos());
                
                QRect faceRect = getFaceRectAtPoint(event->pos());
                if (!faceRect.isNull()) {
                }
            }
        }
    }
}

void SplitScreenWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isDragging && event->buttons() & Qt::LeftButton) {
        double newPosition;
        if (m_horizontalSplit) {
            int newX = event->pos().x() - m_dragOffset;
            newPosition = static_cast<double>(newX) / width();
        } else {
            int newY = event->pos().y() - m_dragOffset;
            newPosition = static_cast<double>(newY) / height();
        }
        
        setSplitterPosition(newPosition);
    } else {
        updateCursor();
        
        // Update hovered face index for visual feedback
        int oldIndex = m_hoveredFaceIndex;
        m_hoveredFaceIndex = -1;
        
        if (getProcessedImageRect().contains(event->pos())) {
            QPoint imagePos = mapToProcessed(event->pos());
            if (imagePos.x() >= 0) {
                for (int i = 0; i < static_cast<int>(m_faceRects.size()); ++i) {
                    if (m_faceRects[i].rect.contains(imagePos)) {
                        m_hoveredFaceIndex = i;
                        break;
                    }
                }
            }
        }
        
        if (oldIndex != m_hoveredFaceIndex) {
            update();
        }
    }
}

void SplitScreenWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_isDragging) {
        m_isDragging = false;
        updateCursor();
    }
}

void SplitScreenWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)
    update();
}

void SplitScreenWidget::wheelEvent(QWheelEvent* event)
{
    // Allow zooming with Ctrl+Wheel
    if (event->modifiers() & Qt::ControlModifier) {
        double delta = event->angleDelta().y() / 1200.0; // 0.1 per step
        setSplitterPosition(m_splitterPosition + delta);
        event->accept();
    } else {
        QWidget::wheelEvent(event);
    }
}

void SplitScreenWidget::drawImages(QPainter& painter)
{
    QRect originalRect = getOriginalRect();
    QRect processedRect = getProcessedImageRect();
    
    // Draw original image
    if (!m_originalImage.isNull()) {
        QImage scaledOriginal = m_originalImage.scaled(originalRect.size(), 
                                                       Qt::KeepAspectRatio, 
                                                       Qt::SmoothTransformation);
        
        // Center the image in its rect
        QRect imageRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, 
                                             scaledOriginal.size(), originalRect);
        painter.drawImage(imageRect, scaledOriginal);
    }
    
    // Draw processed image
    if (!m_processedImage.isNull()) {
        QImage scaledProcessed = m_processedImage.scaled(processedRect.size(), 
                                                        Qt::KeepAspectRatio, 
                                                        Qt::SmoothTransformation);
        
        // Center the image in its rect
        QRect imageRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, 
                                             scaledProcessed.size(), processedRect);
        
        // Show checkerboard behind images with transparency
        if (m_processedImage.hasAlphaChannel()) {
            static QPixmap s_checkerboard;
            if (s_checkerboard.isNull()) {
                s_checkerboard = QPixmap(16, 16);
                QPainter cp(&s_checkerboard);
                cp.fillRect(0, 0, 8, 8, QColor(180, 180, 180));
                cp.fillRect(8, 0, 8, 8, QColor(220, 220, 220));
                cp.fillRect(0, 8, 8, 8, QColor(220, 220, 220));
                cp.fillRect(8, 8, 8, 8, QColor(180, 180, 180));
            }
            painter.drawTiledPixmap(imageRect, s_checkerboard);
        }
        
        painter.drawImage(imageRect, scaledProcessed);
        
        // Draw face rectangles on processed image using utility class
        FaceRenderingUtils::drawFaceRects(
            painter,
            m_faceRects,
            imageRect,
            m_processedImage.size(),
            m_hoveredFaceIndex
        );
    }
}

void SplitScreenWidget::drawSplitter(QPainter& painter)
{
    QRect splitterRect = getSplitterRect();
    
    // Draw splitter handle
    QLinearGradient gradient;
    if (m_horizontalSplit) {
        gradient = QLinearGradient(splitterRect.left(), 0, splitterRect.right(), 0);
    } else {
        gradient = QLinearGradient(0, splitterRect.top(), 0, splitterRect.bottom());
    }
    
    gradient.setColorAt(0.0, m_splitterColor.darker(150));
    gradient.setColorAt(0.5, m_splitterColor);
    gradient.setColorAt(1.0, m_splitterColor.darker(150));
    
    painter.fillRect(splitterRect, gradient);
    
    // Draw handle grip lines
    painter.setPen(QPen(m_splitterColor.lighter(150), 1));
    
    if (m_horizontalSplit) {
        int centerY = splitterRect.center().y();
        for (int y = centerY - 4; y <= centerY + 4; y += 4) {
            painter.drawLine(splitterRect.left() + 2, y, splitterRect.right() - 2, y);
        }
    } else {
        int centerX = splitterRect.center().x();
        for (int x = centerX - 4; x <= centerX + 4; x += 4) {
            painter.drawLine(x, splitterRect.top() + 2, x, splitterRect.bottom() - 2);
        }
    }
}

void SplitScreenWidget::drawLabels(QPainter& painter)
{
    painter.setPen(QPen(m_labelColor, 1));
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    
    QFontMetrics fontMetrics(painter.font());
    
    // Draw "Original" label
    QRect originalRect = getOriginalRect();
    QString originalText = "Original";
    QRect originalTextRect = fontMetrics.boundingRect(originalText);
    originalTextRect.moveTopLeft(originalRect.topLeft() + QPoint(10, 10));
    
    // Draw background for better readability
    painter.fillRect(originalTextRect.adjusted(-2, -2, 2, 2), QColor(0, 0, 0, 128));
    painter.drawText(originalTextRect, Qt::AlignLeft | Qt::AlignTop, originalText);
    
    // Draw "Processed" label
    QRect processedRect = getProcessedImageRect();
    QString processedText = "Processed";
    QRect processedTextRect = fontMetrics.boundingRect(processedText);
    processedTextRect.moveTopLeft(processedRect.topLeft() + QPoint(10, 10));
    
    painter.fillRect(processedTextRect.adjusted(-2, -2, 2, 2), QColor(0, 0, 0, 128));
    painter.drawText(processedTextRect, Qt::AlignLeft | Qt::AlignTop, processedText);
}

QRect SplitScreenWidget::getOriginalRect() const
{
    if (m_horizontalSplit) {
        int splitterX = qRound(m_splitterPosition * width());
        return QRect(0, 0, splitterX, height());
    } else {
        int splitterY = qRound(m_splitterPosition * height());
        return QRect(0, 0, width(), splitterY);
    }
}

QRect SplitScreenWidget::getProcessedImageRect() const
{
    if (m_horizontalSplit) {
        int splitterX = qRound(m_splitterPosition * width());
        return QRect(splitterX + m_splitterWidth, 0, 
                    width() - splitterX - m_splitterWidth, height());
    } else {
        int splitterY = qRound(m_splitterPosition * height());
        return QRect(0, splitterY + m_splitterWidth, 
                    width(), height() - splitterY - m_splitterWidth);
    }
}

QRect SplitScreenWidget::getSplitterRect() const
{
    if (m_horizontalSplit) {
        int splitterX = qRound(m_splitterPosition * width());
        return QRect(splitterX, 0, m_splitterWidth, height());
    } else {
        int splitterY = qRound(m_splitterPosition * height());
        return QRect(0, splitterY, width(), m_splitterWidth);
    }
}

QImage SplitScreenWidget::getOriginalImage() const
{
    return m_originalImage;
}

QImage SplitScreenWidget::getProcessedImage() const
{
    return m_processedImage;
}

bool SplitScreenWidget::isNearSplitter(const QPoint& position) const
{
    QRect splitterRect = getSplitterRect();
    QRect expandedRect = splitterRect.adjusted(-Constants::SPLITTER_HIT_AREA, -Constants::SPLITTER_HIT_AREA,
                                              Constants::SPLITTER_HIT_AREA, Constants::SPLITTER_HIT_AREA);
    return expandedRect.contains(position);
}

void SplitScreenWidget::updateCursor()
{
    if (isNearSplitter(cursor().pos()) || m_isDragging) {
        if (m_horizontalSplit) {
            setCursor(Qt::SplitHCursor);
        } else {
            setCursor(Qt::SplitVCursor);
        }
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

bool SplitScreenWidget::isPointInFaceRect(const QPoint& point) const
{
    return !getFaceRectAtPoint(point).isNull();
}

QRect SplitScreenWidget::getFaceRectAtPoint(const QPoint& point) const
{
    QRect processedRect = getProcessedImageRect();
    if (!processedRect.contains(point)) {
        return QRect();
    }
    
    QPoint imagePos = mapToProcessed(point);
    if (imagePos.x() < 0) return QRect();
    
    for (const auto& face : m_faceRects) {
        if (face.rect.contains(imagePos)) {
            return face.rect;
        }
    }
    return QRect();
}

QPoint SplitScreenWidget::mapToOriginal(const QPoint& position) const
{
    QRect originalRect = getOriginalRect();
    if (!originalRect.contains(position)) {
        return QPoint(-1, -1);
    }
    
    // Map to original image coordinates
    if (!m_originalImage.isNull()) {
        QImage scaledOriginal = m_originalImage.scaled(originalRect.size(), 
                                                       Qt::KeepAspectRatio, 
                                                       Qt::SmoothTransformation);
        QRect imageRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, 
                                             scaledOriginal.size(), originalRect);
        
        if (imageRect.contains(position)) {
            QPoint imagePos = position - imageRect.topLeft();
            double scaleX = static_cast<double>(m_originalImage.width()) / scaledOriginal.width();
            double scaleY = static_cast<double>(m_originalImage.height()) / scaledOriginal.height();
            
            return QPoint(qRound(imagePos.x() * scaleX), qRound(imagePos.y() * scaleY));
        }
    }
    
    return QPoint(-1, -1);
}

QPoint SplitScreenWidget::mapToProcessed(const QPoint& position) const
{
    QRect processedRect = getProcessedImageRect();
    if (!processedRect.contains(position)) {
        return QPoint(-1, -1);
    }
    
    // Map to processed image coordinates
    if (!m_processedImage.isNull()) {
        QImage scaledProcessed = m_processedImage.scaled(processedRect.size(), 
                                                        Qt::KeepAspectRatio, 
                                                        Qt::SmoothTransformation);
        QRect imageRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, 
                                             scaledProcessed.size(), processedRect);
        
        if (imageRect.contains(position)) {
            QPoint imagePos = position - imageRect.topLeft();
            double scaleX = static_cast<double>(m_processedImage.width()) / scaledProcessed.width();
            double scaleY = static_cast<double>(m_processedImage.height()) / scaledProcessed.height();
            
            return QPoint(qRound(imagePos.x() * scaleX), qRound(imagePos.y() * scaleY));
        }
    }
    
    return QPoint(-1, -1);
}

} // namespace Glance::UI
