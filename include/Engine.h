#pragma once
#include <windows.h>

namespace Gino
{
	class Input;
	class DXDevice;
	class CentralRenderer;
	class ImGuiRenderer;
	class AssimpLoader;
	struct Texture;
	struct Model;
	struct Material;
	
	class Engine
	{
	public:
		struct Settings
		{
			// Render window
			HWND hwnd;

			// Renderer settings
			bool vsync = false;
			// Pixel resolution
			int resolutionWidth = 2560;
			int resolutionHeight = 1440;
		};

	public:
		Engine(Settings& settings);
		~Engine();

		void SimulateAndRender();

			
		Input* GetInput();

		std::function<void(HWND, UINT, WPARAM, LPARAM)> GetImGuiHook() const;

	private:
		Texture* LoadTexture(const std::string& filePath);
		std::unique_ptr<Model> LoadModel(const std::filesystem::path& filePath);

	private:
		std::unique_ptr<DXDevice> m_dxDev;
		std::unique_ptr<CentralRenderer> m_centralRenderer;

		std::unique_ptr<Input> m_input;

		std::unordered_map<std::string, std::unique_ptr<Texture>> m_loadedTextures;
		std::unique_ptr<Model> m_sponzaModel;

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



