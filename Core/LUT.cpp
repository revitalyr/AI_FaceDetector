module;
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <algorithm>
#include <cmath>
module Core.LUT;

namespace Glance::Core {

static float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

std::optional<CubeLUT> CubeLUT::parse(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open LUT file:" << filePath;
        return std::nullopt;
    }

    CubeLUT lut;
    QTextStream in(&file);
    std::vector<float> entries;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty() || line.startsWith('#'))
            continue;

        if (line.startsWith("LUT_3D_SIZE", Qt::CaseInsensitive)) {
            bool ok = false;
            int s = line.section(' ', 1).trimmed().toInt(&ok);
            if (!ok || s < 2 || s > 256) {
                qWarning() << "Invalid LUT_3D_SIZE:" << line;
                return std::nullopt;
            }
            lut.size = s;
            continue;
        }

        if (line.startsWith("LUT_1D_SIZE", Qt::CaseInsensitive)) {
            qWarning() << "1D LUTs not supported:" << filePath;
            return std::nullopt;
        }

        bool ok1 = false, ok2 = false, ok3 = false;
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.size() >= 3) {
            float r = parts[0].toFloat(&ok1);
            float g = parts[1].toFloat(&ok2);
            float b = parts[2].toFloat(&ok3);
            if (ok1 && ok2 && ok3) {
                entries.push_back(std::clamp(r, 0.0f, 1.0f));
                entries.push_back(std::clamp(g, 0.0f, 1.0f));
                entries.push_back(std::clamp(b, 0.0f, 1.0f));
            }
        }
    }

    file.close();

    if (lut.size == 0) {
        int dim = static_cast<int>(std::round(std::cbrt(static_cast<double>(entries.size()) / 3.0)));
        if (dim * dim * dim * 3 != static_cast<int>(entries.size())) {
            qWarning() << "Could not determine LUT size from" << filePath;
            return std::nullopt;
        }
        lut.size = dim;
    }

    size_t expected = static_cast<size_t>(lut.size) * lut.size * lut.size * 3;
    if (entries.size() != expected) {
        qWarning() << "LUT entry count mismatch: got" << entries.size()
                    << "expected" << expected;
        entries.resize(expected, 0.0f);
    }

    lut.data = std::move(entries);
    qDebug() << "Loaded LUT:" << filePath << "size:" << lut.size;
    return lut;
}

void CubeLUT::apply(float& r, float& g, float& b) const
{
    if (!isValid())
        return;

    const int s = size;
    const int s1 = s - 1;

    float fr = std::clamp(r, 0.0f, 1.0f) * s1;
    float fg = std::clamp(g, 0.0f, 1.0f) * s1;
    float fb = std::clamp(b, 0.0f, 1.0f) * s1;

    int ir = std::min(static_cast<int>(fr), s1 - 1);
    int ig = std::min(static_cast<int>(fg), s1 - 1);
    int ib = std::min(static_cast<int>(fb), s1 - 1);

    float tr = fr - ir;
    float tg = fg - ig;
    float tb = fb - ib;

    auto idx = [s](int r, int g, int b) -> size_t {
        return (static_cast<size_t>(b) * s + g) * s + r;
    };

    auto fetch = [&](int ri, int gi, int bi, float& or_, float& og, float& ob) {
        size_t i = idx(ri, gi, bi) * 3;
        or_ = data[i];
        og = data[i + 1];
        ob = data[i + 2];
    };

    float v000r, v000g, v000b; fetch(ir, ig, ib, v000r, v000g, v000b);
    float v100r, v100g, v100b; fetch(ir + 1, ig, ib, v100r, v100g, v100b);
    float v010r, v010g, v010b; fetch(ir, ig + 1, ib, v010r, v010g, v010b);
    float v110r, v110g, v110b; fetch(ir + 1, ig + 1, ib, v110r, v110g, v110b);
    float v001r, v001g, v001b; fetch(ir, ig, ib + 1, v001r, v001g, v001b);
    float v101r, v101g, v101b; fetch(ir + 1, ig, ib + 1, v101r, v101g, v101b);
    float v011r, v011g, v011b; fetch(ir, ig + 1, ib + 1, v011r, v011g, v011b);
    float v111r, v111g, v111b; fetch(ir + 1, ig + 1, ib + 1, v111r, v111g, v111b);

    auto interp = [&](float v0, float v1, float v2, float v3,
                      float v4, float v5, float v6, float v7) -> float {
        return lerp(
            lerp(lerp(v0, v1, tr), lerp(v2, v3, tr), tg),
            lerp(lerp(v4, v5, tr), lerp(v6, v7, tr), tg),
            tb
        );
    };

    r = interp(v000r, v100r, v010r, v110r, v001r, v101r, v011r, v111r);
    g = interp(v000g, v100g, v010g, v110g, v001g, v101g, v011g, v111g);
    b = interp(v000b, v100b, v010b, v110b, v001b, v101b, v011b, v111b);
}

} // namespace Glance::Core
