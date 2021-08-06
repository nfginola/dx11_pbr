#pragma once
#include <windows.h>

namespace Gino
{
	class DXDevice;
	class Input;

	class Engine
	{
	public:
		struct Settings
		{
			// Window to hook to
			HWND hwnd;

			// Renderer settings
			bool vsync = true;
			// Pixel resolution
			int resolutionWidth = 1920;
			int resolutionHeight = 1080;
		};

	public:
		Engine(Settings& settings);
		~Engine();

		void SimulateAndRender();

		//Input* GetInput();


	private:
		std::unique_ptr<DXDevice> m_dxDev;
		//std::unique_ptr<Input> m_input;
		/*
		Input
		Renderer
		Profiler
		*/

	};
}



