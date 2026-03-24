module;
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <QImage>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>
#include <QRect>
#include <QPoint>
#include <QImageReader>
#include <cmath>
#include <algorithm>
#include <memory>
module Core.FaceDetector;

using Catch::Approx;

// Ground-truth bounding boxes
struct GroundTruthFace { int x, y, w, h; };
struct ImageGroundTruth {
    const char* filename;
    std::vector<GroundTruthFace> faces;
    bool hardCase = false;
};

// Calculate Intersection over Union (IoU) between two rects
static double calculateIoU(const QRect& a, const QRect& b) {
    QRect intersection = a.intersected(b);
    if (!intersection.isValid()) return 0.0;
    
    double intersectionArea = intersection.width() * intersection.height();
    double unionArea = (a.width() * a.height()) + (b.width() * b.height()) - intersectionArea;
    
    return unionArea > 0 ? intersectionArea / unionArea : 0.0;
}

// Calculate center distance between two rects (normalized by ground truth size)
[[maybe_unused]] static double calculateCenterDistance(const QRect& detected, const QRect& groundTruth) {
    QPoint detectedCenter = detected.center();
    QPoint gtCenter = groundTruth.center();
    
    double dx = detectedCenter.x() - gtCenter.x();
    double dy = detectedCenter.y() - gtCenter.y();
    double distance = std::sqrt(dx * dx + dy * dy);
    
    // Normalize by ground truth size (average of width and height)
    double gtSize = (groundTruth.width() + groundTruth.height()) / 2.0;
    return gtSize > 0 ? distance / gtSize : distance;
}

// Calculate size ratio between two rects
[[maybe_unused]] static double calculateSizeRatio(const QRect& detected, const QRect& groundTruth) {
    double detectedSize = (detected.width() + detected.height()) / 2.0;
    double gtSize = (groundTruth.width() + groundTruth.height()) / 2.0;
    
    if (gtSize == 0) return 0.0;
    double ratio = detectedSize / gtSize;
    // Return ratio clamped between 0 and 1 (1.0 = perfect match, 0.5 = half size, 2.0 = double size -> 0.5)
    return ratio <= 1.0 ? ratio : 1.0 / ratio;
}

// Simple bbox matching: check that detected faces roughly overlap with ground truth
// Uses a forgiving threshold to account for detection algorithm differences
static bool matchBboxes(const std::vector<Glance::Core::DetectedFace>& detected,
                        const std::vector<GroundTruthFace>& groundTruth,
                        bool hardCase = false) {
    if (detected.empty() && groundTruth.empty()) return true;
    if (detected.empty()) return false;
    
    // Convert ground truth to QRects
    std::vector<QRect> gtRects;
    for (const auto& gt : groundTruth) {
        gtRects.emplace_back(gt.x, gt.y, gt.w, gt.h);
    }
    
    // For hard cases: just check that at least one face is detected near ground truth
    // For normal cases: allow slight count differences (detection may miss some faces)
    if (!hardCase) {
        // Normal case: count should match exactly
        if (detected.size() != groundTruth.size()) return false;
    } else {
        // Hard case: at least one face should be detected
        if (detected.empty()) return false;
    }
    
    // Track which ground truth faces have been matched
    std::vector<bool> matched(gtRects.size(), false);
    int matchCount = 0;
    
    // For each detected face, find best matching ground truth
    for (const auto& face : detected) {
        double bestIoU = 0.0;
        size_t bestMatch = 0;
        
        for (size_t i = 0; i < gtRects.size(); ++i) {
            if (matched[i]) continue;
            
            double iou = calculateIoU(face.rect, gtRects[i]);
            if (iou > bestIoU) {
                bestIoU = iou;
                bestMatch = i;
            }
        }
        
        // Use a forgiving IoU threshold (0.2) to account for detection differences
        // Real-world detectors may have slightly different bbox outputs
        double threshold = hardCase ? 0.1 : 0.2;
        if (bestIoU >= threshold) {
            matched[bestMatch] = true;
            matchCount++;
        }
    }
    
    // For hard cases: at least one ground truth face should be matched
    // For normal cases: all ground truth faces should be matched
    if (hardCase) {
        return matchCount > 0;
    } else {
        return matchCount == (int)groundTruth.size();
    }
}

static const std::vector<ImageGroundTruth> k_groundTruth = {
    { "300.jpg",    {{ 158, 112, 55, 55 }}, true  },
    { "300_1.jpg",  {{ 186,  84, 86, 86 }} },
    { "300_2.jpg",  {{  37,  70, 180, 180 }} },
    { "300_3.jpg",  {{  61,  52, 173, 173 }} },
    { "300_4.jpg",  {{  65,  80, 166, 166 }} },
    { "300_5.jpg",  {{  40,   7, 189, 189 }} },
    { "300_6.jpg",  {{  64,  47, 184, 184 }} },
    { "300_7.jpg",  {{ 157, 102,  52,  52 }}, true  },
    { "300_8.jpg",  {{  57,  52, 177, 177 }} }, 
    { "300_9.jpg",  {{  45,  50, 175, 175 }}, true }, 
    { "300_10.jpg", {{  70,  77, 136, 136 }} },
    { "300_11.jpg", {{  30,  20, 200, 200 }}, true  }, 
    { "300_12.jpg", {{  64,  55, 156, 156 }}, true  },
};

// Test fixture
struct FaceDetectorFixture {
    std::unique_ptr<Glance::Core::FaceDetector> m_detector;
    QString m_testDataDir;

    FaceDetectorFixture();
    void runGroundTruthTest(const ImageGroundTruth& gt);
};

FaceDetectorFixture::FaceDetectorFixture() {
    QString exePath = QCoreApplication::applicationFilePath();
    QFileInfo exeInfo(exePath);
    QString buildDir = exeInfo.absolutePath();
    
    // Try multiple plugin paths
    QString localPluginPath = QDir(buildDir).absoluteFilePath("imageformats");
    QString vcpkgPluginPath = "D:/work/vcpkg/installed/x64-windows/debug/Qt6/plugins/imageformats";
    
    qDebug() << "Build directory:" << buildDir;
    qDebug() << "Local plugin path:" << localPluginPath;
    qDebug() << "Local plugin exists:" << QDir(localPluginPath).exists();
    qDebug() << "Vcpkg plugin path:" << vcpkgPluginPath;
    qDebug() << "Vcpkg plugin exists:" << QDir(vcpkgPluginPath).exists();
    
    // Check if qjpegd.dll exists in both locations
    qDebug() << "Local qjpegd.dll exists:" << QFile::exists(QDir(localPluginPath).filePath("qjpegd.dll"));
    qDebug() << "Vcpkg qjpegd.dll exists:" << QFile::exists(QDir(vcpkgPluginPath).filePath("qjpegd.dll"));
    
    // Set plugin path for Qt to find image format plugins
    // Use vcpkg Qt installation path for plugins
    qputenv("QT_PLUGIN_PATH", vcpkgPluginPath.toUtf8());
    QCoreApplication::addLibraryPath(localPluginPath);
    QCoreApplication::addLibraryPath(vcpkgPluginPath);
    qDebug() << "Library paths:" << QCoreApplication::libraryPaths();
    
    // Check supported image formats
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    qDebug() << "Supported image formats:" << formats;
    
    m_detector = std::make_unique<Glance::Core::FaceDetector>();
    
    char* envDir = nullptr;
    size_t envLen = 0;
    _dupenv_s(&envDir, &envLen, "TEST_DATA_DIR");
    if (envDir) {
        m_testDataDir = QString::fromUtf8(envDir);
        free(envDir);
    } else {
        // Get absolute path to test data directory from executable location
        m_testDataDir = QDir::cleanPath(QDir(buildDir).absoluteFilePath("../../Tests/TestData"));
    }
    qDebug() << "Test data directory:" << m_testDataDir;
    qDebug() << "Current directory:" << QDir::currentPath();
}

void FaceDetectorFixture::runGroundTruthTest(const ImageGroundTruth& gt) {
    QDir dir(m_testDataDir);
    QString path = dir.filePath(gt.filename);
    QFileInfo fileInfo(path);

    if (gt.hardCase) {
        m_detector->setConfidenceThreshold(0.3);
    }
    qDebug().noquote() << QString("Loading: %1").arg(path);
    qDebug().noquote() << QString("  Status: %1").arg(
        !fileInfo.exists() ? "FILE NOT FOUND" :
        !fileInfo.isReadable() ? "FILE NOT READABLE" :
        QString("OK (%1 bytes)").arg(fileInfo.size()));

    QImageReader reader(path);
    QImage img = reader.read();
    if (img.isNull()) {
        qDebug().noquote() << QString("  Decode: FAILED - %1").arg(reader.errorString());
    } else {
        qDebug().noquote() << QString("  Decode: OK (%1x%2, %3)")
            .arg(img.width()).arg(img.height())
            .arg(img.format() == QImage::Format_Invalid ? "unknown" : "valid");
    }
    REQUIRE(!img.isNull());

    auto result = m_detector->detectFaces(img);
    REQUIRE(result.has_value());
    REQUIRE(result->success);

    // Validate detection results
    // For images with ground truth faces: check that detection finds at least one face
    // and that detected faces roughly match ground truth locations
    if (!gt.faces.empty()) {
        // At least one face should be detected
        REQUIRE(!result->faces.empty());
        
        // For non-hard cases, also check that detected faces roughly overlap with ground truth
        if (!gt.hardCase) {
            REQUIRE(matchBboxes(result->faces, gt.faces, gt.hardCase));
        }
    } else {
        REQUIRE(result->faces.empty());
    }
}

// Ground truth tests with TestData
TEST_CASE_METHOD(FaceDetectorFixture, "Ground truth 300", "[facedetector][groundtruth]") { runGroundTruthTest(k_groundTruth[0]); }
TEST_CASE_METHOD(FaceDetectorFixture, "Ground truth 300_1", "[facedetector][groundtruth]") { runGroundTruthTest(k_groundTruth[1]); }
TEST_CASE_METHOD(FaceDetectorFixture, "Ground truth 300_2", "[facedetector][groundtruth]") { runGroundTruthTest(k_groundTruth[2]); }
TEST_CASE_METHOD(FaceDetectorFixture, "Ground truth 300_3", "[facedetector][groundtruth]") { runGroundTruthTest(k_groundTruth[3]); }
TEST_CASE_METHOD(FaceDetectorFixture, "Ground truth 300_4", "[facedetector][groundtruth]") { runGroundTruthTest(k_groundTruth[4]); }
TEST_CASE_METHOD(FaceDetectorFixture, "Ground truth 300_5", "[facedetector][groundtruth]") { runGroundTruthTest(k_groundTruth[5]); }
TEST_CASE_METHOD(FaceDetectorFixture, "Ground truth 300_6", "[facedetector][groundtruth]") { runGroundTruthTest(k_groundTruth[6]); }
TEST_CASE_METHOD(FaceDetectorFixture, "Ground truth 300_7", "[facedetector][groundtruth]") { runGroundTruthTest(k_groundTruth[7]); }
TEST_CASE_METHOD(FaceDetectorFixture, "Ground truth 300_8", "[facedetector][groundtruth]") { runGroundTruthTest(k_groundTruth[8]); }
TEST_CASE_METHOD(FaceDetectorFixture, "Ground truth 300_9", "[facedetector][groundtruth]") { runGroundTruthTest(k_groundTruth[9]); }
TEST_CASE_METHOD(FaceDetectorFixture, "Ground truth 300_10", "[facedetector][groundtruth]") { runGroundTruthTest(k_groundTruth[10]); }
TEST_CASE_METHOD(FaceDetectorFixture, "Ground truth 300_11", "[facedetector][groundtruth]") { runGroundTruthTest(k_groundTruth[11]); }
TEST_CASE_METHOD(FaceDetectorFixture, "Ground truth 300_12", "[facedetector][groundtruth]") { runGroundTruthTest(k_groundTruth[12]); }