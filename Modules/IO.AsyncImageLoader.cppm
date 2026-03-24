/**
 * @file IO.AsyncImageLoader.cppm
 * @brief Asynchronous multi-threaded image loader with LRU caching
 *
 * Provides AsyncImageLoader for loading images on a background thread
 * pool with configurable concurrency, queue-based request management,
 * LRU cache, and progress/cancellation support. Uses LoadResult for
 * structured return values including metadata and error information.
 */
module;

#include <QObject>
#include <QPointer>
#include <QRunnable>
#include <QThreadPool>
#include <QMutex>
#include <QQueue>
#include <QDateTime>
#include <QString>
#include <QImage>
#include <QSize>
#include <QHash>
#include <QStringList>
#include <QFuture>
#include <memory>
#include <vector>
#include <optional>
#include <functional>
#include <future>
#include <atomic>
#include "../IO/LoadResult.h"

export module IO.AsyncImageLoader;

namespace Glance::IO {

export using ::Glance::IO::LoadResult;

/**
 * @brief Task for loading a single image asynchronously
 * 
 * This class represents a single image loading operation that can be executed
 * in a thread pool. It implements QRunnable for thread pool integration and
 * QObject for signal/slot support.
 */
export class ImageLoadTask : public QObject, public QRunnable {
public:
    using LoadCompletedCallback = std::function<void(const LoadResult&)>;

    /**
     * @brief Construct an image load task
     * @param filePath Path to the image file to load
     * @param callback Optional callback to invoke when loading completes
     * @param parent Optional parent QObject
     */
    explicit ImageLoadTask(const QString& filePath, LoadCompletedCallback callback = nullptr, QObject* parent = nullptr);
    
    /**
     * @brief Execute the image loading operation
     * Called by the thread pool when this task is scheduled
     */
    void run() override;
    
    /**
     * @brief Internal method to load the image from disk
     * @param path Path to the image file
     * @return LoadResult containing the loaded image or error information
     */
    LoadResult loadImageInternal(const QString& path);

private:
    QString m_filePath;
    LoadCompletedCallback m_callback;
    QPointer<QObject> m_parent;
};

/**
 * @brief Asynchronous image loader with caching and queue management
 * 
 * This class provides high-performance asynchronous image loading with:
 * - Thread pool based concurrent loading
 * - LRU cache for frequently accessed images
 * - Load queue management with priority control
 * - Statistics tracking for performance monitoring
 * 
 * Supports both single image and batch loading operations.
 */
export class AsyncImageLoader : public QObject {
public:
    using LoadCompletedCallback = std::function<void(const LoadResult&)>;

    /**
     * @brief Construct an async image loader
     * @param parent Optional parent QObject
     */
    explicit AsyncImageLoader(QObject* parent = nullptr);
    
    /**
     * @brief Destructor - cleans up thread pool and resources
     */
    ~AsyncImageLoader();

    /**
     * @brief Load a single image asynchronously
     * @param filePath Path to the image file
     * @return QFuture that will contain the LoadResult when complete
     */
    QFuture<LoadResult> loadImageAsync(const QString& filePath);
    
    /**
     * @brief Queue an image for loading (managed by internal queue)
     * @param filePath Path to the image file
     */
    void loadImageQueued(const QString& filePath);
    
    /**
     * @brief Load multiple images asynchronously
     * @param filePaths List of paths to image files
     * @return QFuture that will contain vector of LoadResults when complete
     */
    QFuture<std::vector<LoadResult>> loadImagesAsync(const QStringList& filePaths);
    
    /**
     * @brief Set maximum number of concurrent load operations
     * @param maxLoads Maximum concurrent loads (default: 4)
     */
    void setMaxConcurrentLoads(int maxLoads);
    
    /**
     * @brief Enable or disable the image cache
     * @param enabled True to enable caching, false to disable
     */
    void setCacheEnabled(bool enabled);
    
    /**
     * @brief Set maximum cache size in megabytes
     * @param maxSizeMB Maximum cache size in MB (default: 100)
     */
    void setCacheSize(int maxSizeMB);
    
    /**
     * @brief Clear all cached images
     */
    void clearCache();
    
    /**
     * @brief Check if an image is currently cached
     * @param filePath Path to the image file
     * @return True if image is in cache, false otherwise
     */
    bool isCached(const QString& filePath) const;
    
    /**
     * @brief Retrieve a cached image
     * @param filePath Path to the image file
     * @return Cached image, or null QImage if not cached
     */
    QImage getCachedImage(const QString& filePath) const;
    
    /**
     * @brief Clear all pending load operations from the queue
     */
    void clearQueue();
    
    /**
     * @brief Get the current size of the load queue
     * @return Number of pending load operations
     */
    int getQueueSize() const;
    
    /**
     * @brief Statistics structure for load operation tracking
     */
    struct Statistics {
        int totalLoaded = 0;           ///< Total number of load attempts
        int successful = 0;            ///< Number of successful loads
        int failed = 0;                 ///< Number of failed loads
        qint64 totalTimeMs = 0;        ///< Total time spent loading (milliseconds)
        double averageLoadTimeMs = 0.0; ///< Average load time per image
    };
    
    /**
     * @brief Get current loading statistics
     * @return Statistics structure with current metrics
     */
    Statistics getStatistics() const;
    
    /**
     * @brief Reset all statistics to zero
     */
    void resetStatistics();

    /**
     * @brief Set callback to be invoked when any load completes
     * @param callback Function to call with LoadResult
     */
    void setOnLoadCompleted(LoadCompletedCallback callback) { m_onLoadCompleted = std::move(callback); }

private:
    void processCompletedLoad(const LoadResult& result);
    void updateStatistics(const LoadResult& result);
    void addToCache(const QString& filePath, const QImage& image);
    void evictCacheIfNeeded();

private:
    QThreadPool* m_threadPool;
    QQueue<QString> m_loadQueue;
    mutable QMutex m_queueMutex;
    
    struct CacheEntry {
        QImage image;
        QDateTime timestamp;
        qint64 sizeBytes = 0;
    };
    
    mutable QMutex m_cacheMutex;
    QHash<QString, CacheEntry> m_imageCache;
    bool m_cacheEnabled = true;
    int m_maxCacheSizeMB = 100;
    qint64 m_currentCacheSizeBytes = 0;
    
    mutable QMutex m_statsMutex;
    Statistics m_statistics;
    
    int m_maxConcurrentLoads = 4;
    
    LoadCompletedCallback m_onLoadCompleted;
    std::atomic<bool> m_cancelled{false};
};

/**
 * @brief Concept for types that can be converted to image paths
 */
template<typename T>
concept LoadableImage = requires(T t) {
    { QString(t) } -> std::convertible_to<QString>;
};

/**
 * @brief Modern C++23 image loader using std::future
 *
 * This template class provides a more modern interface using std::future
 * instead of QFuture for image loading operations.
 * 
 * @tparam PathType Type that satisfies LoadableImage concept
 */
export template<LoadableImage PathType>
class ModernImageLoader {
public:
    using LoadCallback = std::function<void(const LoadResult&)>;
    
    /**
     * @brief Load an image asynchronously and return a std::future
     * @param path Path to the image file
     * @return std::future that will contain the LoadResult
     */
    std::future<LoadResult> loadAsync(PathType&& path) {
        return std::async(std::launch::async, [path = std::forward<PathType>(path)]() {
            AsyncImageLoader loader;
            auto future = loader.loadImageAsync(QString(path));
            return future.result();
        });
    }
    
    /**
     * @brief Load an image asynchronously with callback
     * @param path Path to the image file
     * @param callback Function to call when load completes
     */
    void loadAsync(PathType&& path, LoadCallback callback) {
        std::thread([path = std::forward<PathType>(path), callback]() {
            AsyncImageLoader loader;
            auto future = loader.loadImageAsync(QString(path));
            callback(future.result());
        }).detach();
    }
};

} // namespace Glance::IO
