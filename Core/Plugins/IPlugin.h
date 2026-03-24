#pragma once

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QVersionNumber>
#include <QObject>
#include <memory>

namespace Glance::Core::Plugins {

/**
 * @brief Base interface for all plugins in the Glance application
 * 
 * This interface provides the basic contract that all plugins must follow.
 * It includes metadata, lifecycle management, and configuration capabilities.
 */
class IPlugin : public QObject {
public:
    virtual ~IPlugin() = default;

    // Plugin metadata
    virtual QString name() const = 0;
    virtual QString version() const = 0;
    virtual QString description() const = 0;
    virtual QString author() const = 0;
    virtual QStringList dependencies() const = 0;
    
    // Plugin lifecycle
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool isInitialized() const = 0;
    
    // Configuration
    virtual QJsonObject defaultConfiguration() const = 0;
    virtual bool configure(const QJsonObject& config) = 0;
    virtual QJsonObject currentConfiguration() const = 0;
    
    // Plugin type identification
    virtual QString pluginType() const = 0;
    
    // Error handling
    virtual QString lastError() const = 0;
    virtual bool hasError() const = 0;
};

using IPluginPtr = std::shared_ptr<IPlugin>;

} // namespace Glance::Core::Plugins
