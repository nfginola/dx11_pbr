#pragma once
#include <windows.h>

namespace Gino
{
	class Input;
	class DXDevice;
	class Renderer;
	class ImGuiRenderer;
	class AssimpLoader;
	class FPCamera;
	struct Texture;
	class Model;
	class Material;
	class Component;
	class Entity;
	class Scene;
	
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

		void SetScene(Scene* scene);

		void SimulateAndRender(float dt);

		Input* GetInput();
		std::function<void(HWND, UINT, WPARAM, LPARAM)> GetImGuiHook() const;

		// Creational functions
		Model* CreateModel(const std::string& id, const std::filesystem::path& filePath, bool PBR = false);
		Model* GetModel(const std::string& id);




	private:
		/*
		
		// The cache should reset if we switch scene.
		// Engine does not support streaming or single-removals
		// It will only Load new or clear completely 
		// Lets make this when we actually have multiple scenes
		TextureCache
		ModelCache
		// These can live inside Model Loader for now

		ModelLoader()

		auto mod = ModelLoader->LoadModel(name, filePath)
		
		Entity e;
		e.AddComponent<ComponentType::ModelType>(mod.get());
		
		*/

		Texture* LoadTexture(const std::string& filePath, bool srgb = true);
		std::unique_ptr<Model> LoadModel(const std::filesystem::path& filePath, bool PBR);

		std::unique_ptr<Model> LoadPhongModel(const AssimpLoader& loader);
		std::unique_ptr<Model> LoadPBRModel(const AssimpLoader& loader);

	private:
		std::unique_ptr<DXDevice> m_dxDev;
		std::unique_ptr<Renderer> m_renderer;
		std::unique_ptr<Input> m_input;

		std::unique_ptr<FPCamera> m_fpCam;

		Scene* m_scene;

		// "Model Loader"
		// Our engine only has one memory context.
		std::unordered_map<std::string, std::unique_ptr<Model>> m_loadedModels; 
		std::unordered_map<std::string, std::unique_ptr<Texture>> m_loadedTextures;
		
		/* To  make: */
		/*
		CPUProfiler
		GPUProfiler
		*/


	};
}



