/**
 * @file IO.ImageReader.cppm
 * @brief Image file reading with format detection and metadata extraction
 *
 * ImageReader supports reading images from files, byte arrays, and
 * I/O devices with automatic format detection (signature + extension),
 * metadata extraction, RAW format support (via LibRaw), auto-rotation,
 * quality control, and max-size constraints. Also provides
 * ImageReaderFactory and ModernImageReader concept-based template.
 */
module;

#include <optional>
#include <memory>
#include <vector>
#include <QSize>
#include <QMap>
#include <QImageReader>
#include <QByteArray>
#include <QBuffer>

export module IO.ImageReader;

import Qt.Wrapper;

export
{
    using ::QString;
    using ::QByteArray;
    using ::QIODevice;
    using ::QImage;
    using ::QSize;
    using ::QMap;
    using ::QStringList;
}

export namespace Glance::IO {

/** @brief Supported image file formats */
enum class ImageFormat {
    Unknown,
    JPEG,
    PNG,
    TIFF,
    BMP,
    RAW,
    HEIC,
    WebP
};

struct ImageMetadata {
    ImageFormat format = ImageFormat::Unknown;
    QSize size;
    int dpi = 72;
    bool hasAlpha = false;
    int colorDepth = 8;
    QString colorSpace;
    QByteArray exifData;
    QMap<QString, QString> customProperties;
};

class ImageReader {
public:
    ImageReader() = default;
    virtual ~ImageReader() = default;

    std::optional<QImage> read(const QString& filePath);
    std::optional<QImage> read(const QByteArray& data);
    std::optional<QImage> read(QIODevice* device);

    std::optional<ImageMetadata> extractMetadata(const QString& filePath);
    ImageFormat detectFormat(const QString& filePath) const;
    ImageFormat detectFormat(const QByteArray& data) const;

    static bool isFormatSupported(ImageFormat format);
    static std::vector<ImageFormat> getSupportedFormats();
    static QStringList getSupportedExtensions();

    static bool isRawFormat(const QString& filePath);
    std::optional<QImage> readRawImage(const QString& filePath);

    static bool isValidImage(const QString& filePath);
    static bool isValidImage(const QByteArray& data);

    void setAutoRotate(bool enabled);
    void setQuality(int quality);
    void setMaxSize(const QSize& maxSize);

private:
    std::optional<QImage> readWithQt(const QString& filePath);
    std::optional<QImage> readWithStb(const QByteArray& data);
    std::optional<QImage> readRawWithLibRaw(const QString& filePath);

    ImageFormat detectFromSignature(const QByteArray& data) const;
    ImageFormat detectFromExtension(const QString& filePath) const;

    ImageMetadata extractQtMetadata(const QString& filePath);
    ImageMetadata extractExifMetadata(const QByteArray& exifData);

    QImage preprocessImage(const QImage& image);
    bool shouldResize(const QSize& originalSize) const;

    bool m_autoRotate = true;
    int m_quality = 95;
    QSize m_maxSize{4096, 4096};

    static const QMap<QByteArray, ImageFormat> s_formatSignatures;
    static const QMap<QString, ImageFormat> s_extensionFormats;
};

class ImageReaderFactory {
public:
    enum class ReaderType {
        Default,
        HighQuality,
        Fast,
        RawSpecialized
    };

    static std::unique_ptr<ImageReader> create(ReaderType type = ReaderType::Default);
};

template<typename T>
concept ImageSource = requires(T t) {
    { QString(t) } -> std::convertible_to<QString>;
} || requires(T t) {
    { QByteArray(t) } -> std::convertible_to<QByteArray>;
};

template<ImageSource Source>
class ModernImageReader {
public:
    std::optional<QImage> read(Source&& source) {
        if constexpr (std::convertible_to<Source, QString>) {
            ImageReader reader;
            return reader.read(QString(std::forward<Source>(source)));
        } else if constexpr (std::convertible_to<Source, QByteArray>) {
            ImageReader reader;
            return reader.read(QByteArray(std::forward<Source>(source)));
        }
        return std::nullopt;
    }
};

}
