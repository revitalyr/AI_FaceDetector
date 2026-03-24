/**
 * @file MagicNumbers.h
 * @brief Core detection and processing constants (legacy header)
 *
 * Legacy header mirroring constants in Core.Constants.cppm (MagicNumbers
 * namespace) for non-module consumers. Kept for backwards compatibility.
 */
#pragma once
namespace Glance::Core::MagicNumbers {
    inline constexpr int INPUT_SIZE = 320;
    inline constexpr float IOU_THRESHOLD = 0.3f;
    inline constexpr float CONFIDENCE_BASE = 0.45f;
    inline constexpr float CONFIDENCE_AREA_DIVISOR = 70000.0f;
    inline constexpr float MIN_ASPECT_RATIO = 1.2f;
    inline constexpr float MAX_ASPECT_RATIO = 2.2f;
    inline constexpr float MAX_SIZE_DIVISOR = 1.5f;
    inline constexpr float MAX_SIZE_MODE_FACTOR = 1.05f;
    inline constexpr float CENTER_Y_RATIO = 0.8f;
    inline constexpr int EXPANSION_SIZE = 10;
    inline constexpr float SIGMA_IOU = 0.5f;
    inline constexpr int MIN_SIZE_DIVISOR_1 = 25;
    inline constexpr int MIN_SIZE_DIVISOR_2 = 40;
    inline constexpr float TINTS_SCALE_FACTOR = 0.2f;
}