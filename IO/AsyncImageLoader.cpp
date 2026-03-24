module;
#include <QElapsedTimer>
#include <QFileInfo>
#include <QImageReader>
#include <QDir>
#include <QDebug>
#include <QtConcurrent>
#include <algorithm>
#include <ranges>
#include "LoadResult.h"
module IO.AsyncImageLoader;

namespace Glance::IO {

ImageLoadTask::ImageLoadTask(const QString& filePath, LoadCompletedCallback callback, QObject* parent)
    : QObject(nullptr)
    , m_filePath(filePath)
    , m_callback(std::move(callback))
    , m_parent(parent)
{
    setAutoDelete(false);
}

void ImageLoadTask::run() {
    LoadResult result = loadImageInternal(m_filePath);
    if (m_callback && m_parent) {
        QMetaObject::invokeMethod(m_parent, [this, result]() {
            m_callback(result);
        }, Qt::QueuedConnection);
    }
}

LoadResult ImageLoadTask::loadImageInternal(const QString& path) {
    QElapsedTimer timer;
    timer.start();
    
    LoadResult result;
    result.filePath = path;
    
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        result.success = false;
        result.errorMessage = "File does not exist";
        result.loadTimeMs = timer.elapsed();
        return result;
    }
    
    if (!fileInfo.isReadable()) {
        result.success = false;
        result.errorMessage = "File is not readable";
        result.loadTimeMs = timer.elapsed();
        return result;
    }
    
    try {
        QImageReader reader(path);
        if (!reader.canRead()) {
            result.success = false;
            result.errorMessage = "Unsupported image format or corrupted file";
            result.loadTimeMs = timer.elapsed();
            return result;
        }
        
        result.originalSize = reader.size();
        result.format = reader.format();
        
        reader.setAutoTransform(true);
        reader.setQuality(100);
        
        QImage image = reader.read();
        if (image.isNull()) {
            result.success = false;
            result.errorMessage = "Failed to decode image";
            result.loadTimeMs = timer.elapsed();
            return result;
        }
        
        if (image.format() != QImage::Format_RGB32 &&
            image.format() != QImage::Format_ARGB32 &&
            image.format() != QImage::Format_ARGB32_Premultiplied &&
            image.format() != QImage::Format_RGBA8888) {
            image = image.convertToFormat(QImage::Format_RGB32);
        }
        
        result.image = image;
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = QString("Exception during loading: %1").arg(e.what());
    }
    
    result.loadTimeMs = timer.elapsed();
    return result;
}

AsyncImageLoader::AsyncImageLoader(QObject* parent)
    : QObject(parent)
    , m_threadPool(new QThreadPool(this))
{
    m_threadPool->setMaxThreadCount(m_maxConcurrentLoads);
}

AsyncImageLoader::~AsyncImageLoader() {
    m_cancelled.store(true, std::memory_order_relaxed);
    m_threadPool->clear();
    m_threadPool->waitForDone(500);
}

QFuture<LoadResult> AsyncImageLoader::loadImageAsync(const QString& filePath) {
    return QtConcurrent::run(m_threadPool, [filePath]() -> LoadResult {
        ImageLoadTask task(filePath);
        return task.loadImageInternal(filePath);
    });
}

void AsyncImageLoader::loadImageQueued(const QString& filePath) {
    QMutexLocker locker(&m_queueMutex);
    
    if (m_loadQueue.contains(filePath)) {
        return;
    }
    
    m_loadQueue.enqueue(filePath);
    
    QPointer<AsyncImageLoader> guard(this);
    auto* task = new ImageLoadTask(filePath, [guard](const LoadResult& result) {
        if (guard) guard->processCompletedLoad(result);
    }, this);

    m_threadPool->start(task);
}

QFuture<std::vector<LoadResult>> AsyncImageLoader::loadImagesAsync(const QStringList& filePaths) {
    QPointer<AsyncImageLoader> guard(this);
    return QtConcurrent::run(m_threadPool, [guard, filePaths]() -> std::vector<LoadResult> {
        std::vector<LoadResult> results;
        results.reserve(filePaths.size());
        
        for (const auto& path : filePaths) {
            ImageLoadTask task(path);
            LoadResult result = task.loadImageInternal(path);
            results.push_back(result);
            
            if (guard) guard->updateStatistics(result);
        }
        
        return results;
    });
}

void AsyncImageLoader::setMaxConcurrentLoads(int maxLoads) {
    m_maxConcurrentLoads = maxLoads;
    m_threadPool->setMaxThreadCount(maxLoads);
}

void AsyncImageLoader::setCacheEnabled(bool enabled) {
    m_cacheEnabled = enabled;
    if (!enabled) {
        clearCache();
    }
}

void AsyncImageLoader::setCacheSize(int maxSizeMB) {
    m_maxCacheSizeMB = maxSizeMB;
    evictCacheIfNeeded();
}

void AsyncImageLoader::clearCache() {
    QMutexLocker locker(&m_cacheMutex);
    m_imageCache.clear();
    m_currentCacheSizeBytes = 0;
}

bool AsyncImageLoader::isCached(const QString& filePath) const {
    QMutexLocker locker(&m_cacheMutex);
    return m_imageCache.contains(filePath);
}

QImage AsyncImageLoader::getCachedImage(const QString& filePath) const {
    QMutexLocker locker(&m_cacheMutex);
    auto it = m_imageCache.find(filePath);
    if (it != m_imageCache.end()) {
        // Update timestamp to refresh LRU status
        const_cast<CacheEntry&>(*it).timestamp = QDateTime::currentDateTime();
        return it->image;
    }
    return QImage();
}

void AsyncImageLoader::clearQueue() {
    QMutexLocker locker(&m_queueMutex);
    m_loadQueue.clear();
    m_threadPool->clear();
}

int AsyncImageLoader::getQueueSize() const {
    QMutexLocker locker(&m_queueMutex);
    return m_loadQueue.size();
}

AsyncImageLoader::Statistics AsyncImageLoader::getStatistics() const {
    QMutexLocker locker(&m_statsMutex);
    return m_statistics;
}

void AsyncImageLoader::resetStatistics() {
    QMutexLocker locker(&m_statsMutex);
    m_statistics = Statistics{};
}

void AsyncImageLoader::processCompletedLoad(const LoadResult& result) {
    {
        QMutexLocker locker(&m_queueMutex);
        m_loadQueue.removeOne(result.filePath);
    }
    
    if (result.success && m_cacheEnabled) {
        addToCache(result.filePath, result.image);
    }
    
    updateStatistics(result);
    
    if (m_onLoadCompleted) {
        m_onLoadCompleted(result);
    }
}

void AsyncImageLoader::updateStatistics(const LoadResult& result) {
    QMutexLocker locker(&m_statsMutex);
    
    m_statistics.totalLoaded++;
    
    if (result.success) {
        m_statistics.successful++;
    } else {
        m_statistics.failed++;
    }
    
    m_statistics.totalTimeMs += result.loadTimeMs;
    m_statistics.averageLoadTimeMs = static_cast<double>(m_statistics.totalTimeMs) / m_statistics.totalLoaded;
}

void AsyncImageLoader::addToCache(const QString& filePath, const QImage& image) {
    QMutexLocker locker(&m_cacheMutex);
    
    // Calculate image size in bytes
    qint64 imageSizeBytes = static_cast<qint64>(image.sizeInBytes());
    
    // Check if entry already exists and update size
    auto it = m_imageCache.find(filePath);
    if (it != m_imageCache.end()) {
        m_currentCacheSizeBytes -= it->sizeBytes;
    }
    
    CacheEntry entry;
    entry.image = image;
    entry.timestamp = QDateTime::currentDateTime();
    entry.sizeBytes = imageSizeBytes;
    
    m_imageCache[filePath] = entry;
    m_currentCacheSizeBytes += imageSizeBytes;
    
    evictCacheIfNeeded();
}

void AsyncImageLoader::evictCacheIfNeeded() {
    qint64 maxCacheSizeBytes = static_cast<qint64>(m_maxCacheSizeMB) * 1024 * 1024;
    
    while (m_currentCacheSizeBytes > maxCacheSizeBytes && !m_imageCache.isEmpty()) {
        // Find the least recently used entry (oldest timestamp)
        QString oldestKey;
        QDateTime oldestTimestamp = QDateTime::currentDateTime();
        
        for (auto it = m_imageCache.constBegin(); it != m_imageCache.constEnd(); ++it) {
            if (it->timestamp < oldestTimestamp) {
                oldestTimestamp = it->timestamp;
                oldestKey = it.key();
            }
        }
        
        if (!oldestKey.isEmpty()) {
            m_currentCacheSizeBytes -= m_imageCache[oldestKey].sizeBytes;
            m_imageCache.remove(oldestKey);
        }
    }
}

} // namespace Glance::IO
