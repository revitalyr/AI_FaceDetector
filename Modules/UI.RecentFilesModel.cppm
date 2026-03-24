/**
 * @file UI.RecentFilesModel.cppm
 * @brief Qt model for recently opened image files
 *
 * RecentFilesModel provides a QAbstractListModel-backed list of
 * recently accessed image files with metadata (path, access time,
 * display name, size, format). Persists to QSettings and supports
 * configurable maximum file count.
 */
module;
#include <QAbstractListModel>
#include <QPixmap>
#include <QSize>
#include <QDateTime>
#include <QSettings>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QList>

export module UI.RecentFilesModel;

import Core.Constants;

namespace Glance::UI {

/** @brief Metadata for a single recently opened file */
export struct RecentFile {
    QString filePath;
    QDateTime lastAccessed;
    QString displayName;
    QSize size;
    QString format;
};

export class RecentFilesModel : public QAbstractListModel {

public:
    explicit RecentFilesModel(QObject* parent = nullptr);
    
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    
    void addFile(const QString& filePath);
    void removeFile(const QString& filePath);
    void clearFiles();
    void setFiles(const QStringList& filePaths);
    
    QStringList files() const;
    QString filePath(const QModelIndex& index) const;
    QString filePath(int row) const;
    
    void setMaxFiles(int maxFiles);
    int maxFiles() const;
    
    RecentFile fileInfo(const QModelIndex& index) const;
    RecentFile fileInfo(int row) const;

private:
    void loadSettings();
    void saveSettings();
    void updateFileInfo(RecentFile& fileInfo);
    void trimToMaxFiles();

private:
    QList<RecentFile> m_recentFiles;
    int m_maxFiles = 10;
    
};

} // namespace Glance::UI
