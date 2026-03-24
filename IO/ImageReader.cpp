module;
#include <QImageReader>
#include <QFileInfo>
#include <QBuffer>
#include <QDir>
#include <QDebug>
#include <QRegularExpression>
module IO.ImageReader;

#ifdef HAVE_LIBRAW
#include <libraw/libraw.h>
#endif

#ifndef NO_OPENCV
#include <opencv4/opencv2/opencv.hpp>
#endif

namespace Glance::IO {

const QMap<QByteArray, ImageFormat> ImageReader::s_formatSignatures = {
    {QByteArray("\xFF\xD8\xFF"), ImageFormat::JPEG},
    {QByteArray("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"), ImageFormat::PNG},
    {QByteArray("II*\x00"), ImageFormat::TIFF},
    {QByteArray("MM\x00*"), ImageFormat::TIFF},
    {QByteArray("BM"), ImageFormat::BMP},
    {QByteArray("RIFF"), ImageFormat::WebP},
    {QByteArray::fromHex("00000010"), ImageFormat::HEIC},
    {QByteArray::fromHex("00000020"), ImageFormat::HEIC},
    {QByteArray("\x49\x49\x2A\x00"), ImageFormat::TIFF},
    {QByteArray("\x4D\x4D\x00\x2A"), ImageFormat::TIFF},
};

const QMap<QString, ImageFormat> ImageReader::s_extensionFormats = {
    {"jpg", ImageFormat::JPEG},
    {"jpeg", ImageFormat::JPEG},
    {"png", ImageFormat::PNG},
    {"tiff", ImageFormat::TIFF},
    {"tif", ImageFormat::TIFF},
    {"bmp", ImageFormat::BMP},
    {"webp", ImageFormat::WebP},
    {"heic", ImageFormat::HEIC},
    {"heif", ImageFormat::HEIC},
    {"cr2", ImageFormat::RAW},
    {"nef", ImageFormat::RAW},
    {"arw", ImageFormat::RAW},
    {"dng", ImageFormat::RAW},
    {"orf", ImageFormat::RAW},
    {"rw2", ImageFormat::RAW},
    {"pef", ImageFormat::RAW},
    {"srw", ImageFormat::RAW},
    {"mos", ImageFormat::RAW},
    {"mrw", ImageFormat::RAW},
    {"3fr", ImageFormat::RAW},
    {"erf", ImageFormat::RAW},
    {"kdc", ImageFormat::RAW},
    {"ptx", ImageFormat::RAW},
    {"r3d", ImageFormat::RAW}
};

std::optional<QImage> ImageReader::read(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isReadable()) {
        return std::nullopt;
    }
    
    // Try RAW first if it's a RAW format
    if (isRawFormat(filePath)) {
        auto rawImage = readRawImage(filePath);
        if (rawImage) {
            return preprocessImage(*rawImage);
        }
    }
    
    // Fall back to Qt's image reader
    auto qtImage = readWithQt(filePath);
    if (qtImage) {
        return preprocessImage(*qtImage);
    }
    
    return std::nullopt;
}

std::optional<QImage> ImageReader::read(const QByteArray& data) {
    if (data.isEmpty()) {
        return std::nullopt;
    }
    
    // Try with Qt first
    QBuffer buffer(const_cast<QByteArray*>(&data));
    buffer.open(QIODevice::ReadOnly);
    
    QImageReader reader(&buffer);
    if (reader.canRead()) {
        QImage image = reader.read();
        if (!image.isNull()) {
            return preprocessImage(image);
        }
    }
    
    // Try with stb_image if available
    auto stbImage = readWithStb(data);
    if (stbImage) {
        return preprocessImage(*stbImage);
    }
    
    return std::nullopt;
}

std::optional<QImage> ImageReader::read(QIODevice* device) {
    if (!device || !device->isReadable()) {
        return std::nullopt;
    }
    
    QImageReader reader(device);
    if (reader.canRead()) {
        QImage image = reader.read();
        if (!image.isNull()) {
            device->close();
            return preprocessImage(image);
        }
    }
    
    device->close();
    return std::nullopt;
}

std::optional<ImageMetadata> ImageReader::extractMetadata(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return std::nullopt;
    }
    
    ImageMetadata metadata;
    metadata.format = detectFormat(filePath);
    
    // Extract basic metadata using Qt
    QImageReader reader(filePath);
    if (reader.canRead()) {
        metadata.size = reader.size();
        metadata.format = detectFormat(filePath);
        
        // Get additional metadata
        auto qtMetadata = extractQtMetadata(filePath);
        metadata.dpi = qtMetadata.dpi;
        metadata.hasAlpha = qtMetadata.hasAlpha;
        metadata.colorDepth = qtMetadata.colorDepth;
        metadata.colorSpace = qtMetadata.colorSpace;
        metadata.exifData = qtMetadata.exifData;
    }
    
    return metadata;
}

ImageFormat ImageReader::detectFormat(const QString& filePath) const {
    // First try to detect from file signature
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray signature = file.read(16);
        file.close();
        
        ImageFormat fromSignature = detectFromSignature(signature);
        if (fromSignature != ImageFormat::Unknown) {
            return fromSignature;
        }
    }
    
    // Fall back to extension detection
    return detectFromExtension(filePath);
}

ImageFormat ImageReader::detectFormat(const QByteArray& data) const {
    return detectFromSignature(data);
}

bool ImageReader::isFormatSupported(ImageFormat format) {
    switch (format) {
        case ImageFormat::JPEG:
        case ImageFormat::PNG:
        case ImageFormat::BMP:
        case ImageFormat::WebP:
            return true;
            
        case ImageFormat::TIFF:
            return QImageReader::supportedImageFormats().contains("tiff");
            
        case ImageFormat::HEIC:
            return QImageReader::supportedImageFormats().contains("heic");
            
        case ImageFormat::RAW:
#ifdef HAVE_LIBRAW
            return true;
#else
            return false;
#endif
            
        default:
            return false;
    }
}

std::vector<ImageFormat> ImageReader::getSupportedFormats() {
    std::vector<ImageFormat> formats;
    
    for (const auto& format : s_formatSignatures.values()) {
        if (isFormatSupported(format)) {
            formats.push_back(format);
        }
    }
    
    return formats;
}

QStringList ImageReader::getSupportedExtensions() {
    QStringList extensions;
    
    // Add all extensions from our format mapping
    extensions << s_extensionFormats.keys();
    
    // Add RAW extensions if libraw is available
#ifdef HAVE_LIBRAW
    extensions << "cr2" << "nef" << "arw" << "dng" << "orf" << "rw2";
#endif
    
    return extensions;
}

bool ImageReader::isRawFormat(const QString& filePath) {
    QString extension = QFileInfo(filePath).suffix().toLower();
    return s_extensionFormats.contains(extension) && 
           s_extensionFormats.value(extension) == ImageFormat::RAW;
}

std::optional<QImage> ImageReader::readRawImage(const QString& filePath) {
#ifdef HAVE_LIBRAW
    try {
        LibRaw processor;
        
        int ret = processor.open_file(filePath.toLocal8Bit().constData());
        if (ret != LIBRAW_SUCCESS) {
            return std::nullopt;
        }
        
        ret = processor.unpack();
        if (ret != LIBRAW_SUCCESS) {
            return std::nullopt;
        }
        
        ret = processor.dcraw_process();
        if (ret != LIBRAW_SUCCESS) {
            return std::nullopt;
        }
        
        libraw_processed_image_t* processed = processor.dcraw_make_mem_image();
        if (!processed) {
            return std::nullopt;
        }
        
        // Sanity-check image dimensions before touching pixel data
        constexpr int kMaxDim = 65536;
        int w = processed->width;
        int h = processed->height;
        if (w <= 0 || h <= 0 || w > kMaxDim || h > kMaxDim) {
            LibRaw::dcraw_clear_mem(processed);
            qWarning() << "RAW image has invalid dimensions:" << w << "x" << h;
            return std::nullopt;
        }
        
        // Guard against 32-bit overflow in total byte calculation
        size_t expectedRow = static_cast<size_t>(w) * 3;
        size_t expectedTotal = expectedRow * static_cast<size_t>(h);
        if (expectedTotal > static_cast<size_t>(processed->data_size)) {
            LibRaw::dcraw_clear_mem(processed);
            qWarning() << "RAW image data_size" << processed->data_size
                       << "smaller than expected" << expectedTotal;
            return std::nullopt;
        }
        
        // Convert to QImage (wraps external buffer, QImage::Format_RGB888 is 3 bpp)
        QImage image(processed->data, w, h,
                     static_cast<int>(expectedRow), QImage::Format_RGB888);
        
        // Make a copy as the data will be freed
        QImage result = image.copy();
        LibRaw::dcraw_clear_mem(processed);
        
        return result;
        
    } catch (const std::exception& e) {
        qWarning() << "RAW processing error:" << e.what();
        return std::nullopt;
    }
#else
    Q_UNUSED(filePath)
    qWarning() << "RAW support not available - LibRaw not found at compile time";
    return std::nullopt;
#endif
}

bool ImageReader::isValidImage(const QString& filePath) {
    QImageReader reader(filePath);
    return reader.canRead();
}

bool ImageReader::isValidImage(const QByteArray& data) {
    QBuffer buffer(const_cast<QByteArray*>(&data));
    buffer.open(QIODevice::ReadOnly);
    
    QImageReader reader(&buffer);
    return reader.canRead();
}

void ImageReader::setAutoRotate(bool enabled) {
    m_autoRotate = enabled;
}

void ImageReader::setQuality(int quality) {
    m_quality = qBound(0, quality, 100);
}

void ImageReader::setMaxSize(const QSize& maxSize) {
    m_maxSize = maxSize;
}

std::optional<QImage> ImageReader::readWithQt(const QString& filePath) {
    QImageReader reader(filePath);
    
    if (!reader.canRead()) {
        // Fallback for JPG and other formats when plugins fail
        QImage directImage;
        if (directImage.load(filePath)) {
            qDebug() << "Direct QImage load succeeded for:" << filePath;
            return preprocessImage(directImage);
        }
        qDebug() << "Direct QImage load failed for:" << filePath;
        return std::nullopt;
    }
    
    // Configure reader
    reader.setAutoTransform(m_autoRotate);
    reader.setQuality(m_quality);
    
    // Apply size limit if needed
    if (shouldResize(reader.size())) {
        reader.setScaledSize(reader.size().scaled(m_maxSize, Qt::KeepAspectRatio));
    }
    
    QImage image = reader.read();
    return image.isNull() ? std::nullopt : std::optional<QImage>(image);
}

std::optional<QImage> ImageReader::readWithStb(const QByteArray& data) {
    // stb_image integration not implemented - using Qt's built-in image readers instead
    Q_UNUSED(data)
    return std::nullopt;
}

std::optional<QImage> ImageReader::readRawWithLibRaw(const QString& filePath) {
    return readRawImage(filePath);
}

ImageFormat ImageReader::detectFromSignature(const QByteArray& data) const {
    for (auto it = s_formatSignatures.constBegin(); 
         it != s_formatSignatures.constEnd(); 
         ++it) {
        if (data.startsWith(it.key())) {
            return it.value();
        }
    }
    return ImageFormat::Unknown;
}

ImageFormat ImageReader::detectFromExtension(const QString& filePath) const {
    QString extension = QFileInfo(filePath).suffix().toLower();
    return s_extensionFormats.value(extension, ImageFormat::Unknown);
}

ImageMetadata ImageReader::extractQtMetadata(const QString& filePath) {
    ImageMetadata metadata;
    
    QImageReader reader(filePath);
    if (reader.canRead()) {
        metadata.size = reader.size();
        metadata.format = detectFormat(filePath);
        
        // Get DPI (QImageReader doesn't have dotsPerMeterX in Qt6)
        // DPI detection for Qt6 not implemented - using default DPI
        metadata.dpi = 72; // Default DPI

        // Try to read EXIF data
        // EXIF extraction not implemented - metadata not currently supported
    }
    
    return metadata;
}

ImageMetadata ImageReader::extractExifMetadata(const QByteArray& exifData) {
    ImageMetadata metadata;
    // EXIF parsing not implemented - metadata not currently supported
    Q_UNUSED(exifData)
    return metadata;
}

QImage ImageReader::preprocessImage(const QImage& image) {
    QImage result = image;
    
    // Convert to standard format if needed
    if (result.format() != QImage::Format_RGB888 && 
        result.format() != QImage::Format_RGBA8888) {
        result = result.convertToFormat(QImage::Format_RGB888);
    }
    
    // Apply size limit if needed
    if (shouldResize(result.size())) {
        result = result.scaled(m_maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    return result;
}

bool ImageReader::shouldResize(const QSize& originalSize) const {
    return originalSize.width() > m_maxSize.width() || 
           originalSize.height() > m_maxSize.height();
}

std::unique_ptr<ImageReader> ImageReaderFactory::create(ReaderType type) {
    auto reader = std::make_unique<ImageReader>();
    
    switch (type) {
        case ReaderType::HighQuality:
            reader->setQuality(100);
            reader->setAutoRotate(true);
            break;
            
        case ReaderType::Fast:
            reader->setQuality(85);
            reader->setAutoRotate(false);
            reader->setMaxSize({2048, 2048});
            break;
            
        case ReaderType::RawSpecialized:
            // No special configuration needed, RAW support is automatic
            break;
            
        case ReaderType::Default:
        default:
            // Use default settings
            break;
    }
    
    return reader;
}

} // namespace Glance::IO
