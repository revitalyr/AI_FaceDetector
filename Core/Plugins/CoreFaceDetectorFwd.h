#pragma once

// ============================================================================
// CoreFaceDetectorFwd.h
// ============================================================================
// Bridge header: makes the types defined in the C++23 module Core.FaceDetector
// available to legacy .h files that cannot use `import` directly.
//
// WHY THIS FILE EXISTS
// --------------------
// After the migration to C++23 modules, FaceDetector.h was removed.
// The types DetectedFace, FaceDetectionResult, FaceType, and FaceDetector
// are now declared inside the module interface Core.FaceDetector.cppm.
//
// C++23 rules forbid `import` inside a header file that is included by
// non-module translation units (a "legacy header unit"), so we cannot
// simply write `import Core.FaceDetector;` here.
//
// The canonical solution is a **module adapter header**: we copy only the
// struct/enum/class *declarations* that headers need, guarded by a check
// that the full module definition is the authoritative source.
//
// Any translation unit that needs the *full* implementation (method bodies)
// must itself be a module unit and use:
//     import Core.FaceDetector;
//
// MAINTENANCE
// -----------
// Keep this file in sync with Modules/Core.FaceDetector.cppm.
// Only the types used by plugin/header interfaces need to be listed here.
// ============================================================================

#include <memory>
#include <optional>
#include <vector>
#include <QFuture>
#include <QImage>
#include <QString>
#include <QRect>
#include <QPointF>
#include <cstdint>

// Pull in the type aliases that the structs below depend on
#include "../TypeAliases.h"   // Glance::Core::{ConfidenceValue, ProcessingTimeMs, Bool, …}

namespace Glance::Core {

enum class FaceType {
    Unknown,
    Frontal,
    Profile
};

struct DetectedFace {
    QRect           rect;
    FaceType        type       = FaceType::Frontal;
    ConfidenceValue confidence = 0.0;
    std::vector<QPointF> landmarks;
};

struct FaceDetectionResult {
    std::vector<DetectedFace> faces;
    Bool             success        = false;
    QString          errorMessage;
    ProcessingTimeMs processingTimeMs = 0;
};

class FaceDetector;

} // namespace Glance::Core
