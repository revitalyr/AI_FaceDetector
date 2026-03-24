/**
 * @file Core.PluginConfiguration.cppm
 * @brief JSON-based plugin configuration management
 *
 * PluginConfiguration provides CRUD operations for plugin settings
 * stored as JSON files. Supports per-plugin config sections, global
 * config, validation against schemas, and template creation for
 * image processor and face detector plugins.
 */
module;

#include <memory>

export module Core.PluginConfiguration;

import Qt.Wrapper;

export
{
    using ::QString;
    using ::QJsonObject;
    using ::QJsonArray;
    using ::QStringList;
    using ::QVariant;
}

export namespace Glance::Core {

/** @brief JSON-based plugin settings with validation and templates */
class PluginConfiguration {
public:
    PluginConfiguration() = default;
    ~PluginConfiguration() = default;

    bool loadFromFile(const QString& filePath);
    bool saveToFile(const QString& filePath) const;

    QJsonObject getPluginConfig(const QString& pluginName) const;
    void setPluginConfig(const QString& pluginName, const QJsonObject& config);

    QJsonObject getGlobalConfig() const;
    void setGlobalConfig(const QJsonObject& config);

    bool validateConfig(const QString& pluginName, const QJsonObject& config);
    QStringList getValidationErrors() const;

    void loadDefaults();
    void resetToDefaults();

    QJsonObject mergeConfigs(const QJsonObject& base, const QJsonObject& override) const;

    bool hasKey(const QString& pluginName, const QString& key) const;
    QVariant getValue(const QString& pluginName, const QString& key, const QVariant& defaultValue = QVariant()) const;
    void setValue(const QString& pluginName, const QString& key, const QVariant& value);

    QJsonObject getSection(const QString& pluginName, const QString& section) const;
    void setSection(const QString& pluginName, const QString& section, const QJsonObject& config);

    QJsonObject createImageProcessorTemplate() const;
    QJsonObject createFaceDetectorTemplate() const;
    QJsonObject createGlobalTemplate() const;

    bool validateAgainstSchema(const QJsonObject& config, const QJsonObject& schema);

private:
    QJsonObject m_globalConfig;
    QJsonObject m_pluginConfigs;
    QStringList m_validationErrors;

    QString getPluginConfigPath(const QString& pluginName) const;
    QString getGlobalConfigPath() const;
    void ensureConfigDirectory() const;
    QJsonObject loadJsonFile(const QString& filePath) const;
    bool saveJsonFile(const QString& filePath, const QJsonObject& json) const;
};

using PluginConfigurationPtr = std::shared_ptr<PluginConfiguration>;

}
