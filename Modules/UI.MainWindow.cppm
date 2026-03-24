/**
 * @file UI.MainWindow.cppm
 * @brief Main application window with slider-based image adjustments
 *
 * MainWindow is the primary UI entry point. Provides dual-pane
 * split-screen comparison, slider controls for exposure/contrast/gamma
 * etc., integration with Orchestrator for face detection and processing,
 * undo/redo via EditStack, file loading, LUT application, background
 * removal, and crop-to-faces.
 */
module;
#include <QMainWindow>
#include <QFutureWatcher>
#include <QDialog>
#include <QProgressBar>
#include <QUndoStack>
#include <QPushButton>
#include <QImage>
#include <QString>
#include <QWidget>
#include <vector>

export module UI.MainWindow;

import Core.Orchestrator;
import Core.EditStack;
import Core.ImageProcessor;
import Core.FaceDetector;
import UI.CustomSlider;
import UI.SplitScreenWidget;

namespace Glance::UI {

/** @brief Primary application window with slider controls and split-screen view */
export class MainWindow : public QMainWindow {
public:
    MainWindow(Core::Orchestrator& orchestrator, QWidget* parent = nullptr);
    ~MainWindow();

    void undo();
    void redo();

private:
    void onSliderValueChanged();
    void onRemoveBackgroundTriggered();
    void onLUTTriggered();
    void onCropToFacesTriggered();
    void onDetectFacesTriggered();
    void onImageLoaded(const QImage& image);

private:
    void updateProcessedImage();
    Core::ProcessingParams getSliderParams();
    void resetSlidersSilent();
    void setupMenus();
    void openFile();

    SplitScreenWidget* m_splitView;
    QProgressBar* m_progressBar;
    QUndoStack* m_undoStack;
    QPushButton* m_removeBgBtn;
    QPushButton* m_cropToFacesBtn;
    QPushButton* m_detectFacesBtn;
    QPushButton* m_lutButton;
    
    CustomSlider* m_exposureSlider;
    CustomSlider* m_contrastSlider;
    CustomSlider* m_brightnessSlider;
    CustomSlider* m_saturationSlider;
    CustomSlider* m_temperatureSlider;
    CustomSlider* m_tintSlider;
    CustomSlider* m_vibranceSlider;
    CustomSlider* m_highlightsSlider;
    CustomSlider* m_shadowsSlider;
    CustomSlider* m_detailsSlider;
    CustomSlider* m_gammaSlider;
    
    Core::Orchestrator* m_orchestrator;

    QString m_currentFilePath;
    QImage m_originalImage;
    QImage m_currentBaseImage;
    
    struct HistoryEntry {
        QImage image;
        std::vector<Core::DetectedFace> faces;
    };

    std::vector<HistoryEntry> m_imageHistory;
    std::vector<Core::DetectedFace> m_detectedFaces;
    int m_currentHistoryIndex = -1;
    const int m_maxHistorySize = 20;
    
    bool m_isProcessing = false;
    Core::EditStack m_editStack;
    QString m_lutFilePath;
};

} // namespace Glance::UI
