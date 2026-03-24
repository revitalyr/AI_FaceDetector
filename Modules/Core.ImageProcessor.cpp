module;

#include <QColor>
#include <QtGlobal>
#include <QtConcurrent>
#include <QFile>
#include <mutex>
#include <algorithm>
#include <numeric>
#include <vector>

module Core.ImageProcessor;

import Core.LUT;

namespace Glance::Core {

QImage ImageProcessor::applyExposure(const QImage& source, const Exposure& exposure) const {
    if (qFuzzyIsNull(exposure.value)) return source;
    
    QImage result = source.copy();
    const int width = result.width();
    const int height = result.height();
    
    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < width; ++x) {
            QRgb pix = line[x];
            float r = qRed(pix) / 255.0f;
            float g = qGreen(pix) / 255.0f;
            float b = qBlue(pix) / 255.0f;
            
            float factor = std::pow(2.0f, exposure.value);
            r = std::clamp(r * factor, 0.0f, 1.0f);
            g = std::clamp(g * factor, 0.0f, 1.0f);
            b = std::clamp(b * factor, 0.0f, 1.0f);

            line[x] = qRgba(
                static_cast<int>(r * 255),
                static_cast<int>(g * 255),
                static_cast<int>(b * 255),
                qAlpha(pix));
        }
    }
    
    return result;
}

QImage ImageProcessor::applyContrast(const QImage& source, const Contrast& contrast) const {
    if (qFuzzyCompare(contrast.value, 1.0f)) return source;
    
    QImage result = source.copy();
    const int width = result.width();
    const int height = result.height();

    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < width; ++x) {
            QRgb pix = line[x];
            float r = qRed(pix) / 255.0f;
            float g = qGreen(pix) / 255.0f;
            float b = qBlue(pix) / 255.0f;
            
            r = std::clamp((r - 0.5f) * contrast.value + 0.5f, 0.0f, 1.0f);
            g = std::clamp((g - 0.5f) * contrast.value + 0.5f, 0.0f, 1.0f);
            b = std::clamp((b - 0.5f) * contrast.value + 0.5f, 0.0f, 1.0f);

            line[x] = qRgba(
                static_cast<int>(r * 255),
                static_cast<int>(g * 255),
                static_cast<int>(b * 255),
                qAlpha(pix));
        }
    }
    
    return result;
}

QImage ImageProcessor::applyBrightness(const QImage& source, const Brightness& brightness) const {
    if (qFuzzyIsNull(brightness.value)) return source;
    
    QImage result = source.copy();
    const int width = result.width();
    const int height = result.height();

    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < width; ++x) {
            QRgb pix = line[x];
            float r = std::clamp((qRed(pix) / 255.0f) + brightness.value, 0.0f, 1.0f);
            float g = std::clamp((qGreen(pix) / 255.0f) + brightness.value, 0.0f, 1.0f);
            float b = std::clamp((qBlue(pix) / 255.0f) + brightness.value, 0.0f, 1.0f);

            line[x] = qRgba(
                static_cast<int>(r * 255),
                static_cast<int>(g * 255),
                static_cast<int>(b * 255),
                qAlpha(pix));
        }
    }
    return result;
}

QImage ImageProcessor::applyGamma(const QImage& source, const Gamma& gamma) const {
    if (qFuzzyCompare(gamma.value, 1.0f)) return source;
    
    QImage result = source.copy();
    const int width = result.width();
    const int height = result.height();

    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < width; ++x) {
            QRgb pix = line[x];
            float r = qRed(pix) / 255.0f;
            float g = qGreen(pix) / 255.0f;
            float b = qBlue(pix) / 255.0f;
            
            r = std::pow(r, 1.0f / gamma.value);
            g = std::pow(g, 1.0f / gamma.value);
            b = std::pow(b, 1.0f / gamma.value);
            
            r = std::clamp(r, 0.0f, 1.0f);
            g = std::clamp(g, 0.0f, 1.0f);
            b = std::clamp(b, 0.0f, 1.0f);

            line[x] = qRgba(
                static_cast<int>(r * 255),
                static_cast<int>(g * 255),
                static_cast<int>(b * 255),
                qAlpha(pix));
        }
    }
    
    return result;
}

QImage ImageProcessor::convertToGrayscale(const QImage& source) const {
    return source.convertToFormat(QImage::Format_Grayscale8);
}

QImage ImageProcessor::applyTemperature(const QImage& source, const Temperature& temperature) const {
    if (source.isNull() || temperature.value == 0.0f) {
        return source;
    }
    
    QImage result = source.convertToFormat(QImage::Format_ARGB32);
    if (result.isNull()) {
        return source;
    }

    const float tempStrength = temperature.value / 100.0f;
    
    for (int y = 0; y < result.height(); ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pix = line[x];
            float r = qRed(pix) / 255.0f;
            float g = qGreen(pix) / 255.0f;
            float b = qBlue(pix) / 255.0f;

            b -= tempStrength * 0.3f;
            r += tempStrength * 0.1f;
            g += tempStrength * 0.05f;

            line[x] = qRgba(
                static_cast<int>(std::clamp(r, 0.0f, 1.0f) * 255),
                static_cast<int>(std::clamp(g, 0.0f, 1.0f) * 255),
                static_cast<int>(std::clamp(b, 0.0f, 1.0f) * 255),
                qAlpha(pix));
        }
    }
    
    return result;
}

QImage ImageProcessor::applyTint(const QImage& source, const Tint& tint) const {
    if (source.isNull() || tint.value == 0.0f) {
        return source;
    }
    
    QImage result = source.convertToFormat(QImage::Format_ARGB32);
    if (result.isNull()) {
        return source;
    }

    const float tintStrength = tint.value / 100.0f;
    
    for (int y = 0; y < result.height(); ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pix = line[x];
            float r = qRed(pix) / 255.0f;
            float g = qGreen(pix) / 255.0f;
            float b = qBlue(pix) / 255.0f;
            
            if (tintStrength > 0.0f) {
                g -= tintStrength * 0.2f;
                r += tintStrength * 0.1f;
                b += tintStrength * 0.1f;
            } else {
                g -= tintStrength * 0.2f;
                r += tintStrength * 0.1f;
                b += tintStrength * 0.1f;
            }
            
            line[x] = qRgba(
                static_cast<int>(std::clamp(r, 0.0f, 1.0f) * 255),
                static_cast<int>(std::clamp(g, 0.0f, 1.0f) * 255),
                static_cast<int>(std::clamp(b, 0.0f, 1.0f) * 255),
                qAlpha(pix));
        }
    }
    
    return result;
}

QImage ImageProcessor::applySaturation(const QImage& source, const Saturation& saturation) const {
    if (source.isNull() || saturation.value == 0.0f) {
        return source;
    }
    
    QImage result = source.convertToFormat(QImage::Format_ARGB32);
    if (result.isNull()) {
        return source;
    }

    const float satStrength = saturation.value / 100.0f;
    
    for (int y = 0; y < result.height(); ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pix = line[x];
            QColor color(pix);
            float h, s, v;
            color.getHsvF(&h, &s, &v);
            
            s = std::clamp(s + satStrength, 0.0f, 1.0f);
            
            color.setHsvF(h, s, v);
            line[x] = color.rgba();
        }
    }
    
    return result;
}

QImage ImageProcessor::applyVibrance(const QImage& source, const Vibrance& vibrance) const {
    if (source.isNull() || vibrance.value == 0.0f) {
        return source;
    }
    
    QImage result = source.convertToFormat(QImage::Format_ARGB32);
    if (result.isNull()) {
        return source;
    }

    const float vibStrength = vibrance.value / 100.0f;
    
    for (int y = 0; y < result.height(); ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pix = line[x];
            float r = qRed(pix) / 255.0f;
            float g = qGreen(pix) / 255.0f;
            float b = qBlue(pix) / 255.0f;
            
            const auto mx = std::max({r, g, b});
            const auto avg = (r + g + b) / 3.0f;
            const auto amt = (mx - avg) * vibStrength;
            
            r = r + (mx - r) * amt;
            g = g + (mx - g) * amt;
            b = b + (mx - b) * amt;
            
            line[x] = qRgba(
                static_cast<int>(std::clamp(r, 0.0f, 1.0f) * 255),
                static_cast<int>(std::clamp(g, 0.0f, 1.0f) * 255),
                static_cast<int>(std::clamp(b, 0.0f, 1.0f) * 255),
                qAlpha(pix));
        }
    }
    
    return result;
}

QImage ImageProcessor::applyHighlightsShadows(const QImage& source, const Highlights& highlights, const Shadows& shadows) const {
    if (source.isNull() || (highlights.value == 0.0f && shadows.value == 0.0f)) {
        return source;
    }
    
    QImage result = source.convertToFormat(QImage::Format_ARGB32);
    if (result.isNull()) {
        return source;
    }

    const float highlightStrength = highlights.value / 100.0f;
    const float shadowStrength = shadows.value / 100.0f;
    
    for (int y = 0; y < result.height(); ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pix = line[x];
            float r = qRed(pix) / 255.0f;
            float g = qGreen(pix) / 255.0f;
            float b = qBlue(pix) / 255.0f;
            
            const auto luminance = 0.299f * r + 0.587f * g + 0.114f * b;
            
            float adjustment = 0.0f;
            if (luminance > 0.5f) {
                adjustment = highlightStrength * (luminance - 0.5f) * 2.0f;
            } else {
                adjustment = shadowStrength * (0.5f - luminance) * 2.0f;
            }
            
            r = std::clamp(r + adjustment, 0.0f, 1.0f);
            g = std::clamp(g + adjustment, 0.0f, 1.0f);
            b = std::clamp(b + adjustment, 0.0f, 1.0f);
            
            line[x] = qRgba(
                static_cast<int>(r * 255),
                static_cast<int>(g * 255),
                static_cast<int>(b * 255),
                qAlpha(pix));
        }
    }
    
    return result;
}

void ImageProcessor::applyDetailsEnhancement(const QImage& src, QImage& dst, const Details& details) const {
    const int    w     = dst.width();
    const int    h     = dst.height();
    const float  amount = details.value / 100.0f;

    if (h < 3 || w < 3) {
        for (int y = 0; y < h; ++y)
            std::memcpy(dst.scanLine(y), src.scanLine(y), static_cast<size_t>(w) * 4);
        return;
    }

    std::memcpy(dst.scanLine(0),     src.scanLine(0),     static_cast<size_t>(w) * 4);
    std::memcpy(dst.scanLine(h - 1), src.scanLine(h - 1), static_cast<size_t>(w) * 4);

    std::vector<int> rows(h - 2);
    std::iota(rows.begin(), rows.end(), 1);

    QtConcurrent::blockingMap(rows, [&](int y) {
        const QRgb* prev = reinterpret_cast<const QRgb*>(src.scanLine(y - 1));
        const QRgb* curr = reinterpret_cast<const QRgb*>(src.scanLine(y));
        const QRgb* next = reinterpret_cast<const QRgb*>(src.scanLine(y + 1));
        QRgb* dstLine = reinterpret_cast<QRgb*>(dst.scanLine(y));

        dstLine[0]     = curr[0];
        dstLine[w - 1] = curr[w - 1];

        for (int x = 1; x < w - 1; ++x) {
            QRgb p = curr[x];

            float lr = 4.0f * qRed(p)   - (qRed(prev[x])   + qRed(next[x])   + qRed(curr[x-1])   + qRed(curr[x+1]));
            float lg = 4.0f * qGreen(p) - (qGreen(prev[x]) + qGreen(next[x]) + qGreen(curr[x-1]) + qGreen(curr[x+1]));
            float lb = 4.0f * qBlue(p)  - (qBlue(prev[x])  + qBlue(next[x])  + qBlue(curr[x-1])  + qBlue(curr[x+1]));

            float r = std::clamp((qRed(p)   + amount * lr) / 255.0f, 0.0f, 1.0f);
            float g = std::clamp((qGreen(p) + amount * lg) / 255.0f, 0.0f, 1.0f);
            float b = std::clamp((qBlue(p)  + amount * lb) / 255.0f, 0.0f, 1.0f);

            dstLine[x] = qRgba(
                static_cast<int>(r * 255.0f),
                static_cast<int>(g * 255.0f),
                static_cast<int>(b * 255.0f),
                qAlpha(p));
        }
    });
}

QImage ImageProcessor::processImage(const QImage& source, const ProcessingParams& params) const {
    if (source.isNull()) {
        return {};
    }

    QImage result = source.convertToFormat(QImage::Format_ARGB32);
    const int w = result.width();
    const int h = result.height();

    // ── Precompute coefficients ──────────────────────────────────────────
    const float expVal = params.getExposure();
    const bool  hasExp = !qFuzzyIsNull(expVal);
    const float expFac = hasExp ? std::pow(2.0f, expVal) : 1.0f;

    const float conVal = params.getContrast();
    const bool  hasCon = !qFuzzyCompare(conVal, 1.0f);

    const float briVal = params.getBrightness();
    const bool  hasBri = !qFuzzyIsNull(briVal);

    const float gamVal = params.getGamma();
    const bool  hasGam = !qFuzzyCompare(gamVal, 1.0f);
    const float gamInv = hasGam ? 1.0f / gamVal : 1.0f;

    const float tempVal = params.getTemperature();
    const bool  hasTemp = tempVal != 0.0f;
    const float tempS   = tempVal / 100.0f;

    const float tintVal = params.getTint();
    const bool  hasTint = tintVal != 0.0f;
    const float tintS   = tintVal / 100.0f;

    const float satVal = params.getSaturation();
    const bool  hasSat = satVal != 0.0f;
    const float satS   = satVal / 100.0f;

    const float vibVal = params.getVibrance();
    const bool  hasVib = vibVal != 0.0f;
    const float vibS   = vibVal / 100.0f;

    const float hlVal = params.getHighlights();
    const float shVal = params.getShadows();
    const bool  hasHlSh = hlVal != 0.0f || shVal != 0.0f;
    const float hlS = hlVal / 100.0f;
    const float shS = shVal / 100.0f;

    const float detVal = params.getDetails();
    const bool  hasDet = detVal != 0.0f;

    // ── Fused parallel pass: all per-pixel adjustments ──────────────────
    if (hasExp || hasCon || hasBri || hasGam || hasTemp ||
        hasTint || hasSat || hasVib || hasHlSh) {
        std::vector<int> rows(h);
        std::iota(rows.begin(), rows.end(), 0);

        QtConcurrent::blockingMap(rows, [&](int y) {
            QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
            for (int x = 0; x < w; ++x) {
                QRgb p = line[x];
                float r = qRed(p)   / 255.0f;
                float g = qGreen(p) / 255.0f;
                float b = qBlue(p)  / 255.0f;

                // Temperature
                if (hasTemp) {
                    b -= tempS * 0.3f;
                    r += tempS * 0.1f;
                    g += tempS * 0.05f;
                }

                // Tint
                if (hasTint) {
                    g -= tintS * 0.2f;
                    r += tintS * 0.1f;
                    b += tintS * 0.1f;
                }

                // Highlights / Shadows
                if (hasHlSh) {
                    const float lum = 0.299f * r + 0.587f * g + 0.114f * b;
                    const float adj = (lum > 0.5f)
                        ? hlS * (lum - 0.5f) * 2.0f
                        : shS * (0.5f - lum) * 2.0f;
                    r += adj; g += adj; b += adj;
                }

                // Brightness
                if (hasBri) {
                    r += briVal;
                    g += briVal;
                    b += briVal;
                }

                // Exposure
                if (hasExp) {
                    r *= expFac;
                    g *= expFac;
                    b *= expFac;
                }

                // Contrast
                if (hasCon) {
                    r = (r - 0.5f) * conVal + 0.5f;
                    g = (g - 0.5f) * conVal + 0.5f;
                    b = (b - 0.5f) * conVal + 0.5f;
                }

                // Saturation
                if (hasSat) {
                    QColor c = QColor::fromRgbF(
                        std::clamp(r, 0.0f, 1.0f),
                        std::clamp(g, 0.0f, 1.0f),
                        std::clamp(b, 0.0f, 1.0f));
                    float hF, sF, vF;
                    c.getHsvF(&hF, &sF, &vF);
                    sF = std::clamp(sF + satS, 0.0f, 1.0f);
                    c.setHsvF(hF, sF, vF);
                    r = c.redF(); g = c.greenF(); b = c.blueF();
                }

                // Vibrance
                if (hasVib) {
                    const float mx  = std::max({r, g, b});
                    const float avg = (r + g + b) / 3.0f;
                    const float amt = (mx - avg) * vibS;
                    r += (mx - r) * amt;
                    g += (mx - g) * amt;
                    b += (mx - b) * amt;
                }

                r = std::clamp(r, 0.0f, 1.0f);
                g = std::clamp(g, 0.0f, 1.0f);
                b = std::clamp(b, 0.0f, 1.0f);

                // Gamma
                if (hasGam) {
                    r = std::pow(r, gamInv);
                    g = std::pow(g, gamInv);
                    b = std::pow(b, gamInv);
                }

                line[x] = qRgba(
                    static_cast<int>(r * 255.0f),
                    static_cast<int>(g * 255.0f),
                    static_cast<int>(b * 255.0f),
                    qAlpha(p));
            }
        });
    }

    // ── Details enhancement (Laplacian, needs neighbour data) ───────────
    if (hasDet && params.details.value != 0.0f) {
        QImage detailResult(result.size(), QImage::Format_ARGB32);
        applyDetailsEnhancement(result, detailResult, params.details);
        result = std::move(detailResult);
    }

    // ── Grayscale & LUT ────────────────────────────────────────────────
    if (params.grayscale) {
        result = result.convertToFormat(QImage::Format_Grayscale8);
    }

    if (params.enableLUT && !params.lutFilePath.isEmpty()) {
        result = applyLUT(result, params.lutFilePath);
    }

    return result;
}

PixelInfo ImageProcessor::getPixelInfo(const QImage& source, const QPoint& position) const noexcept {
    PixelInfo info;
    info.position = position;
    
    if (!source.rect().contains(position)) {
        info.r = info.g = info.b = 0.0f;
        info.normalized_r = info.normalized_g = info.normalized_b = 0.0f;
        return info;
    }
    
    QColor color = source.pixelColor(position);
    
    info.r = static_cast<Float32>(color.red());
    info.g = static_cast<Float32>(color.green());
    info.b = static_cast<Float32>(color.blue());
    info.normalized_r = color.redF();
    info.normalized_g = color.greenF();
    info.normalized_b = color.blueF();
    
    return info;
}

std::vector<int> ImageProcessor::calculateHistogram(const QImage& source, int channel) const {
    std::vector<int> histogram(256, 0);
    
    const int width = source.width();
    const int height = source.height();
    
    QImage img = (source.format() == QImage::Format_RGB32 || source.format() == QImage::Format_ARGB32)
                     ? source
                     : source.convertToFormat(QImage::Format_RGB32);
    for (int y = 0; y < height; ++y) {
        const QRgb* line = reinterpret_cast<const QRgb*>(img.constScanLine(y));
        for (int x = 0; x < width; ++x) {
            QRgb pixel = line[x];
            if (channel == -1) {
                histogram[qGray(pixel)]++;
            } else if (channel == 0) {
                histogram[qRed(pixel)]++;
            } else if (channel == 1) {
                histogram[qGreen(pixel)]++;
            } else if (channel == 2) {
                histogram[qBlue(pixel)]++;
            }
        }
    }
    
    return histogram;
}

QFuture<QImage> ImageProcessor::processImageAsync(const QImage& source, const ProcessingParams& params) const {
    return QtConcurrent::run([this, source, params]() {
        return processImage(source, params);
    });
}

QImage ImageProcessor::applyLUT(const QImage& source, const QString& lutFilePath) const
{
    struct LUTCache {
        std::mutex mtx;
        CubeLUT lut;
        QString path;
    };
    static LUTCache cache;

    {
        std::lock_guard<std::mutex> lock(cache.mtx);
        if (lutFilePath != cache.path) {
            auto parsed = CubeLUT::parse(lutFilePath);
            if (!parsed) {
                qWarning() << "Failed to load LUT:" << lutFilePath;
                return source;
            }
            cache.lut = *parsed;
            cache.path = lutFilePath;
        }

        if (!cache.lut.isValid()) {
            return source;
        }
    }

    QImage result = source.convertToFormat(QImage::Format_ARGB32);
    if (result.isNull()) {
        return source;
    }

    CubeLUT localLut;
    {
        std::lock_guard<std::mutex> lock(cache.mtx);
        localLut = cache.lut;
    }

    for (int y = 0; y < result.height(); ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));
        for (int x = 0; x < result.width(); ++x) {
            QRgb pix = line[x];
            float r = qRed(pix) / 255.0f;
            float g = qGreen(pix) / 255.0f;
            float b = qBlue(pix) / 255.0f;
            localLut.apply(r, g, b);
            line[x] = qRgba(
                static_cast<int>(std::clamp(r, 0.0f, 1.0f) * 255),
                static_cast<int>(std::clamp(g, 0.0f, 1.0f) * 255),
                static_cast<int>(std::clamp(b, 0.0f, 1.0f) * 255),
                qAlpha(pix));
        }
    }

    return result;
}

constexpr Float32 ImageProcessor::clamp(Float32 value, Float32 min, Float32 max) const noexcept {
    return std::clamp(value, min, max);
}

Float32 ImageProcessor::clamp(Float32 value) const noexcept {
    return std::clamp(value, 0.0f, 1.0f);
}

QColor ImageProcessor::adjustPixelColor(const QColor& color, const ProcessingParams& params) const noexcept {
    float r = color.redF();
    float g = color.greenF();
    float b = color.blueF();
    
    if (!qFuzzyIsNull(params.getExposure())) {
        float exposure_factor = std::pow(2.0f, params.getExposure());
        r = clamp(r * exposure_factor);
        g = clamp(g * exposure_factor);
        b = clamp(b * exposure_factor);
    }
    
    if (!qFuzzyCompare(params.getContrast(), 1.0f)) {
        r = clamp((r - 0.5f) * params.getContrast() + 0.5f);
        g = clamp((g - 0.5f) * params.getContrast() + 0.5f);
        b = clamp((b - 0.5f) * params.getContrast() + 0.5f);
    }
    
    if (!qFuzzyIsNull(params.getBrightness())) {
        r = clamp(r + params.getBrightness());
        g = clamp(g + params.getBrightness());
        b = clamp(b + params.getBrightness());
    }
    
    if (!qFuzzyCompare(params.getGamma(), 1.0f)) {
        r = std::pow(r, 1.0f / params.getGamma());
        g = std::pow(g, 1.0f / params.getGamma());
        b = std::pow(b, 1.0f / params.getGamma());
    }
    
    QColor result = color;
    result.setRedF(r);
    result.setGreenF(g);
    result.setBlueF(b);
    
    return result;
}

} // namespace Glance::Core
