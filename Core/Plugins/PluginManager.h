#pragma once

#include "IPlugin.h"
#include "IImageProcessorPlugin.h"
#include "IFaceDetectorPlugin.h"
#include <QDir>
#include <QJsonObject>
#include <QPluginLoader>
#include <QMap>
#include <QHash>
#include <QVector>
#include <memory>

namespace Glance::Core::Plugins {

/**
 * @brief Manager for loading and managing plugins
 * 
 * This class handles the discovery, loading, and lifecycle management
 * of plugins in the Glance application.
 */
class PluginManager {
public:
    PluginManager() = default;
    ~PluginManager() = default;
    
    // Plugin discovery and loading
    bool loadPlugins(const QString& pluginDirectory = "plugins");
    bool loadPlugin(const QString& pluginPath);
    void unloadPlugin(const QString& pluginName);
    void unloadAllPlugins();
    
    // Plugin access
    template<typename T>
    std::shared_ptr<T> getPlugin(const QString& name) const;
    
    std::shared_ptr<IImageProcessorPlugin> getImageProcessorPlugin(const QString& name = "default") const;
    std::shared_ptr<IFaceDetectorPlugin> getFaceDetectorPlugin(const QString& name = "default") const;
    
    // Plugin enumeration
    QStringList availablePlugins() const;
    QStringList loadedPlugins() const;
    QStringList pluginsByType(const QString& type) const;
    
    // Configuration
    bool configurePlugin(const QString& name, const QJsonObject& config);
    QJsonObject getPluginConfiguration(const QString& name) const;
    void savePluginConfigurations(const QString& configFile = "plugins.json");
    void loadPluginConfigurations(const QString& configFile = "plugins.json");
    
    // Plugin information
    QJsonObject getPluginInfo(const QString& name) const;
    bool isPluginLoaded(const QString& name) const;
    bool hasPluginError(const QString& name) const;
    QString getPluginError(const QString& name) const;
    
    // Dependency management
    bool resolveDependencies();
    QStringList getDependencyOrder(const QStringList& plugins) const;
    bool checkDependencies(const QString& pluginName) const;
    
    // Performance monitoring
    void startPerformanceMonitoring();
    void stopPerformanceMonitoring();
    QJsonObject getPerformanceMetrics() const;

private:
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;
    
    struct PluginInfo {
        std::shared_ptr<IPlugin> plugin;
        QPluginLoader* loader;
        QString path;
        QJsonObject configuration;
        bool isLoaded = false;
    };
    
    QMap<QString, PluginInfo> m_plugins;
    QDir m_pluginDir;
    QHash<QString, QByteArray> m_expectedSha256;
    bool m_performanceMonitoring = false;
    
    // Internal methods
    bool loadPluginInternal(const QString& pluginPath);
    void unloadPluginInternal(const QString& name);
    bool validatePlugin(IPlugin* plugin);
    void registerPlugin(const QString& name, std::shared_ptr<IPlugin> plugin, QPluginLoader* loader, const QString& path);
    QString generatePluginName(IPlugin* plugin) const;
    bool checkPluginCompatibility(const QString& pluginName) const;
    bool visitDependencies(const QString& plugin, QStringList& visited, QStringList& order) const;
};

// Template implementation
template<typename T>
std::shared_ptr<T> PluginManager::getPlugin(const QString& name) const {
    qDebug() << "[PluginManager] getPlugin: name=" << name;
    auto it = m_plugins.find(name);
    if (it != m_plugins.end() && it->isLoaded) {
        qDebug() << "[PluginManager] getPlugin: found and loaded" << name;
        return std::dynamic_pointer_cast<T>(it->plugin);
    }
    qDebug() << "[PluginManager] getPlugin: not found or not loaded" << name;
    return nullptr;
}

} // namespace Glance::Core::Plugins
