#include "DemoWindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QSizePolicy>
#include <QHeaderView>
#include <QTextCursor>
#include <QFont>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace Glance::Demo {

DemoWindow::DemoWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_demoRunning(false)
{
    setWindowTitle("Glance - Demo & Testing Suite");
    setMinimumSize(1200, 800);
    resize(1400, 900);

    // Initialize demo controller
    m_demoController = std::make_unique<DemoController>(this);

    setupUI();
    connectSignals();

    // Initialize demo
    m_demoController->initializeDemo();
    
    statusBar()->showMessage("Demo ready - Click 'Start Demo' to begin");
}

DemoWindow::~DemoWindow() = default;

void DemoWindow::setupUI()
{
    setupMenuBar();
    setupCentralWidget();
    setupStatusBar();
}

void DemoWindow::setupMenuBar()
{
    // File Menu
    QMenu* fileMenu = menuBar()->addMenu("File");
    
    QAction* loadImagesAction = fileMenu->addAction("Load Test Images...");
    connect(loadImagesAction, &QAction::triggered, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Select Test Images Directory");
        if (!dir.isEmpty()) {
            // Handle loading custom test images
            statusBar()->showMessage("Custom images loading not implemented in demo");
        }
    });
    
    fileMenu->addSeparator();
    
    QAction* exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // Demo Menu
    QMenu* demoMenu = menuBar()->addMenu("Demo");
    
    QAction* startDemoAction = demoMenu->addAction("Start Demo");
    connect(startDemoAction, &QAction::triggered, this, [this]() { onStartDemoClicked(); });
    
    QAction* stopDemoAction = demoMenu->addAction("Stop Demo");
    connect(stopDemoAction, &QAction::triggered, this, [this]() { onStopDemoClicked(); });
    
    QAction* resetDemoAction = demoMenu->addAction("Reset Demo");
    connect(resetDemoAction, &QAction::triggered, this, [this]() { onResetDemoClicked(); });
    
    demoMenu->addSeparator();
    
    QAction* benchmarkAction = demoMenu->addAction("Run Benchmarks");
    connect(benchmarkAction, &QAction::triggered, this, [this]() { onRunBenchmarksClicked(); });

    // Help Menu
    QMenu* helpMenu = menuBar()->addMenu("Help");
    
    QAction* aboutAction = helpMenu->addAction("About Glance Demo");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About Glance Demo",
            "Glance Demo & Testing Suite\n\n"
            "This demo showcases the capabilities of the Glance image processing application:\n\n"
            "• Advanced image processing with GPU acceleration\n"
            "• AI-powered face detection\n"
            "• Asynchronous image loading\n"
            "• Real-time histogram calculation\n"
            "• Performance benchmarking\n\n"
            "Built with C++23 and Qt6");
    });
}

void DemoWindow::setupCentralWidget()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    // Main horizontal splitter
    m_mainSplitter = new QSplitter(Qt::Horizontal, m_centralWidget);
    
    setupControlPanel();
    setupDisplayArea();
    setupStatusPanel();

    // Add panels to splitter
    m_mainSplitter->addWidget(m_controlPanel);
    m_mainSplitter->addWidget(m_rightSplitter);
    
    // Set splitter sizes (control panel 25%, display area 75%)
    m_mainSplitter->setSizes({300, 900});

    // Main layout
    QHBoxLayout* mainLayout = new QHBoxLayout(m_centralWidget);
    mainLayout->addWidget(m_mainSplitter);
    mainLayout->setContentsMargins(5, 5, 5, 5);
}

void DemoWindow::setupControlPanel()
{
    m_controlPanel = new QWidget();
    m_controlPanel->setMaximumWidth(350);
    m_controlPanel->setMinimumWidth(250);

    QVBoxLayout* controlLayout = new QVBoxLayout(m_controlPanel);

    // Title
    QLabel* titleLabel = new QLabel("Demo Controls");
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(14);
    titleLabel->setFont(titleFont);
    controlLayout->addWidget(titleLabel);

    // Demo controls
    QGroupBox* demoGroup = new QGroupBox("Demo Control");
    QVBoxLayout* demoGroupLayout = new QVBoxLayout(demoGroup);

    m_startDemoButton = new QPushButton("Start Demo");
    m_startDemoButton->setIcon(QIcon(":/icons/play.png"));
    demoGroupLayout->addWidget(m_startDemoButton);

    m_stopDemoButton = new QPushButton("Stop Demo");
    m_stopDemoButton->setEnabled(false);
    m_stopDemoButton->setIcon(QIcon(":/icons/stop.png"));
    demoGroupLayout->addWidget(m_stopDemoButton);

    m_resetDemoButton = new QPushButton("Reset Demo");
    m_resetDemoButton->setIcon(QIcon(":/icons/reset.png"));
    demoGroupLayout->addWidget(m_resetDemoButton);

    m_runBenchmarksButton = new QPushButton("Run Benchmarks");
    m_runBenchmarksButton->setIcon(QIcon(":/icons/benchmark.png"));
    demoGroupLayout->addWidget(m_runBenchmarksButton);

    // Progress
    QLabel* progressLabel = new QLabel("Demo Progress:");
    demoGroupLayout->addWidget(progressLabel);

    m_demoProgressBar = new QProgressBar();
    m_demoProgressBar->setRange(0, 8);
    m_demoProgressBar->setValue(0);
    demoGroupLayout->addWidget(m_demoProgressBar);

    m_stepLabel = new QLabel("Step: 0/8");
    m_stepLabel->setAlignment(Qt::AlignCenter);
    demoGroupLayout->addWidget(m_stepLabel);

    controlLayout->addWidget(demoGroup);

    // Image list
    QGroupBox* imageListGroup = new QGroupBox("Test Images");
    QVBoxLayout* imageListLayout = new QVBoxLayout(imageListGroup);

    m_imageListWidget = new QListWidget();
    m_imageListWidget->setMaximumHeight(200);
    imageListLayout->addWidget(m_imageListWidget);

    controlLayout->addWidget(imageListGroup);

    // Benchmark results
    QGroupBox* benchmarkGroup = new QGroupBox("Benchmark Results");
    QVBoxLayout* benchmarkLayout = new QVBoxLayout(benchmarkGroup);

    m_benchmarkListWidget = new QListWidget();
    m_benchmarkListWidget->setMaximumHeight(200);
    benchmarkLayout->addWidget(m_benchmarkListWidget);

    controlLayout->addWidget(benchmarkGroup);

    controlLayout->addStretch();
}

void DemoWindow::setupDisplayArea()
{
    m_rightSplitter = new QSplitter(Qt::Vertical);
    setupImageDisplay();
    setupStatusPanel();
}

void DemoWindow::setupImageDisplay()
{
    m_displayArea = new QWidget();
    QVBoxLayout* displayLayout = new QVBoxLayout(m_displayArea);

    // Title
    QLabel* displayTitle = new QLabel("Image Processing Display");
    QFont titleFont = displayTitle->font();
    titleFont.setBold(true);
    titleFont.setPointSize(14);
    displayTitle->setFont(titleFont);
    displayLayout->addWidget(displayTitle);

    // Operation label
    m_operationLabel = new QLabel("Operation: None");
    m_operationLabel->setAlignment(Qt::AlignCenter);
    QFont operationFont = m_operationLabel->font();
    operationFont.setItalic(true);
    m_operationLabel->setFont(operationFont);
    displayLayout->addWidget(m_operationLabel);

    // Image display area
    QWidget* imageWidget = new QWidget();
    QHBoxLayout* imageLayout = new QHBoxLayout(imageWidget);

    // Original image
    QGroupBox* originalGroup = new QGroupBox("Original Image");
    QVBoxLayout* originalLayout = new QVBoxLayout(originalGroup);

    m_originalImageScroll = new QScrollArea();
    m_originalImageLabel = new QLabel();
    m_originalImageLabel->setAlignment(Qt::AlignCenter);
    m_originalImageLabel->setMinimumSize(400, 300);
    m_originalImageLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid #ccc; }");
    m_originalImageLabel->setText("No image loaded");
    m_originalImageScroll->setWidget(m_originalImageLabel);
    m_originalImageScroll->setWidgetResizable(true);
    originalLayout->addWidget(m_originalImageScroll);

    imageLayout->addWidget(originalGroup);

    // Processed image
    QGroupBox* processedGroup = new QGroupBox("Processed Image");
    QVBoxLayout* processedLayout = new QVBoxLayout(processedGroup);

    m_processedImageScroll = new QScrollArea();
    m_processedImageLabel = new QLabel();
    m_processedImageLabel->setAlignment(Qt::AlignCenter);
    m_processedImageLabel->setMinimumSize(400, 300);
    m_processedImageLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid #ccc; }");
    m_processedImageLabel->setText("No processing applied");
    m_processedImageScroll->setWidget(m_processedImageLabel);
    m_processedImageScroll->setWidgetResizable(true);
    processedLayout->addWidget(m_processedImageScroll);

    imageLayout->addWidget(processedGroup);

    displayLayout->addWidget(imageWidget);

    m_rightSplitter->addWidget(m_displayArea);
}

void DemoWindow::setupStatusPanel()
{
    m_statusPanel = new QWidget();
    m_statusPanel->setMaximumHeight(250);
    m_statusPanel->setMinimumHeight(150);

    QVBoxLayout* statusLayout = new QVBoxLayout(m_statusPanel);

    // Title
    QLabel* statusTitle = new QLabel("Status & Log");
    QFont titleFont = statusTitle->font();
    titleFont.setBold(true);
    titleFont.setPointSize(12);
    statusTitle->setFont(titleFont);
    statusLayout->addWidget(statusTitle);

    // Status label
    m_statusLabel = new QLabel("Status: Ready");
    m_statusLabel->setStyleSheet("QLabel { background-color: #e8f4e8; padding: 5px; border: 1px solid #ccc; }");
    statusLayout->addWidget(m_statusLabel);

    // Log text area
    m_logTextEdit = new QTextEdit();
    m_logTextEdit->setMaximumHeight(100);
    m_logTextEdit->setReadOnly(true);
    QFont logFont = m_logTextEdit->font();
    logFont.setFamily("Courier New");
    logFont.setPointSize(9);
    m_logTextEdit->setFont(logFont);
    statusLayout->addWidget(m_logTextEdit);

    m_rightSplitter->addWidget(m_statusPanel);

    // Set splitter sizes (display 70%, status 30%)
    m_rightSplitter->setSizes({500, 200});
}

void DemoWindow::setupStatusBar()
{
    statusBar()->showMessage("Demo ready");
}

void DemoWindow::connectSignals()
{
    connect(m_startDemoButton, &QPushButton::clicked, this, [this]() { onStartDemoClicked(); });
    connect(m_stopDemoButton, &QPushButton::clicked, this, [this]() { onStopDemoClicked(); });
    connect(m_resetDemoButton, &QPushButton::clicked, this, [this]() { onResetDemoClicked(); });
    connect(m_runBenchmarksButton, &QPushButton::clicked, this, [this]() { onRunBenchmarksClicked(); });

    connect(m_imageListWidget, &QListWidget::currentRowChanged, this, [this](int index) { onImageSelected(index); });
}

void DemoWindow::onStartDemoClicked()
{
    if (!m_demoRunning) {
        m_demoRunning = true;
        m_startDemoButton->setEnabled(false);
        m_stopDemoButton->setEnabled(true);
        m_resetDemoButton->setEnabled(false);
        m_runBenchmarksButton->setEnabled(false);

        clearResults();
        m_demoController->startAutoDemo();

        statusBar()->showMessage("Demo running...");
    }
}

void DemoWindow::onStopDemoClicked()
{
    if (m_demoRunning) {
        m_demoRunning = false;
        m_demoController->stopAutoDemo();

        m_startDemoButton->setEnabled(true);
        m_stopDemoButton->setEnabled(false);
        m_resetDemoButton->setEnabled(true);
        m_runBenchmarksButton->setEnabled(true);

        statusBar()->showMessage("Demo stopped");
    }
}

void DemoWindow::onResetDemoClicked()
{
    m_demoController->resetDemo();
    clearResults();
    
    m_demoProgressBar->setValue(0);
    m_stepLabel->setText("Step: 0/8");
    m_operationLabel->setText("Operation: None");
    
    m_originalImageLabel->clear();
    m_originalImageLabel->setText("No image loaded");
    m_processedImageLabel->clear();
    m_processedImageLabel->setText("No processing applied");

    statusBar()->showMessage("Demo reset");
}

void DemoWindow::onRunBenchmarksClicked()
{
    clearResults();
    m_demoController->runPerformanceBenchmarks();
    statusBar()->showMessage("Benchmarks completed");
}

void DemoWindow::onStatusChanged(const QString& status)
{
    m_statusLabel->setText("Status: " + status);
    statusBar()->showMessage(status);

    // Add to log
    QTextCursor cursor = m_logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(QString("[%1] %2\n")
                      .arg(QTime::currentTime().toString("hh:mm:ss"))
                      .arg(status));
    m_logTextEdit->setTextCursor(cursor);
    m_logTextEdit->ensureCursorVisible();
}

void DemoWindow::onDemoStepChanged(int current, int total)
{
    m_demoProgressBar->setValue(current);
    m_stepLabel->setText(QString("Step: %1/%2").arg(current).arg(total));
}

void DemoWindow::onImageProcessed(const QImage& original, const QImage& processed, const QString& operation)
{
    m_operationLabel->setText("Operation: " + operation);

    // Display original image
    if (!original.isNull()) {
        QPixmap originalPixmap = QPixmap::fromImage(original);
        m_originalImageLabel->setPixmap(originalPixmap);
        m_originalImageLabel->setAlignment(Qt::AlignCenter);
    }

    // Display processed image
    if (!processed.isNull()) {
        QPixmap processedPixmap = QPixmap::fromImage(processed);
        m_processedImageLabel->setPixmap(processedPixmap);
        m_processedImageLabel->setAlignment(Qt::AlignCenter);
    }

    // Add to log
    QTextCursor cursor = m_logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(QString("[%1] Applied: %2\n")
                      .arg(QTime::currentTime().toString("hh:mm:ss"))
                      .arg(operation));
    m_logTextEdit->setTextCursor(cursor);
    m_logTextEdit->ensureCursorVisible();
}

void DemoWindow::onBenchmarkCompleted(const QString& testName, qint64 timeMs)
{
    addBenchmarkResult(testName, timeMs);
}

void DemoWindow::onDemoCompleted()
{
    m_demoRunning = false;
    m_startDemoButton->setEnabled(true);
    m_stopDemoButton->setEnabled(false);
    m_resetDemoButton->setEnabled(true);
    m_runBenchmarksButton->setEnabled(true);

    statusBar()->showMessage("Demo completed successfully!");

    // Add completion message to log
    QTextCursor cursor = m_logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(QString("\n[%1] === DEMO COMPLETED ===\n")
                      .arg(QTime::currentTime().toString("hh:mm:ss")));
    m_logTextEdit->setTextCursor(cursor);
    m_logTextEdit->ensureCursorVisible();
}

void DemoWindow::onImageSelected(int index)
{
    const auto& demoImages = m_demoController->getDemoImages();
    if (index >= 0 && index < demoImages.size()) {
        const auto& demoImage = demoImages[index];
        if (!demoImage.thumbnail.isNull()) {
            QPixmap pixmap = QPixmap::fromImage(demoImage.thumbnail);
            m_originalImageLabel->setPixmap(pixmap);
            m_originalImageLabel->setAlignment(Qt::AlignCenter);
            m_operationLabel->setText("Operation: Selected - " + demoImage.name);
        }
    }
}

void DemoWindow::updateDisplay()
{
    // Update image list
    m_imageListWidget->clear();
    const auto& demoImages = m_demoController->getDemoImages();
    
    for (const auto& demoImage : demoImages) {
        QListWidgetItem* item = new QListWidgetItem(demoImage.name);
        item->setData(Qt::UserRole, demoImage.description);
        if (demoImage.isLoaded) {
            item->setIcon(QIcon(QPixmap::fromImage(demoImage.thumbnail)));
        }
        m_imageListWidget->addItem(item);
    }
}

void DemoWindow::addBenchmarkResult(const QString& testName, qint64 timeMs)
{
    QString result = QString("%1: %2ms").arg(testName).arg(timeMs);
    m_benchmarkListWidget->addItem(result);

    // Add to log
    QTextCursor cursor = m_logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(QString("[%1] Benchmark: %2\n")
                      .arg(QTime::currentTime().toString("hh:mm:ss"))
                      .arg(result));
    m_logTextEdit->setTextCursor(cursor);
    m_logTextEdit->ensureCursorVisible();
}

void DemoWindow::clearResults()
{
    m_benchmarkListWidget->clear();
    m_logTextEdit->clear();
    
    QTextCursor cursor = m_logTextEdit->textCursor();
    cursor.insertText(QString("[%1] Demo session started\n")
                      .arg(QTime::currentTime().toString("hh:mm:ss")));
    m_logTextEdit->setTextCursor(cursor);
}

} // namespace Glance::Demo
