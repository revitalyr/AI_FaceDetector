#pragma once

#include "IPlugin.h"
#include "../ImageProcessingTypes.h"
#include <QImage>
#include <QFuture>
#include <vector>
#include <memory>

namespace Glance::Core::Plugins {

/**
 * @brief Interface for image processing plugins
 * 
 * This interface defines the contract for image processing plugins.
 * Plugins implementing this interface can perform various image processing operations.
 */
class IImageProcessorPlugin : public IPlugin {
public:
    ~IImageProcessorPlugin() override = default;

    // Plugin type identification
    QString pluginType() const override {
        return "ImageProcessor";
    }

    // Core image processing operations
    virtual QImage processImage(const QImage& source, const ProcessingParams& params) = 0;
    virtual QFuture<QImage> processImageAsync(const QImage& source, const ProcessingParams& params) = 0;
    
    // Individual processing operations
    virtual QImage applyExposure(const QImage& source, const Exposure& exposure) = 0;
    virtual QImage applyContrast(const QImage& source, const Contrast& contrast) = 0;
    virtual QImage applyGamma(const QImage& source, const Gamma& gamma) = 0;
    virtual QImage applyTemperature(const QImage& source, const Temperature& temperature) = 0;
    virtual QImage applyTint(const QImage& source, const Tint& tint) = 0;
    virtual QImage applySaturation(const QImage& source, const Saturation& saturation) = 0;
    virtual QImage applyVibrance(const QImage& source, const Vibrance& vibrance) = 0;
    virtual QImage applyHighlights(const QImage& source, const Highlights& highlights) = 0;
    virtual QImage applyShadows(const QImage& source, const Shadows& shadows) = 0;
    virtual QImage convertToGrayscale(const QImage& source) = 0;
    
    // Utility functions
    virtual std::vector<int> calculateHistogram(const QImage& source, int channel = -1) = 0;
    
    // Capabilities
    virtual QStringList supportedOperations() const = 0;
    virtual bool supportsAsync() const = 0;
    virtual bool supportsParallel() const = 0;
};

using IImageProcessorPluginPtr = std::shared_ptr<IImageProcessorPlugin>;

} // namespace Glance::Core::Plugins
