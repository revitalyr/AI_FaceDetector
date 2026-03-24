/**
 * @file UI.Utils.QtContainers.cppm
 * @brief Adapters and utilities bridging Qt containers with std::span/ranges
 *
 * Provides QtSpan (span-like adapter for QList, QVector, QStringList)
 * and QtRanges (STL-style algorithms adapted for Qt containers).
 */
module;

#include <span>
#include <ranges>
#include <algorithm>
#include <iterator>
#include <vector>

export module UI.Utils.QtContainers;

import Qt.Wrapper;

export
{
    using ::QList;
    using ::QVector;
    using ::QStringList;
}

export namespace Glance::Utils {

template<typename Container>
class QtSpan {
public:
    using value_type = typename Container::value_type;
    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;

    QtSpan() = default;
    explicit QtSpan(Container& container)
        : m_container(&container) {}

    auto begin() const { return m_container->begin(); }
    auto end() const { return m_container->end(); }
    auto cbegin() const { return m_container->cbegin(); }
    auto cend() const { return m_container->cend(); }

    size_t size() const noexcept { return m_container->size(); }
    bool empty() const noexcept { return m_container->isEmpty(); }

    value_type& operator[](size_t index) { return (*m_container)[index]; }
    const value_type& operator[](size_t index) const { return (*m_container)[index]; }

    value_type& front() { return m_container->first(); }
    const value_type& front() const { return m_container->first(); }
    value_type& back() { return m_container->last(); }
    const value_type& back() const { return m_container->last(); }

    value_type* data() { return m_container->data(); }
    const value_type* data() const { return m_container->data(); }

    std::span<value_type> to_span() {
        return std::span<value_type>(m_container->data(), m_container->size());
    }

    std::span<const value_type> to_span() const {
        return std::span<const value_type>(m_container->data(), m_container->size());
    }

private:
    Container* m_container = nullptr;
};

struct QtRanges {
    template<typename Container, typename Transform>
    static auto transform(const Container& container, Transform&& transform) {
        using ResultType = std::invoke_result_t<Transform, typename Container::value_type>;
        QVector<ResultType> result;
        result.reserve(container.size());

        for (const auto& item : container) {
            result.append(transform(item));
        }

        return result;
    }

    template<typename Container, typename Predicate>
    static auto filter(const Container& container, Predicate&& predicate) {
        QVector<typename Container::value_type> result;
        result.reserve(container.size());

        for (const auto& item : container) {
            if (predicate(item)) {
                result.append(item);
            }
        }

        return result;
    }

    template<typename Container, typename Predicate>
    static auto find_if(const Container& container, Predicate&& predicate) {
        return std::find_if(container.begin(), container.end(), std::forward<Predicate>(predicate));
    }

    template<typename Container, typename T>
    static bool contains(const Container& container, const T& value) {
        return std::find(container.begin(), container.end(), value) != container.end();
    }

    template<typename Container>
    static auto to_std_vector(const Container& container) {
        return std::vector<typename Container::value_type>(container.begin(), container.end());
    }

    template<typename T>
    static QVector<T> from_std_vector(const std::vector<T>& vec) {
        QVector<T> result;
        result.reserve(vec.size());
        for (const auto& item : vec) {
            result.append(item);
        }
        return result;
    }
};

using QListSpan = QtSpan<QList<QString>>;
using QVectorSpan = QtSpan<QVector<QString>>;
using QStringListSpan = QtSpan<QStringList>;

}
