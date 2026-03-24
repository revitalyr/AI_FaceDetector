#include "AIProcessor.h"
#include "BasicProcessor.h"
#include <QImage>

namespace Glance::Core {

AIProcessor::AIProcessor() {}

bool AIProcessor::isAIHardwareAvailable() {
    return std::getenv("AI_ENABLED") != nullptr;
}

QString AIProcessor::getAIHardwareInfo() {
    return "AI hardware: placeholder"s;
}

QImage AIProcessor::applyExposure(const QImage& source, const Exposure& exposure) const {
    if (!isAIHardwareAvailable()) return BasicProcessor::applyExposure(source, exposure);
    qDebug() << "AI exposure processed";
    return BasicProcessor::applyExposure(source, exposure);
}

} // namespace Glance::Core