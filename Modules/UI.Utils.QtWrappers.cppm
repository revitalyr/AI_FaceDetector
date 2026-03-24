/**
 * @file UI.Utils.QtWrappers.cppm
 * @brief RAII ownership wrappers for QObject and QWidget
 *
 * Provides QtObjectRAII (RAII wrapper for any QObject-derived type)
 * and QtWidgetPtr (RAII wrapper for QWidget using deleteLater for
 * thread safety), plus factory functions make_qt_object/make_qt_widget.
 */
module;

#include <memory>
#include <functional>
#include <type_traits>
#include <optional>

export module UI.Utils.QtWrappers;

import Qt.Wrapper;

export
{
    using ::QObject;
    using ::QWidget;
}

export namespace Glance::Utils {

template<typename T>
requires std::derived_from<T, QObject>
class QtObjectRAII {
public:
    using value_type = T;
    using pointer = T*;

    QtObjectRAII() = default;

    explicit QtObjectRAII(T* ptr, QObject* parent = nullptr)
        : m_ptr(ptr) {
        if (m_ptr && parent) {
            m_ptr->setParent(parent);
        }
    }

    QtObjectRAII(QtObjectRAII&& other) noexcept
        : m_ptr(std::exchange(other.m_ptr, nullptr)) {}

    QtObjectRAII& operator=(QtObjectRAII&& other) noexcept {
        if (this != &other) {
            reset();
            m_ptr = std::exchange(other.m_ptr, nullptr);
        }
        return *this;
    }

    QtObjectRAII(const QtObjectRAII&) = delete;
    QtObjectRAII& operator=(const QtObjectRAII&) = delete;

    ~QtObjectRAII() {
        reset();
    }

    T* get() const noexcept { return m_ptr; }
    T& operator*() const noexcept { return *m_ptr; }
    T* operator->() const noexcept { return m_ptr; }

    explicit operator bool() const noexcept { return m_ptr != nullptr; }

    void reset() noexcept {
        if (m_ptr) {
            delete m_ptr;
            m_ptr = nullptr;
        }
    }

    void reset(T* ptr, QObject* parent = nullptr) {
        reset();
        m_ptr = ptr;
        if (m_ptr && parent) {
            m_ptr->setParent(parent);
        }
    }

    T* release() noexcept {
        return std::exchange(m_ptr, nullptr);
    }

private:
    T* m_ptr = nullptr;
};

template<typename T>
requires std::derived_from<T, QWidget>
class QtWidgetPtr {
public:
    using value_type = T;
    using pointer = T*;

    QtWidgetPtr() = default;

    explicit QtWidgetPtr(T* widget, QWidget* parent = nullptr)
        : m_widget(widget) {
        if (m_widget && parent) {
            m_widget->setParent(parent);
        }
    }

    QtWidgetPtr(QtWidgetPtr&& other) noexcept
        : m_widget(std::exchange(other.m_widget, nullptr)) {}

    QtWidgetPtr& operator=(QtWidgetPtr&& other) noexcept {
        if (this != &other) {
            reset();
            m_widget = std::exchange(other.m_widget, nullptr);
        }
        return *this;
    }

    QtWidgetPtr(const QtWidgetPtr&) = delete;
    QtWidgetPtr& operator=(const QtWidgetPtr&) = delete;

    ~QtWidgetPtr() {
        reset();
    }

    T* get() const noexcept { return m_widget; }
    T& operator*() const noexcept { return *m_widget; }
    T* operator->() const noexcept { return m_widget; }

    explicit operator bool() const noexcept { return m_widget != nullptr; }

    void show() { if (m_widget) m_widget->show(); }
    void hide() { if (m_widget) m_widget->hide(); }
    void close() { if (m_widget) m_widget->close(); }

    void reset() noexcept {
        if (m_widget) {
            m_widget->deleteLater();
            m_widget = nullptr;
        }
    }

    void reset(T* widget, QWidget* parent = nullptr) {
        reset();
        m_widget = widget;
        if (m_widget && parent) {
            m_widget->setParent(parent);
        }
    }

    T* release() noexcept {
        return std::exchange(m_widget, nullptr);
    }

    template<typename... Args>
    static QtWidgetPtr<T> create(QWidget* parent, Args&&... args) {
        return QtWidgetPtr<T>(new T(parent, std::forward<Args>(args)...), parent);
    }

private:
    T* m_widget = nullptr;
};

using QObjectPtr = QtObjectRAII<QObject>;
using QWidgetPtr = QtWidgetPtr<QWidget>;

template<typename T, typename... Args>
QtObjectRAII<T> make_qt_object(Args&&... args) {
    return QtObjectRAII<T>(new T(std::forward<Args>(args)...));
}

template<typename T, typename... Args>
QtWidgetPtr<T> make_qt_widget(QWidget* parent, Args&&... args) {
    return QtWidgetPtr<T>(new T(parent, std::forward<Args>(args)...), parent);
}

}
