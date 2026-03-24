/**
 * @file FaceDetectionConstants.h
 * @brief Face detection heuristics and thresholds (legacy header)
 *
 * Legacy header mirroring constants in Core.Constants.cppm
 * (FaceDetection::Constants namespace) for non-module consumers.
 * Kept for backwards compatibility.
 */
#pragma once

namespace Glance::Core::FaceDetection::Constants {

// ── Post-processing filter thresholds ────────────────────────────────────────

// Minimum face size as a fraction of the smaller image dimension
// Faces smaller than this percentage will be filtered out
constexpr double MIN_FACE_SIZE_FRACTION = 0.05;  // 5% of min dimension

// Maximum face size as a fraction of the smaller image dimension
// Faces larger than this percentage will be filtered out
constexpr double MAX_FACE_SIZE_FRACTION = 0.80;  // 80% of min dimension

// Maximum Y position as a fraction of image height
// Faces with center Y position below this will be filtered out
// Set to 1.0 to disable position filtering (allow faces anywhere)
constexpr double MAX_Y_POSITION_FRACTION = 1.00;  // 100% of image height (disabled)

// Bounding box expansion in pixels
// Expands detected bounding boxes to include more context around the face
constexpr int BOUNDING_BOX_EXPANSION_PIXELS = 10;

// ─────────────────────────────────────────────────────────────────────────────
// Aspect ratio correction thresholds
// ─────────────────────────────────────────────────────────────────────────────

// Minimum aspect ratio (height/width) before correction
// Faces with aspect ratio below this are considered "too wide"
constexpr double MIN_ASPECT_RATIO = 1.2;

// Maximum aspect ratio (height/width) before correction
// Faces with aspect ratio above this are considered "too tall"
constexpr double MAX_ASPECT_RATIO = 1.8;

// Target aspect ratio for wide faces (height/width)
constexpr double TARGET_ASPECT_RATIO_WIDE = 1.3;

// Target aspect ratio for tall faces (height/width)
constexpr double TARGET_ASPECT_RATIO_TALL = 1.5;

// ─────────────────────────────────────────────────────────────────────────────
// Detection confidence thresholds
// ─────────────────────────────────────────────────────────────────────────────

// Default confidence threshold for face detection
// Detections with confidence below this will be filtered out
constexpr double DEFAULT_CONFIDENCE_THRESHOLD = 0.5;

// ─────────────────────────────────────────────────────────────────────────────
// Non-Maximum Suppression (NMS) thresholds
// ─────────────────────────────────────────────────────────────────────────────

// Intersection over Union (IoU) threshold for NMS
// Detections with IoU above this are considered duplicates
constexpr double NMS_IOU_THRESHOLD = 0.5;

// ─────────────────────────────────────────────────────────────────────────────
// DNN model configuration
// ─────────────────────────────────────────────────────────────────────────────

// Default input size for YuNet DNN model
constexpr int DEFAULT_DNN_INPUT_SIZE = 320;

// ─────────────────────────────────────────────────────────────────────────────
// Aspect ratio validation
// ─────────────────────────────────────────────────────────────────────────────

// Minimum valid aspect ratio for face detection
constexpr double MIN_VALID_ASPECT_RATIO = 0.3;

// Maximum valid aspect ratio for face detection
constexpr double MAX_VALID_ASPECT_RATIO = 3.0;

} // namespace Glance::Core::FaceDetection::Constants
