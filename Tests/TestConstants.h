#pragma once

#include <QColor>
#include <QString>

namespace Glance::Tests::Constants {

// ─────────────────────────────────────────────────────────────────────────────
// Image dimensions for synthetic test images
// ─────────────────────────────────────────────────────────────────────────────
constexpr int SYNTHETIC_FACE_IMAGE_WIDTH = 800;
constexpr int SYNTHETIC_FACE_IMAGE_HEIGHT = 1000;
constexpr int SYNTHETIC_EMPTY_IMAGE_SIZE = 800;
constexpr int SYNTHETIC_LARGE_IMAGE_WIDTH = 1000;
constexpr int SYNTHETIC_LARGE_IMAGE_HEIGHT = 800;

// Preprocessing test image size (must be >500 for HOG detector to run)
constexpr int PREPROCESSING_TEST_IMAGE_SIZE = 800;

// ─────────────────────────────────────────────────────────────────────────────
// Colors for synthetic face generation
// ─────────────────────────────────────────────────────────────────────────────
const QColor SKIN_COLOR(255, 220, 177);
const QColor FEATURE_COLOR(50, 50, 50);
const QColor EMPTY_IMAGE_COLOR(Qt::white);
const QColor LARGE_IMAGE_COLOR(200, 200, 200);
const QColor PREPROCESSING_TEST_COLOR(128, 128, 128);

// ─────────────────────────────────────────────────────────────────────────────
// Face feature positions for synthetic image (800x1000)
// ─────────────────────────────────────────────────────────────────────────────
constexpr int LEFT_EYE_X = 280;
constexpr int LEFT_EYE_Y = 380;
constexpr int LEFT_EYE_WIDTH = 80;
constexpr int LEFT_EYE_HEIGHT = 56;

constexpr int RIGHT_EYE_X = 440;
constexpr int RIGHT_EYE_Y = 380;
constexpr int RIGHT_EYE_WIDTH = 80;
constexpr int RIGHT_EYE_HEIGHT = 56;

constexpr int NOSE_X = 360;
constexpr int NOSE_Y = 475;
constexpr int NOSE_WIDTH = 56;
constexpr int NOSE_HEIGHT = 80;

constexpr int MOUTH_X = 320;
constexpr int MOUTH_Y = 575;
constexpr int MOUTH_WIDTH = 160;
constexpr int MOUTH_HEIGHT = 44;

// ─────────────────────────────────────────────────────────────────────────────
// Detection thresholds
// ─────────────────────────────────────────────────────────────────────────────
constexpr double DEFAULT_IOU_THRESHOLD = 0.35;
constexpr int MIN_DLIB_HOG_SIZE = 500;  // Minimum image size for dlib HOG detector

// ─────────────────────────────────────────────────────────────────────────────
// Ground truth test data
// ─────────────────────────────────────────────────────────────────────────────
constexpr int GROUND_TRUTH_IMAGE_SIZE = 300;
constexpr const char* GROUND_TRUTH_FILENAME_PREFIX = "300";
constexpr const char* TEST_DATA_ENV_VAR = "TEST_DATA_DIR";
constexpr const char* TEST_DATA_SUBDIR = "/../TestData";
constexpr const char* TEST_DATA_DIR = "../../Tests/TestData";

// ─────────────────────────────────────────────────────────────────────────────
// Performance test limits
// ─────────────────────────────────────────────────────────────────────────────
constexpr int PERFORMANCE_TEST_MAX_TIME_MS = 10000;

} // namespace Glance::Tests::Constants
