#include "pch.h"
#include "Application.h"

#include <thread>

void DoWork(Gino::Application& app)
{
    std::string input;
    while (app.IsAlive() && std::cin >> input)
    {
        app.ParseConsoleInput(input);
    }
}

//int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
int main()
{
    // We use GetModuleHandle to get hInstance while still using Console subsystem and with normal main entry!

    // Considering reading from a file for settings in the future
    Gino::Application::Settings settings
    {
        .hInstance = GetModuleHandle(nullptr),

        // 16:9 aspect ratio
        .windowWidth = 2464,
        .windowHeight = 1386
    };
    
    std::thread consoleThread;
    {
        Gino::Application app(settings);

        consoleThread = std::thread(DoWork, std::ref(app));
        app.Run();
    }

    std::cout << "GINOAPP: Please type any key to exit\n";
    consoleThread.join();

    return 0;
}