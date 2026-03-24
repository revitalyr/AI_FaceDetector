module;

#include <memory>
#include <QDebug>

module Core.ImageProcessor;

namespace Glance::Core {

std::unique_ptr<ImageProcessor> ProcessorFactory::create(ProcessorType type) {
    switch (type) {
        case ProcessorType::Basic:
            return std::make_unique<ImageProcessor>();

        case ProcessorType::GPU:
            qWarning("GPU processor not implemented, using basic processor");
            return std::make_unique<ImageProcessor>();

        case ProcessorType::AI:
            qWarning("AI processor not implemented, using basic processor");
            return std::make_unique<ImageProcessor>();

        default:
            qWarning("Unknown processor type, using basic processor");
            return std::make_unique<ImageProcessor>();
    }
}