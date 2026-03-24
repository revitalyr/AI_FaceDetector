module;
#include <QDebug>
#include <QOpenGLContext>
#include <QOffscreenSurface>
module UI.ComputeHistogram;

namespace Glance::UI {

ComputeHistogram::ComputeHistogram()
    : m_histogramBuffer(QOpenGLBuffer::VertexBuffer)
    , m_histogramData(256, 0)
{
    QOpenGLContext* shareContext = QOpenGLContext::currentContext();
    if (!shareContext) {
        shareContext = QOpenGLContext::globalShareContext();
    }

    m_context = new QOpenGLContext();
    if (shareContext) {
        m_context->setShareContext(shareContext);
    }
    m_context->setFormat(QSurfaceFormat::defaultFormat());
    m_context->create();

    m_surface = new QOffscreenSurface();
    m_surface->setFormat(m_context->format());
    m_surface->create();
}

ComputeHistogram::~ComputeHistogram()
{
    cleanupGL();
    // Intentionally NOT deleting m_context / m_surface:
    // ~QOpenGLFunctions_4_3_Core asserts that the function-resolver ref count
    // is still > 0 after its own deref. The context holds the other reference,
    // so we must let the context outlive this base-class destructor.
    // The OS reclaims these GPU resources on process exit.
}

bool ComputeHistogram::initialize()
{
    if (!m_context->makeCurrent(m_surface)) {
        qWarning() << "ComputeHistogram: Failed to make OpenGL context current";
        return false;
    }

    initializeOpenGLFunctions();
    setupShader();
    setupBuffers();

    m_context->doneCurrent();
    return true;
}

void ComputeHistogram::cleanupGL()
{
    m_context->makeCurrent(m_surface);
    m_computeShader.reset();
    m_imageTexture.reset();
    m_histogramTexture.reset();
    m_histogramBuffer.destroy();
    // Don't call doneCurrent() here — ~QOpenGLFunctions_4_3_Core runs after
    // this cleanup and asserts that the function resolver still has references.
    // The context (which holds the other reference) is intentionally leaked so
    // it outlives the base-class destructor.
}

void ComputeHistogram::setupShader()
{
    m_computeShader = std::make_unique<QOpenGLShaderProgram>();
    m_computeShader->addShaderFromSourceFile(QOpenGLShader::Compute, ":/shaders/histogram.comp");
    if (!m_computeShader->link()) {
        qWarning() << "Failed to link compute shader:" << m_computeShader->log();
    }
}

void ComputeHistogram::setupBuffers()
{
    m_histogramBuffer.create();
    m_histogramBuffer.bind();
    m_histogramBuffer.allocate(m_histogramData.data(), static_cast<int>(m_histogramData.size() * sizeof(uint32_t)));
    m_histogramBuffer.release();
}

void ComputeHistogram::setupTextures(const QImage& image)
{
    m_imageTexture = std::make_unique<QOpenGLTexture>(image.flipped(Qt::Vertical));
    m_imageTexture->setMinificationFilter(QOpenGLTexture::Nearest);
    m_imageTexture->setMagnificationFilter(QOpenGLTexture::Nearest);
    m_imageTexture->setWrapMode(QOpenGLTexture::ClampToEdge);

    m_histogramTexture = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);
    m_histogramTexture->create();
    m_histogramTexture->setSize(image.width(), image.height());
    m_histogramTexture->setFormat(QOpenGLTexture::RGBA8U);
    m_histogramTexture->allocateStorage();
    m_histogramTexture->setMinificationFilter(QOpenGLTexture::Nearest);
    m_histogramTexture->setMagnificationFilter(QOpenGLTexture::Nearest);
    m_histogramTexture->setWrapMode(QOpenGLTexture::ClampToEdge);

    m_histogramImage = QImage(image.width(), image.height(), QImage::Format_RGBA8888);
}

void ComputeHistogram::computeHistogram(const QImage& image)
{
    if (!m_computeShader || !m_computeShader->isLinked()) {
        qWarning() << "Compute shader not initialized";
        return;
    }

    m_context->makeCurrent(m_surface);

    setupTextures(image);

    m_computeShader->bind();

    m_imageTexture->bind(0);
    m_histogramTexture->bind(1);

    m_histogramBuffer.bind();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_histogramBuffer.bufferId());

    glDispatchCompute((image.width() + 255) / 256, (image.height() + 1) / 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    m_histogramBuffer.bind();
    void* data = m_histogramBuffer.map(QOpenGLBuffer::ReadOnly);
    if (data) {
        memcpy(m_histogramData.data(), data, m_histogramData.size() * sizeof(uint32_t));
        m_histogramBuffer.unmap();
    }

    m_histogramTexture->bind();
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_histogramImage.bits());

    m_histogramTexture->release();
    m_imageTexture->release();
    m_histogramBuffer.release();
    m_computeShader->release();

    m_imageTexture.reset();
    m_histogramTexture.reset();

    m_context->doneCurrent();
}

QImage ComputeHistogram::getHistogramImage() const
{
    return m_histogramImage;
}

std::vector<uint32_t> ComputeHistogram::getHistogramData() const
{
    return m_histogramData;
}

} // namespace Glance::UI
