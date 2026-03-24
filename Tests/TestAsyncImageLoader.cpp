#include <catch2/catch_test_macros.hpp>
#include <QImage>
#include <QDir>
#include <QFile>
#include "../IO/LoadResult.h"
import IO.AsyncImageLoader;

using namespace Glance::IO;

struct AsyncImageLoaderFixture {
    std::unique_ptr<AsyncImageLoader> loader;
    QString testImagePath;

    AsyncImageLoaderFixture() {
        loader = std::make_unique<AsyncImageLoader>();
        createTestImage();
    }

    void createTestImage() {
        testImagePath = QDir::tempPath() + "/async_loader_test.png";

        QImage testImage(400, 300, QImage::Format_RGB32);
        testImage.fill(Qt::blue);
        REQUIRE(testImage.save(testImagePath));
    }

    ~AsyncImageLoaderFixture() {
        QFile::remove(testImagePath);
    }
};

TEST_CASE_METHOD(AsyncImageLoaderFixture, "AsyncImageLoader creation", "[asyncloader]") {
    REQUIRE(loader != nullptr);
}

TEST_CASE_METHOD(AsyncImageLoaderFixture, "Queue management", "[asyncloader]") {
    REQUIRE(loader->getQueueSize() >= 0);
    loader->clearQueue();
    REQUIRE(loader->getQueueSize() == 0);
}

TEST_CASE_METHOD(AsyncImageLoaderFixture, "Cache management", "[asyncloader]") {
    loader->setCacheEnabled(true);
    loader->clearCache();
    REQUIRE(!loader->isCached(testImagePath));
}
