#pragma once
#include <windows.h>

namespace Gino
{
	class Input;
	class DXDevice;
	class CentralRenderer;

	class Engine
	{
	public:
		struct Settings
		{
			// Render window
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

		const std::function<void(HWND, UINT, WPARAM, LPARAM)>& GetImGuiHook() const;

	private:
		std::unique_ptr<DXDevice> m_dxDev;
		std::unique_ptr<CentralRenderer> m_centralRenderer;

		// -- Maybe we can wrap them in an AssetContext in the future (where we can load/unload multiple context depending on e.g Scene)
		// Loaded Mesh Data (Vertex Buffer)
		// Loaded Meshes (References into a VB (offsets and the like))
		// Loaded Textures (Texture2D)
		

		//std::unique_ptr<Input> m_input;
		/*
		Input
		Renderer
		Profiler
		*/

	};
}



