/**
 * @file Core.Constants.cppm
 * @brief Application-wide constants for face detection, image processing, and UI
 *
 * Organises constant values by domain:
 * - MagicNumbers: Core algorithmic parameters (IOU thresholds, confidence bases)
 * - FaceDetection::Constants: Face detection heuristics and limits
 * - ImageProcessing::Ranges: Valid ranges for all image adjustment parameters
 * - IO::Constants: File and image dimension limits
 * - UI::Constants: Zoom factors, face rect rendering, and settings keys
 */
module;

#include <QtGlobal>

export module Core.Constants;

export import Core.TypeAliases;

export namespace Glance::Core::MagicNumbers {
    inline constexpr Int32 INPUT_SIZE = 320;
    inline constexpr Float32 IOU_THRESHOLD = 0.3f;
    inline constexpr Float32 CONFIDENCE_BASE = 0.45f;
    inline constexpr Float32 CONFIDENCE_AREA_DIVISOR = 70000.0f;
    inline constexpr Float32 MIN_ASPECT_RATIO = 1.2f;
    inline constexpr Float32 MAX_ASPECT_RATIO = 2.2f;
    inline constexpr Float32 MAX_SIZE_DIVISOR = 1.5f;
    inline constexpr Float32 MAX_SIZE_MODE_FACTOR = 1.05f;
    inline constexpr Float32 CENTER_Y_RATIO = 0.8f;
    inline constexpr Int32 EXPANSION_SIZE = 10;
    inline constexpr Float32 SIGMA_IOU = 0.5f;
    inline constexpr Int32 MIN_SIZE_DIVISOR_1 = 25;
    inline constexpr Int32 MIN_SIZE_DIVISOR_2 = 40;
    inline constexpr Float32 TINTS_SCALE_FACTOR = 0.2f;
}

export namespace Glance::Core::FaceDetection::Constants {
    constexpr Float64 MIN_FACE_SIZE_FRACTION = 0.05;
    constexpr Float64 MAX_FACE_SIZE_FRACTION = 0.80;
    constexpr Float64 MAX_Y_POSITION_FRACTION = 1.00;
    constexpr Int32   BOUNDING_BOX_EXPANSION_PIXELS = 10;
    constexpr Float64 MIN_ASPECT_RATIO = 1.2;
    constexpr Float64 MAX_ASPECT_RATIO = 1.8;
    constexpr Float64 TARGET_ASPECT_RATIO_WIDE = 1.3;
    constexpr Float64 TARGET_ASPECT_RATIO_TALL = 1.5;
    constexpr Float64 DEFAULT_CONFIDENCE_THRESHOLD = 0.5;
    constexpr Float64 NMS_IOU_THRESHOLD = 0.5;
    constexpr Int32   DEFAULT_DNN_INPUT_SIZE = 320;
    constexpr Float64 MIN_VALID_ASPECT_RATIO = 0.3;
    constexpr Float64 MAX_VALID_ASPECT_RATIO = 3.0;
}

export namespace Glance::Core::ImageProcessing::Ranges {
    constexpr Float32 EXPOSURE_MIN = -3.0f;
    constexpr Float32 EXPOSURE_MAX = 3.0f;
    constexpr Float32 CONTRAST_MIN = 0.0f;
    constexpr Float32 CONTRAST_MAX = 3.0f;
    constexpr Float32 BRIGHTNESS_MIN = -1.0f;
    constexpr Float32 BRIGHTNESS_MAX = 1.0f;
    constexpr Float32 GAMMA_MIN = 0.1f;
    constexpr Float32 GAMMA_MAX = 3.0f;
    constexpr Float32 TEMPERATURE_MIN = -100.0f;
    constexpr Float32 TEMPERATURE_MAX = 100.0f;
    constexpr Float32 TINT_MIN = -100.0f;
    constexpr Float32 TINT_MAX = 100.0f;
    constexpr Float32 SATURATION_MIN = -100.0f;
    constexpr Float32 SATURATION_MAX = 100.0f;
    constexpr Float32 VIBRANCE_MIN = -100.0f;
    constexpr Float32 VIBRANCE_MAX = 100.0f;
    constexpr Float32 HIGHLIGHTS_MIN = -100.0f;
    constexpr Float32 HIGHLIGHTS_MAX = 100.0f;
    constexpr Float32 SHADOWS_MIN = -100.0f;
    constexpr Float32 SHADOWS_MAX = 100.0f;
    constexpr Float32 DETAILS_MIN = -100.0f;
    constexpr Float32 DETAILS_MAX = 100.0f;
}

export namespace Glance::IO::Constants {
    using namespace Glance::Core;
    constexpr Int32 MAX_IMAGE_DIMENSION = 65536;
}

export namespace Glance::UI::Constants {
    using namespace Glance::Core;
    constexpr Float32 MIN_ZOOM = 0.1f;
    constexpr Float32 MAX_ZOOM = 50.0f;
    constexpr Float32 ZOOM_FACTOR = 1.2f;
    constexpr Int32   FACE_RECT_COLOR = 0xFFFF0000;
    constexpr Int32   FACE_RECT_HOVER_COLOR = 0xFFFFFF00;
    constexpr Int32   FACE_RECT_LINE_WIDTH = 2;
    constexpr Int32   FACE_RECT_FILL_ALPHA = 50;
    constexpr Int32   SPLITTER_HIT_AREA = 8;

    inline constexpr const char* RECENT_FILES_GROUP = "RecentFiles";
    inline constexpr const char* RECENT_FILES_MAX_KEY = "MaxFiles";
    inline constexpr const char* RECENT_FILES_LIST_KEY = "Files";
}
