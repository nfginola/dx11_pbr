#include "pch.h"
#include "Application.h"


//int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
int main()
{
    // We may want to read from a text file for our settings in the future
    Gino::Application::Settings settings
    {
        .hInstance = GetModuleHandle(nullptr),

        // 16:9 aspect ratio
        .windowWidth = 2464,
        .windowHeight = 1386,
        .resolutionWidth = 1920,
        .resolutionHeight = 1080
    };

    Gino::Application app(settings);

    return 0;
}