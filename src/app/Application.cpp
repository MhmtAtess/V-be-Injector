#include "app/Application.hpp"
#include "app/Logger.hpp"
#include "ui/MainWindow.hpp"

namespace viibe
{
    bool Application::Initialize()
    {
        Logger::Info("Application initialization started.");

        if (!MainWindow::Initialize())
        {
            Logger::Error("MainWindow initialization failed.");
            return false;
        }

        m_initialized = true;

        Logger::Info("Application initialized successfully.");
        return true;
    }

    int Application::Run()
    {
        if (!m_initialized)
            return -1;

        return MainWindow::Run();
    }

    void Application::Shutdown()
    {
        if (!m_initialized)
            return;

        MainWindow::Shutdown();

        Logger::Info("Application shutdown complete.");

        m_initialized = false;
    }
}
