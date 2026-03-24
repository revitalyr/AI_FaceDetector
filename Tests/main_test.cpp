#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <QApplication>
#include <iostream>

// Global QApplication for all Qt tests
static int g_argc = 1;
static char g_appName[] = "test";
static char* g_argv[] = {g_appName, nullptr};
static QApplication* g_app = nullptr;

struct QtGlobalFixture {
    QtGlobalFixture() {
        if (!QApplication::instance()) {
            g_app = new QApplication(g_argc, g_argv);
            std::cout << "QApplication initialized for tests" << std::endl;
        }
    }
    ~QtGlobalFixture() {
        // QApplication deleted after all tests
    }
};

// Global fixture instance - created before first test
static QtGlobalFixture g_qtFixture;

TEST_CASE("Basic sanity check", "[basic]") {
    REQUIRE(2 + 2 == 4);
}

TEST_CASE("Qt is initialized", "[qt]") {
    REQUIRE(QCoreApplication::instance() != nullptr);
}

// Custom main - initializes QApplication before Catch2
int main(int argc, char* argv[]) {
    qunsetenv("QT_QPA_PLATFORM_PLUGIN_PATH");
    // Ensure QApplication exists before running tests
    if (!QApplication::instance()) {
        g_app = new QApplication(g_argc, g_argv);
    }

    // Run Catch2 session
    Catch::Session session;
    int result = session.run(argc, argv);

    // Cleanup
    delete g_app;
    g_app = nullptr;

    return result;
}
