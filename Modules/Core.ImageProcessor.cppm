/**
 * @file Core.ImageProcessor.cppm
 * @brief Image processing engine with per-pixel adjustment operations
 *
 * Defines the ImageProcessor class for applying photographic adjustments
 * (exposure, contrast, gamma, temperature, tint, saturation, vibrance,
 * highlights/shadows, detail enhancement) to QImage objects.
 *
 * Also provides the ImageProcessorType concept, PixelInfo struct,
 * and ProcessorFactory for creating processor variants.
 */
module;

#include <vector>
#include <memory>
#include <concepts>
#include <functional>
#include <thread>
#include <future>
#include <QtConcurrent>
#include "../Core/ImageProcessingTypes.h"

export module Core.ImageProcessor;

import Qt.Wrapper;

export
{
    using ::QImage;
    using ::QString;
    using ::QPoint;
    using ::QFuture;
}
export import Core.TypeAliases;
export import Core.LUT;

export namespace Glance::Core {

/** @brief Concept for any type that can process a QImage */
template<typename T>
concept ImageProcessorType = requires(T t, QImage& img) {
    { t.process(img) } -> std::convertible_to<QImage>;
};

using ::Glance::Core::Exposure;
using ::Glance::Core::Contrast;
using ::Glance::Core::Brightness;
using ::Glance::Core::Gamma;
using ::Glance::Core::Temperature;
using ::Glance::Core::Tint;
using ::Glance::Core::Saturation;
using ::Glance::Core::Vibrance;
using ::Glance::Core::Highlights;
using ::Glance::Core::Shadows;
using ::Glance::Core::Details;
using ::Glance::Core::ProcessingParams;

struct PixelInfo {
    ColorComponent r, g, b;
    Float32 normalized_r, normalized_g, normalized_b;
    QPoint position;

    constexpr PixelInfo() noexcept = default;
    constexpr PixelInfo(ColorComponent red, ColorComponent green, ColorComponent blue,
                        Float32 norm_r, Float32 norm_g, Float32 norm_b, QPoint pos) noexcept
        : r{red}, g{green}, b{blue},
          normalized_r{norm_r}, normalized_g{norm_g}, normalized_b{norm_b},
          position{pos} {}
};

/** @brief Per-pixel image adjustment engine with GPU and LUT support */
class ImageProcessor {
public:
    ImageProcessor() = default;
    virtual ~ImageProcessor() = default;

    QImage applyExposure(const QImage& source, const Exposure& exposure) const;
    QImage applyContrast(const QImage& source, const Contrast& contrast) const;
    QImage applyBrightness(const QImage& source, const Brightness& brightness) const;
    QImage applyGamma(const QImage& source, const Gamma& gamma) const;
    QImage convertToGrayscale(const QImage& source) const;

    QImage applyTemperature(const QImage& source, const Temperature& temperature) const;
    QImage applyTint(const QImage& source, const Tint& tint) const;
    QImage applySaturation(const QImage& source, const Saturation& saturation) const;
    QImage applyVibrance(const QImage& source, const Vibrance& vibrance) const;
    QImage applyHighlightsShadows(const QImage& source, const Highlights& highlights, const Shadows& shadows) const;
    void applyDetailsEnhancement(const QImage& src, QImage& dst, const Details& details) const;

    QImage processImage(const QImage& source, const ProcessingParams& params) const;

    PixelInfo getPixelInfo(const QImage& source, const QPoint& position) const noexcept;

    std::vector<int> calculateHistogram(const QImage& source, int channel = -1) const;

    QImage applyLUT(const QImage& source, const QString& lutFilePath) const;

    QFuture<QImage> processImageAsync(const QImage& source, const ProcessingParams& params) const;

private:
    constexpr Float32 clamp(Float32 value, Float32 min, Float32 max) const noexcept;
    Float32 clamp(Float32 value) const noexcept;
    QColor adjustPixelColor(const QColor& color, const ProcessingParams& params) const noexcept;
};

/** @brief Factory for creating ImageProcessor variants (Basic, GPU, AI) */
class ProcessorFactory {
public:
    enum class ProcessorType {
        Basic,
        GPU,
        AI
    };

    static std::unique_ptr<ImageProcessor> create(ProcessorType type) {
        switch (type) {
            case ProcessorType::Basic:
                return std::make_unique<ImageProcessor>();
            case ProcessorType::GPU:
            case ProcessorType::AI:
            default:
                return std::make_unique<ImageProcessor>();
        }
    }
};

}
