#pragma once
#include "Window.h"

//class Engine;
namespace Gino
{
	class Application
	{
	public:
		struct Settings
		{
			// Window settings
			HINSTANCE hInstance;
			int windowWidth = 1920;
			int windowHeight = 1080;
			bool fullscreenOnStart = false;

			// Renderer settings
			bool vsync = true;
			// Pixel resolution
			int resolutionWidth = 1920;
			int resolutionHeight = 1080;
		};

	public:
		Application(Settings& settings);
		~Application();

		Application() = delete;

	private:
		void InitWindow(Settings& settings);

		// Handle window procedures
		LRESULT MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		bool m_appIsAlive;

		std::unique_ptr<Window> m_mainWindow;
		//std::unique_ptr<Engine> m_engine;

	};
}

