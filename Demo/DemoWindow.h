#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QScrollArea>
#include <QImage>
#include <QTimer>
#include <QTextEdit>
#include <QListWidget>
#include <QSplitter>
#include <memory>

#include "DemoController.h"

namespace Glance::Demo {

class DemoWindow : public QMainWindow
{
public:
    explicit DemoWindow(QWidget *parent = nullptr);
    ~DemoWindow();

private:
    void onStartDemoClicked();
    void onStopDemoClicked();
    void onResetDemoClicked();
    void onRunBenchmarksClicked();
    void onStatusChanged(const QString& status);
    void onDemoStepChanged(int current, int total);
    void onImageProcessed(const QImage& original, const QImage& processed, const QString& operation);
    void onBenchmarkCompleted(const QString& testName, qint64 timeMs);
    void onDemoCompleted();
    void onImageSelected(int index);

private:
    void setupUI();
    void setupMenuBar();
    void setupCentralWidget();
    void setupControlPanel();
    void setupDisplayArea();
    void setupImageDisplay();
    void setupStatusPanel();
    void setupStatusBar();
    void connectSignals();
    void updateDisplay();
    void addBenchmarkResult(const QString& testName, qint64 timeMs);
    void clearResults();

    // UI Components
    QWidget* m_centralWidget;
    QSplitter* m_mainSplitter;
    QSplitter* m_rightSplitter;
    
    // Control Panel
    QWidget* m_controlPanel;
    QPushButton* m_startDemoButton;
    QPushButton* m_stopDemoButton;
    QPushButton* m_resetDemoButton;
    QPushButton* m_runBenchmarksButton;
    QProgressBar* m_demoProgressBar;
    QLabel* m_stepLabel;
    
    // Display Area
    QWidget* m_displayArea;
    QScrollArea* m_originalImageScroll;
    QScrollArea* m_processedImageScroll;
    QLabel* m_originalImageLabel;
    QLabel* m_processedImageLabel;
    QLabel* m_operationLabel;
    QListWidget* m_imageListWidget;
    
    // Status Panel
    QWidget* m_statusPanel;
    QLabel* m_statusLabel;
    QTextEdit* m_logTextEdit;
    QListWidget* m_benchmarkListWidget;
    
    // Demo Controller
    std::unique_ptr<DemoController> m_demoController;
    
    // State
    bool m_demoRunning;
};

} // namespace Glance::Demo
