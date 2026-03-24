module;
#include <catch2/catch_test_macros.hpp>
#include <QImage>
#include <QTemporaryFile>
#include <QDir>
#include <QFile>
module IO.ImageReader;

using namespace Glance::IO;

struct ImageReaderFixture {
    std::unique_ptr<ImageReader> reader;
    QString testImagePath;
    QString testDir;

    ImageReaderFixture() {
        reader = std::make_unique<ImageReader>();
        testDir = QDir::tempPath() + "/image_reader_test";
        QDir().mkpath(testDir);

        testImagePath = testDir + "/test.png";
        QImage testImage(400, 300, QImage::Format_RGB32);
        testImage.fill(Qt::green);
        testImage.save(testImagePath);
    }

    ~ImageReaderFixture() {
        QDir(testDir).removeRecursively();
    }
};

TEST_CASE_METHOD(ImageReaderFixture, "ImageReader creation", "[imagereader]") {
    REQUIRE(reader != nullptr);
}

TEST_CASE_METHOD(ImageReaderFixture, "Read image from file", "[imagereader]") {
    auto result = reader->read(testImagePath);
    REQUIRE(result.has_value());
    REQUIRE(!result->isNull());
    REQUIRE(result->width() == 400);
    REQUIRE(result->height() == 300);
}

TEST_CASE_METHOD(ImageReaderFixture, "Read non-existent file", "[imagereader]") {
    auto result = reader->read(QString("/nonexistent/file.png"));
    REQUIRE(!result.has_value());
}

TEST_CASE_METHOD(ImageReaderFixture, "Read invalid file", "[imagereader]") {
    QString invalidFile = testDir + "/invalid.txt";
    QFile f(invalidFile);
    (void)f.open(QIODevice::WriteOnly);
    f.write("not an image");
    f.close();

    auto result = reader->read(invalidFile);
    REQUIRE(!result.has_value());
}

TEST_CASE_METHOD(ImageReaderFixture, "Get supported formats", "[imagereader]") {
    auto formats = ImageReader::getSupportedFormats();
    REQUIRE(!formats.empty());
}

TEST_CASE_METHOD(ImageReaderFixture, "Check valid image", "[imagereader]") {
    REQUIRE(ImageReader::isValidImage(testImagePath));
    REQUIRE(!ImageReader::isValidImage(QString("/nonexistent/file.png")));
}

TEST_CASE_METHOD(ImageReaderFixture, "Detect format", "[imagereader]") {
    auto format = reader->detectFormat(testImagePath);
    REQUIRE(format != ImageFormat::Unknown);
}