module;
#include <QtConcurrent>
#include <QDebug>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
module UI.MainWindow;
import Core.Orchestrator;
import Core.FaceDetector;
import Core.ImageProcessor;

namespace Glance::UI {

MainWindow::MainWindow(Core::Orchestrator& orchestrator, QWidget* parent)
    : QMainWindow(parent)
    , m_orchestrator(&orchestrator)
{
    resize(1200, 800);
    setupMenus();

    // UI and connection setup
    m_splitView = new SplitScreenWidget(this);
    m_splitView->setMinimumSize(600, 400);
    m_progressBar = new QProgressBar(this);
    m_undoStack = new QUndoStack(this);

    m_exposureSlider = new CustomSlider(-3.0, 3.0, 0.1, 0.0, this);
    m_contrastSlider = new CustomSlider(0.0, 3.0, 0.1, 1.0, this);
    m_brightnessSlider = new CustomSlider(-1.0, 1.0, 0.01, 0.0, this);
    m_saturationSlider = new CustomSlider(-100.0, 100.0, 1.0, 0.0, this);
    m_temperatureSlider = new CustomSlider(-100.0, 100.0, 1.0, 0.0, this);
    m_tintSlider = new CustomSlider(-100.0, 100.0, 1.0, 0.0, this);
    m_vibranceSlider = new CustomSlider(-100.0, 100.0, 1.0, 0.0, this);
    m_highlightsSlider = new CustomSlider(-100.0, 100.0, 1.0, 0.0, this);
    m_shadowsSlider = new CustomSlider(-100.0, 100.0, 1.0, 0.0, this);
    m_detailsSlider = new CustomSlider(-100.0, 100.0, 1.0, 0.0, this);
    m_gammaSlider = new CustomSlider(0.1, 3.0, 0.1, 1.0, this);

    connect(m_exposureSlider, &CustomSlider::valueChanged, this, [this]() { onSliderValueChanged(); });
    connect(m_contrastSlider, &CustomSlider::valueChanged, this, [this]() { onSliderValueChanged(); });
    connect(m_brightnessSlider, &CustomSlider::valueChanged, this, [this]() { onSliderValueChanged(); });
    connect(m_saturationSlider, &CustomSlider::valueChanged, this, [this]() { onSliderValueChanged(); });
    connect(m_temperatureSlider, &CustomSlider::valueChanged, this, [this]() { onSliderValueChanged(); });
    connect(m_tintSlider, &CustomSlider::valueChanged, this, [this]() { onSliderValueChanged(); });
    connect(m_vibranceSlider, &CustomSlider::valueChanged, this, [this]() { onSliderValueChanged(); });
    connect(m_highlightsSlider, &CustomSlider::valueChanged, this, [this]() { onSliderValueChanged(); });
    connect(m_shadowsSlider, &CustomSlider::valueChanged, this, [this]() { onSliderValueChanged(); });
    connect(m_detailsSlider, &CustomSlider::valueChanged, this, [this]() { onSliderValueChanged(); });
    connect(m_gammaSlider, &CustomSlider::valueChanged, this, [this]() { onSliderValueChanged(); });

    // Layout setup
    QWidget* centralWidget = new QWidget(this);
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

    // Left: image viewport (expanding)
    mainLayout->addWidget(m_splitView, 1);

    // Right: control panel
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setFixedWidth(260);
    scrollArea->setWidgetResizable(true);
    
    QWidget* controlPanel = new QWidget(scrollArea);
    QVBoxLayout* controlLayout = new QVBoxLayout(controlPanel);

    controlLayout->addWidget(new QLabel("Exposure:", this));
    controlLayout->addWidget(m_exposureSlider);
    
    controlLayout->addWidget(new QLabel("Contrast:", this));
    controlLayout->addWidget(m_contrastSlider);
    
    controlLayout->addWidget(new QLabel("Brightness:", this));
    controlLayout->addWidget(m_brightnessSlider);
    
    controlLayout->addWidget(new QLabel("Saturation:", this));
    controlLayout->addWidget(m_saturationSlider);
    
    controlLayout->addWidget(new QLabel("Temperature:", this));
    controlLayout->addWidget(m_temperatureSlider);
    
    controlLayout->addWidget(new QLabel("Tint:", this));
    controlLayout->addWidget(m_tintSlider);
    
    controlLayout->addWidget(new QLabel("Vibrance:", this));
    controlLayout->addWidget(m_vibranceSlider);
    
    controlLayout->addWidget(new QLabel("Highlights:", this));
    controlLayout->addWidget(m_highlightsSlider);
    
    controlLayout->addWidget(new QLabel("Shadows:", this));
    controlLayout->addWidget(m_shadowsSlider);
    
    controlLayout->addWidget(new QLabel("Details:", this));
    controlLayout->addWidget(m_detailsSlider);
    
    controlLayout->addWidget(new QLabel("Gamma:", this));
    controlLayout->addWidget(m_gammaSlider);

    controlLayout->addSpacing(20);
    m_removeBgBtn = new QPushButton("Remove Background", this);
    connect(m_removeBgBtn, &QPushButton::clicked, this, [this]() { onRemoveBackgroundTriggered(); });
    controlLayout->addWidget(m_removeBgBtn);

    m_cropToFacesBtn = new QPushButton("Crop to Faces", this);
    m_cropToFacesBtn->setEnabled(false);
    connect(m_cropToFacesBtn, &QPushButton::clicked, this, [this]() { onCropToFacesTriggered(); });
    controlLayout->addWidget(m_cropToFacesBtn);

    m_detectFacesBtn = new QPushButton("Detect Faces...", this);
    m_detectFacesBtn->setEnabled(false);
    connect(m_detectFacesBtn, &QPushButton::clicked, this, [this]() { onDetectFacesTriggered(); });
    controlLayout->addWidget(m_detectFacesBtn);

    m_lutButton = new QPushButton("Apply LUT", this);
    connect(m_lutButton, &QPushButton::clicked, this, [this]() { onLUTTriggered(); });
    controlLayout->addWidget(m_lutButton);

    controlLayout->addStretch();
    scrollArea->setWidget(controlPanel);
    mainLayout->addWidget(scrollArea);

    setCentralWidget(centralWidget);

    statusBar()->addPermanentWidget(m_progressBar);
    m_progressBar->setVisible(false);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupMenus()
{
    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Open...", QKeySequence::Open, this, [this]() { openFile(); });
    fileMenu->addSeparator();
    fileMenu->addAction("&Exit", QKeySequence::Quit, qApp, &QApplication::quit);

    QMenu* editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction("&Undo", QKeySequence::Undo, this, [this]() { undo(); });
    editMenu->addAction("&Redo", QKeySequence::Redo, this, [this]() { redo(); });
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Image", "", 
        "Images (*.png *.jpg *.jpeg *.bmp *.tiff *.raw *.dng);;All Files (*)");
        
    if (!fileName.isEmpty()) {
        QImage image(fileName);
        if (!image.isNull()) {
            onImageLoaded(image);
        }
    }
}

Core::ProcessingParams MainWindow::getSliderParams()
{
    Core::ProcessingParams params;
    
    // Logic for converting UI values to core parameters using semantic aliases
    params.setExposure(static_cast<Core::Float32>(m_exposureSlider->doubleValue()));
    params.setContrast(static_cast<Core::Float32>(m_contrastSlider->doubleValue()));
    params.setBrightness(static_cast<Core::Float32>(m_brightnessSlider->doubleValue()));
    params.setSaturation(static_cast<Core::Float32>(m_saturationSlider->doubleValue()));
    params.setTemperature(static_cast<Core::Float32>(m_temperatureSlider->doubleValue()));
    params.setTint(static_cast<Core::Float32>(m_tintSlider->doubleValue()));
    params.setVibrance(static_cast<Core::Float32>(m_vibranceSlider->doubleValue()));
    params.setHighlights(static_cast<Core::Float32>(m_highlightsSlider->doubleValue()));
    params.setShadows(static_cast<Core::Float32>(m_shadowsSlider->doubleValue()));
    params.setDetails(static_cast<Core::Float32>(m_detailsSlider->doubleValue()));
    params.setGamma(static_cast<Core::Float32>(m_gammaSlider->doubleValue()));
    
    params.enableLUT = !m_lutFilePath.isEmpty();
    params.lutFilePath = m_lutFilePath;
    
    return params;
}

void MainWindow::resetSlidersSilent()
{
    // Block signals to prevent recursive processing during reset
    m_exposureSlider->blockSignals(true);
    m_contrastSlider->blockSignals(true);
    m_brightnessSlider->blockSignals(true);
    m_saturationSlider->blockSignals(true);
    m_temperatureSlider->blockSignals(true);
    m_tintSlider->blockSignals(true);
    m_vibranceSlider->blockSignals(true);
    m_highlightsSlider->blockSignals(true);
    m_shadowsSlider->blockSignals(true);
    m_detailsSlider->blockSignals(true);
    m_gammaSlider->blockSignals(true);

    m_exposureSlider->setDoubleValue(0.0);
    m_contrastSlider->setDoubleValue(1.0); // Default contrast is 1.0
    m_brightnessSlider->setDoubleValue(0.0);
    m_saturationSlider->setDoubleValue(0.0);
    m_temperatureSlider->setDoubleValue(0.0);
    m_tintSlider->setDoubleValue(0.0);
    m_vibranceSlider->setDoubleValue(0.0);
    m_highlightsSlider->setDoubleValue(0.0);
    m_shadowsSlider->setDoubleValue(0.0);
    m_detailsSlider->setDoubleValue(0.0);
    m_gammaSlider->setDoubleValue(1.0);

    m_exposureSlider->blockSignals(false);
    m_contrastSlider->blockSignals(false);
    m_brightnessSlider->blockSignals(false);
    m_saturationSlider->blockSignals(false);
    m_temperatureSlider->blockSignals(false);
    m_tintSlider->blockSignals(false);
    m_vibranceSlider->blockSignals(false);
    m_highlightsSlider->blockSignals(false);
    m_shadowsSlider->blockSignals(false);
    m_detailsSlider->blockSignals(false);
    m_gammaSlider->blockSignals(false);
}

// 1. SLOT: Called when any slider value changes
void MainWindow::onSliderValueChanged()
{
    // Keep m_currentBaseImage intact — sliders work on top of current base (with or without background).
    // Run the processing pipeline
    updateProcessedImage();
}

// 2. PIPELINE: applies current slider parameters to the base image
void MainWindow::updateProcessedImage()
{
    // Bail if no image loaded or processing is already in flight
    if (m_currentBaseImage.isNull() || m_isProcessing) return;

    m_isProcessing = true;
    m_progressBar->setVisible(true);

    // Gather current slider parameters
    Core::ProcessingParams params = getSliderParams();

    // Create watcher for async processing
    auto watcher = new QFutureWatcher<QImage>(this);
    
    connect(watcher, &QFutureWatcher<QImage>::finished, this, [this, watcher]() {
        // Display processed image
        m_splitView->setProcessedImage(watcher->result());
        
        m_isProcessing = false;
        m_progressBar->setVisible(false);
        m_removeBgBtn->setEnabled(true);
        watcher->deleteLater();
    });

    // Run heavy processing off the main thread via QtConcurrent
    // Use shared_ptr to avoid deep copy of QImage
    auto basePtr = std::make_shared<QImage>(m_currentBaseImage);
    QPointer<MainWindow> guard(this);
    QFuture<QImage> future = QtConcurrent::run([guard, basePtr, params]() {
        if (!guard) {
            return QImage();
        }
        // If background was removed, base already has transparency.
        // Sliders are applied on top of the current state.
        return guard->m_orchestrator->processImage(*basePtr, params);
    });
    
    watcher->setFuture(future);
}

// 3. SLOT: Called when "Remove Background" is pressed
void MainWindow::onRemoveBackgroundTriggered()
{
    if (m_currentBaseImage.isNull()) return;

    m_removeBgBtn->setEnabled(false);
    m_progressBar->setVisible(true);
    
    auto watcher = new QFutureWatcher<QImage>(this);
    connect(watcher, &QFutureWatcher<QImage>::finished, this, [this, watcher]() {
        QImage resultImage = watcher->result();
        
        if (!resultImage.isNull()) {
            // Commit background removal result as the NEW pipeline base
            m_currentBaseImage = resultImage;

            // Record this step in history for Undo/Redo
            if (m_currentHistoryIndex < static_cast<int>(m_imageHistory.size()) - 1) {
                m_imageHistory.erase(m_imageHistory.begin() + m_currentHistoryIndex + 1, m_imageHistory.end());
            }
            m_imageHistory.push_back({m_currentBaseImage, m_detectedFaces});
            m_currentHistoryIndex = static_cast<int>(m_imageHistory.size()) - 1;

            // Display the current state
            m_splitView->setProcessedImage(m_currentBaseImage);
            
            // If slider values are already set, force re-render with the new base
            updateProcessedImage();
        }
        
        if (!m_isProcessing) {
            m_progressBar->setVisible(false);
            m_removeBgBtn->setEnabled(true);
        }
        watcher->deleteLater();
    });

    auto basePtr = std::make_shared<QImage>(m_currentBaseImage);
    QPointer<MainWindow> guard(this);
    QFuture<QImage> future = QtConcurrent::run([guard, basePtr]() -> QImage {
        if (!guard) {
            return QImage();
        }
        // Always remove background from m_currentBaseImage
        auto result = guard->m_orchestrator->removeBackground(*basePtr);
        return result ? result->composited : QImage();
    });
    
    watcher->setFuture(future);
}

void MainWindow::onLUTTriggered()
{
    QString dir = m_lutFilePath.isEmpty() ? QDir::currentPath() : QFileInfo(m_lutFilePath).absolutePath();
    QString filePath = QFileDialog::getOpenFileName(this, "Select LUT (.cube)", dir, "Cube LUT (*.cube)");
    if (filePath.isEmpty()) return;

    m_lutFilePath = filePath;
    m_lutButton->setText(QString("LUT: %1").arg(QFileInfo(filePath).fileName()));
    updateProcessedImage();
}

void MainWindow::onCropToFacesTriggered()
{
    if (m_currentBaseImage.isNull() || m_detectedFaces.empty()) return;

    // 1. Compute the bounding rectangle of all detected faces
    QRect combinedRect;
    for (const auto& face : m_detectedFaces) {
        combinedRect = combinedRect.united(face.rect);
    }

    // 2. Add 25% margin around the face region for context
    int marginX = combinedRect.width() * 0.25;
    int marginY = combinedRect.height() * 0.25;
    QRect cropRect = combinedRect.adjusted(-marginX, -marginY, marginX, marginY);

    // 3. Clamp the crop rect to the image bounds
    cropRect = cropRect.intersected(m_currentBaseImage.rect());

    if (cropRect.isValid() && cropRect != m_currentBaseImage.rect()) {
        // 4. Crop the current base image
        m_currentBaseImage = m_currentBaseImage.copy(cropRect);

        // 5. Adjust face rects and landmarks to match the cropped image
        for (auto& face : m_detectedFaces) {
            face.rect.translate(-cropRect.topLeft());
            for (auto& pt : face.landmarks) {
                pt -= cropRect.topLeft();
            }
        }

        // 6. Push to history
        if (m_currentHistoryIndex < static_cast<int>(m_imageHistory.size()) - 1) {
            m_imageHistory.erase(m_imageHistory.begin() + m_currentHistoryIndex + 1, m_imageHistory.end());
        }
        m_imageHistory.push_back({m_currentBaseImage, m_detectedFaces});
        m_currentHistoryIndex = static_cast<int>(m_imageHistory.size()) - 1;

        // 7. Update widgets
        m_splitView->setProcessedImage(m_currentBaseImage);
        m_splitView->setFaceRects(m_detectedFaces);
        
        // 8. Re-apply filters on the new crop
        updateProcessedImage();
    }
}

void MainWindow::onDetectFacesTriggered()
{
    if (m_originalImage.isNull()) return;

    // 1. Show Parameters Dialog
    QDialog dialog(this);
    dialog.setWindowTitle("Face Detection Parameters");
    QFormLayout form(&dialog);

    QDoubleSpinBox* thresholdSpin = new QDoubleSpinBox(&dialog);
    thresholdSpin->setRange(0.1, 0.9);
    thresholdSpin->setSingleStep(0.1);
    thresholdSpin->setValue(0.5);
    form.addRow("Confidence Threshold:", thresholdSpin);

    QSpinBox* minSizeSpin = new QSpinBox(&dialog);
    minSizeSpin->setRange(10, 1000);
    minSizeSpin->setValue(30);
    form.addRow("Min Face Size (px):", minSizeSpin);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        // 2. Update Orchestrator settings
        m_orchestrator->setConfidenceThreshold(thresholdSpin->value());
        m_orchestrator->setMinFaceSize(minSizeSpin->value());

        // 3. Start Detection
        m_progressBar->setVisible(true);
        m_detectFacesBtn->setEnabled(false);

        auto faceWatcher = new QFutureWatcher<Core::FaceDetectionResult>(this);
        connect(faceWatcher, &QFutureWatcher<Core::FaceDetectionResult>::finished, this, [this, faceWatcher]() {
            const auto result = faceWatcher->result();
            if (result.success) {
                m_detectedFaces = result.faces;
                if (m_currentHistoryIndex >= 0 && m_currentHistoryIndex < static_cast<int>(m_imageHistory.size())) {
                    m_imageHistory[m_currentHistoryIndex].faces = m_detectedFaces;
                }
                m_cropToFacesBtn->setEnabled(!m_detectedFaces.empty());
                m_splitView->setFaceRects(result.faces);
                statusBar()->showMessage(QString("Found %1 faces").arg(result.faces.size()), 3000);
            } else {
                qWarning() << "Manual detection failed:" << result.errorMessage;
            }
            
            m_progressBar->setVisible(false);
            m_detectFacesBtn->setEnabled(true);
            faceWatcher->deleteLater();
        });

        faceWatcher->setFuture(m_orchestrator->detectFacesAsync(m_originalImage));
    }
}

// 4. SLOT: Initialize state when a new file is opened
void MainWindow::onImageLoaded(const QImage& image)
{
    m_originalImage = image;
    m_currentBaseImage = image; // Initially base equals original

    // Clear history from the previous image
    m_imageHistory.clear();
    m_imageHistory.push_back({m_currentBaseImage, {}});
    m_currentHistoryIndex = 0;
    m_detectedFaces.clear();
    m_cropToFacesBtn->setEnabled(false);
    m_detectFacesBtn->setEnabled(true);

    // Reset sliders silently (no onSliderValueChanged to avoid triggering pipeline)
    resetSlidersSilent();

    m_splitView->setOriginalImage(m_originalImage);
    m_splitView->setProcessedImage(m_currentBaseImage);

    // Reuse the detection slot to avoid duplicating logic
    onDetectFacesTriggered();
}

// 5. REDO
void MainWindow::redo()
{
    qDebug() << "Redo called. Index:" << m_currentHistoryIndex;
    
    if (m_currentHistoryIndex < static_cast<int>(m_imageHistory.size()) - 1) {
        m_currentHistoryIndex++;
        // Restore base from history (may or may not have background removed)
        const auto& entry = m_imageHistory[m_currentHistoryIndex];
        m_currentBaseImage = entry.image;
        m_detectedFaces = entry.faces;
        
        // Display and re-apply slider parameters on top
        m_splitView->setProcessedImage(m_currentBaseImage);
        m_splitView->setFaceRects(m_detectedFaces);
        updateProcessedImage(); 
    }
    if (m_undoStack) m_undoStack->redo();
}

// 6. UNDO
void MainWindow::undo()
{
    qDebug() << "Undo called. Index:" << m_currentHistoryIndex;
    
    if (m_currentHistoryIndex > 0) {
        m_currentHistoryIndex--;
        // Restore previous base from history
        const auto& entry = m_imageHistory[m_currentHistoryIndex];
        m_currentBaseImage = entry.image;
        m_detectedFaces = entry.faces;
        
        // Display and re-apply slider parameters on top
        m_splitView->setProcessedImage(m_currentBaseImage);
        m_splitView->setFaceRects(m_detectedFaces);
        updateProcessedImage();
    }
    if (m_undoStack) m_undoStack->undo();
}

} // namespace Glance::UI