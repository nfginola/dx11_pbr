#include "pch.h"
#include "Engine.h"
#include "Graphics/ResourceTypes.h"
#include "Graphics/DXDevice.h"
#include "Graphics/CentralRenderer.h"
#include "AssimpLoader.h"
#include "Input.h"

namespace Gino
{
	Engine::Engine(Settings& settings)
	{
		m_input = std::make_unique<Input>(settings.hwnd);
		m_dxDev = std::make_unique<DXDevice>(settings.hwnd, settings.resolutionWidth, settings.resolutionHeight);
		// DXState state
		/*
			this should PRIMARILY be used inside CentralRenderer and other Render modules
			state->Set(...)
			state->FinalizeBinds();		--> All API calls in one go
		*/

		m_centralRenderer = std::make_unique<CentralRenderer>(m_dxDev.get(), settings.vsync);
		

		/*
		
			Load assimp data into Model
				- Have texture hash map (hash function with filePath)
				
			LoadModel(filePath, Material::Phong)
		
		
		
		*/

		m_sponzaModel = LoadModel("../assets/Models/Sponza_new/sponza.obj");

		std::cout << "hell yeah\n";

	}

	Engine::~Engine()
	{

	}

	void Engine::SimulateAndRender()
	{
		m_input->Update();

		/*
		
		UpdateObjects(dt)

		culler->cull(scene)
		
		for each non-culled geometry in scene:
			cr->SubmitOpaqueModel(mesh, material);
			cr->SubmitTransparentModel(mesh, material);
		
		*/

		m_centralRenderer->Render(m_sponzaModel.get());
	}

	Input* Engine::GetInput()
	{
		return m_input.get();
	}

	std::function<void(HWND, UINT, WPARAM, LPARAM)> Engine::GetImGuiHook() const
	{
		if (m_centralRenderer && m_centralRenderer->GetImGui())
		{
			return m_centralRenderer->GetImGui()->GetWin32Hook();
		}
		else
		{
			return[](HWND, UINT, WPARAM, LPARAM) {};
		}
	}


	Texture* Engine::LoadTexture(const std::string& filePath)
	{
		if (m_loadedTextures.find(filePath) == m_loadedTextures.end())
		{
			auto text = std::make_unique<Texture>();
			text->InitializeFromFile(m_dxDev->GetDevice(), m_dxDev->GetContext(), filePath);
			auto pair = m_loadedTextures.insert({ filePath, std::move(text) });
			
			return pair.first->second.get();
		}
		else
		{
			std::cout << "Skipping loading texture..: " << filePath << std::endl;
			return nullptr;
		}
	}

	std::unique_ptr<Model> Engine::LoadModel(const std::filesystem::path& filePath)
	{
		static std::string defaultDiffuseFilePath = "../assets/Textures/Default/defaultdiffuse.jpg";
		static std::string defaultSpecularFilePath = "../assets/Textures/Default/defaultspecular.jpg";
		static std::string defaultOpacityFilePath = "../assets/Textures/Default/defaultopacity.jpg";
		static std::string defaultNormalFilePath = "../assets/Textures/Default/defaultnormal.jpg";


		AssimpLoader loader(filePath);

		auto verts = loader.GetVertices();
		auto indices = loader.GetIndices();
		auto subsets = loader.GetSubsets();
		auto mats = loader.GetMaterials();

		const std::string directory = filePath.parent_path().string() + "/";

		// Load textures
		for (auto& mat : mats)
		{
			Texture* diffuse = nullptr;
			Texture* specular = nullptr;
			Texture* opacity = nullptr;
			Texture* normal = nullptr;

			if (mat.diffuseFilePath.has_value())
			{
				diffuse = LoadTexture(directory + mat.diffuseFilePath.value());
			}
			else
			{
				diffuse = LoadTexture(defaultDiffuseFilePath);
			}

			if (mat.normalFilePath.has_value())
			{
				normal = LoadTexture(directory + mat.normalFilePath.value());
			}
			else
			{
				normal = LoadTexture(defaultNormalFilePath);
			}

			if (mat.opacityFilePath.has_value())
			{
				opacity = LoadTexture(directory + mat.opacityFilePath.value());
			}
			else
			{
				opacity = LoadTexture(defaultOpacityFilePath);
			}

			if (mat.specularFilePath.has_value())
			{
				specular = LoadTexture(directory + mat.specularFilePath.value());
			}
			else
			{
				specular = LoadTexture(defaultSpecularFilePath);
			}
			
		}


		// Transform verts data into our specified input layout
		std::vector<Vertex_POS_UV_NORMAL> vertsIn;
		vertsIn.reserve(verts.size());
		for (const auto& vert : verts)
		{
			Vertex_POS_UV_NORMAL vertex;
			vertex.pos.x = vert.position.x;
			vertex.pos.y = vert.position.y;
			vertex.pos.z = vert.position.z;

			vertex.uv.x = vert.uv.x;
			vertex.uv.y = vert.uv.y;

			vertex.normal.x = vert.normal.x;
			vertex.normal.y = vert.normal.y;
			vertex.normal.z = vert.normal.z;
			vertsIn.push_back(vertex);
		}

		Buffer vb;
		Buffer ib;
		vb.Initialize(m_dxDev->GetDevice(), VertexBufferDesc<Vertex_POS_UV_NORMAL>{ .data = vertsIn });
		ib.Initialize(m_dxDev->GetDevice(), IndexBufferDesc{ .data = indices });

		// Setup meshes
		std::vector<std::pair<Mesh, Material>> materialsAndMeshes;
		materialsAndMeshes.reserve(subsets.size());
		for (const auto& subset : subsets)
		{
			Mesh mesh
			{
				.numIndices = subset.indexCount,
				.indicesFirstIndex = subset.indexStart,
				.vertexOffset = subset.vertexStart
			};

			std::string diffLook = subset.diffuseFilePath.has_value() ? directory + subset.diffuseFilePath.value() : defaultDiffuseFilePath;
			std::string specLook = subset.specularFilePath.has_value() ? directory + subset.specularFilePath.value() : defaultSpecularFilePath;
			std::string opacityLook = subset.opacityFilePath.has_value() ? directory + subset.opacityFilePath.value() : defaultOpacityFilePath;
			std::string normalLook = subset.normalFilePath.has_value() ? directory + subset.normalFilePath.value() : defaultNormalFilePath;

			Material mat;
			mat.Initialize(PhongMaterialData
				{
					.diffuse = m_loadedTextures.find(diffLook)->second.get(),
					.specular = m_loadedTextures.find(specLook)->second.get(),
					.opacity = m_loadedTextures.find(opacityLook)->second.get(),
					.normal = m_loadedTextures.find(normalLook)->second.get()
				});
			
			materialsAndMeshes.push_back({ mesh, mat });
		}

		auto model = std::make_unique<Model>();
		model->Initialize(vb, ib, materialsAndMeshes);
		return model;
	}

}


