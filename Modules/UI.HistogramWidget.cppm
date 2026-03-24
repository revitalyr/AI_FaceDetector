/**
 * @file UI.HistogramWidget.cppm
 * @brief Custom-painted histogram visualisation widget
 *
 * Renders image histograms for individual RGB channels and luminance.
 * Supports multiple channel display modes, configurable bar width/spacing,
 * grid, colours, statistics overlay (mean/median/stddev/min/max/mode),
 * and integration with ComputeHistogram for GPU-accelerated computation.
 */
module;
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QImage>
#include <QColor>
#include <QFont>
#include <QDebug>
#include <vector>
#include <unordered_map>
#include <memory>

export module UI.HistogramWidget;

import Core.ImageProcessor;
import UI.ComputeHistogram;

namespace Glance::UI {

/** @brief Selectable histogram channel display mode */
export enum class HistogramChannel {
    Luminance,
    Red,
    Green,
    Blue,
    RGB
};

export class HistogramWidget : public QWidget {

public:
    explicit HistogramWidget(QWidget* parent = nullptr);
    
    void setHistogram(const std::vector<int>& data, HistogramChannel channel = HistogramChannel::Luminance);
    void setRGBHistograms(const std::vector<int>& red, 
                         const std::vector<int>& green, 
                         const std::vector<int>& blue);
    void calculateFromImage(const QImage& image);
    void clear();
    
    void setChannel(HistogramChannel channel);
    void setShowGrid(bool show);
    void setShowStats(bool show);
    void setBarWidth(int width);
    void setBarSpacing(int spacing);
    void setBackgroundColor(const QColor& color);
    void setGridColor(const QColor& color);
    
    void setLuminanceColor(const QColor& color);
    void setRedColor(const QColor& color);
    void setGreenColor(const QColor& color);
    void setBlueColor(const QColor& color);
    
    struct Statistics {
        double mean = 0.0;
        double median = 0.0;
        double stdDev = 0.0;
        int min = 0;
        int max = 0;
        int mode = 0;
    };
    
    Statistics getStatistics(HistogramChannel channel = HistogramChannel::Luminance) const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateLayout();
    void drawBackground(QPainter& painter);
    void drawHistogram(QPainter& painter);
    void drawGrid(QPainter& painter);
    void drawAxes(QPainter& painter);
    void drawStatistics(QPainter& painter);
    void drawChannel(QPainter& painter, const std::vector<int>& data, const QColor& color);
    
    void calculateStatistics();
    void updateDisplay();
    
    int valueToY(int value) const;
    int binToX(int bin) const;
    int xToBin(int x) const;

private:
    std::vector<int> m_luminanceHistogram;
    std::vector<int> m_redHistogram;
    std::vector<int> m_greenHistogram;
    std::vector<int> m_blueHistogram;
    
    HistogramChannel m_currentChannel = HistogramChannel::Luminance;
    bool m_showGrid = true;
    bool m_showStats = true;
    int m_barWidth = 2;
    int m_barSpacing = 1;
    
    QColor m_backgroundColor = QColor(30, 30, 30);
    QColor m_gridColor = QColor(60, 60, 60);
    QColor m_luminanceColor = QColor(200, 200, 200);
    QColor m_redColor = QColor(255, 100, 100);
    QColor m_greenColor = QColor(100, 255, 100);
    QColor m_blueColor = QColor(100, 100, 255);
    
    std::unordered_map<HistogramChannel, Statistics> m_statistics;
    bool m_statisticsValid = false;
    
    QRect m_histogramRect;
    QRect m_statsRect;
    int m_margin = 10;
    int m_statsHeight = 60;
    
    std::unique_ptr<ComputeHistogram> m_computeHistogram;
    std::vector<int> m_histogramData;
    int m_maxValue = 1;

    std::unique_ptr<Core::ImageProcessor> m_imageProcessor;
};

} // namespace Glance::UI
