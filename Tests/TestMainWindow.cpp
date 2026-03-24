module;
#include <catch2/catch_test_macros.hpp>
module UI.MainWindow;

// MainWindow has complex initialization (OpenGL, UI components, menus).
// Full GUI tests require window server/display. These are minimal compilation tests.

TEST_CASE("MainWindow header compiles", "[mainwindow]") {
    // Just verify header inclusion works
    REQUIRE(true);
}

TEST_CASE("MainWindow class exists", "[mainwindow]") {
    // Verify MainWindow type is available
    // Note: Cannot instantiate without full GUI initialization
    REQUIRE(std::is_base_of_v<QMainWindow, Glance::UI::MainWindow>);
}