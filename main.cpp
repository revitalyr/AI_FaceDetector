module;
#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <iostream>
module UI.MainWindow;
import Core.Orchestrator;

int main(int argc, char* argv[])
{
    qunsetenv("QT_QPA_PLATFORM_PLUGIN_PATH");
    QApplication app(argc, argv);
    
    app.setApplicationName("GlanceRawViewer");
    app.setOrganizationName("Glance");
    
    try {
        Glance::Core::Orchestrator orchestrator;
        Glance::UI::MainWindow window(orchestrator);
        window.show();
        return app.exec();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        QMessageBox::critical(nullptr, "Fatal Error", 
            QString("Application failed to start: %1").arg(e.what()));
        return 1;
    }
}
