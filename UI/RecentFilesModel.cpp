module;
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QFileIconProvider>
#include <QImageReader>
#include <algorithm>
module UI.RecentFilesModel;
import Core.Constants;

namespace Glance::UI {

RecentFilesModel::RecentFilesModel(QObject* parent)
    : QAbstractListModel(parent)
{
    loadSettings();
}

int RecentFilesModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_recentFiles.size();
}

QVariant RecentFilesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_recentFiles.size()) {
        return QVariant();
    }
    
    const RecentFile& file = m_recentFiles[index.row()];
    
    switch (role) {
        case Qt::DisplayRole:
            return file.displayName;
            
        case Qt::ToolTipRole:
            return QString("%1\n%2\n%3x%4 %5\nLast accessed: %6")
                   .arg(file.displayName)
                   .arg(file.filePath)
                   .arg(file.size.width())
                   .arg(file.size.height())
                   .arg(file.format)
                   .arg(file.lastAccessed.toString(Qt::TextDate));
            
        case Qt::DecorationRole:
        {
            QFileIconProvider provider;
            QFileInfo info(file.filePath);
            if (info.exists() && info.isFile()) { // Ensure the file exists and is a regular file
                return provider.icon(info);
            } else {
                // Fallback to a generic file icon if the file doesn't exist or is not a regular file
                return provider.icon(QFileIconProvider::File);
            }
        }
            
        case Qt::UserRole:
            return file.filePath;
            
        case Qt::UserRole + 1:
            return file.size;
            
        case Qt::UserRole + 2:
            return file.format;
            
        case Qt::UserRole + 3:
            return file.lastAccessed;
            
        default:
            return QVariant();
    }
}

QVariant RecentFilesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return "Recent Files";
    }
    
    return QVariant();
}

void RecentFilesModel::addFile(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return;
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return;
    }
    
    // Check if file already exists
    for (int i = 0; i < m_recentFiles.size(); ++i) {
        if (m_recentFiles[i].filePath == filePath) {
            // If the file is already at the top, just update its info and return
            if (i == 0) {
                m_recentFiles[0].lastAccessed = QDateTime::currentDateTime();
                updateFileInfo(m_recentFiles[0]);
                // Notify views that data at index 0 has changed for all relevant roles
                emit dataChanged(index(0), index(0), {Qt::DisplayRole, Qt::ToolTipRole, Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 3});
                saveSettings();
                return;
            }

            // Move the existing file to the top (index 0)
            beginMoveRows(QModelIndex(), i, i, QModelIndex(), 0);
            m_recentFiles.move(i, 0); // Use QList::move for atomic move
            endMoveRows();

            // Now the item is at index 0. Update its properties.
            m_recentFiles[0].lastAccessed = QDateTime::currentDateTime();
            updateFileInfo(m_recentFiles[0]); // Update the actual item in the list
            // Notify views that data at index 0 has changed for all relevant roles
            emit dataChanged(index(0), index(0), {Qt::DisplayRole, Qt::ToolTipRole, Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 3});
            saveSettings();
            return;
        }
    }
    
    // Add new file
    RecentFile file;
    file.filePath = filePath;
    file.lastAccessed = QDateTime::currentDateTime();
    updateFileInfo(file);
    
    beginInsertRows(QModelIndex(), 0, 0); // Notify views *after* data is ready
    m_recentFiles.prepend(file);
    endInsertRows();
    
    trimToMaxFiles();
    saveSettings();
}

void RecentFilesModel::removeFile(const QString& filePath)
{
    for (int i = 0; i < m_recentFiles.size(); ++i) {
        if (m_recentFiles[i].filePath == filePath) {
            beginRemoveRows(QModelIndex(), i, i);
            m_recentFiles.removeAt(i);
            endRemoveRows();
            saveSettings();
            break;
        }
    }
}

void RecentFilesModel::clearFiles()
{
    if (m_recentFiles.isEmpty()) {
        return;
    }
    
    beginRemoveRows(QModelIndex(), 0, m_recentFiles.size() - 1);
    m_recentFiles.clear();
    endRemoveRows();
    saveSettings();
}

void RecentFilesModel::setFiles(const QStringList& filePaths)
{
    beginResetModel();
    m_recentFiles.clear();
    
    for (const QString& filePath : filePaths) {
        QFileInfo fileInfo(filePath);
        if (fileInfo.exists()) {
            RecentFile file;
            file.filePath = filePath;
            file.lastAccessed = QDateTime::currentDateTime();
            updateFileInfo(file);
            m_recentFiles.append(file);
        }
    }
    
    trimToMaxFiles();
    endResetModel();
    saveSettings();
}

QStringList RecentFilesModel::files() const
{
    QStringList filePaths;
    for (const RecentFile& file : m_recentFiles) {
        filePaths << file.filePath;
    }
    return filePaths;
}

QString RecentFilesModel::filePath(const QModelIndex& index) const
{
    if (!index.isValid() || index.row() >= m_recentFiles.size()) {
        return QString();
    }
    
    return m_recentFiles[index.row()].filePath;
}

QString RecentFilesModel::filePath(int row) const
{
    if (row < 0 || row >= m_recentFiles.size()) {
        return QString();
    }
    
    return m_recentFiles[row].filePath;
}

void RecentFilesModel::setMaxFiles(int maxFiles)
{
    m_maxFiles = qMax(1, maxFiles);
    trimToMaxFiles();
    saveSettings();
}

int RecentFilesModel::maxFiles() const
{
    return m_maxFiles;
}

RecentFile RecentFilesModel::fileInfo(const QModelIndex& index) const
{
    if (!index.isValid() || index.row() >= m_recentFiles.size()) {
        return RecentFile{};
    }
    
    return m_recentFiles[index.row()];
}

RecentFile RecentFilesModel::fileInfo(int row) const
{
    if (row < 0 || row >= m_recentFiles.size()) {
        return RecentFile{};
    }
    
    return m_recentFiles[row];
}

void RecentFilesModel::loadSettings()
{
    QSettings settings;
    settings.beginGroup(Constants::RECENT_FILES_GROUP);
    
    m_maxFiles = settings.value(Constants::RECENT_FILES_MAX_KEY, 10).toInt();
    
    QStringList filePaths = settings.value(Constants::RECENT_FILES_LIST_KEY).toStringList();
    settings.endGroup();
    
    setFiles(filePaths);
}

void RecentFilesModel::saveSettings()
{
    QSettings settings;
    settings.beginGroup(Constants::RECENT_FILES_GROUP);
    
    settings.setValue(Constants::RECENT_FILES_MAX_KEY, m_maxFiles);
    settings.setValue(Constants::RECENT_FILES_LIST_KEY, files());
    
    settings.endGroup();
}

void RecentFilesModel::updateFileInfo(RecentFile& fileInfo)
{
    QFileInfo info(fileInfo.filePath);
    
    fileInfo.displayName = info.fileName();
    
    qDebug() << "updateFileInfo: Trying to get image dimensions for" << fileInfo.filePath;
    
    QFile file(fileInfo.filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "updateFileInfo: Failed to open file for QImageReader:" << fileInfo.filePath;
        fileInfo.size = QSize(); // Default to empty size
        fileInfo.format = info.suffix().toUpper(); // Use suffix as fallback
    } else {
        QImageReader reader(&file); // Use QIODevice directly
        qDebug() << "updateFileInfo: QImageReader created with QIODevice. Can read:" << reader.canRead();
        if (reader.canRead()) {
            fileInfo.size = reader.size();
            fileInfo.format = reader.format().toUpper();
        } else {
            qDebug() << "updateFileInfo: QImageReader cannot read file. Using suffix.";
            fileInfo.size = QSize();
            fileInfo.format = info.suffix().toUpper();
        }
        file.close(); // Ensure file is closed
    }
    qDebug() << "updateFileInfo: Completed for" << fileInfo.filePath;
}

void RecentFilesModel::trimToMaxFiles()
{
    while (m_recentFiles.size() > m_maxFiles) {
        beginRemoveRows(QModelIndex(), m_recentFiles.size() - 1, m_recentFiles.size() - 1);
        m_recentFiles.removeLast();
        endRemoveRows();
    }
}

} // namespace Glance::UI
