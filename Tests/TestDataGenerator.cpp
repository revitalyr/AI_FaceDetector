#include "TestDataGenerator.h"
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QConicalGradient>
#include <QRandomGenerator>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>
#include <QJsonArray>

namespace Glance::Testing {

// Static definitions
const QStringList TestDataGenerator::BASIC_PATTERNS = {
    "solid", "checkerboard", "grid", "circles", "rectangles", "lines", "triangles"
};

const QStringList TestDataGenerator::TECHNICAL_PATTERNS = {
    "ruler", "grid_fine", "circles_concentric", "sine_wave", "bar_code", "qr_code", "circuit"
};

const QStringList TestDataGenerator::GRADIENT_TYPES = {
    "linear", "radial", "conical", "rainbow", "grayscale", "hue_shift"
};

const QStringList TestDataGenerator::NOISE_TYPES = {
    "random", "gaussian", "perlin", "salt_pepper", "uniform"
};

TestDataGenerator::TestDataGenerator(QObject *parent)
    : QObject(parent)
    , m_defaultFormat("PNG")
    , m_defaultQuality(90)
{
    // Set default output directory
    m_outputDirectory = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/GlanceTestData";
    
    // Initialize statistics
    m_generationStats["total_images"] = 0;
    m_generationStats["total_datasets"] = 0;
    m_generationStats["start_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
}

TestDataGenerator::~TestDataGenerator()
{
    // Cleanup if needed
}

void TestDataGenerator::setOutputDirectory(const QString& directory)
{
    m_outputDirectory = directory;
    ensureDirectoryExists(m_outputDirectory);
}

void TestDataGenerator::setDefaultFormat(const QString& format)
{
    m_defaultFormat = format.toUpper();
}

void TestDataGenerator::setDefaultQuality(int quality)
{
    m_defaultQuality = qBound(0, quality, 100);
}

bool TestDataGenerator::generateDataset(TestScenario scenario)
{
    TestDataSet dataset;
    
    switch (scenario) {
    case TestScenario::UnitTesting:
        dataset = createUnitTestingDataset();
        break;
    case TestScenario::Integration:
        dataset = createIntegrationDataset();
        break;
    case TestScenario::Performance:
        dataset = createPerformanceDataset();
        break;
    case TestScenario::Stress:
        dataset = createStressDataset();
        break;
    case TestScenario::Regression:
        dataset = createRegressionDataset();
        break;
    case TestScenario::Demo:
        dataset = createDemoDataset();
        break;
    case TestScenario::Benchmark:
        dataset = createBenchmarkDataset();
        break;
    case TestScenario::EdgeCase:
        dataset = createEdgeCaseDataset();
        break;
    default:
        generationError("Unknown test scenario");
        return false;
    }
    
    return generateCustomDataset(dataset);
}

bool TestDataGenerator::generateAllDatasets()
{
    QList<TestScenario> scenarios = {
        TestScenario::UnitTesting,
        TestScenario::Integration,
        TestScenario::Performance,
        TestScenario::Stress,
        TestScenario::Regression,
        TestScenario::Demo,
        TestScenario::Benchmark,
        TestScenario::EdgeCase
    };
    
    bool allSuccess = true;
    
    for (TestScenario scenario : scenarios) {
        if (!generateDataset(scenario)) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool TestDataGenerator::generateCustomDataset(const TestDataSet& dataset)
{
    if (!validateDataset(dataset)) {
        generationError("Invalid dataset specification");
        return false;
    }
    
    QString datasetPath = m_outputDirectory + "/" + dataset.name;
    if (!ensureDirectoryExists(datasetPath)) {
        generationError("Failed to create dataset directory");
        return false;
    }
    
    int successCount = 0;
    int totalCount = dataset.images.size();
    
    for (int i = 0; i < totalCount; ++i) {
        const TestImageSpec& spec = dataset.images[i];
        
        generationProgress(i + 1, totalCount);
        
        QImage image = generateImage(spec);
        if (!image.isNull()) {
            QString imagePath = datasetPath + "/" + sanitizeFileName(spec.name) + "." + spec.format.toLower();
            
            if (saveImage(image, imagePath, spec.format, spec.quality)) {
                m_generatedFiles << imagePath;
                imageGenerated(imagePath, spec);
                successCount++;
            } else {
                generationError(QString("Failed to save image: %1").arg(spec.name));
            }
        } else {
            generationError(QString("Failed to generate image: %1").arg(spec.name));
        }
    }
    
    // Save dataset metadata
    QJsonObject metadata = dataset.metadata;
    metadata["image_count"] = successCount;
    metadata["generated_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QJsonDocument doc(metadata);
    QFile metadataFile(datasetPath + "/metadata.json");
    if (metadataFile.open(QIODevice::WriteOnly)) {
        metadataFile.write(doc.toJson());
    }
    
    datasetGenerated(dataset.name, successCount);
    
    // Update statistics
    m_generationStats["total_images"] = m_generationStats["total_images"].toInt() + successCount;
    m_generationStats["total_datasets"] = m_generationStats["total_datasets"].toInt() + 1;
    
    return successCount == totalCount;
}

QImage TestDataGenerator::generateImage(const TestImageSpec& spec)
{
    if (!validateImageSpec(spec)) {
        return QImage();
    }
    
    QImage image;
    
    switch (spec.type) {
    case TestImageType::Basic:
        image = generateBasicImage(spec.size, spec.parameters);
        break;
    case TestImageType::Portrait:
        image = generatePortraitImage(spec.size, spec.parameters);
        break;
    case TestImageType::Landscape:
        image = generateLandscapeImage(spec.size, spec.parameters);
        break;
    case TestImageType::Technical:
        image = generateTechnicalImage(spec.size, spec.parameters);
        break;
    case TestImageType::Noise:
        image = generateNoiseImage(spec.size, spec.parameters);
        break;
    case TestImageType::Gradient:
        image = generateGradientImage(spec.size, spec.parameters);
        break;
    case TestImageType::HighContrast:
        image = generateHighContrastImage(spec.size, spec.parameters);
        break;
    case TestImageType::LowContrast:
        image = generateLowContrastImage(spec.size, spec.parameters);
        break;
    case TestImageType::Large:
        image = generateLargeImage(spec.size, spec.parameters);
        break;
    case TestImageType::Corrupted:
        image = generateCorruptedImage(spec.size, spec.parameters);
        break;
    case TestImageType::Exotic:
        image = generateExoticImage(spec.size, spec.parameters);
        break;
    default:
        return QImage();
    }
    
    if (!validateImage(image, spec)) {
        return QImage();
    }
    
    return image;
}

QImage TestDataGenerator::generateBasicImage(const QSize& size, const QJsonObject& params)
{
    QImage image(size, QImage::Format_RGB32);
    image.fill(Qt::white);
    
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    QString pattern = params.contains("pattern") ? params["pattern"].toString() : "checkerboard";
    QColor color1 = params.contains("color1") ? QColor(params["color1"].toString()) : QColor("#000000");
    QColor color2 = params.contains("color2") ? QColor(params["color2"].toString()) : QColor("#FFFFFF");
    
    drawBasicPattern(painter, size, pattern);
    
    painter.end();
    return image;
}

QImage TestDataGenerator::generatePortraitImage(const QSize& size, const QJsonObject& params)
{
    QImage image(size, QImage::Format_RGB32);
    image.fill(QColor(params.contains("background") ? params["background"].toString() : "#FFDCB1")); // Skin tone
    
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    drawPortraitFeatures(painter, size, params);
    
    painter.end();
    return image;
}

QImage TestDataGenerator::generateLandscapeImage(const QSize& size, const QJsonObject& params)
{
    QImage image(size, QImage::Format_RGB32);
    
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    drawLandscapeScene(painter, size, params);
    
    painter.end();
    return image;
}

QImage TestDataGenerator::generateTechnicalImage(const QSize& size, const QJsonObject& params)
{
    QImage image(size, QImage::Format_RGB32);
    image.fill(Qt::white);
    
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, false);
    
    QString skinTone = params.contains("skinTone") ? params["skinTone"].toString() : "#FDBCB4";
    drawTechnicalPattern(painter, size, skinTone);
    
    painter.end();
    return image;
}

QImage TestDataGenerator::generateNoiseImage(const QSize& size, const QJsonObject& params)
{
    QImage image(size, QImage::Format_RGB32);
    image.fill(Qt::gray);
    
    double noiseLevel = params.contains("noise_level") ? params["noise_level"].toDouble() : 0.1;
    QString noiseType = params.contains("noise_type") ? params["noise_type"].toString() : "random";
    
    addNoisePattern(image, noiseLevel);
    
    return image;
}

QImage TestDataGenerator::generateGradientImage(const QSize& size, const QJsonObject& params)
{
    QImage image(size, QImage::Format_RGB32);
    
    QString gradientType = params.contains("gradient_type") ? params["gradient_type"].toString() : "linear";
    createGradientPattern(image, gradientType);
    
    return image;
}

QImage TestDataGenerator::generateHighContrastImage(const QSize& size, const QJsonObject& params)
{
    QImage image(size, QImage::Format_RGB32);
    
    // Create high contrast pattern
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // Black and white checkerboard
    int squareSize = 20;
    for (int y = 0; y < size.height(); y += squareSize) {
        for (int x = 0; x < size.width(); x += squareSize) {
            QColor color = ((x / squareSize + y / squareSize) % 2 == 0) ? Qt::black : Qt::white;
            painter.fillRect(x, y, squareSize, squareSize, color);
        }
    }
    
    painter.end();
    return image;
}

QImage TestDataGenerator::generateLowContrastImage(const QSize& size, const QJsonObject& params)
{
    QImage image(size, QImage::Format_RGB32);
    
    // Create very low contrast gradient
    QPainter painter(&image);
    
    QColor baseColor(128, 128, 128); // Medium gray
    image.fill(baseColor);
    
    // Add subtle variation
    for (int y = 0; y < size.height(); ++y) {
        for (int x = 0; x < size.width(); ++x) {
            int variation = (x + y) % 10 - 5; // -5 to +5
            QColor pixel = baseColor;
            pixel.setRed(qBound(0, pixel.red() + variation, 255));
            pixel.setGreen(qBound(0, pixel.green() + variation, 255));
            pixel.setBlue(qBound(0, pixel.blue() + variation, 255));
            image.setPixelColor(x, y, pixel);
        }
    }
    
    painter.end();
    return image;
}

QImage TestDataGenerator::generateLargeImage(const QSize& size, const QJsonObject& params)
{
    // Generate a scaled version of a complex pattern
    QImage baseImage(1000, 1000, QImage::Format_RGB32);
    
    QPainter painter(&baseImage);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    drawLandscapeScene(painter, baseImage.size(), params);
    
    painter.end();
    
    return baseImage.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QImage TestDataGenerator::generateCorruptedImage(const QSize& size, const QJsonObject& params)
{
    // This will be handled specially in saveImage
    QImage image(size, QImage::Format_RGB32);
    image.fill(Qt::red);
    
    // Add "CORRUPTED" text
    QPainter painter(&image);
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(20);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(image.rect(), Qt::AlignCenter, "CORRUPTED");
    painter.end();
    
    return image;
}

QImage TestDataGenerator::generateExoticImage(const QSize& size, const QJsonObject& params)
{
    QImage image(size, QImage::Format_RGB32);
    
    // Create an unusual pattern for edge case testing
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // Rainbow spiral pattern
    QConicalGradient gradient(size.width() / 2, size.height() / 2, 0);
    for (int i = 0; i < 360; i += 10) {
        gradient.setColorAt(i / 360.0, QColor::fromHsv(i, 255, 255));
    }
    
    painter.fillRect(image.rect(), gradient);
    
    // Add some unusual shapes
    painter.setPen(QPen(Qt::black, 2));
    for (int i = 0; i < 20; ++i) {
        int x = QRandomGenerator::global()->bounded(size.width());
        int y = QRandomGenerator::global()->bounded(size.height());
        int radius = QRandomGenerator::global()->bounded(20, 50);
        painter.drawEllipse(QPoint(x, y), radius, radius);
    }
    
    painter.end();
    return image;
}

TestDataSet TestDataGenerator::createUnitTestingDataset()
{
    TestDataSet dataset;
    dataset.name = "unit_testing";
    dataset.description = "Images for unit testing of core functionality";
    dataset.outputPath = m_outputDirectory + "/unit_testing";
    
    // Basic test images
    QList<TestImageSpec> images;
    
    // Small basic images
    images.append({TestImageType::Basic, TestScenario::UnitTesting, QSize(100, 100), 
                   "basic_100x100", "Basic 100x100 test image", 
                   {{"pattern", "checkerboard"}, {"color1", "#000000"}, {"color2", "#FFFFFF"}}, 
                   "PNG", -1});
    
    images.append({TestImageType::Basic, TestScenario::UnitTesting, QSize(200, 200), 
                   "basic_200x200", "Basic 200x200 test image", 
                   {{"pattern", "grid"}, {"color1", "#FF0000"}, {"color2", "#00FF00"}}, 
                   "PNG", -1});
    
    // Portrait images for face detection
    images.append({TestImageType::Portrait, TestScenario::UnitTesting, QSize(300, 400), 
                   "portrait_simple", "Simple portrait for face detection", 
                   {{"complexity", "simple"}}, 
                   "PNG", -1});
    
    images.append({TestImageType::Portrait, TestScenario::UnitTesting, QSize(400, 500), 
                   "portrait_complex", "Complex portrait for face detection", 
                   {{"complexity", "complex"}}, 
                   "PNG", -1});
    
    // Gradient images for processing tests
    images.append({TestImageType::Gradient, TestScenario::UnitTesting, QSize(256, 256), 
                   "gradient_linear", "Linear gradient test image", 
                   {{"gradient_type", "linear"}}, 
                   "PNG", -1});
    
    images.append({TestImageType::Gradient, TestScenario::UnitTesting, QSize(256, 256), 
                   "gradient_radial", "Radial gradient test image", 
                   {{"gradient_type", "radial"}}, 
                   "PNG", -1});
    
    // Technical images
    images.append({TestImageType::Technical, TestScenario::UnitTesting, QSize(500, 500), 
                   "technical_grid", "Technical grid pattern", 
                   {{"pattern", "grid_fine"}, {"spacing", 10}}, 
                   "PNG", -1});
    
    dataset.images = images;
    return dataset;
}

TestDataSet TestDataGenerator::createIntegrationDataset()
{
    TestDataSet dataset;
    dataset.name = "integration";
    dataset.description = "Images for integration testing of complete workflows";
    dataset.outputPath = m_outputDirectory + "/integration";
    
    QList<TestImageSpec> images;
    
    // Real-world scenario images
    images.append({TestImageType::Landscape, TestScenario::Integration, QSize(800, 600), 
                   "landscape_day", "Daytime landscape scene", 
                   {{"time_of_day", "day"}, {"complexity", "medium"}}, 
                   "PNG", -1});
    
    images.append({TestImageType::Landscape, TestScenario::Integration, QSize(800, 600), 
                   "landscape_sunset", "Sunset landscape scene", 
                   {{"time_of_day", "sunset"}, {"complexity", "medium"}}, 
                   "PNG", -1});
    
    images.append({TestImageType::Portrait, TestScenario::Integration, QSize(600, 800), 
                   "portrait_professional", "Professional portrait", 
                   {{"style", "professional"}, {"lighting", "studio"}}, 
                   "PNG", -1});
    
    images.append({TestImageType::Technical, TestScenario::Integration, QSize(1000, 1000), 
                   "technical_schematic", "Technical schematic", 
                   {{"pattern", "circuit"}, {"detail_level", "high"}}, 
                   "PNG", -1});
    
    dataset.images = images;
    return dataset;
}

TestDataSet TestDataGenerator::createPerformanceDataset()
{
    TestDataSet dataset;
    dataset.name = "performance";
    dataset.description = "Images for performance testing and benchmarking";
    dataset.outputPath = m_outputDirectory + "/performance";
    
    QList<TestImageSpec> images;
    
    // Various sizes for performance testing
    QList<QSize> sizes = {
        QSize(500, 500),
        QSize(1000, 1000),
        QSize(2000, 2000),
        QSize(4000, 4000)
    };
    
    for (const QSize& size : sizes) {
        QString sizeStr = QString("%1x%2").arg(size.width()).arg(size.height());
        
        images.append({TestImageType::Landscape, TestScenario::Performance, size, 
                       QString("landscape_%1").arg(sizeStr), 
                       QString("Landscape %1 for performance testing").arg(sizeStr), 
                       {{"complexity", "high"}}, 
                       "PNG", -1});
        
        images.append({TestImageType::Technical, TestScenario::Performance, size, 
                       QString("technical_%1").arg(sizeStr), 
                       QString("Technical %1 for performance testing").arg(sizeStr), 
                       {{"pattern", "grid_fine"}}, 
                       "PNG", -1});
    }
    
    dataset.images = images;
    return dataset;
}

TestDataSet TestDataGenerator::createStressDataset()
{
    TestDataSet dataset;
    dataset.name = "stress";
    dataset.description = "Images for stress testing and memory testing";
    dataset.outputPath = m_outputDirectory + "/stress";
    
    QList<TestImageSpec> images;
    
    // Very large images for stress testing
    QList<QSize> stressSizes = {
        QSize(8000, 6000),
        QSize(10000, 8000),
        QSize(12000, 10000)
    };
    
    for (const QSize& size : stressSizes) {
        QString sizeStr = QString("%1x%2").arg(size.width()).arg(size.height());
        
        images.append({TestImageType::Large, TestScenario::Stress, size, 
                       QString("large_%1").arg(sizeStr), 
                       QString("Large image %1 for stress testing").arg(sizeStr), 
                       {{"complexity", "medium"}}, 
                       "PNG", -1});
    }
    
    // High noise images
    for (int i = 0; i < 10; ++i) {
        double noiseLevel = 0.1 + i * 0.1;
        
        images.append({TestImageType::Noise, TestScenario::Stress, QSize(2000, 2000), 
                       QString("noise_%1").arg(i + 1), 
                       QString("Noise image %1 with level %2").arg(i + 1).arg(noiseLevel), 
                       {{"noise_level", noiseLevel}, {"noise_type", "random"}}, 
                       "PNG", -1});
    }
    
    dataset.images = images;
    return dataset;
}

TestDataSet TestDataGenerator::createRegressionDataset()
{
    TestDataSet dataset;
    dataset.name = "regression";
    dataset.description = "Reference images for regression testing";
    dataset.outputPath = m_outputDirectory + "/regression";
    
    QList<TestImageSpec> images;
    
    // Standardized reference images
    images.append({TestImageType::Basic, TestScenario::Regression, QSize(512, 512), 
                   "reference_basic", "Reference basic pattern", 
                   {{"pattern", "checkerboard"}, {"seed", 42}}, 
                   "PNG", -1});
    
    images.append({TestImageType::Gradient, TestScenario::Regression, QSize(512, 512), 
                   "reference_gradient", "Reference gradient pattern", 
                   {{"gradient_type", "linear"}, {"seed", 42}}, 
                   "PNG", -1});
    
    images.append({TestImageType::Portrait, TestScenario::Regression, QSize(512, 512), 
                   "reference_portrait", "Reference portrait", 
                   {{"complexity", "standard"}, {"seed", 42}}, 
                   "PNG", -1});
    
    images.append({TestImageType::Technical, TestScenario::Regression, QSize(512, 512), 
                   "reference_technical", "Reference technical pattern", 
                   {{"pattern", "grid"}, {"seed", 42}}, 
                   "PNG", -1});
    
    dataset.images = images;
    return dataset;
}

TestDataSet TestDataGenerator::createDemoDataset()
{
    TestDataSet dataset;
    dataset.name = "demo";
    dataset.description = "High-quality images for demo application";
    dataset.outputPath = m_outputDirectory + "/demo";
    
    QList<TestImageSpec> images;
    
    // Beautiful demo images
    images.append({TestImageType::Landscape, TestScenario::Demo, QSize(1920, 1080), 
                   "demo_landscape_sunset", "Beautiful sunset landscape", 
                   {{"time_of_day", "sunset"}, {"quality", "high"}, {"detail_level", "ultra"}}, 
                   "PNG", -1});
    
    images.append({TestImageType::Landscape, TestScenario::Demo, QSize(1920, 1080), 
                   "demo_landscape_mountain", "Mountain landscape", 
                   {{"scene", "mountains"}, {"quality", "high"}, {"detail_level", "ultra"}}, 
                   "PNG", -1});
    
    images.append({TestImageType::Portrait, TestScenario::Demo, QSize(800, 1000), 
                   "demo_portrait_studio", "Studio portrait", 
                   {{"style", "professional"}, {"lighting", "studio"}, {"quality", "high"}}, 
                   "PNG", -1});
    
    images.append({TestImageType::Technical, TestScenario::Demo, QSize(1600, 1200), 
                   "demo_technical_blueprint", "Technical blueprint", 
                   {{"style", "blueprint"}, {"detail_level", "high"}, {"quality", "high"}}, 
                   "PNG", -1});
    
    images.append({TestImageType::Gradient, TestScenario::Demo, QSize(1024, 1024), 
                   "demo_gradient_rainbow", "Rainbow gradient", 
                   {{"gradient_type", "rainbow"}, {"quality", "high"}}, 
                   "PNG", -1});
    
    dataset.images = images;
    return dataset;
}

TestDataSet TestDataGenerator::createBenchmarkDataset()
{
    TestDataSet dataset;
    dataset.name = "benchmark";
    dataset.description = "Standardized images for benchmarking";
    dataset.outputPath = m_outputDirectory + "/benchmark";
    
    QList<TestImageSpec> images;
    
    // Standard benchmark sizes (based on common image processing benchmarks)
    QList<QSize> benchmarkSizes = {
        QSize(640, 480),   // VGA
        QSize(1280, 720),  // HD
        QSize(1920, 1080), // Full HD
        QSize(2560, 1440), // 2K
        QSize(3840, 2160)  // 4K
    };
    
    for (const QSize& size : benchmarkSizes) {
        QString sizeName;
        if (size == QSize(640, 480)) sizeName = "vga";
        else if (size == QSize(1280, 720)) sizeName = "hd";
        else if (size == QSize(1920, 1080)) sizeName = "fhd";
        else if (size == QSize(2560, 1440)) sizeName = "2k";
        else if (size == QSize(3840, 2160)) sizeName = "4k";
        else sizeName = QString("%1x%2").arg(size.width()).arg(size.height());
        
        images.append({TestImageType::Landscape, TestScenario::Benchmark, size, 
                       QString("benchmark_landscape_%1").arg(sizeName), 
                       QString("Benchmark landscape %1").arg(sizeName.toUpper()), 
                       {{"standard", "true"}, {"complexity", "medium"}}, 
                       "PNG", -1});
        
        images.append({TestImageType::Technical, TestScenario::Benchmark, size, 
                       QString("benchmark_technical_%1").arg(sizeName), 
                       QString("Benchmark technical %1").arg(sizeName.toUpper()), 
                       {{"standard", "true"}, {"pattern", "grid_fine"}}, 
                       "PNG", -1});
    }
    
    dataset.images = images;
    return dataset;
}

TestDataSet TestDataGenerator::createEdgeCaseDataset()
{
    TestDataSet dataset;
    dataset.name = "edge_case";
    dataset.description = "Edge case images for robustness testing";
    dataset.outputPath = m_outputDirectory + "/edge_case";
    
    QList<TestImageSpec> images;
    
    // Edge case images
    images.append({TestImageType::HighContrast, TestScenario::EdgeCase, QSize(100, 100), 
                   "edge_high_contrast", "High contrast edge case", 
                   {{"contrast_level", "maximum"}}, 
                   "PNG", -1});
    
    images.append({TestImageType::LowContrast, TestScenario::EdgeCase, QSize(100, 100), 
                   "edge_low_contrast", "Low contrast edge case", 
                   {{"contrast_level", "minimum"}}, 
                   "PNG", -1});
    
    images.append({TestImageType::Noise, TestScenario::EdgeCase, QSize(100, 100), 
                   "edge_noise_high", "High noise edge case", 
                   {{"noise_level", 0.9}, {"noise_type", "random"}}, 
                   "PNG", -1});
    
    images.append({TestImageType::Exotic, TestScenario::EdgeCase, QSize(100, 100), 
                   "edge_exotic_pattern", "Exotic pattern edge case", 
                   {{"pattern_type", "spiral"}, {"complexity", "high"}}, 
                   "PNG", -1});
    
    // Single color images
    images.append({TestImageType::Basic, TestScenario::EdgeCase, QSize(100, 100), 
                   "edge_single_color", "Single color edge case", 
                   {{"pattern", "solid"}, {"color", "#FF0000"}}, 
                   "PNG", -1});
    
    // Corrupted image (will be handled specially)
    images.append({TestImageType::Corrupted, TestScenario::EdgeCase, QSize(100, 100), 
                   "edge_corrupted", "Corrupted image edge case", 
                   {{"corruption_type", "header"}}, 
                   "PNG", -1});
    
    dataset.images = images;
    return dataset;
}

bool TestDataGenerator::saveImage(const QImage& image, const QString& path, const QString& format, int quality)
{
    if (path.contains("corrupted")) {
        return createCorruptedFile(path, format);
    }
    
    ensureDirectoryExists(QFileInfo(path).absolutePath());
    
    bool success = image.save(path, format.toUtf8().constData(), quality);
    if (!success) {
        qDebug() << "Failed to save image:" << path;
    }
    
    return success;
}

bool TestDataGenerator::createCorruptedFile(const QString& path, const QString& format)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    // Write invalid image data
    QByteArray corruptedData = "This is not a valid image file - corrupted for testing";
    file.write(corruptedData);
    file.close();
    
    return true;
}

QString TestDataGenerator::generateImagePath(const TestImageSpec& spec)
{
    return QString("%1/%2/%3.%4")
           .arg(m_outputDirectory)
           .arg(spec.scenario == TestScenario::UnitTesting ? "unit_testing" : "test")
           .arg(sanitizeFileName(spec.name))
           .arg(spec.format.toLower());
}

QJsonObject TestDataGenerator::generateImageMetadata(const QImage& image, const TestImageSpec& spec)
{
    QJsonObject metadata;
    metadata["name"] = spec.name;
    metadata["type"] = QString::number(static_cast<int>(spec.type));
    metadata["scenario"] = QString::number(static_cast<int>(spec.scenario));
    metadata["width"] = image.width();
    metadata["height"] = image.height();
    metadata["format"] = spec.format;
    metadata["size_bytes"] = image.sizeInBytes();
    metadata["has_alpha"] = image.hasAlphaChannel();
    metadata["color_count"] = image.colorCount();
    metadata["depth"] = image.depth();
    metadata["generated_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    return metadata;
}

bool TestDataGenerator::generateBatch(const QList<TestImageSpec>& specs)
{
    int successCount = 0;
    
    for (const TestImageSpec& spec : specs) {
        QImage image = generateImage(spec);
        if (!image.isNull()) {
            QString path = generateImagePath(spec);
            if (saveImage(image, path, spec.format, spec.quality)) {
                successCount++;
            }
        }
    }
    
    return successCount == specs.size();
}

bool TestDataGenerator::validateDataset(const TestDataSet& dataset)
{
    if (dataset.name.isEmpty() || dataset.images.isEmpty()) {
        return false;
    }
    
    for (const TestImageSpec& spec : dataset.images) {
        if (!validateImageSpec(spec)) {
            return false;
        }
    }
    
    return true;
}

QJsonObject TestDataGenerator::getGenerationStatistics()
{
    QJsonObject stats = m_generationStats;
    stats["generated_files"] = QJsonArray::fromStringList(m_generatedFiles);
    stats["end_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    return stats;
}

QStringList TestDataGenerator::getGeneratedFiles()
{
    return m_generatedFiles;
}

void TestDataGenerator::clearGeneratedFiles()
{
    for (const QString& file : m_generatedFiles) {
        QFile::remove(file);
    }
    
    m_generatedFiles.clear();
    
    // Reset statistics
    m_generationStats["total_images"] = 0;
    m_generationStats["total_datasets"] = 0;
    m_generationStats["start_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
}

// Private helper methods

void TestDataGenerator::drawBasicPattern(QPainter& painter, const QSize& size, const QString& pattern)
{
    if (pattern == "checkerboard") {
        drawCheckerboard(painter, QRect(0, 0, size.width(), size.height()), 20);
    } else if (pattern == "grid") {
        drawGrid(painter, QRect(0, 0, size.width(), size.height()), 20);
    } else if (pattern == "circles") {
        drawCircles(painter, QRect(0, 0, size.width(), size.height()), 10);
    } else if (pattern == "rectangles") {
        drawRectangles(painter, QRect(0, 0, size.width(), size.height()), 8);
    } else if (pattern == "lines") {
        drawLines(painter, QRect(0, 0, size.width(), size.height()), 15);
    } else if (pattern == "triangles") {
        // Draw triangles
        painter.setPen(QPen(Qt::black, 2));
        for (int i = 0; i < 10; ++i) {
            int x = QRandomGenerator::global()->bounded(size.width() - 100);
            int y = QRandomGenerator::global()->bounded(size.height() - 100);
            QPolygon triangle;
            triangle << QPoint(x + 50, y) << QPoint(x, y + 100) << QPoint(x + 100, y + 100);
            painter.drawPolygon(triangle);
        }
    }
}

void TestDataGenerator::drawPortraitFeatures(QPainter& painter, const QSize& size, const QJsonObject& params)
{
    QString complexity = params.contains("complexity") ? params["complexity"].toString() : "simple";
    
    if (complexity == "simple") {
        // Simple face features
        painter.setBrush(QBrush(QColor(50, 50, 50)));
        painter.setPen(Qt::NoPen);
        
        // Eyes
        painter.drawEllipse(size.width() * 0.3, size.height() * 0.3, size.width() * 0.15, size.height() * 0.1);
        painter.drawEllipse(size.width() * 0.55, size.height() * 0.3, size.width() * 0.15, size.height() * 0.1);
        
        // Nose
        painter.drawEllipse(size.width() * 0.45, size.height() * 0.5, size.width() * 0.1, size.height() * 0.15);
        
        // Mouth
        painter.drawEllipse(size.width() * 0.35, size.height() * 0.7, size.width() * 0.3, size.height() * 0.08);
        
    } else if (complexity == "complex") {
        // More detailed face with shading
        // Face outline
        painter.setBrush(QBrush(QColor(200, 180, 160)));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(size.width() * 0.5, size.height() * 0.5, size.width() * 0.4, size.height() * 0.5);
        
        // Eyes with detail
        painter.setBrush(QBrush(QColor(50, 50, 50)));
        painter.drawEllipse(size.width() * 0.35, size.height() * 0.4, size.width() * 0.08, size.height() * 0.06);
        painter.drawEllipse(size.width() * 0.57, size.height() * 0.4, size.width() * 0.08, size.height() * 0.06);
        
        // Eye whites
        painter.setBrush(QBrush(QColor(255, 255, 255)));
        painter.drawEllipse(size.width() * 0.36, size.height() * 0.41, size.width() * 0.04, size.height() * 0.03);
        painter.drawEllipse(size.width() * 0.58, size.height() * 0.41, size.width() * 0.04, size.height() * 0.03);
        
        // Pupils
        painter.setBrush(QBrush(QColor(0, 0, 0)));
        painter.drawEllipse(size.width() * 0.37, size.height() * 0.42, size.width() * 0.02, size.height() * 0.02);
        painter.drawEllipse(size.width() * 0.59, size.height() * 0.42, size.width() * 0.02, size.height() * 0.02);
        
        // Nose with shading
        painter.setBrush(QBrush(QColor(180, 160, 140)));
        painter.drawEllipse(size.width() * 0.48, size.height() * 0.55, size.width() * 0.06, size.height() * 0.1);
        
        // Mouth
        painter.setBrush(QBrush(QColor(150, 50, 50)));
        painter.drawEllipse(size.width() * 0.4, size.height() * 0.75, size.width() * 0.2, size.height() * 0.05);
        
        // Hair
        painter.setBrush(QBrush(QColor(80, 60, 40)));
        painter.drawEllipse(size.width() * 0.5, size.height() * 0.25, size.width() * 0.35, size.height() * 0.2);
    }
}

void TestDataGenerator::drawLandscapeScene(QPainter& painter, const QSize& size, const QJsonObject& params)
{
    QString timeOfDay = params.contains("time_of_day") ? params["time_of_day"].toString() : "day";
    
    // Sky
    if (timeOfDay == "day") {
        QLinearGradient skyGradient(0, 0, 0, size.height() * 0.6);
        skyGradient.setColorAt(0, QColor(135, 206, 235)); // Sky blue
        skyGradient.setColorAt(1, QColor(255, 255, 200)); // Light yellow
        painter.fillRect(0, 0, size.width(), size.height() * 0.6, skyGradient);
        
        // Sun
        painter.setBrush(QBrush(QColor(255, 255, 0)));
        painter.drawEllipse(size.width() * 0.8, size.height() * 0.15, size.width() * 0.1, size.height() * 0.1);
        
    } else if (timeOfDay == "sunset") {
        QLinearGradient skyGradient(0, 0, 0, size.height() * 0.6);
        skyGradient.setColorAt(0, QColor(255, 94, 77));   // Orange-red
        skyGradient.setColorAt(0.5, QColor(255, 154, 0));  // Orange
        skyGradient.setColorAt(1, QColor(255, 206, 84));  // Yellow
        painter.fillRect(0, 0, size.width(), size.height() * 0.6, skyGradient);
        
        // Setting sun
        painter.setBrush(QBrush(QColor(255, 200, 0)));
        painter.drawEllipse(size.width() * 0.5, size.height() * 0.4, size.width() * 0.15, size.height() * 0.15);
    }
    
    // Mountains
    painter.setBrush(QBrush(QColor(139, 69, 19))); // Brown
    QPolygon mountains;
    mountains << QPoint(0, size.height() * 0.6)
              << QPoint(size.width() * 0.2, size.height() * 0.3)
              << QPoint(size.width() * 0.4, size.height() * 0.5)
              << QPoint(size.width() * 0.6, size.height() * 0.2)
              << QPoint(size.width() * 0.8, size.height() * 0.4)
              << QPoint(size.width(), size.height() * 0.6);
    painter.drawPolygon(mountains);
    
    // Ground
    painter.setBrush(QBrush(QColor(34, 139, 34))); // Forest green
    painter.drawRect(0, size.height() * 0.6, size.width(), size.height() * 0.4);
    
    // Trees
    painter.setBrush(QBrush(QColor(0, 100, 0))); // Dark green
    for (int i = 0; i < 5; ++i) {
        int x = size.width() * (0.1 + i * 0.15);
        painter.drawEllipse(x, size.height() * 0.5, size.width() * 0.08, size.height() * 0.15);
        painter.drawRect(x + size.width() * 0.03, size.height() * 0.6, size.width() * 0.02, size.height() * 0.1);
    }
}

void TestDataGenerator::drawTechnicalPattern(QPainter& painter, const QSize& size, const QString& pattern)
{
    if (pattern == "grid_fine") {
        drawGrid(painter, QRect(0, 0, size.width(), size.height()), 10);
    } else if (pattern == "circles_concentric") {
        painter.setPen(QPen(Qt::black, 1));
        QPoint center(size.width() / 2, size.height() / 2);
        for (int r = 10; r < qMin(size.width(), size.height()) / 2; r += 20) {
            painter.drawEllipse(center, r, r);
        }
    } else if (pattern == "sine_wave") {
        painter.setPen(QPen(Qt::black, 2));
        for (int x = 0; x < size.width(); ++x) {
            double y = size.height() / 2 + sin(x * 0.05) * size.height() * 0.3;
            if (x > 0) {
                double prevY = size.height() / 2 + sin((x-1) * 0.05) * size.height() * 0.3;
                painter.drawLine(x-1, prevY, x, y);
            }
        }
    } else if (pattern == "ruler") {
        painter.setPen(QPen(Qt::black, 1));
        drawGrid(painter, QRect(0, 0, size.width(), size.height()), 50);
        
        // Add numbers
        QFont font = painter.font();
        font.setPointSize(8);
        painter.setFont(font);
        
        for (int x = 50; x < size.width(); x += 50) {
            painter.drawText(x - 10, size.height() - 5, QString::number(x));
        }
        
        for (int y = 50; y < size.height(); y += 50) {
            painter.drawText(5, y + 3, QString::number(y));
        }
    }
}

void TestDataGenerator::addNoisePattern(QImage& image, double noiseLevel)
{
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (QRandomGenerator::global()->bounded(1.0) < noiseLevel) {
                int noise = QRandomGenerator::global()->bounded(256);
                QColor color(noise, noise, noise);
                image.setPixelColor(x, y, color);
            }
        }
    }
}

void TestDataGenerator::createGradientPattern(QImage& image, const QString& gradientType)
{
    QPainter painter(&image);
    
    if (gradientType == "linear") {
        QLinearGradient gradient(0, 0, image.width(), image.height());
        gradient.setColorAt(0, Qt::red);
        gradient.setColorAt(0.5, Qt::green);
        gradient.setColorAt(1, Qt::blue);
        painter.fillRect(image.rect(), gradient);
        
    } else if (gradientType == "radial") {
        QRadialGradient gradient(image.width() / 2, image.height() / 2, qMin(image.width(), image.height()) / 2);
        gradient.setColorAt(0, Qt::white);
        gradient.setColorAt(0.5, Qt::gray);
        gradient.setColorAt(1, Qt::black);
        painter.fillRect(image.rect(), gradient);
        
    } else if (gradientType == "conical") {
        QConicalGradient gradient(image.width() / 2, image.height() / 2, 0);
        for (int i = 0; i < 360; i += 60) {
            gradient.setColorAt(i / 360.0, QColor::fromHsv(i, 255, 255));
        }
        painter.fillRect(image.rect(), gradient);
        
    } else if (gradientType == "rainbow") {
        QLinearGradient gradient(0, 0, image.width(), 0);
        gradient.setColorAt(0.0, QColor(255, 0, 0));    // Red
        gradient.setColorAt(0.17, QColor(255, 165, 0)); // Orange
        gradient.setColorAt(0.33, QColor(255, 255, 0)); // Yellow
        gradient.setColorAt(0.5, QColor(0, 255, 0));    // Green
        gradient.setColorAt(0.67, QColor(0, 0, 255));   // Blue
        gradient.setColorAt(0.83, QColor(75, 0, 130));  // Indigo
        gradient.setColorAt(1.0, QColor(238, 130, 238)); // Violet
        painter.fillRect(image.rect(), gradient);
        
    } else if (gradientType == "grayscale") {
        QLinearGradient gradient(0, 0, image.width(), image.height());
        gradient.setColorAt(0, Qt::black);
        gradient.setColorAt(1, Qt::white);
        painter.fillRect(image.rect(), gradient);
    }
}

void TestDataGenerator::adjustContrast(QImage& image, double contrastLevel)
{
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor color = image.pixelColor(x, y);
            
            // Adjust contrast
            int r = qBound(0, (int)((color.red() - 128) * contrastLevel + 128), 255);
            int g = qBound(0, (int)((color.green() - 128) * contrastLevel + 128), 255);
            int b = qBound(0, (int)((color.blue() - 128) * contrastLevel + 128), 255);
            
            image.setPixelColor(x, y, QColor(r, g, b));
        }
    }
}

QColor TestDataGenerator::generateRandomColor()
{
    return QColor(QRandomGenerator::global()->bounded(256),
                  QRandomGenerator::global()->bounded(256),
                  QRandomGenerator::global()->bounded(256));
}

QColor TestDataGenerator::generateSkinTone()
{
    // Generate realistic skin tones
    QList<QColor> skinTones = {
        QColor(255, 220, 177), // Light
        QColor(245, 200, 150), // Medium-light
        QColor(220, 180, 140), // Medium
        QColor(180, 140, 110), // Medium-dark
        QColor(140, 100, 80)   // Dark
    };
    
    return skinTones[QRandomGenerator::global()->bounded(skinTones.size())];
}

QColor TestDataGenerator::generateNaturalColor()
{
    // Generate colors found in nature
    QList<QColor> naturalColors = {
        QColor(34, 139, 34),   // Forest green
        QColor(135, 206, 235), // Sky blue
        QColor(139, 69, 19),   // Brown
        QColor(255, 215, 0),   // Gold
        QColor(128, 128, 128), // Gray
        QColor(255, 255, 255), // White
        QColor(0, 0, 0)        // Black
    };
    
    return naturalColors[QRandomGenerator::global()->bounded(naturalColors.size())];
}

QList<QColor> TestDataGenerator::generateColorPalette(int count)
{
    QList<QColor> palette;
    
    for (int i = 0; i < count; ++i) {
        QColor color = QColor::fromHsv((i * 360) / count, 255, 255);
        palette << color;
    }
    
    return palette;
}

void TestDataGenerator::drawCheckerboard(QPainter& painter, const QRect& rect, int squareSize)
{
    QColor color1(Qt::black);
    QColor color2(Qt::white);
    
    for (int y = rect.top(); y < rect.bottom(); y += squareSize) {
        for (int x = rect.left(); x < rect.right(); x += squareSize) {
            QColor color = ((x / squareSize + y / squareSize) % 2 == 0) ? color1 : color2;
            painter.fillRect(x, y, squareSize, squareSize, color);
        }
    }
}

void TestDataGenerator::drawGrid(QPainter& painter, const QRect& rect, int spacing)
{
    painter.setPen(QPen(Qt::black, 1));
    
    for (int x = rect.left(); x <= rect.right(); x += spacing) {
        painter.drawLine(x, rect.top(), x, rect.bottom());
    }
    
    for (int y = rect.top(); y <= rect.bottom(); y += spacing) {
        painter.drawLine(rect.left(), y, rect.right(), y);
    }
}

void TestDataGenerator::drawCircles(QPainter& painter, const QRect& rect, int count)
{
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(Qt::NoBrush);
    
    for (int i = 0; i < count; ++i) {
        int x = QRandomGenerator::global()->bounded(rect.left(), rect.right());
        int y = QRandomGenerator::global()->bounded(rect.top(), rect.bottom());
        int radius = QRandomGenerator::global()->bounded(10, 50);
        
        painter.drawEllipse(QPoint(x, y), radius, radius);
    }
}

void TestDataGenerator::drawRectangles(QPainter& painter, const QRect& rect, int count)
{
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(Qt::NoBrush);
    
    for (int i = 0; i < count; ++i) {
        int x = QRandomGenerator::global()->bounded(rect.left(), rect.right() - 50);
        int y = QRandomGenerator::global()->bounded(rect.top(), rect.bottom() - 50);
        int width = QRandomGenerator::global()->bounded(20, 100);
        int height = QRandomGenerator::global()->bounded(20, 100);
        
        painter.drawRect(x, y, width, height);
    }
}

void TestDataGenerator::drawLines(QPainter& painter, const QRect& rect, int count)
{
    painter.setPen(QPen(Qt::black, 2));
    
    for (int i = 0; i < count; ++i) {
        int x1 = QRandomGenerator::global()->bounded(rect.left(), rect.right());
        int y1 = QRandomGenerator::global()->bounded(rect.top(), rect.bottom());
        int x2 = QRandomGenerator::global()->bounded(rect.left(), rect.right());
        int y2 = QRandomGenerator::global()->bounded(rect.top(), rect.bottom());
        
        painter.drawLine(x1, y1, x2, y2);
    }
}

bool TestDataGenerator::ensureDirectoryExists(const QString& path)
{
    QDir dir(path);
    return dir.mkpath(path);
}

QString TestDataGenerator::sanitizeFileName(const QString& name)
{
    QString sanitized = name;
    sanitized.replace(QRegularExpression("[^a-zA-Z0-9_-]"), "_");
    return sanitized;
}

bool TestDataGenerator::validateImageSpec(const TestImageSpec& spec)
{
    if (spec.size.isEmpty() || spec.size.width() <= 0 || spec.size.height() <= 0) {
        return false;
    }
    
    if (spec.name.isEmpty()) {
        return false;
    }
    
    if (spec.format.isEmpty()) {
        return false;
    }
    
    return true;
}

bool TestDataGenerator::validateImage(const QImage& image, const TestImageSpec& spec)
{
    if (image.isNull()) {
        return false;
    }
    
    if (image.size() != spec.size) {
        return false;
    }
    
    return true;
}

} // namespace Glance::Testing
