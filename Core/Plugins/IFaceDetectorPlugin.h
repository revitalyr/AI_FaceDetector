#pragma once

#include "IPlugin.h"
#include <QImage>
#include <QFuture>
#include <QStringList>

// The types FaceDetectionResult and FaceDetector are provided by the
// Core.FaceDetector module.  Headers cannot `import` a module, so we
// forward-declare what we need here.  Any .cpp / .cppm file that includes
// this header MUST also:
//     import Core.FaceDetector;
// to obtain the complete definitions.
#include "CoreFaceDetectorFwd.h"

namespace Glance::Core::Plugins {

/**
 * @brief Interface for face detection plugins
 */
class IFaceDetectorPlugin : public IPlugin {
public:
    ~IFaceDetectorPlugin() = default;

    QString pluginType() const override {
        return "FaceDetector";
    }

    virtual std::optional<Glance::Core::FaceDetectionResult> detectFaces(const QImage& image) = 0;
    virtual QFuture<Glance::Core::FaceDetectionResult> detectFacesAsync(const QImage& image) = 0;

    virtual void setMinFaceSize(int minSize) = 0;
    virtual void setMaxFaceSize(int maxSize) = 0;
    virtual void setConfidenceThreshold(double threshold) = 0;
    virtual void setDetectionTypes(const QStringList& types) = 0;

    virtual QStringList supportedDetectionTypes() const = 0;
    virtual bool supportsAsync() const = 0;
    virtual bool supportsParallel() const = 0;
    virtual bool supportsProfileDetection() const = 0;
    virtual bool supportsAIEnhancement() const = 0;

    virtual bool isSupported() const = 0;
    virtual QString getDetectionModelInfo() const = 0;

    virtual double getAverageDetectionTime() const = 0;
    virtual int getTotalDetections() const = 0;
    virtual void resetMetrics() = 0;
};

using IFaceDetectorPluginPtr = std::shared_ptr<IFaceDetectorPlugin>;

} // namespace Glance::Core::Plugins
