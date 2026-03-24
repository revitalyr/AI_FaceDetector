/**
 * @file UI.CustomSlider.cppm
 * @brief Styled double-precision slider with scroll-wheel support
 *
 * Extends QSlider with double-precision value range and stepping,
 * custom track/handle colours and sizing, scroll wheel support
 * (with optional proportional scrolling), and click-to-position
 * value setting.
 */
module;
#include <QSlider>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOptionSlider>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QColor>

export module UI.CustomSlider;

namespace Glance::UI {

/** @brief Styled double-precision slider extending QSlider */
export class CustomSlider : public QSlider {

public:
    explicit CustomSlider(QWidget* parent = nullptr);
    CustomSlider(double min, double max, double step, double value, QWidget* parent = nullptr);
    
    void setDoubleValue(double value);
    double doubleValue() const;
    void setDoubleRange(double min, double max);
    void setDoubleStep(double step);
    
    void setTrackColor(const QColor& color);
    void setHandleColor(const QColor& color);
    void setGrooveHeight(int height);
    void setHandleSize(const QSize& size);
    
    void setScrollWheelEnabled(bool enabled);
    void setProportionalScroll(bool enabled);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void updateValueFromPosition(const QPoint& position);
    QRect getHandleRect() const;
    QRect getGrooveRect() const;
    double positionToValue(const QPoint& position) const;
    int valueToPosition(double value) const;

private:
    double m_doubleMin = 0.0;
    double m_doubleMax = 100.0;
    double m_doubleStep = 1.0;
    double m_doubleValue = 0.0;
    
    QColor m_trackColor = QColor(100, 100, 100);
    QColor m_handleColor = QColor(200, 200, 200);
    QColor m_activeTrackColor = QColor(0, 120, 215);
    
    int m_grooveHeight = 4;
    QSize m_handleSize = QSize(16, 16);
    
    bool m_scrollWheelEnabled = true;
    bool m_proportionalScroll = true;
    bool m_isDragging = false;
};

} // namespace Glance::UI
