#pragma once

/**
 * @file TypeAliases.h
 * @brief Bridge header mirroring Core.TypeAliases.cppm for legacy consumers
 *
 * This file mirrors the type aliases exported by Modules/Core.TypeAliases.cppm
 * so that legacy .h headers (IFaceDetectorPlugin.h, CoreFaceDetectorFwd.h, etc.)
 * can include them without using `import Core.TypeAliases;`.
 *
 * The module Modules/Core.TypeAliases.cppm remains the single source of truth.
 * This file must be kept in sync with it.
 */

#include <cstdint>
#include <cstddef>

namespace Glance::Core {

using Int8    = std::int8_t;
using Int16   = std::int16_t;
using Int32   = std::int32_t;
using Int64   = std::int64_t;

using UInt8   = std::uint8_t;
using UInt16  = std::uint16_t;
using UInt32  = std::uint32_t;
using UInt64  = std::uint64_t;

using Float32 = float;
using Float64 = double;

using Bool    = bool;
using Byte    = UInt8;

using PixelCount    = Int32;
using ImageWidth    = Int32;
using ImageHeight   = Int32;
using ImageSize     = Int32;

using CoordX        = Int32;
using CoordY        = Int32;

using ConfidenceValue     = Float64;
using ConfidenceThreshold = Float64;
using IoUValue            = Float64;
using DetectionScore      = Float64;

using ExposureValue    = Float32;
using ContrastValue    = Float32;
using GammaValue       = Float32;
using TemperatureValue = Float32;
using TintValue        = Float32;
using SaturationValue  = Float32;
using VibranceValue    = Float32;
using HighlightsValue  = Float32;
using ShadowsValue     = Float32;

using FaceSizeMin     = Int32;
using FaceSizeMax     = Int32;
using FaceCount       = Int32;
using FaceIndex       = Int32;

using ScaleFactor      = Float64;
using RotationAngle    = Float64;
using ProcessingTimeMs = Int64;

using RectWidth  = Int32;
using RectHeight = Int32;
using RectArea   = Int32;

using Distance   = Float64;
using Sigma      = Float64;
using ClipLimit  = Float64;

using StrategyIndex  = std::size_t;
using ChannelIndex   = Int32;
using HistogramBin   = Int32;

using ColorComponent = Int32;

} // namespace Glance::Core
