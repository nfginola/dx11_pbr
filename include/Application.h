#pragma once
#include "Window.h"
#include <mutex>
#include <unordered_map>
#include <functional>

namespace Gino
{
	class Engine;
	class Scene;

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

		};

	public:
		Application(Settings& settings);
		~Application();

		void Run();
		bool IsAlive() const;
		void ParseConsoleInput(std::string input);

		Application() = delete;

	private:
		void InitWindow(Settings& settings);
		void InitConsoleCommands();
		void KillApp();

		// Handle window procedures
		LRESULT MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		std::mutex m_appKillMutex;
		bool m_appIsAlive;

		std::unique_ptr<Window> m_mainWindow;
		std::unique_ptr<Engine> m_engine;

		std::unique_ptr<Scene> m_scene;

		std::unordered_map<std::string, std::function<void()>> m_consoleCommands;


	};
}

