cmake_minimum_required(VERSION 3.28)  # 3.28+ required for C++23 modules support
project(GlanceRawViewer VERSION 1.0.0 LANGUAGES CXX)

# ── Qt Version Management ──────────────────────────────────────────────────
if(DEFINED QT_BASE_DIR AND NOT "${QT_BASE_DIR}" STREQUAL "")
    list(PREPEND CMAKE_PREFIX_PATH "${QT_BASE_DIR}")
    set(Qt6_DIR "${QT_BASE_DIR}/lib/cmake/Qt6")
    set(Qt6Core_DIR "${QT_BASE_DIR}/lib/cmake/Qt6Core")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Sanitizer Options ───────────────────────────────────────────────────────────
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

if(ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fsanitize=address")
    message(STATUS "AddressSanitizer enabled")
endif()

if(ENABLE_TSAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fsanitize=thread")
    message(STATUS "ThreadSanitizer enabled")
endif()

if(ENABLE_UBSAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -g")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fsanitize=undefined")
    message(STATUS "UndefinedBehaviorSanitizer enabled")
endif()

# MSVC sanitizer support
if(MSVC)
    option(ENABLE_MSVC_ASAN "Enable MSVC AddressSanitizer" OFF)
    if(ENABLE_MSVC_ASAN)
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address)
        message(STATUS "MSVC AddressSanitizer enabled")
    endif()
endif()

# C++23 modules support (CMake 3.28+ / MSVC)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

# vcpkg toolchain — must be set BEFORE first project() call ideally,
# but this guard handles the case when it wasn't passed via -DCMAKE_TOOLCHAIN_FILE.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "vcpkg toolchain" FORCE)
        message(STATUS "vcpkg toolchain auto-detected from VCPKG_ROOT: $ENV{VCPKG_ROOT}")
    endif()
endif()

# Qt RCC/UIC — must be set BEFORE add_executable/add_library
# AUTOMOC is not needed - no Q_OBJECT macros remain in the codebase
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find packages
find_package(Qt6 6.2 REQUIRED
    COMPONENTS Core Widgets OpenGL OpenGLWidgets Concurrent PrintSupport Network
)

# Auto-detect QT_BASE_DIR from Qt6::Core target if not explicitly set.
# Expected vcpkg layout: <base>/[debug/]lib/Qt6Core[d].dll
# The deploy macro expects <base> (e.g. D:/work/vcpkg/installed/x64-windows).
if(NOT DEFINED QT_BASE_DIR OR "${QT_BASE_DIR}" STREQUAL "")
    get_target_property(_qt_core_loc_release Qt6::Core IMPORTED_LOCATION_RELEASE)
    if(_qt_core_loc_release)
        get_filename_component(QT_BASE_DIR "${_qt_core_loc_release}" DIRECTORY)  # .../lib
        get_filename_component(QT_BASE_DIR "${QT_BASE_DIR}" DIRECTORY)           # <base>
    else()
        get_target_property(_qt_core_loc_debug Qt6::Core IMPORTED_LOCATION_DEBUG)
        if(_qt_core_loc_debug)
            get_filename_component(QT_BASE_DIR "${_qt_core_loc_debug}" DIRECTORY)  # .../debug/lib
            get_filename_component(QT_BASE_DIR "${QT_BASE_DIR}" DIRECTORY)         # .../debug
            get_filename_component(QT_BASE_DIR "${QT_BASE_DIR}" DIRECTORY)         # <base>
        else()
            message(FATAL_ERROR "QT_BASE_DIR not set and could not be auto-detected from Qt6::Core target."
                " Pass -DQT_BASE_DIR=... to cmake.")
        endif()
    endif()
    message(STATUS "QT_BASE_DIR auto-detected: ${QT_BASE_DIR}")
endif()

find_package(Catch2 3)
if(NOT Catch2_FOUND)
    message(WARNING "Catch2 not found. Tests will be disabled. Install Catch2 via vcpkg: vcpkg install catch2")
endif()

find_package(OpenCV 4.8 REQUIRED COMPONENTS core imgproc dnn photo stitching objdetect)
find_package(TBB 2021 CONFIG REQUIRED)
find_package(libraw 0.21 CONFIG REQUIRED)

# ── Verification: Check for Qt Mismatch in OpenCV ────────────────────────────
if(TARGET opencv_highgui)
    get_target_property(_cv_deps opencv_highgui INTERFACE_LINK_LIBRARIES)
    foreach(_dep ${_cv_deps})
        if(_dep MATCHES "Qt6::")
            get_target_property(_qt_path ${_dep} IMPORTED_LOCATION_RELEASE)
            if(NOT _qt_path)
                get_target_property(_qt_path ${_dep} IMPORTED_LOCATION_DEBUG)
            endif()

            if(_qt_path)
                file(TO_CMAKE_PATH "${_qt_path}" _normalized_qt_path)
                file(TO_CMAKE_PATH "${QT_BASE_DIR}" _normalized_base_dir)

                if(NOT _normalized_qt_path MATCHES "^${_normalized_base_dir}")
                    message(FATAL_ERROR
                        "FATAL: OpenCV dependency mismatch! OpenCV links to Qt at: ${_normalized_qt_path}\n"
                        "But your project is configured to use Qt at: ${_normalized_base_dir}\n"
                        "This will cause binary incompatibility and crashes.")
                endif()
            endif()
        endif()
    endforeach()
endif()

# Enable testing
enable_testing()

# ── Sources ──────────────────────────────────────────────────────────────────

set(CORE_SOURCES
    Core/FaceDetector.cpp
    Core/PostProcessingStrategy.cpp
    Core/AIFaceDetector.cpp
    Core/Segmenter.cpp
    Core/ProcessingPipeline.cpp
    Core/Plugins/PluginManager.cpp
    Core/Orchestrator.cpp
    Core/PluginConfiguration.cpp
    Core/EditStack.cpp
    Core/ExposureOperation.cpp
    Core/LUT.cpp
    Modules/Core.ImageProcessor.cpp
)

set(PLUGIN_SOURCES
    Core/Plugins/Default/DefaultImageProcessorPlugin.cpp
    Core/Plugins/Default/DefaultFaceDetectorPlugin.cpp
)

set(MODULE_SOURCES
    Modules/Qt.Wrapper.cppm
    Modules/Core.TypeAliases.cppm
    Modules/Core.Constants.cppm
    Modules/Core.LUT.cppm
    Modules/Core.FaceDetector.cppm
    Modules/Core.Segmenter.cppm
    Modules/Core.ImageProcessor.cppm
    Modules/Core.EditOperation.cppm
    Modules/Core.EditStack.cppm
    Modules/Core.ExposureOperation.cppm
    Modules/Core.AIFaceDetector.cppm
    Modules/Core.PluginConfiguration.cppm
    Modules/Core.Orchestrator.cppm
    Modules/Core.ProcessingPipeline.cppm
    Modules/UI.FaceRenderingUtils.cppm
    Modules/UI.ComputeHistogram.cppm
    Modules/UI.Utils.ExpectedUtils.cppm
    Modules/UI.Utils.QtContainers.cppm
    Modules/UI.Utils.QtSignals.cppm
    Modules/UI.Utils.QtWrappers.cppm
    Modules/IO.ImageReader.cppm
    Modules/IO.AsyncImageLoader.cppm
    Modules/UI.CustomSlider.cppm
    Modules/UI.SplitScreenWidget.cppm
    Modules/UI.ImageViewWidget.cppm
    Modules/UI.HistogramWidget.cppm
    Modules/UI.RecentFilesModel.cppm
    Modules/UI.MainWindow.cppm
)

# Convert relative module paths to absolute
set(MODULE_ABSOLUTE)
foreach(_m ${MODULE_SOURCES})
    list(APPEND MODULE_ABSOLUTE "${CMAKE_CURRENT_SOURCE_DIR}/${_m}")
endforeach()

set(IO_SOURCES
    IO/AsyncImageLoader.cpp
    IO/ImageReader.cpp
)

set(UI_SOURCES
    UI/MainWindow.cpp
    UI/ImageViewWidget.cpp
    UI/CustomSlider.cpp
    UI/HistogramWidget.cpp
    UI/ComputeHistogram.cpp
    UI/RecentFilesModel.cpp
    UI/SplitScreenWidget.cpp
    UI/FaceRenderingUtils.cpp
)

set(SHADER_FILES
    Shaders/image.vert
    Shaders/image.frag
    Shaders/histogram.comp
)

set(TEST_SOURCES
    Tests/main_test.cpp
    Tests/TestImageProcessor.cpp
    Tests/TestFaceDetector.cpp
    Tests/TestAsyncImageLoader.cpp
    Tests/TestImageReader.cpp
    Tests/TestMainWindow.cpp
    Tests/PerformanceBenchmarks.cpp
    Tests/IntegrationTests.cpp
    Tests/VisualRegressionTests.cpp
    Tests/StressTests.cpp
    Tests/ShaderTests.cpp
)

# Disable AUTOMOC for test files (Catch2 doesn't need MOC)

# ── Helper: shared link libraries ────────────────────────────────────────────
set(COMMON_LINK_LIBS
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Concurrent
    Qt6::PrintSupport
    Qt6::Network
    TBB::tbb
    ${OpenCV_LIBS}
    libraw::raw_r
)

# Additional OpenCV libraries that might not be in ${OpenCV_LIBS}
if(TARGET opencv_photo)
    list(APPEND COMMON_LINK_LIBS opencv_photo)
else()
    find_package(OpenCV 4.8 REQUIRED COMPONENTS photo)
    list(APPEND COMMON_LINK_LIBS opencv_photo)
endif()

# ── ONNX model download (used by Segmenter via cv::dnn) ─────────────────────
set(SEGMENTATION_MODEL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/models/u2netp.onnx")
if(NOT EXISTS ${SEGMENTATION_MODEL_PATH})
    message(STATUS "Downloading u2netp segmentation model...")
    file(DOWNLOAD "https://huggingface.co/Heliosoph/u2net-onnx/resolve/main/u2netp.onnx"
        ${SEGMENTATION_MODEL_PATH}
        EXPECTED_HASH SHA256=309C8469258DDA742793DCE0EBEA8E6DD393174F89934733ECC8B14C76F4DDD8
        SHOW_PROGRESS)
endif()

# ── YuNet model download (used by FaceDetector) ──────────────────────────────
set(YUNET_MODEL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/data/models/face_detection_yunet_2023mar.onnx")
if(NOT EXISTS ${YUNET_MODEL_PATH})
    message(STATUS "Downloading YuNet face detection model...")
    file(DOWNLOAD "https://github.com/opencv/opencv_zoo/raw/master/models/face_detection_yunet/face_detection_yunet_2023mar.onnx"
        ${YUNET_MODEL_PATH}
        # TODO: Add EXPECTED_HASH after downloading and calculating SHA256
        SHOW_PROGRESS)
endif()

# ── Main executable ───────────────────────────────────────────────────────────

add_executable(${PROJECT_NAME}
    main.cpp
    ${CORE_SOURCES}
    ${IO_SOURCES}
    ${UI_SOURCES}
    ${PLUGIN_SOURCES}
    ${SHADER_FILES}
)

target_sources(${PROJECT_NAME} PRIVATE
    FILE_SET CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Modules
    FILES ${MODULE_ABSOLUTE}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Plugins
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Plugins/Default
    ${CMAKE_CURRENT_SOURCE_DIR}/IO
    ${CMAKE_CURRENT_SOURCE_DIR}/UI
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${COMMON_LINK_LIBS})
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_LIBRAW)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive- /Zc:__cplusplus /utf-8)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ── Copy shaders to build directory ──────────────────────────────────────────

foreach(SHADER ${SHADER_FILES})
    set(_shader_src "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}")
    if(EXISTS "${_shader_src}")
        get_filename_component(SHADER_NAME "${SHADER}" NAME)
        configure_file("${_shader_src}" "${CMAKE_BINARY_DIR}/${SHADER_NAME}" COPYONLY)
    endif()
endforeach()

# ── Test executable ───────────────────────────────────────────────────────────

if(Catch2_FOUND)
    add_executable(${PROJECT_NAME}_Tests
        ${CORE_SOURCES}
        ${IO_SOURCES}
        ${UI_SOURCES}
        ${TEST_SOURCES}
        ${SHADER_FILES}
    )

    target_sources(${PROJECT_NAME}_Tests PRIVATE
        FILE_SET CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Modules
        FILES ${MODULE_ABSOLUTE}
    )

    target_include_directories(${PROJECT_NAME}_Tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Plugins
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Plugins/Default
        ${CMAKE_CURRENT_SOURCE_DIR}/IO
        ${CMAKE_CURRENT_SOURCE_DIR}/UI
    )

    target_link_libraries(${PROJECT_NAME}_Tests PRIVATE
        ${COMMON_LINK_LIBS}
        Catch2::Catch2
    )
    target_compile_definitions(${PROJECT_NAME}_Tests PRIVATE HAVE_LIBRAW)
    if(MSVC)
        target_compile_options(${PROJECT_NAME}_Tests PRIVATE /W4 /permissive- /Zc:__cplusplus /utf-8 /wd4100 /wd4456)
    else()
        target_compile_options(${PROJECT_NAME}_Tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME GlanceUnitTests COMMAND ${PROJECT_NAME}_Tests)
endif()

# ── Qt DLL + platform plugin deployment (vcpkg layout) ─────────────────────
# applocal.ps1 (from vcpkg toolchain) deploys Qt DLLs automatically.
# We only need to copy the platform plugin (dynamic dependency).
if(WIN32)
    macro(deploy_platform_plugin TARGET_NAME)
        set(_qt_plugin_src
            "${QT_BASE_DIR}/$<IF:$<CONFIG:Debug>,debug/Qt6,Qt6>/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll")
        set(_qt_plugin_dst
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/platforms/qwindows$<$<CONFIG:Debug>:d>.dll")
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${TARGET_NAME}>/platforms"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${_qt_plugin_src}" "${_qt_plugin_dst}"
            COMMENT "Copying Qt platform plugin for ${TARGET_NAME}..."
        )
    endmacro()

    macro(deploy_imageformat_plugins TARGET_NAME)
        set(_qt_plugin_src
            "${QT_BASE_DIR}/$<IF:$<CONFIG:Debug>,debug/Qt6,Qt6>/plugins/imageformats/qjpeg$<$<CONFIG:Debug>:d>.dll")
        set(_qt_plugin_dst
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/imageformats/qjpeg$<$<CONFIG:Debug>:d>.dll")
        set(_jpeg_lib_src
            "${QT_BASE_DIR}/$<IF:$<CONFIG:Debug>,debug/bin,bin>/jpeg62.dll")
        set(_jpeg_lib_dst
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/jpeg62.dll")
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${TARGET_NAME}>/imageformats"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${_qt_plugin_src}" "${_qt_plugin_dst}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${_jpeg_lib_src}" "${_jpeg_lib_dst}"
            COMMENT "Copying Qt image format plugins and dependencies for ${TARGET_NAME}..."
        )
    endmacro()

    deploy_platform_plugin(${PROJECT_NAME})
    deploy_imageformat_plugins(${PROJECT_NAME})
    if(Catch2_FOUND AND TARGET ${PROJECT_NAME}_Tests)
        deploy_platform_plugin(${PROJECT_NAME}_Tests)
        deploy_imageformat_plugins(${PROJECT_NAME}_Tests)
    endif()
endif()

# ── Installation (cross-platform) ────────────────────────────────────────────
if(WIN32)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
elseif(APPLE)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
    install(FILES Shaders/image.vert Shaders/image.frag DESTINATION share/${PROJECT_NAME}/shaders)
else()
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
    install(FILES Shaders/image.vert Shaders/image.frag DESTINATION share/${PROJECT_NAME}/shaders)
endif()

# Install models if they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/models")
    install(DIRECTORY models/ DESTINATION share/${PROJECT_NAME}/models)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/data/models")
    install(DIRECTORY data/models/ DESTINATION share/${PROJECT_NAME}/models)
endif()