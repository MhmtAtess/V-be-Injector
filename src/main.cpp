#include "app/Application.hpp"
#include "app/Logger.hpp"

#include <Windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    viibe::Logger::Initialize();

    viibe::Application app;

    if (!app.Initialize())
        return 1;

    const int result = app.Run();

    app.Shutdown();

    return result;
}

int main()
{
    return WinMain(GetModuleHandleA(nullptr), nullptr, GetCommandLineA(), SW_SHOW);
}
