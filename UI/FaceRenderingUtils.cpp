module;
#include <QPainter>
module UI.FaceRenderingUtils;
import Core.Constants;

namespace Glance {
namespace UI {

const QColor& FaceRenderingUtils::getFaceRectColor()
{
    static const QColor color(0, 255, 0); // Green
    return color;
}

const QColor& FaceRenderingUtils::getFaceRectHoverColor()
{
    static const QColor color(255, 255, 0); // Yellow
    return color;
}

const QColor& FaceRenderingUtils::getFaceLabelColor()
{
    static const QColor color(Qt::white);
    return color;
}

QPen FaceRenderingUtils::getFaceRectPen(bool hovered)
{
    QColor color = hovered ? getFaceRectHoverColor() : getFaceRectColor();
    return QPen(color, Constants::FACE_RECT_LINE_WIDTH);
}

QBrush FaceRenderingUtils::getFaceRectBrush()
{
    QColor fillColor = getFaceRectColor();
    fillColor.setAlpha(Constants::FACE_RECT_FILL_ALPHA);
    return QBrush(fillColor);
}

void FaceRenderingUtils::drawFaceRects(QPainter& painter,
                                       const std::vector<Core::DetectedFace>& faces,
                                       const QRect& imageRect,
                                       const QSize& imageSize,
                                       int hoveredIndex,
                                       double scale)
{
    if (faces.empty() || imageSize.isEmpty()) {
        return;
    }
    
    painter.setRenderHint(QPainter::Antialiasing);
    
    for (size_t i = 0; i < faces.size(); ++i) {
        const auto& face = faces[i];
        bool isHovered = (static_cast<int>(i) == hoveredIndex);
        
        // Scale face rectangle to screen coordinates
        QRect scaledRect = scaleFaceRect(face.rect, imageRect, imageSize);
        
        // Apply additional scale factor if needed
        if (scale != 1.0) {
            int newWidth = static_cast<int>(scaledRect.width() * scale);
            int newHeight = static_cast<int>(scaledRect.height() * scale);
            int centerX = scaledRect.center().x();
            int centerY = scaledRect.center().y();
            scaledRect.setRect(centerX - newWidth / 2, centerY - newHeight / 2, newWidth, newHeight);
        }
        
        // Draw rectangle
        painter.setPen(getFaceRectPen(isHovered));
        painter.setBrush(getFaceRectBrush());
        painter.drawRect(scaledRect);

        // Draw Landmarks
        if (!face.landmarks.empty()) {
            painter.setBrush(isHovered ? Qt::cyan : Qt::red);
            painter.setPen(Qt::NoPen);
            
            double scaleX = static_cast<double>(imageRect.width()) / imageSize.width();
            double scaleY = static_cast<double>(imageRect.height()) / imageSize.height();

            for (const auto& pt : face.landmarks) {
                QPointF scaledPt(
                    imageRect.left() + pt.x() * scaleX,
                    imageRect.top() + pt.y() * scaleY
                );
                painter.drawEllipse(scaledPt, 3.0, 3.0);
            }
        }
        
        // Draw label if face has type information
        if (face.type != Core::FaceType::Unknown) {
            painter.setPen(getFaceLabelColor());
            QString label = getFaceLabelText(face.type);
            
            if (face.confidence > 0.0) {
                label += QString(" %1%").arg(qRound(face.confidence * 100));
            }
            
            // Position label above the rectangle
            QPoint labelPos = scaledRect.topLeft() + QPoint(0, -5);
            
            // Use small font for labels
            QFont font = painter.font();
            font.setPointSize(8);
            painter.setFont(font);
            
            // Draw text with background for readability
            QRect textRect = painter.boundingRect(QRect(labelPos, QSize(100, 20)), 
                                                   Qt::AlignLeft | Qt::AlignVCenter, 
                                                   label);
            painter.fillRect(textRect, QColor(0, 0, 0, 180)); // Semi-transparent black background
            painter.drawText(textRect, Qt::AlignCenter, label);
        }
    }
}

QRect FaceRenderingUtils::scaleFaceRect(const QRect& faceRect,
                                        const QRect& imageRect,
                                        const QSize& imageSize)
{
    if (imageSize.isEmpty()) {
        return QRect();
    }
    
    // Calculate scaling factors
    double scaleX = static_cast<double>(imageRect.width()) / imageSize.width();
    double scaleY = static_cast<double>(imageRect.height()) / imageSize.height();
    
    // Scale the face rectangle
    int scaledX = imageRect.left() + static_cast<int>(faceRect.x() * scaleX);
    int scaledY = imageRect.top() + static_cast<int>(faceRect.y() * scaleY);
    int scaledWidth = static_cast<int>(faceRect.width() * scaleX);
    int scaledHeight = static_cast<int>(faceRect.height() * scaleY);
    
    return QRect(scaledX, scaledY, scaledWidth, scaledHeight);
}

QString FaceRenderingUtils::getFaceLabelText(Core::FaceType type)
{
    switch (type) {
        case Core::FaceType::Frontal:
            return QStringLiteral("Frontal");
        case Core::FaceType::Profile:
            return QStringLiteral("Profile");
        case Core::FaceType::Unknown:
        default:
            return QStringLiteral("Face");
    }
}

} // namespace UI
} // namespace Glance
