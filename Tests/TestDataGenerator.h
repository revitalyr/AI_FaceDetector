#pragma once

#include <QObject>
#include <QImage>
#include <QString>
#include <QDir>
#include <QSize>
#include <QColor>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

namespace Glance::Testing {

enum class TestImageType {
    Basic,           // Simple geometric patterns
    Portrait,        // Face-like images for face detection
    Landscape,       // Nature scenes
    Technical,       // Technical drawings and patterns
    Noise,           // Noise patterns for edge cases
    Gradient,        // Color gradients
    HighContrast,    // High contrast images
    LowContrast,     // Low contrast images
    Large,           // Large resolution images
    Corrupted,       // Intentionally corrupted data
    Exotic          // Unusual formats and edge cases
};

enum class TestScenario {
    UnitTesting,     // Images for unit tests
    Integration,     // Images for integration tests
    Performance,     // Images for performance testing
    Stress,          // Images for stress testing
    Regression,      // Images for regression testing
    Demo,            // Images for demo application
    Benchmark,       // Images for benchmarking
    EdgeCase         // Images for edge case testing
};

struct TestImageSpec {
    TestImageType type;
    TestScenario scenario;
    QSize size;
    QString name;
    QString description;
    QJsonObject parameters;
    QString format; // PNG, JPEG, BMP, etc.
    int quality; // For JPEG compression
};

struct TestDataSet {
    QString name;
    QString description;
    QList<TestImageSpec> images;
    QString outputPath;
    QJsonObject metadata;
};

class TestDataGenerator : public QObject
{

public:
    explicit TestDataGenerator(QObject *parent = nullptr);
    ~TestDataGenerator();

    // Configuration
    void setOutputDirectory(const QString& directory);
    void setDefaultFormat(const QString& format);
    void setDefaultQuality(int quality);
    
    // Dataset generation
    bool generateDataset(TestScenario scenario);
    bool generateAllDatasets();
    bool generateCustomDataset(const TestDataSet& dataset);
    
    // Individual image generation
    QImage generateImage(const TestImageSpec& spec);
    QImage generateBasicImage(const QSize& size, const QJsonObject& params);
    QImage generatePortraitImage(const QSize& size, const QJsonObject& params);
    QImage generateLandscapeImage(const QSize& size, const QJsonObject& params);
    QImage generateTechnicalImage(const QSize& size, const QJsonObject& params);
    QImage generateNoiseImage(const QSize& size, const QJsonObject& params);
    QImage generateGradientImage(const QSize& size, const QJsonObject& params);
    QImage generateHighContrastImage(const QSize& size, const QJsonObject& params);
    QImage generateLowContrastImage(const QSize& size, const QJsonObject& params);
    QImage generateLargeImage(const QSize& size, const QJsonObject& params);
    QImage generateCorruptedImage(const QSize& size, const QJsonObject& params);
    QImage generateExoticImage(const QSize& size, const QJsonObject& params);
    
    // Predefined datasets
    TestDataSet createUnitTestingDataset();
    TestDataSet createIntegrationDataset();
    TestDataSet createPerformanceDataset();
    TestDataSet createStressDataset();
    TestDataSet createRegressionDataset();
    TestDataSet createDemoDataset();
    TestDataSet createBenchmarkDataset();
    TestDataSet createEdgeCaseDataset();
    
    // Utility functions
    bool saveImage(const QImage& image, const QString& path, const QString& format, int quality = -1);
    bool createCorruptedFile(const QString& path, const QString& format);
    QString generateImagePath(const TestImageSpec& spec);
    QJsonObject generateImageMetadata(const QImage& image, const TestImageSpec& spec);
    
    // Batch operations
    bool generateBatch(const QList<TestImageSpec>& specs);
    bool validateDataset(const TestDataSet& dataset);
    
    // Statistics and reporting
    QJsonObject getGenerationStatistics();
    QStringList getGeneratedFiles();
    void clearGeneratedFiles();

private:
    // Image generation helpers
    void drawBasicPattern(QPainter& painter, const QSize& size, const QString& pattern);
    void drawPortraitFeatures(QPainter& painter, const QSize& size, const QJsonObject& params);
    void drawLandscapeScene(QPainter& painter, const QSize& size, const QJsonObject& params);
    void drawTechnicalPattern(QPainter& painter, const QSize& size, const QString& pattern);
    void addNoisePattern(QImage& image, double noiseLevel = 0.1);
    void createGradientPattern(QImage& image, const QString& gradientType);
    void adjustContrast(QImage& image, double contrastLevel);
    
    // Color utilities
    QColor generateRandomColor();
    QColor generateSkinTone();
    QColor generateNaturalColor();
    QList<QColor> generateColorPalette(int count);
    
    // Pattern utilities
    void drawCheckerboard(QPainter& painter, const QRect& rect, int squareSize);
    void drawGrid(QPainter& painter, const QRect& rect, int spacing);
    void drawCircles(QPainter& painter, const QRect& rect, int count);
    void drawRectangles(QPainter& painter, const QRect& rect, int count);
    void drawLines(QPainter& painter, const QRect& rect, int count);
    
    // File operations
    bool ensureDirectoryExists(const QString& path);
    QString sanitizeFileName(const QString& name);
    
    // Validation
    bool validateImageSpec(const TestImageSpec& spec);
    bool validateImage(const QImage& image, const TestImageSpec& spec);
    
    // Data members
    QString m_outputDirectory;
    QString m_defaultFormat;
    int m_defaultQuality;
    
    QJsonObject m_generationStats;
    QStringList m_generatedFiles;
    
    // Predefined patterns and colors
    static const QStringList BASIC_PATTERNS;
    static const QStringList TECHNICAL_PATTERNS;
    static const QStringList GRADIENT_TYPES;
    static const QStringList NOISE_TYPES;
};

} // namespace Glance::Testing
