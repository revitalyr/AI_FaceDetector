/**
 * @file TraceScope.h
 * @brief RAII function tracing utility
 *
 * TraceScope logs enter/exit events with elapsed time for debugging
 * and performance profiling. Used via the TRACE_SCOPE() macro.
 */
#pragma once

#include <QElapsedTimer>
#include <QDebug>

namespace Glance::Core {

/** @brief Logs function entry/exit with elapsed time */
struct TraceScope {
    const char* m_func;
    QElapsedTimer m_timer;

    explicit TraceScope(const char* func) : m_func(func) {
        m_timer.start();
        qDebug().nospace() << "[ENTER] " << m_func;
    }
    ~TraceScope() {
        qDebug().nospace() << "[EXIT]  " << m_func << " (" << m_timer.elapsed() << "ms)";
    }
};

} // namespace Glance::Core

#define TRACE_SCOPE() Glance::Core::TraceScope _trace_(__FUNCTION__)
