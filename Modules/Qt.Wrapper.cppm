/**
 * @file Qt.Wrapper.cppm
 * @brief Central re-export module for commonly used Qt types
 *
 * Aggregates and re-exports frequently used Qt types (QString, QImage,
 * QRect, QColor, QOpenGL*, etc.) so that other modules in this project
 * can import them from a single location rather than managing individual
 * Qt module imports.
 *
 * @note This module only contains using-declarations; all types are
 *       imported from their original Qt modules.
 */
module;

#include <QObject>
#include <QWidget>
#include <QString>
#include <QStringList>
#include <QImage>
#include <QByteArray>
#include <QRect>
#include <QSize>
#include <QPoint>
#include <QPointF>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QVariant>
#include <QFuture>
#include <QFutureWatcher>
#include <QVector>
#include <QList>
#include <QMap>
#include <QHash>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QIODevice>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>

export module Qt.Wrapper;

export
{
    using ::QObject;
    using ::QWidget;

    using ::QString;
    using ::QStringList;
    using ::QImage;
    using ::QByteArray;

    using ::QRect;
    using ::QSize;
    using ::QPoint;
    using ::QPointF;

    using ::QColor;
    using ::QPen;
    using ::QBrush;
    using ::QPainter;

    using ::QVariant;

    using ::QFuture;
    using ::QFutureWatcher;

    using ::QVector;
    using ::QList;
    using ::QMap;
    using ::QHash;

    using ::QJsonObject;
    using ::QJsonDocument;
    using ::QJsonArray;

    using ::QIODevice;

    using ::QOpenGLFunctions_4_3_Core;
    using ::QOpenGLContext;
    using ::QOffscreenSurface;
    using ::QOpenGLShaderProgram;
    using ::QOpenGLTexture;
    using ::QOpenGLBuffer;
}
