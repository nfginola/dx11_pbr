#include "pch.h"
#include "Application.h"


//int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
int main()
{
    // We use GetModuleHandle to get hInstance while still using Console subsystem with normal main entry!

    // Considering reading from a file for settings in the future
    Gino::Application::Settings settings
    {
        .hInstance = GetModuleHandle(nullptr),

        // 16:9 aspect ratio
        .windowWidth = 2464,
        .windowHeight = 1386
    };

    Gino::Application app(settings);
    return 0;
}