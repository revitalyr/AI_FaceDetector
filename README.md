# Glance Raw Image Viewer

Image viewer with GPU shader-based processing, face detection, and segmentation.

## Overview

C++23 and Qt6 application implementing GPU shader-based image adjustments, OpenCV DNN face detection, segmentation, and a plugin-based architecture using C++23 modules.

## Features

- GPU shader-based image processing (exposure, contrast, gamma, temperature, tint) via custom GLSL shaders
- Face detection using OpenCV DNN (YuNet model)
- Image segmentation (background removal)
- Edit stack with undo/redo support
- LUT (Look-Up Table) support for color grading
- Real-time histogram computation
- Asynchronous image loading
- Split-screen before/after comparison
- Recent files management
- Plugin system for extensible functionality

## Architecture

Directory structure organized by functional responsibility using C++23 modules:

```
AI_FaceDetector/
├── Core/                    Business logic and image processing
│   ├── TypeAliases.cppm         Semantic type definitions
│   ├── ImageProcessor.cppm      Image processing algorithms
│   ├── FaceDetector.cppm        Face detection implementation (OpenCV DNN YuNet)
│   ├── Segmenter.cppm           Image segmentation (background removal)
│   ├── ProcessingPipeline.cppm  Async processing pipeline
│   ├── EditStack.cppm           Undo/redo functionality
│   ├── LUT.cppm                 Look-Up Table support
│   ├── Orchestrator.cppm        Plugin orchestration
│   └── Plugins/                 Plugin interfaces
│       └── IFaceDetectorPlugin.h
├── IO/                      Input/Output operations
│   ├── AsyncImageLoader.cppm    Multi-threaded image loading
│   └── ImageReader.cppm         Format detection and metadata
├── UI/                      User interface components
│   ├── MainWindow.cppm          Main application window
│   ├── ImageViewWidget.cppm     OpenGL-based image display
│   ├── CustomSlider.cppm        Custom slider controls
│   ├── HistogramWidget.cppm     Histogram display widget
│   ├── RecentFilesModel.cppm    Recent files management
│   └── SplitScreenWidget.cppm   Split-screen comparison
├── Shaders/                 GLSL shader programs
│   ├── image.vert               Vertex shader
│   └── image.frag               Fragment shader
├── Modules/                 C++23 module interface files
│   ├── Core.*.cppm              Core module exports
│   ├── IO.*.cppm                I/O module exports
│   └── UI.*.cppm                UI module exports
├── Tests/                   Unit and integration tests
│   └── TestFaceDetector.cpp     Face detection ground truth tests
├── models/                  AI model files (alternative location)
│   └── face_detection_yunet_2023mar.onnx
├── data/                   Data directory (default model location)
│   └── models/
│       └── face_detection_yunet_2023mar.onnx
└── CMakeLists.txt         Build configuration
```

## Technology Stack

| Component | Technology |
|-----------|------------|
| Language | C++23 (modules, concepts, ranges, std::generator) |
| GUI Framework | Qt6 Widgets |
| Graphics | OpenGL 3.3+ with custom GLSL shaders |
| Face Detection | OpenCV DNN (YuNet) |
| Segmentation | OpenCV DNN (u2net) |
| Build System | CMake 3.28+ (required for C++23 modules) |
| Concurrency | QtConcurrent, std::jthread |
| Image I/O | Qt Image I/O, LibRaw (for RAW formats) |
| Module System | C++23 Modules (.cppm) |

## Building the Project

### Prerequisites

- C++23 compliant compiler (MSVC 2022, GCC 13+, Clang 16+)
- Qt6.2+ with Widgets, OpenGL, and Concurrent modules
- CMake 3.28+ (required for C++23 modules support)
- OpenCV 4.x with DNN module
- LibRaw (for RAW image format support)
- OpenGL 3.3+ capable graphics drivers
- vcpkg (recommended for dependency management)

### Build Instructions

Using CMake presets:

```bash
cmake --preset local-debug
cmake --build --preset local-debug
```

Manual build:

```bash
git clone <repository-url>
cd AI_FaceDetector
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

Run the built executable:
- Linux/macOS: `./GlanceRawViewer`
- Windows: `GlanceRawViewer.exe`

### Dependencies Installation

Windows (vcpkg):
```bash
vcpkg install qt6 opencv4 libraw
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
```

Ubuntu/Debian:
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev libgl1-mesa-dev libopencv-dev libraw-dev cmake build-essential
```

macOS (Homebrew):
```bash
brew install qt6 opencv libraw cmake
```

## Usage

- Open images via File → Open or drag-and-drop
- Adjust parameters using sliders (exposure, contrast, gamma, temperature, tint)
- Space: Toggle original/processed view
- Ctrl+F: Fit to window
- Ctrl+0: Reset to 100% zoom
- Ctrl+Z: Undo last edit
- Ctrl+Y: Redo last edit

## Key Components

### Face Detection
Uses OpenCV DNN with the YuNet model. The model is loaded from the `models/` directory.

### Segmentation
The `Segmenter` class provides image segmentation for background removal.

### Edit Stack
The `EditStack` class implements undo/redo functionality for all image operations, allowing users to revert changes.

### LUT Support
The `LUT` class enables color grading through Look-Up Tables for professional color correction workflows.

### Plugin System
The plugin architecture allows extending functionality through the `Orchestrator` and plugin interfaces in `Core/Plugins/`.

## Dependencies

- C++23 compiler (MSVC 2022, GCC 13+, Clang 16+)
- Qt6.2+ (Widgets, OpenGL, Concurrent, PrintSupport)
- OpenCV 4.x (with DNN module)
- LibRaw (for RAW format support)
- CMake 3.28+ (required for C++23 modules)

## License

Proprietary - All rights reserved.
