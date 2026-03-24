module;
#include <QDebug>
#include <QFile>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
module Core.Segmenter;

namespace Glance::Core {

Segmenter::Segmenter() = default;
Segmenter::~Segmenter() = default;

bool Segmenter::loadModel(const QString& modelPath)
{
    try {
        if (!QFile::exists(modelPath)) {
            qWarning() << "Model file not found:" << modelPath;
            return false;
        }

        m_net = std::make_unique<cv::dnn::Net>(
            cv::dnn::readNetFromONNX(modelPath.toStdString())
        );

        if (m_net->empty()) {
            qWarning() << "Failed to load ONNX model:" << modelPath;
            m_net.reset();
            return false;
        }

        m_net->setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        m_net->setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

        qDebug() << "Model loaded:" << modelPath
                 << "input:" << m_inputWidth << "x" << m_inputHeight;
        m_loaded = true;
        return true;

    } catch (const cv::Exception& e) {
        qWarning() << "Failed to load model:" << e.what();
        return false;
    }
}

bool Segmenter::isLoaded() const
{
    return m_loaded;
}

QImage Segmenter::postprocessFromFloat(const float* data, int width, int height, const QImage& original)
{
    // Make a copy of the data to avoid modifying const memory
    cv::Mat maskFloat(height, width, CV_32F);
    std::memcpy(maskFloat.data, data, width * height * sizeof(float));

    cv::Mat maskNorm;
    cv::normalize(maskFloat, maskNorm, 0.0, 1.0, cv::NORM_MINMAX, CV_32F);

    cv::Mat maskResized;
    cv::resize(maskNorm, maskResized, cv::Size(original.width(), original.height()), 0, 0, cv::INTER_LINEAR);

    cv::Mat mask8u;
    maskResized.convertTo(mask8u, CV_8UC1, 255.0);

    QImage src = original;
    if (src.format() != QImage::Format_RGB32 &&
        src.format() != QImage::Format_ARGB32 &&
        src.format() != QImage::Format_ARGB32_Premultiplied) {
        src = src.convertToFormat(QImage::Format_RGB32);
    }

    // Qt's Format_RGB32 / Format_ARGB32 stores pixels as 0xAARRGGBB, which in
    // memory (little-endian) is laid out as bytes: B, G, R, A.
    // OpenCV CV_8UC4 split therefore gives: ch[0]=B, ch[1]=G, ch[2]=R, ch[3]=A.
    // After we replace ch[3] with the mask and merge, the result is still BGRA
    // in memory — matching Qt's Format_ARGB32 layout exactly.
    // Using Format_RGBA8888 (which expects R,G,B,A byte order) would swap
    // red and blue, causing blue-tinted / inverted-colour artefact.
    // Make a copy to avoid modifying const QImage data
    cv::Mat imgBgra(src.height(), src.width(), CV_8UC4);
    std::memcpy(imgBgra.data, src.constBits(), src.sizeInBytes());

    std::vector<cv::Mat> channels(4);
    cv::split(imgBgra, channels);
    channels[3] = mask8u.clone();
    cv::Mat resultMat;
    cv::merge(channels, resultMat);

    QImage result(resultMat.data, resultMat.cols, resultMat.rows,
                  static_cast<int>(resultMat.step), QImage::Format_ARGB32);
    return result.copy();
}

std::optional<SegmentationResult> Segmenter::segment(const QImage& image)
{
    if (!m_loaded || !m_net) {
        qWarning() << "Segmenter not loaded";
        return std::nullopt;
    }

    try {
        // Convert QImage to cv::Mat RGB (Format_RGB888 = R,G,B byte order)
        QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);
        // Make a copy to avoid modifying const QImage data
        cv::Mat imgRgb(rgbImage.height(), rgbImage.width(), CV_8UC3);
        std::memcpy(imgRgb.data, rgbImage.constBits(), rgbImage.sizeInBytes());

        // Input is already RGB, so swapRB=false — no channel swap needed.
        // swapRB=true on an RGB input would produce BRG, not BGR or RGB.
        cv::Mat blob = cv::dnn::blobFromImage(imgRgb, 1.0 / 255.0,
                                               cv::Size(m_inputWidth, m_inputHeight),
                                               cv::Scalar(0.485, 0.456, 0.406),
                                               false /* swapRB */, false /* crop */);
        cv::divide(blob, cv::Scalar(0.229, 0.224, 0.225), blob);

        m_net->setInput(blob);
        std::vector<cv::Mat> outs;
        m_net->forward(outs, m_net->getUnconnectedOutLayersNames());

        if (outs.empty()) {
            qWarning() << "Segmenter: no output from model";
            return std::nullopt;
        }

        cv::Mat prediction = outs[0];
        qDebug() << "Segmenter: output shape (dims)" << prediction.dims
                 << "sizes" << (prediction.dims >= 1 ? prediction.size[0] : -1)
                 << (prediction.dims >= 2 ? prediction.size[1] : -1)
                 << (prediction.dims >= 3 ? prediction.size[2] : -1)
                 << (prediction.dims >= 4 ? prediction.size[3] : -1);

        cv::Mat maskFloat;
        if (prediction.dims == 4) {
            int numChannels = prediction.size[1];
            int H = prediction.size[2];
            int W = prediction.size[3];
            int ch = (numChannels >= 2) ? 1 : 0;
            int sizes[2] = {H, W};
            // Make a copy to avoid modifying const prediction data
            cv::Mat temp(H, W, CV_32F, const_cast<float*>(prediction.ptr<float>(0, ch)));
            maskFloat = temp.clone();
        } else if (prediction.dims == 3) {
            int H = prediction.size[1];
            int W = prediction.size[2];
            int sizes[2] = {H, W};
            // Make a copy to avoid modifying const prediction data
            cv::Mat temp(H, W, CV_32F, const_cast<float*>(prediction.ptr<float>(0)));
            maskFloat = temp.clone();
        } else if (prediction.dims == 2) {
            maskFloat = prediction.clone();
        } else {
            qWarning() << "Segmenter: unexpected output dimensions" << prediction.dims;
            return std::nullopt;
        }

        double minV, maxV;
        cv::minMaxLoc(maskFloat, &minV, &maxV);
        qDebug() << "Segmenter: output range" << minV << "-" << maxV;

        cv::Mat sigmoided;
        cv::exp(-maskFloat, sigmoided);
        sigmoided = 1.0 / (1.0 + sigmoided);

        SegmentationResult result;
        result.composited = postprocessFromFloat(
            sigmoided.ptr<float>(), sigmoided.cols, sigmoided.rows, image);

        cv::Scalar mean = cv::mean(sigmoided);
        result.confidence = std::clamp(static_cast<float>(mean[0]), 0.0f, 1.0f);

        cv::Mat mask8u;
        sigmoided.convertTo(mask8u, CV_8UC1, 255.0);
        QImage maskImg(mask8u.data, mask8u.cols, mask8u.rows,
                       static_cast<int>(mask8u.step), QImage::Format_Grayscale8);
        result.mask = maskImg.copy();

        return result;

    } catch (const cv::Exception& e) {
        qWarning() << "Segmentation error:" << e.what();
        return std::nullopt;
    }
}

} // namespace Glance::Core