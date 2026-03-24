#include "PluginManager.h"
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QPluginLoader>
#include <QMap>
#include <QVector>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDirIterator>
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QDebug>

namespace Glance::Core::Plugins {

bool PluginManager::loadPlugins(const QString& pluginDirectory) {
    m_pluginDir = QDir(pluginDirectory);
    
    if (!m_pluginDir.exists()) {
        qWarning() << "Plugin directory does not exist:" << pluginDirectory;
        return false;
    }
    
    qDebug() << "Loading plugins from:" << m_pluginDir.absolutePath();
    
    // Discover plugin files
    QStringList pluginFiles;
    QDirIterator it(m_pluginDir.absolutePath(), QStringList() << "*.dll" << "*.so" << "*.dylib", 
                   QDir::Files, QDirIterator::Subdirectories);
    
    while (it.hasNext()) {
        pluginFiles << it.next();
    }
    
    // Load plugins
    int loadedCount = 0;
    for (const QString& pluginPath : pluginFiles) {
        // Pre-check: skip DLLs that don't have Qt plugin metadata
        QPluginLoader testLoader(pluginPath);
        QJsonObject metaData = testLoader.metaData();
        if (metaData.isEmpty() || !metaData.contains("IID")) {
            continue; // Skip non-plugin DLLs
        }
        
        if (loadPluginInternal(pluginPath)) {
            loadedCount++;
        }
    }
    
    // Resolve dependencies
    if (!resolveDependencies()) {
        qWarning() << "Failed to resolve plugin dependencies";
    }
    
    qDebug() << "Loaded" << loadedCount << "out of" << pluginFiles.size() << "plugins";
    return loadedCount > 0;
}

bool PluginManager::loadPlugin(const QString& pluginPath) {
    return loadPluginInternal(pluginPath);
}

void PluginManager::unloadPlugin(const QString& pluginName) {
    unloadPluginInternal(pluginName);
}

void PluginManager::unloadAllPlugins() {
    qDebug() << "[PluginManager] unloadAllPlugins: starting";
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        qDebug() << "[PluginManager] unloadAllPlugins: unloading" << it.key();
        unloadPluginInternal(it.key());
    }
    m_plugins.clear();
    qDebug() << "[PluginManager] unloadAllPlugins: done";
}

std::shared_ptr<IImageProcessorPlugin> PluginManager::getImageProcessorPlugin(const QString& name) const {
    qDebug() << "[PluginManager] getImageProcessorPlugin: name=" << name;
    if (name == "default") {
        // Return first available image processor plugin
        for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
            qDebug() << "[PluginManager] getImageProcessorPlugin: checking" << it.key() << "loaded=" << it->isLoaded;
            if (it->isLoaded) {
                auto plugin = std::dynamic_pointer_cast<IImageProcessorPlugin>(it->plugin);
                if (plugin) {
                    qDebug() << "[PluginManager] getImageProcessorPlugin: found default plugin" << it.key();
                    return plugin;
                }
            }
        }
    }
    
    auto result = getPlugin<IImageProcessorPlugin>(name);
    qDebug() << "[PluginManager] getImageProcessorPlugin: result=" << (result != nullptr);
    return result;
}

std::shared_ptr<IFaceDetectorPlugin> PluginManager::getFaceDetectorPlugin(const QString& name) const {
    qDebug() << "[PluginManager] getFaceDetectorPlugin: name=" << name;
    if (name == "default") {
        // Return first available face detector plugin
        for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
            qDebug() << "[PluginManager] getFaceDetectorPlugin: checking" << it.key() << "loaded=" << it->isLoaded;
            if (it->isLoaded) {
                auto plugin = std::dynamic_pointer_cast<IFaceDetectorPlugin>(it->plugin);
                if (plugin) {
                    qDebug() << "[PluginManager] getFaceDetectorPlugin: found default plugin" << it.key();
                    return plugin;
                }
            }
        }
    }
    
    auto result = getPlugin<IFaceDetectorPlugin>(name);
    qDebug() << "[PluginManager] getFaceDetectorPlugin: result=" << (result != nullptr);
    return result;
}

QStringList PluginManager::availablePlugins() const {
    QStringList plugins;
    QDirIterator it(m_pluginDir.absolutePath(), QStringList() << "*.dll" << "*.so" << "*.dylib", 
                   QDir::Files, QDirIterator::Subdirectories);
    
    while (it.hasNext()) {
        plugins << QFileInfo(it.next()).baseName();
    }
    
    return plugins;
}

QStringList PluginManager::loadedPlugins() const {
    QStringList plugins;
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        if (it->isLoaded) {
            plugins << it.key();
        }
    }
    return plugins;
}

QStringList PluginManager::pluginsByType(const QString& type) const {
    QStringList plugins;
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        if (it->isLoaded && it->plugin->pluginType() == type) {
            plugins << it.key();
        }
    }
    return plugins;
}

bool PluginManager::configurePlugin(const QString& name, const QJsonObject& config) {
    qDebug() << "[PluginManager] configurePlugin: name=" << name;
    auto it = m_plugins.find(name);
    if (it != m_plugins.end() && it->isLoaded) {
        qDebug() << "[PluginManager] configurePlugin: plugin found and loaded";
        bool success = it->plugin->configure(config);
        if (success) {
            it->configuration = config;
        }
        qDebug() << "[PluginManager] configurePlugin: success=" << success;
        return success;
    }
    qDebug() << "[PluginManager] configurePlugin: plugin not found or not loaded";
    return false;
}

QJsonObject PluginManager::getPluginConfiguration(const QString& name) const {
    qDebug() << "[PluginManager] getPluginConfiguration: name=" << name;
    auto it = m_plugins.find(name);
    if (it != m_plugins.end()) {
        qDebug() << "[PluginManager] getPluginConfiguration: found";
        return it->configuration;
    }
    qDebug() << "[PluginManager] getPluginConfiguration: not found";
    return QJsonObject();
}

void PluginManager::savePluginConfigurations(const QString& configFile) {
    qDebug() << "[PluginManager] savePluginConfigurations: file=" << configFile;
    QJsonObject root;
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        qDebug() << "[PluginManager] savePluginConfigurations: checking" << it.key() << "loaded=" << it->isLoaded;
        if (it->isLoaded) {
            root[it.key()] = it->configuration;
        }
    }
    
    QJsonDocument doc(root);
    QFile file(configFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        qDebug() << "[PluginManager] savePluginConfigurations: saved";
    } else {
        qWarning() << "[PluginManager] savePluginConfigurations: failed to open file";
    }
}

void PluginManager::loadPluginConfigurations(const QString& configFile) {
    QFile file(configFile);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject root = doc.object();
        
        for (auto it = root.begin(); it != root.end(); ++it) {
            configurePlugin(it.key(), it.value().toObject());
        }
    }

    // Load expected SHA-256 hashes from companion file
    QFileInfo fi(configFile);
    QString shaFile = fi.absolutePath() + '/' + fi.completeBaseName() + ".sha256";
    QFile sf(shaFile);
    if (sf.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_expectedSha256.clear();
        while (!sf.atEnd()) {
            QString line = QString::fromUtf8(sf.readLine()).trimmed();
            int eq = line.indexOf('=');
            if (eq > 0 && eq < line.length() - 1) {
                QString name = line.left(eq);
                QByteArray hash = QByteArray::fromHex(line.mid(eq + 1).toLatin1());
                if (hash.size() == 32)
                    m_expectedSha256.insert(name, hash);
            }
        }
    }
}

QJsonObject PluginManager::getPluginInfo(const QString& name) const {
    auto it = m_plugins.find(name);
    if (it != m_plugins.end() && it->isLoaded) {
        QJsonObject info;
        auto plugin = it->plugin;
        info["name"] = plugin->name();
        info["version"] = plugin->version();
        info["description"] = plugin->description();
        info["author"] = plugin->author();
        info["type"] = plugin->pluginType();
        info["initialized"] = plugin->isInitialized();
        info["path"] = it->path;
        
        QJsonArray deps;
        for (const QString& dep : plugin->dependencies()) {
            deps.append(dep);
        }
        info["dependencies"] = deps;
        
        return info;
    }
    return QJsonObject();
}

bool PluginManager::isPluginLoaded(const QString& name) const {
    qDebug() << "[PluginManager] isPluginLoaded: name=" << name;
    auto it = m_plugins.find(name);
    bool result = it != m_plugins.end() && it->isLoaded;
    qDebug() << "[PluginManager] isPluginLoaded: result=" << result;
    return result;
}

bool PluginManager::hasPluginError(const QString& name) const {
    qDebug() << "[PluginManager] hasPluginError: name=" << name;
    auto it = m_plugins.find(name);
    if (it == m_plugins.end()) {
        qDebug() << "[PluginManager] hasPluginError: plugin not found";
        return false;
    }
    bool result = it->plugin->hasError();
    qDebug() << "[PluginManager] hasPluginError: result=" << result;
    return result;
}

QString PluginManager::getPluginError(const QString& name) const {
    qDebug() << "[PluginManager] getPluginError: name=" << name;
    auto it = m_plugins.find(name);
    if (it != m_plugins.end()) {
        QString error = it->plugin->lastError();
        qDebug() << "[PluginManager] getPluginError: error=" << error;
        return error;
    }
    qDebug() << "[PluginManager] getPluginError: plugin not found";
    return QString();
}

bool PluginManager::resolveDependencies() {
    QStringList loaded = loadedPlugins();
    QStringList order = getDependencyOrder(loaded);
    
    if (order.isEmpty()) {
        return false;
    }
    
    // Reorder plugins based on dependencies
    QMap<QString, PluginInfo> reordered;
    for (const QString& name : order) {
        auto it = m_plugins.find(name);
        if (it != m_plugins.end()) {
            reordered[name] = it.value();
        }
    }
    
    m_plugins = reordered;
    return true;
}

QStringList PluginManager::getDependencyOrder(const QStringList& plugins) const {
    QStringList order;
    QStringList visited;
    
    for (const QString& plugin : plugins) {
        if (!visited.contains(plugin)) {
            if (!visitDependencies(plugin, visited, order)) {
                qWarning() << "Circular dependency detected for plugin:" << plugin;
                return QStringList();
            }
        }
    }
    
    return order;
}

bool PluginManager::checkDependencies(const QString& pluginName) const {
    qDebug() << "[PluginManager] checkDependencies: pluginName=" << pluginName;
    auto it = m_plugins.find(pluginName);
    if (it == m_plugins.end() || !it->isLoaded) {
        qDebug() << "[PluginManager] checkDependencies: plugin not found or not loaded";
        return false;
    }
    
    qDebug() << "[PluginManager] checkDependencies: checking dependencies for" << pluginName;
    for (const QString& dep : it->plugin->dependencies()) {
        qDebug() << "[PluginManager] checkDependencies: dependency" << dep << "loaded=" << isPluginLoaded(dep);
        if (!isPluginLoaded(dep)) {
            qDebug() << "[PluginManager] checkDependencies: dependency not loaded" << dep;
            return false;
        }
    }
    
    qDebug() << "[PluginManager] checkDependencies: all dependencies satisfied";
    return true;
}

void PluginManager::startPerformanceMonitoring() {
    m_performanceMonitoring = true;
}

void PluginManager::stopPerformanceMonitoring() {
    m_performanceMonitoring = false;
}

QJsonObject PluginManager::getPerformanceMetrics() const {
    QJsonObject metrics;
    // Performance metrics collection not implemented - basic metrics not currently tracked
    return metrics;
}

// Private methods implementation

bool PluginManager::loadPluginInternal(const QString& pluginPath) {
    // Path containment: only allow plugins within the configured plugin directory
    QFileInfo fi(pluginPath);
    if (!fi.exists() || !fi.isFile()) {
        qWarning() << "Plugin file not found or not a regular file:" << pluginPath;
        return false;
    }

    QFileInfo fiCanonical(fi.canonicalFilePath());
    if (m_pluginDir.exists()) {
        QFileInfo dirCanonical(m_pluginDir.canonicalPath());
        if (!fiCanonical.absoluteFilePath().startsWith(
                dirCanonical.absoluteFilePath() + '/', Qt::CaseInsensitive) &&
            fiCanonical.absoluteFilePath() != dirCanonical.absoluteFilePath()) {
            qWarning() << "Plugin path outside allowed directory:" << pluginPath
                       << "(resolved:" << fiCanonical.absoluteFilePath() << ")";
            return false;
        }
    }

    // Compute SHA-256 once, verify after loading by plugin name
    QCryptographicHash fileHasher(QCryptographicHash::Sha256);
    {
        QFile pluginFile(pluginPath);
        if (pluginFile.open(QIODevice::ReadOnly))
            fileHasher.addData(&pluginFile);
    }
    QByteArray pluginSha256 = fileHasher.result();

    QPluginLoader* loader = new QPluginLoader(pluginPath);
    
    if (!loader->load()) {
        qWarning() << "Failed to load plugin:" << pluginPath << loader->errorString();
        delete loader;
        return false;
    }
    
    QObject* pluginObj = loader->instance();
    if (!pluginObj) {
        qWarning() << "Failed to get plugin instance from:" << pluginPath;
        loader->unload();
        delete loader;
        return false;
    }
    
    IPlugin* plugin = dynamic_cast<IPlugin*>(pluginObj);
    if (!plugin) {
        qWarning() << "Plugin does not implement IPlugin interface:" << pluginPath;
        loader->unload();
        delete loader;
        return false;
    }
    
    // SHA-256 integrity check by plugin name
    QString pluginName = generatePluginName(plugin);
    if (m_expectedSha256.contains(pluginName) && !pluginSha256.isEmpty()) {
        if (m_expectedSha256.value(pluginName) != pluginSha256) {
            qWarning() << "Plugin SHA-256 mismatch for" << pluginName
                       << "expected" << m_expectedSha256.value(pluginName).toHex()
                       << "got" << pluginSha256.toHex();
            loader->unload();
            delete loader;
            return false;
        }
    }

    if (!validatePlugin(plugin)) {
        qWarning() << "Plugin validation failed:" << pluginPath;
        loader->unload();
        delete loader;
        return false;
    }
    
    // Initialize plugin
    if (!plugin->initialize()) {
        qWarning() << "Plugin initialization failed:" << plugin->name();
        loader->unload();
        delete loader;
        return false;
    }
    
    // Use no-op deleter since QPluginLoader manages the plugin object lifetime
    auto pluginPtr = std::shared_ptr<IPlugin>(plugin, [](IPlugin*) {
        // No-op: QPluginLoader owns the object and will delete it when unloaded
    });
    
    registerPlugin(pluginName, pluginPtr, loader, pluginPath);
    
    qDebug() << "Successfully loaded plugin:" << pluginName << "from" << pluginPath;
    return true;
}

void PluginManager::unloadPluginInternal(const QString& name) {
    auto it = m_plugins.find(name);
    if (it != m_plugins.end()) {
        if (it->isLoaded) {
            it->plugin->shutdown();
        }
        
        if (it->loader) {
            it->loader->unload();
            delete it->loader;
        }
        
        m_plugins.erase(it);
        qDebug() << "Unloaded plugin:" << name;
    }
}

bool PluginManager::validatePlugin(IPlugin* plugin) {
    if (!plugin) return false;
    
    // Check required metadata
    if (plugin->name().isEmpty() || plugin->version().isEmpty() || plugin->pluginType().isEmpty()) {
        return false;
    }
    
    // Check for duplicate names
    QString name = generatePluginName(plugin);
    if (m_plugins.contains(name)) {
        qWarning() << "Plugin with name" << name << "already exists";
        return false;
    }
    
    return true;
}

void PluginManager::registerPlugin(const QString& name, std::shared_ptr<IPlugin> plugin, 
                               QPluginLoader* loader, const QString& path) {
    PluginInfo info;
    info.plugin = plugin;
    info.loader = loader;
    info.path = path;
    info.configuration = plugin->defaultConfiguration();
    info.isLoaded = true;
    
    m_plugins[name] = info;
}

QString PluginManager::generatePluginName(IPlugin* plugin) const {
    QString name = plugin->name();
    if (name.isEmpty()) {
        name = QString("Unknown_%1").arg(reinterpret_cast<quintptr>(plugin));
    }
    return name;
}

bool PluginManager::checkPluginCompatibility(const QString& pluginName) const {
    // Version compatibility checking not implemented - all plugins assumed compatible
    Q_UNUSED(pluginName);
    return true;
}

bool PluginManager::visitDependencies(const QString& plugin, QStringList& visited, QStringList& order) const {
    if (visited.contains(plugin)) {
        return false; // Circular dependency
    }
    
    visited.append(plugin);
    
    auto it = m_plugins.find(plugin);
    if (it != m_plugins.end() && it->isLoaded) {
        for (const QString& dep : it->plugin->dependencies()) {
            if (!visitDependencies(dep, visited, order)) {
                return false;
            }
        }
    }
    
    order.append(plugin);
    return true;
}

} // namespace Glance::Core::Plugins
