/**
 * @file UI.FaceRenderingUtils.cppm
 * @brief Utility functions for rendering face detection overlays
 *
 * Provides static methods for drawing face rectangles on QPainter
 * with hover highlighting, scaling face rects from image coordinates
 * to widget coordinates, and getting face label text by FaceType.
 */
module;

#include <vector>

export module UI.FaceRenderingUtils;

import Core.Constants;
import Qt.Wrapper;
export import Core.FaceDetector;

export
{
    using ::QColor;
    using ::QPen;
    using ::QBrush;
    using ::QPainter;
    using ::QRect;
    using ::QSize;
    using ::QString;
}

export namespace Glance {
namespace UI {

/** @brief Static helpers for drawing face detection overlays */
class FaceRenderingUtils {
public:
    static const QColor& getFaceRectColor();
    static const QColor& getFaceRectHoverColor();
    static const QColor& getFaceLabelColor();

    static QPen getFaceRectPen(bool hovered = false);
    static QBrush getFaceRectBrush();

    static void drawFaceRects(QPainter& painter,
                              const std::vector<Core::DetectedFace>& faces,
                              const QRect& imageRect,
                              const QSize& imageSize,
                              int hoveredIndex = -1,
                              double scale = 1.0);

    static QRect scaleFaceRect(const QRect& faceRect,
                               const QRect& imageRect,
                               const QSize& imageSize);

    static QString getFaceLabelText(Core::FaceType type);

private:
    FaceRenderingUtils() = delete;
};

}
}
