/**
 * @file UI.ComputeHistogram.cppm
 * @brief GPU-accelerated histogram computation using OpenGL compute shaders
 *
 * Wraps OpenGL 4.3 Core compute shaders to calculate image histograms
 * on the GPU. Uses an offscreen surface context for headless operation.
 * Provides both image and raw data access to histogram results.
 */
module;

#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOffscreenSurface>
#include <memory>
#include <vector>

export module UI.ComputeHistogram;

import Qt.Wrapper;

export
{
    using ::QOpenGLFunctions_4_3_Core;
    using ::QOpenGLContext;
    using ::QOffscreenSurface;
    using ::QOpenGLShaderProgram;
    using ::QOpenGLTexture;
    using ::QOpenGLBuffer;
    using ::QImage;
}

export namespace Glance::UI {

/** @brief GPU compute-shader-based histogram calculator */
class ComputeHistogram : protected QOpenGLFunctions_4_3_Core {
public:
    ComputeHistogram();
    ~ComputeHistogram();

    bool initialize();
    void computeHistogram(const QImage& image);
    QImage getHistogramImage() const;
    std::vector<uint32_t> getHistogramData() const;

private:
    void setupShader();
    void setupTextures(const QImage& image);
    void setupBuffers();
    void cleanupGL();

    QOpenGLContext* m_context = nullptr;
    QOffscreenSurface* m_surface = nullptr;
    std::unique_ptr<QOpenGLShaderProgram> m_computeShader;
    std::unique_ptr<QOpenGLTexture> m_imageTexture;
    std::unique_ptr<QOpenGLTexture> m_histogramTexture;
    QOpenGLBuffer m_histogramBuffer;
    QImage m_histogramImage;
    std::vector<uint32_t> m_histogramData;
};

}
