/**
 * @file ImageProcessingTypes.h
 * @brief Shared image processing type definitions for header/module co-existence
 *
 * Defines image processing types (Exposure, Contrast, Gamma, ProcessingParams,
 * etc.) in a single location so that both legacy headers and C++23 modules
 * see the same entities, avoiding MSVC module-decoration mismatch.
 *
 * Modules that own these types must NOT redeclare them — they re-export
 * via `using` instead.
 */
#pragma once

#include <QString>
#include <algorithm>

#include "TypeAliases.h"

namespace Glance::Core {

struct Exposure {
    Float32 value = 0.0f;
    constexpr static Float32 min() { return -3.0f; }
    constexpr static Float32 max() { return 3.0f; }
};

struct Contrast {
    Float32 value = 1.0f;
    constexpr static Float32 min() { return 0.0f; }
    constexpr static Float32 max() { return 3.0f; }
};

struct Brightness {
    Float32 value = 0.0f;
    constexpr static Float32 min() { return -1.0f; }
    constexpr static Float32 max() { return 1.0f; }
};

struct Gamma {
    Float32 value = 1.0f;
    constexpr static Float32 min() { return 0.1f; }
    constexpr static Float32 max() { return 3.0f; }
};

struct Temperature {
    Float32 value = 0.0f;
    constexpr static Float32 min() { return -100.0f; }
    constexpr static Float32 max() { return 100.0f; }
};

struct Tint {
    Float32 value = 0.0f;
    constexpr static Float32 min() { return -100.0f; }
    constexpr static Float32 max() { return 100.0f; }
};

struct Saturation {
    Float32 value = 0.0f;
    constexpr static Float32 min() { return -100.0f; }
    constexpr static Float32 max() { return 100.0f; }
};

struct Vibrance {
    Float32 value = 0.0f;
    constexpr static Float32 min() { return -100.0f; }
    constexpr static Float32 max() { return 100.0f; }
};

struct Highlights {
    Float32 value = 0.0f;
    constexpr static Float32 min() { return -100.0f; }
    constexpr static Float32 max() { return 100.0f; }
};

struct Shadows {
    Float32 value = 0.0f;
    constexpr static Float32 min() { return -100.0f; }
    constexpr static Float32 max() { return 100.0f; }
};

struct Details {
    Float32 value = 0.0f;
    constexpr static Float32 min() { return -100.0f; }
    constexpr static Float32 max() { return 100.0f; }
};

struct ProcessingParams {
    Exposure exposure;
    Contrast contrast;
    Brightness brightness;
    Gamma gamma;
    bool grayscale = false;

    Temperature temperature;
    Tint tint;
    Saturation saturation;
    Vibrance vibrance;
    Highlights highlights;
    Shadows shadows;
    Details details;

    bool autoDetectFaces = true;

    bool enableLUT = false;
    QString lutFilePath;

    float getExposure() const { return exposure.value; }
    float getContrast() const { return contrast.value; }
    float getBrightness() const { return brightness.value; }
    float getGamma() const { return gamma.value; }
    float getTemperature() const { return temperature.value; }
    float getTint() const { return tint.value; }
    float getSaturation() const { return saturation.value; }
    float getVibrance() const { return vibrance.value; }
    float getHighlights() const { return highlights.value; }
    float getShadows() const { return shadows.value; }
    float getDetails() const { return details.value; }

    void setExposure(float val) { exposure.value = std::clamp(val, exposure.min(), exposure.max()); }
    void setContrast(float val) { contrast.value = std::clamp(val, contrast.min(), contrast.max()); }
    void setBrightness(float val) { brightness.value = std::clamp(val, brightness.min(), brightness.max()); }
    void setGamma(float val) { gamma.value = std::clamp(val, gamma.min(), gamma.max()); }
    void setTemperature(float val) { temperature.value = std::clamp(val, temperature.min(), temperature.max()); }
    void setTint(float val) { tint.value = std::clamp(val, tint.min(), tint.max()); }
    void setSaturation(float val) { saturation.value = std::clamp(val, saturation.min(), saturation.max()); }
    void setVibrance(float val) { vibrance.value = std::clamp(val, vibrance.min(), vibrance.max()); }
    void setHighlights(float val) { highlights.value = std::clamp(val, highlights.min(), highlights.max()); }
    void setShadows(float val) { shadows.value = std::clamp(val, shadows.min(), shadows.max()); }
    void setDetails(float val) { details.value = std::clamp(val, details.min(), details.max()); }
};

} // namespace Glance::Core
