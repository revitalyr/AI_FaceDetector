/**
 * @file GPUImageProcessor.h
 * @brief GPU-accelerated image processing (legacy, unused with module system)
 */
#pragma once

#include "ImageProcessor.h"
#include <QImage>

namespace Glance::Core {

/** @brief GPU-accelerated variant of ImageProcessor (legacy) */
class GPUImageProcessor : public ImageProcessor {
public:
    GPUImageProcessor();
    virtual ~GPUImageProcessor() = default;

    // Override processing methods to use GPU acceleration
    QImage applyExposure(const QImage& source, const Exposure& exposure) const override;
    QImage applyContrast(const QImage& source, const Contrast& contrast) const override;
    QImage applyBrightness(const QImage& source, const Brightness& brightness) const override;
    QImage applyGamma(const QImage& source, const Gamma& gamma) const override;
    QImage applyTemperature(const QImage& source, const Temperature& temperature) const override;
    QImage applyTint(const QImage& source, const Tint& tint) const override;
    QImage applySaturation(const QImage& source, const Saturation& saturation) const override;
    QImage applyVibrance(const QImage& source, const Vibrance& vibrance) const override;
    QImage applyHighlights(const QImage& source, const Highlights& highlights) const override;
    QImage applyShadows(const QImage& source, const Shadows& shadows) const override;
    QImage convertToGrayscale(const QImage& source) const override;

    // Check GPU availability
    static bool isGPUAvailable();
    
    // Get GPU capability info
    static QString getGPUInfo();
};

} // namespace Glance::Core