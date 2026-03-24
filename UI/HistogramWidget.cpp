module;
#include <QPainter>
#include <QDebug>
#include <algorithm>
#include <numeric>
#include <cmath>
module UI.HistogramWidget;
import UI.ComputeHistogram;
import Core.ImageProcessor;

namespace Glance::UI {

HistogramWidget::HistogramWidget(QWidget* parent)
    : QWidget(parent)
    // REMOVED: m_imageProcessor initialization from constructor
{
    setMinimumHeight(150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    // Initialize histogram vectors
    m_luminanceHistogram.resize(256, 0);
    m_redHistogram.resize(256, 0);
    m_greenHistogram.resize(256, 0);
    m_blueHistogram.resize(256, 0);
}

void HistogramWidget::setHistogram(const std::vector<int>& histogramData, HistogramChannel channel)
{
    if (histogramData.size() != 256) {
        qWarning() << "Histogram data must have 256 bins";
        return;
    }
    
    switch (channel) {
        case HistogramChannel::Luminance:
            m_luminanceHistogram = histogramData;
            break;
        case HistogramChannel::Red:
            m_redHistogram = histogramData;
            break;
        case HistogramChannel::Green:
            m_greenHistogram = histogramData;
            break;
        case HistogramChannel::Blue:
            m_blueHistogram = histogramData;
            break;
        default:
            break;
    }
    
    m_statisticsValid = false;
    updateDisplay();
}

void HistogramWidget::setRGBHistograms(const std::vector<int>& red, 
                                      const std::vector<int>& green, 
                                      const std::vector<int>& blue)
{
    if (red.size() != 256 || green.size() != 256 || blue.size() != 256) {
        qWarning() << "RGB histogram data must have 256 bins each";
        return;
    }
    
    m_redHistogram = red;
    m_greenHistogram = green;
    m_blueHistogram = blue;
    
    m_statisticsValid = false;
    updateDisplay();
}

void HistogramWidget::calculateFromImage(const QImage& image)
{
    if (!m_computeHistogram) {
        m_computeHistogram = std::make_unique<ComputeHistogram>();
        m_computeHistogram->initialize();
    }

    m_computeHistogram->computeHistogram(image);
    auto rawData = m_computeHistogram->getHistogramData();
    m_histogramData.assign(rawData.begin(), rawData.end());
    m_maxValue = *std::max_element(m_histogramData.begin(), m_histogramData.end());
    if (m_maxValue == 0) m_maxValue = 1;

    // Calculate per-channel histograms using the core processor
    if (!m_imageProcessor) {
        m_imageProcessor = std::make_unique<Core::ImageProcessor>();
        qDebug() << "ImageProcessor lazy-initialized in HistogramWidget";
    }
    m_luminanceHistogram = m_imageProcessor->calculateHistogram(image, -1);
    m_redHistogram = m_imageProcessor->calculateHistogram(image, 0);
    m_greenHistogram = m_imageProcessor->calculateHistogram(image, 1);
    m_blueHistogram = m_imageProcessor->calculateHistogram(image, 2);

    m_statisticsValid = false;
    updateDisplay();
}

void HistogramWidget::clear()
{
    std::fill(m_luminanceHistogram.begin(), m_luminanceHistogram.end(), 0);
    std::fill(m_redHistogram.begin(), m_redHistogram.end(), 0);
    std::fill(m_greenHistogram.begin(), m_greenHistogram.end(), 0);
    std::fill(m_blueHistogram.begin(), m_blueHistogram.end(), 0);
    
    m_statistics.clear();
    m_statisticsValid = false;
    update();
}

void HistogramWidget::setChannel(HistogramChannel channel)
{
    if (m_currentChannel != channel) {
        m_currentChannel = channel;
        update();
    }
}

void HistogramWidget::setShowGrid(bool show)
{
    if (m_showGrid != show) {
        m_showGrid = show;
        update();
    }
}

void HistogramWidget::setShowStats(bool show)
{
    if (m_showStats != show) {
        m_showStats = show;
        updateLayout();
        update();
    }
}

void HistogramWidget::setBarWidth(int width)
{
    m_barWidth = qMax(1, width);
    updateDisplay();
}

void HistogramWidget::setBarSpacing(int spacing)
{
    m_barSpacing = qMax(0, spacing);
    updateDisplay();
}

void HistogramWidget::setBackgroundColor(const QColor& color)
{
    m_backgroundColor = color;
    update();
}

void HistogramWidget::setGridColor(const QColor& color)
{
    m_gridColor = color;
    update();
}

void HistogramWidget::setLuminanceColor(const QColor& color)
{
    m_luminanceColor = color;
    update();
}

void HistogramWidget::setRedColor(const QColor& color)
{
    m_redColor = color;
    update();
}

void HistogramWidget::setGreenColor(const QColor& color)
{
    m_greenColor = color;
    update();
}

void HistogramWidget::setBlueColor(const QColor& color)
{
    m_blueColor = color;
    update();
}

HistogramWidget::Statistics HistogramWidget::getStatistics(HistogramChannel channel) const
{
    qDebug() << "getStatistics() called for channel:" << static_cast<int>(channel);
    
    try {
        auto it = m_statistics.find(channel);
        if (it != m_statistics.end()) {
            qDebug() << "getStatistics() found cached stats";
            return it->second;
        }
        
        // Calculate statistics if not cached
        const std::vector<int>* histogram = nullptr;
        switch (channel) {
            case HistogramChannel::Luminance:
                histogram = &m_luminanceHistogram;
                break;
            case HistogramChannel::Red:
                histogram = &m_redHistogram;
                break;
            case HistogramChannel::Green:
                histogram = &m_greenHistogram;
                break;
            case HistogramChannel::Blue:
                histogram = &m_blueHistogram;
                break;
            case HistogramChannel::RGB:
                histogram = &m_luminanceHistogram; // Use luminance for RGB stats
                break;
        }
        
        if (!histogram || histogram->empty()) {
            qDebug() << "getStatistics() no histogram data";
            return Statistics{};
        }
        
        // Calculate statistics
        Statistics stats;
        auto calculateStats = [](const std::vector<int>& histogramData) -> Statistics {
            if (histogramData.empty()) {
                return Statistics{};
            }
            
            Statistics result;
            
            // Min and Max
            result.min = *std::min_element(histogramData.begin(), histogramData.end());
            result.max = *std::max_element(histogramData.begin(), histogramData.end());
            
            // Mean
            double sum = std::accumulate(histogramData.begin(), histogramData.end(), 0.0);
            result.mean = sum / histogramData.size();
            
            // Median
            std::vector<int> sortedData = histogramData;
            std::sort(sortedData.begin(), sortedData.end());
            size_t n = sortedData.size();
            if (n % 2 == 0) {
                result.median = (sortedData[n/2 - 1] + sortedData[n/2]) / 2.0;
            } else {
                result.median = sortedData[n/2];
            }
            
            // Standard deviation
            double variance = 0.0;
            for (int value : histogramData) {
                variance += (value - result.mean) * (value - result.mean);
            }
            variance /= histogramData.size();
            result.stdDev = std::sqrt(variance);
            
            return result;
        };
        
        stats = calculateStats(*histogram);
        qDebug() << "getStatistics() calculated stats successfully";
        return stats;
        
    } catch (const std::exception& e) {
        qWarning() << "getStatistics() failed:" << e.what();
        return Statistics{};
    } catch (...) {
        qWarning() << "getStatistics() failed with unknown error";
        return Statistics{};
    }
}

void HistogramWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    qDebug() << "HistogramWidget::paintEvent() called";
    
    try {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        drawBackground(painter);
        qDebug() << "drawBackground() completed";
        
        if (m_showGrid) {
            drawGrid(painter);
            qDebug() << "drawGrid() completed";
        }
        
        drawHistogram(painter);
        qDebug() << "drawHistogram() completed";
        
        if (m_showStats) {
            drawStatistics(painter);
            qDebug() << "drawStatistics() completed";
        }
        
        qDebug() << "paintEvent() completed successfully";
        
    } catch (const std::exception& e) {
        qWarning() << "paintEvent() failed:" << e.what();
    } catch (...) {
        qWarning() << "paintEvent() failed with unknown error";
    }
}

void HistogramWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)
    updateLayout();
}

void HistogramWidget::drawBackground(QPainter& painter)
{
    painter.fillRect(rect(), m_backgroundColor);
}

void HistogramWidget::drawGrid(QPainter& painter)
{
    painter.setPen(QPen(m_gridColor, 1, Qt::DashLine));
    
    // Horizontal grid lines
    for (int i = 0; i <= 4; ++i) {
        int y = m_histogramRect.top() + (m_histogramRect.height() * i) / 4;
        painter.drawLine(m_histogramRect.left(), y, m_histogramRect.right(), y);
    }
    
    // Vertical grid lines
    for (int i = 0; i <= 10; ++i) {
        int x = m_histogramRect.left() + (m_histogramRect.width() * i) / 10;
        painter.drawLine(x, m_histogramRect.top(), x, m_histogramRect.bottom());
    }
}

void HistogramWidget::drawHistogram(QPainter& painter)
{
    qDebug() << "drawHistogram() called";
    
    try {
        switch (m_currentChannel) {
            case HistogramChannel::Luminance:
                drawChannel(painter, m_luminanceHistogram, m_luminanceColor);
                break;
            case HistogramChannel::Red:
                drawChannel(painter, m_redHistogram, m_redColor);
                break;
            case HistogramChannel::Green:
                drawChannel(painter, m_greenHistogram, m_greenColor);
                break;
            case HistogramChannel::Blue:
                drawChannel(painter, m_blueHistogram, m_blueColor);
                break;
            case HistogramChannel::RGB:
                // Draw all three channels with transparency
                drawChannel(painter, m_redHistogram, QColor(m_redColor.red(), m_redColor.green(), m_redColor.blue(), 128));
                drawChannel(painter, m_greenHistogram, QColor(m_greenColor.red(), m_greenColor.green(), m_greenColor.blue(), 128));
                drawChannel(painter, m_blueHistogram, QColor(m_blueColor.red(), m_blueColor.green(), m_blueColor.blue(), 128));
                break;
        }
        qDebug() << "drawHistogram() completed successfully";
    } catch (const std::exception& e) {
        qWarning() << "drawHistogram() failed:" << e.what();
    } catch (...) {
        qWarning() << "drawHistogram() failed with unknown error";
    }
}

void HistogramWidget::drawChannel(QPainter& painter, const std::vector<int>& histogramData, const QColor& color)
{
    qDebug() << "drawChannel() called with data size:" << histogramData.size();
    
    if (histogramData.empty()) {
        qDebug() << "drawChannel: data is empty, returning";
        return;
    }
    
    try {
        // Find min and max values for proper scaling
        auto [minVal, maxVal] = std::minmax_element(histogramData.begin(), histogramData.end());
        int minValue = *minVal;
        int maxValue = *maxVal;
        
        if (maxValue == minValue) {
            qDebug() << "drawChannel: all values are the same, returning";
            return;
        }
        
        qDebug() << "drawChannel: min =" << minValue << "max =" << maxValue << "histogramRect:" << m_histogramRect;
        
        painter.setPen(QPen(color, m_barWidth));
        
        for (int i = 0; i < 256; ++i) {
            int x = binToX(i);
            // Scale based on actual min/max values, not just max
            int normalizedValue = (histogramData[i] - minValue) * 100 / (maxValue - minValue);
            int y = valueToY(normalizedValue);
            painter.drawLine(x, m_histogramRect.bottom(), x, y);
        }
        
        qDebug() << "drawChannel() completed successfully";
    } catch (const std::exception& e) {
        qWarning() << "drawChannel() failed:" << e.what();
    } catch (...) {
        qWarning() << "drawChannel() failed with unknown error";
    }
}

void HistogramWidget::drawStatistics(QPainter& painter)
{
    qDebug() << "drawStatistics() called";
    
    try {
        painter.setPen(QPen(Qt::white, 1));
        painter.setFont(QFont("Arial", 8)); // Smaller font for multiple lines
        
        Statistics stats = getStatistics(m_currentChannel);
        
        // Split statistics into multiple lines
        QString line1 = QString("Mean: %1  Median: %2").arg(stats.mean, 0, 'f', 2).arg(stats.median, 0, 'f', 2);
        QString line2 = QString("StdDev: %1  Min: %2  Max: %3").arg(stats.stdDev, 0, 'f', 2).arg(stats.min).arg(stats.max);
        
        // Draw first line
        QRect line1Rect = m_statsRect;
        line1Rect.setHeight(m_statsRect.height() / 2);
        painter.drawText(line1Rect, Qt::AlignLeft | Qt::AlignVCenter, line1);
        
        // Draw second line
        QRect line2Rect = m_statsRect;
        line2Rect.setTop(m_statsRect.center().y());
        painter.drawText(line2Rect, Qt::AlignLeft | Qt::AlignVCenter, line2);
        
        qDebug() << "drawStatistics() completed successfully";
    } catch (const std::exception& e) {
        qWarning() << "drawStatistics() failed:" << e.what();
    } catch (...) {
        qWarning() << "drawStatistics() failed with unknown error";
    }
}

void HistogramWidget::calculateStatistics()
{
    // ...
    auto calculateStats = [](const std::vector<int>& histogramData) -> Statistics {
        if (histogramData.empty()) {
            return Statistics{};
        }
        
        Statistics stats;
        
        // Min and Max
        stats.min = *std::min_element(histogramData.begin(), histogramData.end());
        stats.max = *std::max_element(histogramData.begin(), histogramData.end());
        
        // Mean
        double sum = std::accumulate(histogramData.begin(), histogramData.end(), 0.0);
        stats.mean = sum / histogramData.size();
        
        // Median
        std::vector<int> sortedData = histogramData;
        std::sort(sortedData.begin(), sortedData.end());
        
        if (sortedData.size() % 2 == 0) {
            stats.median = (sortedData[sortedData.size() / 2 - 1] + sortedData[sortedData.size() / 2]) / 2.0;
        } else {
            stats.median = sortedData[sortedData.size() / 2];
        }
        
        // Standard deviation
        double squaredSum = std::accumulate(histogramData.begin(), histogramData.end(), 0.0,
            [stats](double acc, int value) {
                return acc + std::pow(value - stats.mean, 2);
            });
        
        stats.stdDev = std::sqrt(squaredSum / histogramData.size());
        
        // Mode (most frequent value)
        std::vector<int> frequency(256, 0);
        for (int value : histogramData) {
            if (value > 0) {
                frequency[value]++;
            }
        }
        
        auto maxFreqIt = std::max_element(frequency.begin(), frequency.end());
        stats.mode = std::distance(frequency.begin(), maxFreqIt);
        
        return stats;
    };
    
    m_statistics[HistogramChannel::Luminance] = calculateStats(m_luminanceHistogram);
    m_statistics[HistogramChannel::Red] = calculateStats(m_redHistogram);
    m_statistics[HistogramChannel::Green] = calculateStats(m_greenHistogram);
    m_statistics[HistogramChannel::Blue] = calculateStats(m_blueHistogram);
    
    m_statisticsValid = true;
}

void HistogramWidget::updateDisplay()
{
    qDebug() << "HistogramWidget::updateDisplay() called";
    try {
        updateLayout();
        qDebug() << "updateLayout() completed";
        update();
        qDebug() << "update() completed";
    } catch (const std::exception& e) {
        qWarning() << "updateDisplay() failed:" << e.what();
    } catch (...) {
        qWarning() << "updateDisplay() failed with unknown error";
    }
}

void HistogramWidget::updateLayout()
{
    qDebug() << "HistogramWidget::updateLayout() called, size:" << width() << "x" << height();
    
    m_histogramRect = QRect(m_margin, m_margin, 
                           width() - 2 * m_margin, 
                           height() - 2 * m_margin - (m_showStats ? m_statsHeight : 0));
    
    qDebug() << "m_histogramRect:" << m_histogramRect;
    
    if (m_showStats) {
        m_statsRect = QRect(m_margin, height() - m_statsHeight - m_margin,
                          width() - 2 * m_margin, m_statsHeight);
        qDebug() << "m_statsRect:" << m_statsRect;
    }
    
    qDebug() << "updateLayout() completed successfully";
}

int HistogramWidget::valueToY(int value) const
{
    if (m_histogramRect.height() == 0) {
        return 0;
    }
    
    return m_histogramRect.bottom() - (value * m_histogramRect.height()) / 100;
}

int HistogramWidget::binToX(int bin) const
{
    if (m_histogramRect.width() == 0) {
        return 0;
    }
    
    return m_histogramRect.left() + (bin * m_histogramRect.width()) / 255;
}

int HistogramWidget::xToBin(int x) const
{
    if (m_histogramRect.width() == 0) {
        return 0;
    }
    
    int bin = ((x - m_histogramRect.left()) * 255) / m_histogramRect.width();
    return qBound(0, bin, 255);
}

} // namespace Glance::UI
