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

		/*
		void SetScene(scene);		// All data from that scene will be used to Render
		
		*/

		std::function<void(HWND, UINT, WPARAM, LPARAM)> GetImGuiHook() const;

	private:
		Texture* LoadTexture(const std::string& filePath);
		std::unique_ptr<Model> LoadModel(const std::filesystem::path& filePath);

	private:
		std::unique_ptr<DXDevice> m_dxDev;
		std::unique_ptr<CentralRenderer> m_centralRenderer;

		std::unique_ptr<Input> m_input;

		std::unordered_map<std::string, std::unique_ptr<Texture>> m_loadedTextures;

		// For now, instead of Application asking the engine for a model, we will keep our workstation in Engine.cpp!
		std::unique_ptr<Model> m_sponzaModel;
		

		//std::unique_ptr<Input> m_input;
		/*
		Input
		MainRenderer
		Profiler
		*/

	};
}



