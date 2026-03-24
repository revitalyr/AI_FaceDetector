#include <catch2/catch_test_macros.hpp>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QSurfaceFormat>
#include <QGuiApplication>

TEST_CASE("Shader: OpenGL context creation", "[shader]") {
    QSurfaceFormat format;
    format.setVersion(4, 1);
    format.setProfile(QSurfaceFormat::CoreProfile);

    QOpenGLContext context;
    context.setFormat(format);

    bool created = context.create();
    REQUIRE(created);

    QOffscreenSurface surface;
    surface.setFormat(format);
    surface.create();

    REQUIRE(surface.isValid());

    if (context.makeCurrent(&surface)) {
        REQUIRE(context.isValid());
        context.doneCurrent();
    }
}

TEST_CASE("Shader: OpenGL version check", "[shader]") {
    QOpenGLContext context;
    if (!context.create()) {
        SKIP("OpenGL not available");
    }

    auto version = context.format().version();
    REQUIRE(version.first >= 3);
}