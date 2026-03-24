/**
 * @file UI.Utils.ExpectedUtils.cppm
 * @brief C++23 std::expected-based error handling utilities for Qt
 *
 * Provides Expected<T> and ExpectedWithMessage<T> alias templates
 * using std::expected, along with a QtError error category and
 * factory functions for success/failure creation.
 */
module;

#include <expected>
#include <string>
#include <system_error>

export module UI.Utils.ExpectedUtils;

import Qt.Wrapper;

export
{
    using ::QString;
}

export namespace Glance::Utils {

enum class QtError {
    NoError,
    ObjectCreationFailed,
    WidgetCreationFailed,
    SignalConnectionFailed,
    TimerCreationFailed,
    ImageLoadFailed,
    ImageSaveFailed,
    FileOperationFailed,
    NetworkError,
    UnknownError
};

class QtErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override {
        return "QtError";
    }

    std::string message(int ev) const override {
        switch (static_cast<QtError>(ev)) {
            case QtError::NoError:
                return "No error";
            case QtError::ObjectCreationFailed:
                return "Failed to create Qt object";
            case QtError::WidgetCreationFailed:
                return "Failed to create Qt widget";
            case QtError::SignalConnectionFailed:
                return "Failed to connect signal/slot";
            case QtError::TimerCreationFailed:
                return "Failed to create timer";
            case QtError::ImageLoadFailed:
                return "Failed to load image";
            case QtError::ImageSaveFailed:
                return "Failed to save image";
            case QtError::FileOperationFailed:
                return "File operation failed";
            case QtError::NetworkError:
                return "Network error";
            case QtError::UnknownError:
            default:
                return "Unknown error";
        }
    }
};

inline const QtErrorCategory& qtErrorCategory() {
    static QtErrorCategory category;
    return category;
}

inline std::error_code make_error_code(QtError e) {
    return std::error_code(static_cast<int>(e), qtErrorCategory());
}

template<typename T>
using Expected = std::expected<T, QtError>;

template<typename T>
using ExpectedWithMessage = std::expected<T, QString>;

struct ExpectedUtils {
    template<typename T>
    static Expected<T> success(T&& value) {
        return std::expected<T, QtError>(std::forward<T>(value));
    }

    template<typename T>
    static Expected<T> failure(QtError error) {
        return std::unexpected(error);
    }

    template<typename T>
    static ExpectedWithMessage<T> successWithMessage(T&& value) {
        return std::expected<T, QString>(std::forward<T>(value));
    }

    template<typename T>
    static ExpectedWithMessage<T> failureWithMessage(const QString& message) {
        return std::unexpected(message);
    }

    static QString errorToString(QtError error) {
        return QString::fromStdString(make_error_code(error).message());
    }
};

}
