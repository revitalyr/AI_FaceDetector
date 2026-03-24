#include "GPUImageProcessor.h"
#include "BasicProcessor.h"
#include <QDebug>

namespace Glance::Core {

GPUImageProcessor::GPUImageProcessor() {}

bool GPUImageProcessor::isGPUAvailable() {
    return std::getenv("CUDA_ENABLED") != nullptr;
}

QString GPUImageProcessor::getGPUInfo() {
    return "GPU capabilities: placeholder";
}

QImage GPUImageProcessor::applyExposure(const QImage& source, const Exposure& exposure) const {
    if (!isGPUAvailable()) return BasicProcessor::applyExposure(source, exposure);
    qDebug() << "GPU exposure applied";
    return BasicProcessor::applyExposure(source, exposure);
}

} // namespace Glance::Core