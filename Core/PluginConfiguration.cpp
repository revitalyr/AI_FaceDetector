module;

#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QJsonArray>
#include <QDebug>

module Core.PluginConfiguration;

namespace Glance::Core {

bool PluginConfiguration::loadFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open configuration file:" << filePath;
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse configuration file:" << error.errorString();
        return false;
    }
    
    QJsonObject root = doc.object();
    
    // Load global configuration
    if (root.contains("global")) {
        m_globalConfig = root["global"].toObject();
    }
    
    // Load plugin configurations
    if (root.contains("plugins")) {
        QJsonObject plugins = root["plugins"].toObject();
        for (auto it = plugins.begin(); it != plugins.end(); ++it) {
            m_pluginConfigs[it.key()] = it.value().toObject();
        }
    }
    
    qDebug() << "Configuration loaded from:" << filePath;
    return true;
}

bool PluginConfiguration::saveToFile(const QString& filePath) const {
    QJsonObject root;
    root["global"] = m_globalConfig;
    root["plugins"] = m_pluginConfigs;
    
    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open configuration file for writing:" << filePath;
        return false;
    }
    
    file.write(doc.toJson());
    qDebug() << "Configuration saved to:" << filePath;
    return true;
}

QJsonObject PluginConfiguration::getPluginConfig(const QString& pluginName) const {
    auto it = m_pluginConfigs.find(pluginName);
    if (it != m_pluginConfigs.end()) {
        return it.value().toObject();
    }
    return QJsonObject();
}

void PluginConfiguration::setPluginConfig(const QString& pluginName, const QJsonObject& config) {
    if (validateConfig(pluginName, config)) {
        m_pluginConfigs[pluginName] = config;
    } else {
        qWarning() << "Invalid configuration for plugin:" << pluginName;
        for (const QString& error : m_validationErrors) {
            qWarning() << "  -" << error;
        }
    }
}

QJsonObject PluginConfiguration::getGlobalConfig() const {
    return m_globalConfig;
}

void PluginConfiguration::setGlobalConfig(const QJsonObject& config) {
    m_globalConfig = config;
}

bool PluginConfiguration::validateConfig(const QString& pluginName, const QJsonObject& config) {
    m_validationErrors.clear();
    
    // Get schema based on plugin type
    QJsonObject schema;
    if (pluginName.contains("ImageProcessor")) {
        schema = createImageProcessorTemplate();
    } else if (pluginName.contains("FaceDetector")) {
        schema = createFaceDetectorTemplate();
    } else {
        // Basic validation for unknown plugins
        return config.isEmpty();
    }
    
    return validateAgainstSchema(config, schema);
}

QStringList PluginConfiguration::getValidationErrors() const {
    return m_validationErrors;
}

void PluginConfiguration::loadDefaults() {
    m_globalConfig = createGlobalTemplate();
    
    // Load default plugin configurations
    m_pluginConfigs["DefaultImageProcessor"] = createImageProcessorTemplate();
    m_pluginConfigs["DefaultFaceDetector"] = createFaceDetectorTemplate();
    
    qDebug() << "Default configuration loaded";
}

void PluginConfiguration::resetToDefaults() {
    loadDefaults();
}

QJsonObject PluginConfiguration::mergeConfigs(const QJsonObject& base, const QJsonObject& override) const {
    QJsonObject result = base;
    
    for (auto it = override.begin(); it != override.end(); ++it) {
        if (it.value().isObject()) {
            QJsonObject baseObj = base[it.key()].toObject();
            QJsonObject overrideObj = it.value().toObject();
            result[it.key()] = mergeConfigs(baseObj, overrideObj);
        } else {
            result[it.key()] = it.value();
        }
    }
    
    return result;
}

bool PluginConfiguration::hasKey(const QString& pluginName, const QString& key) const {
    QJsonObject config = getPluginConfig(pluginName);
    return config.contains(key);
}

QVariant PluginConfiguration::getValue(const QString& pluginName, const QString& key, const QVariant& defaultValue) const {
    QJsonObject config = getPluginConfig(pluginName);
    if (config.contains(key)) {
        return config[key].toVariant();
    }
    return defaultValue;
}

void PluginConfiguration::setValue(const QString& pluginName, const QString& key, const QVariant& value) {
    QJsonObject config = getPluginConfig(pluginName);
    config[key] = QJsonValue::fromVariant(value);
    setPluginConfig(pluginName, config);
}

QJsonObject PluginConfiguration::getSection(const QString& pluginName, const QString& section) const {
    QJsonObject config = getPluginConfig(pluginName);
    if (config.contains(section)) {
        return config[section].toObject();
    }
    return QJsonObject();
}

void PluginConfiguration::setSection(const QString& pluginName, const QString& section, const QJsonObject& config) {
    QJsonObject pluginConfig = getPluginConfig(pluginName);
    pluginConfig[section] = config;
    setPluginConfig(pluginName, pluginConfig);
}

QJsonObject PluginConfiguration::createImageProcessorTemplate() const {
    QJsonObject config;
    
    // Performance settings
    QJsonObject performance;
    performance["enableAsync"] = true;
    performance["enableParallel"] = true;
    performance["maxThreads"] = -1;
    performance["cacheSize"] = 100;
    config["performance"] = performance;
    
    // Quality settings
    QJsonObject quality;
    quality["highQuality"] = true;
    quality["antialiasing"] = true;
    quality["interpolation"] = "bilinear";
    config["quality"] = quality;
    
    // Memory settings
    QJsonObject memory;
    memory["maxCacheSize"] = 100;
    memory["enableCaching"] = true;
    memory["gcThreshold"] = 80;
    config["memory"] = memory;
    
    return config;
}

QJsonObject PluginConfiguration::createFaceDetectorTemplate() const {
    QJsonObject config;
    
    // Detection settings
    QJsonObject detection;
    detection["minFaceSize"] = 20;
    detection["maxFaceSize"] = 500;
    detection["confidenceThreshold"] = 0.5;
    
    QJsonArray types;
    types.append("frontal");
    types.append("profile");
    detection["types"] = types;
    
    config["detection"] = detection;
    
    // Performance settings
    QJsonObject performance;
    performance["enableAsync"] = true;
    performance["enableParallel"] = true;
    performance["maxThreads"] = -1;
    performance["batchSize"] = 4;
    config["performance"] = performance;
    
    // AI enhancement settings
    QJsonObject ai;
    ai["enableProfileDetection"] = true;
    ai["enableAIEnhancement"] = true;
    ai["enableRotationStrategies"] = true;
    ai["enhancementLevel"] = "medium";
    config["ai"] = ai;
    
    return config;
}

QJsonObject PluginConfiguration::createGlobalTemplate() const {
    QJsonObject config;
    
    // Application settings
    config["autoLoadPlugins"] = true;
    config["pluginDirectory"] = "plugins";
    config["enablePerformanceMonitoring"] = false;
    config["logLevel"] = "info";
    
    // UI settings
    QJsonObject ui;
    ui["theme"] = "default";
    ui["language"] = "en";
    ui["autoSave"] = true;
    ui["autoSaveInterval"] = 300; // seconds
    config["ui"] = ui;
    
    return config;
}

bool PluginConfiguration::validateAgainstSchema(const QJsonObject& config, const QJsonObject& schema) {
    // Basic validation - check for required keys
    QStringList requiredKeys = schema.keys();
    QStringList missingKeys;
    
    for (const QString& key : requiredKeys) {
        if (!config.contains(key)) {
            missingKeys << key;
        }
    }
    
    if (!missingKeys.isEmpty()) {
        m_validationErrors.append("Missing required keys: " + missingKeys.join(", "));
        return false;
    }
    
    return true;
}

// Private methods implementation

QString PluginConfiguration::getPluginConfigPath(const QString& pluginName) const {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return QDir(configDir).filePath(pluginName + ".json");
}

QString PluginConfiguration::getGlobalConfigPath() const {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return QDir(configDir).filePath("config.json");
}

void PluginConfiguration::ensureConfigDirectory() const {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

QJsonObject PluginConfiguration::loadJsonFile(const QString& filePath) const {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QJsonObject();
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        return QJsonObject();
    }
    
    return doc.object();
}

bool PluginConfiguration::saveJsonFile(const QString& filePath, const QJsonObject& json) const {
    ensureConfigDirectory();
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonDocument doc(json);
    file.write(doc.toJson());
    return true;
}

} // namespace Glance::Core
