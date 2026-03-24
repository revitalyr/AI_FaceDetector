module;
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QPen>
#include <QDebug>
module UI.CustomSlider;

namespace Glance::UI {

CustomSlider::CustomSlider(QWidget* parent)
    : QSlider(Qt::Horizontal, parent)
{
    setRange(0, 1000); // Use high resolution for double precision
    setSingleStep(1);
    setPageStep(10);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_Hover);
}

CustomSlider::CustomSlider(double min, double max, double step, double value, QWidget* parent)
    : CustomSlider(parent)
{
    setDoubleRange(min, max);
    setDoubleStep(step);
    setDoubleValue(value);
}

void CustomSlider::setDoubleValue(double value)
{
    m_doubleValue = qBound(m_doubleMin, value, m_doubleMax);
    
    // Map to integer range
    double normalized = (m_doubleValue - m_doubleMin) / (m_doubleMax - m_doubleMin);
    int intValue = qRound(normalized * (maximum() - minimum()) + minimum());
    
    blockSignals(true);
    setValue(intValue);
    blockSignals(false);
}

double CustomSlider::doubleValue() const
{
    return m_doubleValue;
}

void CustomSlider::setDoubleRange(double min, double max)
{
    if (min >= max) {
        qWarning() << "Invalid range: min must be less than max";
        return;
    }
    
    m_doubleMin = min;
    m_doubleMax = max;
    
    // Adjust current value if needed
    if (m_doubleValue < min || m_doubleValue > max) {
        setDoubleValue(m_doubleValue);
    }
}

void CustomSlider::setDoubleStep(double step)
{
    if (step <= 0) {
        qWarning() << "Step must be positive";
        return;
    }
    
    m_doubleStep = step;
}

void CustomSlider::setTrackColor(const QColor& color)
{
    m_trackColor = color;
    update();
}

void CustomSlider::setHandleColor(const QColor& color)
{
    m_handleColor = color;
    update();
}

void CustomSlider::setGrooveHeight(int height)
{
    m_grooveHeight = qMax(2, height);
    update();
}

void CustomSlider::setHandleSize(const QSize& size)
{
    m_handleSize = size.expandedTo(QSize(8, 8)); // Minimum size
    update();
}

void CustomSlider::setScrollWheelEnabled(bool enabled)
{
    m_scrollWheelEnabled = enabled;
}

void CustomSlider::setProportionalScroll(bool enabled)
{
    m_proportionalScroll = enabled;
}

void CustomSlider::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QRect grooveRect = getGrooveRect();
    QRect handleRect = getHandleRect();
    
    // Draw groove background
    painter.setBrush(m_trackColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(grooveRect, m_grooveHeight / 2.0, m_grooveHeight / 2.0);
    
    // Draw active track (from min to current value)
    double normalized = (m_doubleValue - m_doubleMin) / (m_doubleMax - m_doubleMin);
    int activeWidth = qRound(normalized * grooveRect.width());
    QRect activeRect = grooveRect;
    activeRect.setWidth(activeWidth);
    
    painter.setBrush(m_activeTrackColor);
    painter.drawRoundedRect(activeRect, m_grooveHeight / 2.0, m_grooveHeight / 2.0);
    
    // Draw handle
    painter.setBrush(m_handleColor);
    painter.setPen(QPen(m_handleColor.darker(150), 1));
    painter.drawEllipse(handleRect.center(), m_handleSize.width() / 2, m_handleSize.height() / 2);
    
    // Draw handle highlight when hovered or pressed
    if (underMouse() || isSliderDown()) {
        painter.setBrush(QColor(255, 255, 255, 30));
        painter.drawEllipse(QPointF(handleRect.center()), m_handleSize.width() / 2.0 - 1, m_handleSize.height() / 2.0 - 1);
    }
}

void CustomSlider::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        updateValueFromPosition(event->pos());
        setSliderDown(true);
    }
    
    QSlider::mousePressEvent(event);
}

void CustomSlider::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isDragging && event->buttons() & Qt::LeftButton) {
        updateValueFromPosition(event->pos());
    }
    
    QSlider::mouseMoveEvent(event);
}

void CustomSlider::wheelEvent(QWheelEvent* event)
{
    if (!m_scrollWheelEnabled) {
        event->ignore();
        return;
    }
    
    double delta = event->angleDelta().y() / 120.0; // Standard wheel step
    double step = m_doubleStep;
    
    if (m_proportionalScroll) {
        double range = m_doubleMax - m_doubleMin;
        step = range * 0.01 * delta; // 1% of range per wheel step
    } else {
        step *= delta;
    }
    
    setDoubleValue(m_doubleValue + step);
    
    event->accept();
}

void CustomSlider::updateValueFromPosition(const QPoint& position)
{
    double newValue = positionToValue(position);
    if (!qFuzzyCompare(newValue, m_doubleValue)) {
        setDoubleValue(newValue);
    }
}

QRect CustomSlider::getHandleRect() const
{
    QRect grooveRect = getGrooveRect();
    double normalized = (m_doubleValue - m_doubleMin) / (m_doubleMax - m_doubleMin);
    int handleX = grooveRect.left() + qRound(normalized * grooveRect.width());
    int handleY = height() / 2;
    
    return QRect(handleX - m_handleSize.width() / 2, 
                handleY - m_handleSize.height() / 2,
                m_handleSize.width(), 
                m_handleSize.height());
}

QRect CustomSlider::getGrooveRect() const
{
    int margin = m_handleSize.width() / 2;
    return QRect(margin, 
                (height() - m_grooveHeight) / 2,
                width() - 2 * margin, 
                m_grooveHeight);
}

double CustomSlider::positionToValue(const QPoint& position) const
{
    QRect grooveRect = getGrooveRect();
    double normalized = qBound(0.0, 
                              static_cast<double>(position.x() - grooveRect.left()) / grooveRect.width(),
                              1.0);
    return m_doubleMin + normalized * (m_doubleMax - m_doubleMin);
}

int CustomSlider::valueToPosition(double value) const
{
    QRect grooveRect = getGrooveRect();
    double normalized = (value - m_doubleMin) / (m_doubleMax - m_doubleMin);
    return grooveRect.left() + qRound(normalized * grooveRect.width());
}

} // namespace Glance::UI
