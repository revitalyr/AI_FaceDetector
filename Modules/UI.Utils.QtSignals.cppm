/**
 * @file UI.Utils.QtSignals.cppm
 * @brief RAII signal/slot connection management and type-safe signal emitters
 *
 * Provides SignalConnection (RAII wrapper for QObject::connect/disconnect),
 * SignalEmitter (type-safe multi-cast signal with slot registration), and
 * convenience factory functions for connections.
 */
module;

#include <functional>
#include <tuple>
#include <utility>
#include <type_traits>
#include <concepts>
#include <vector>

export module UI.Utils.QtSignals;

import Qt.Wrapper;

export
{
    using ::QObject;
}

export namespace Glance::Utils {

template<typename Sender, typename Signal, typename Receiver, typename Slot>
class SignalConnection {
public:
    SignalConnection(Sender* sender, Signal signal, Receiver* receiver, Slot slot)
        : m_sender(sender)
        , m_signal(signal)
        , m_receiver(receiver)
        , m_slot(slot) {}

    ~SignalConnection() {
        disconnect();
    }

    SignalConnection(SignalConnection&& other) noexcept
        : m_sender(std::exchange(other.m_sender, nullptr))
        , m_signal(other.m_signal)
        , m_receiver(std::exchange(other.m_receiver, nullptr))
        , m_slot(other.m_slot)
        , m_connected(std::exchange(other.m_connected, false)) {}

    SignalConnection& operator=(SignalConnection&& other) noexcept {
        if (this != &other) {
            disconnect();
            m_sender = std::exchange(other.m_sender, nullptr);
            m_signal = other.m_signal;
            m_receiver = std::exchange(other.m_receiver, nullptr);
            m_slot = other.m_slot;
            m_connected = std::exchange(other.m_connected, false);
        }
        return *this;
    }

    SignalConnection(const SignalConnection&) = delete;
    SignalConnection& operator=(const SignalConnection&) = delete;

    bool connect() {
        if (m_connected || !m_sender || !m_receiver) {
            return false;
        }

        bool result = QObject::connect(m_sender, m_signal, m_receiver, m_slot);
        if (result) {
            m_connected = true;
        }
        return result;
    }

    void disconnect() {
        if (m_connected && m_sender && m_receiver) {
            QObject::disconnect(m_sender, m_signal, m_receiver, m_slot);
            m_connected = false;
        }
    }

    bool isConnected() const noexcept { return m_connected; }

private:
    Sender* m_sender = nullptr;
    Signal m_signal;
    Receiver* m_receiver = nullptr;
    Slot m_slot;
    bool m_connected = false;
};

template<typename... Args>
class SignalEmitter {
public:
    template<typename QObjectType>
    void connectTo(QObjectType* obj, void (QObjectType::*slot)(Args...)) {
        m_connections.emplace_back([obj, slot](Args... args) {
            (obj->*slot)(args...);
        });
    }

    void emit(Args... args) {
        for (auto& connection : m_connections) {
            connection(args...);
        }
    }

    size_t connectionCount() const noexcept { return m_connections.size(); }

private:
    std::vector<std::function<void(Args...)>> m_connections;
};

struct QtSignals {
    template<typename Sender, typename Signal, typename Receiver, typename Slot>
    static auto connect(Sender* sender, Signal signal, Receiver* receiver, Slot slot) {
        return SignalConnection<Sender, Signal, Receiver, Slot>(
            sender, signal, receiver, slot
        );
    }

    template<typename Sender, typename Receiver, typename Func>
    static auto connectLambda(Sender* sender, Receiver* receiver, Func&& func) {
        return QObject::connect(sender, receiver, std::forward<Func>(func));
    }
};

}
