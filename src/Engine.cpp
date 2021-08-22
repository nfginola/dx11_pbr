#include "pch.h"
#include "Engine.h"
#include "Graphics/ResourceTypes.h"
#include "Graphics/DXDevice.h"
#include "Graphics/Renderer.h"
#include "AssimpLoader.h"
#include "Input.h"
#include "FPCamera.h"

#include "Graphics/Model.h"

#include "Entity.h"

#include "Scene.h"

#include "Timer.h"

namespace Gino
{
	Engine::Engine(Settings& settings)
	{
		m_input = std::make_unique<Input>(settings.hwnd);
		m_dxDev = std::make_unique<DXDevice>(settings.hwnd, settings.resolutionWidth, settings.resolutionHeight);
		m_renderer = std::make_unique<Renderer>(m_dxDev.get(), settings.vsync);

		m_fpCam = std::make_unique<FPCamera>((float)settings.resolutionWidth / settings.resolutionHeight, 87.f);
		m_renderer->SetRenderCamera(m_fpCam.get());
	}

	Engine::~Engine()
	{

	}

	void Engine::SetScene(Scene* scene)
	{
		m_scene = scene;
		m_renderer->SetModels(m_scene->GetModelInstances());
	}

	void Engine::SimulateAndRender(float dt)
	{
		m_input->Update();

		// Update camera state
		if (m_input->KeyIsDown(Keys::W))				m_fpCam->Move(MoveDirection::Forward);
		if (m_input->KeyIsDown(Keys::A))				m_fpCam->Move(MoveDirection::Left);
		if (m_input->KeyIsDown(Keys::S))				m_fpCam->Move(MoveDirection::Backward);
		if (m_input->KeyIsDown(Keys::D))				m_fpCam->Move(MoveDirection::Right);
		if (m_input->KeyIsDown(Keys::Space))			m_fpCam->Move(MoveDirection::Up);
		if (m_input->KeyIsDown(Keys::LeftShift))		m_fpCam->Move(MoveDirection::Down);

		if (m_input->KeyIsPressed(Keys::LeftControl))	m_fpCam->SetMoveSpeed(MoveSpeed::Slow);
		if (m_input->KeyIsReleased(Keys::LeftControl))	m_fpCam->SetMoveSpeed(MoveSpeed::Normal);

		if (m_input->RMBIsPressed())					m_input->SetMouseMode(MouseMode::Relative);
		if (m_input->RMBIsReleased())					m_input->SetMouseMode(MouseMode::Absolute);

		if (m_input->RMBIsDown())						m_fpCam->RotateCamera(m_input->GetMouseDelta());	

		// Finalize camera changes for this frame
		m_fpCam->Update(dt);

		m_renderer->BeginFrame();

		ImGui::Begin("Frame Statistics");
		ImGui::Text("Total Engine CPU %s ms", std::to_string(dt).c_str());
		ImGui::End();

		m_scene->Update(dt);
		m_renderer->Render();
		m_renderer->EndFrame();

		/*
		
		UpdateObjects(dt)

		culler->cull(scene)
		
		for each non-culled geometry in scene:
			cr->SubmitOpaqueModel(mesh, material);
			cr->SubmitTransparentModel(mesh, material);
		
		*/

		m_input->Reset();
	}

	Input* Engine::GetInput()
	{
		return m_input.get();
	}

	std::function<void(HWND, UINT, WPARAM, LPARAM)> Engine::GetImGuiHook() const
	{
		if (m_renderer && m_renderer->GetImGui())
		{
			return m_renderer->GetImGui()->GetWin32Hook();
		}
		else
		{
			return[](HWND, UINT, WPARAM, LPARAM) {};
		}
	}

	Model* Engine::CreateModel(const std::string& id, const std::filesystem::path& filePath, bool PBR)
	{
		auto it = m_loadedModels.find(id);
		if (it != m_loadedModels.end())
		{
			std::cout << "ID already taken!\n";
			assert(false);
		}

		auto model = LoadModel(filePath, PBR);
		auto ret = model.get();
		m_loadedModels.insert({ id, std::move(model) });
		return ret;
	}

	Model* Engine::GetModel(const std::string& id)
	{
		auto it = m_loadedModels.find(id);
		if (it == m_loadedModels.end())
		{
			std::cout << "Could not find model with ID: '" << id << "'\n";
			assert(false);
		}

		return (*it).second.get();
	}

	Texture* Engine::LoadTexture(const std::string& filePath, bool srgb)
	{
		if (m_loadedTextures.find(filePath) == m_loadedTextures.end())
		{
			auto text = std::make_unique<Texture>();
			text->InitializeFromFile(m_dxDev->GetDevice(), m_dxDev->GetContext(), filePath, srgb);
			auto pair = m_loadedTextures.insert({ filePath, std::move(text) });
			
			return pair.first->second.get();
		}
		else
		{
			std::cout << "Skipping loading texture..: " << filePath << std::endl;
			return nullptr;
		}
	}

	std::unique_ptr<Model> Engine::LoadModel(const std::filesystem::path& filePath, bool PBR)
	{
		AssimpLoader loader(filePath, PBR);

		if (!PBR)
			return LoadPhongModel(loader);
		else
			return LoadPBRModel(loader);
	}

	std::unique_ptr<Model> Engine::LoadPhongModel(const AssimpLoader& loader)
	{
		static std::string defaultDiffuseFilePath = "../assets/Textures/Default/defaultdiffuse.jpg";
		static std::string defaultSpecularFilePath = "../assets/Textures/Default/defaultspecular.jpg";
		static std::string defaultOpacityFilePath = "../assets/Textures/Default/defaultopacity.jpg";
		static std::string defaultNormalFilePath = "../assets/Textures/Default/defaultnormal.jpg";

		auto verts = loader.GetVertices();
		auto indices = loader.GetIndices();
		auto subsets = loader.GetSubsets();
		auto mats = loader.GetMaterials();

		//const std::string directory = filePath.parent_path().string() + "/";

		// Load textures
		for (auto& mat : mats)
		{
			Texture* diffuse = nullptr;
			Texture* specular = nullptr;
			Texture* opacity = nullptr;
			Texture* normal = nullptr;

			if (mat.diffuseFilePath.has_value())	
				diffuse = LoadTexture(mat.diffuseFilePath.value());
			else									
				diffuse = LoadTexture(defaultDiffuseFilePath);

			if (mat.normalFilePath.has_value())		
				normal = LoadTexture(mat.normalFilePath.value(), false);
			else									
				normal = LoadTexture(defaultNormalFilePath);

			if (mat.opacityFilePath.has_value())	
				opacity = LoadTexture(mat.opacityFilePath.value());
			else									
				opacity = LoadTexture(defaultOpacityFilePath);

			if (mat.specularFilePath.has_value())	
				specular = LoadTexture(mat.specularFilePath.value());
			else									
				specular = LoadTexture(defaultSpecularFilePath);
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

			vertex.tangent.x = vert.tangent.x;
			vertex.tangent.y = vert.tangent.y;
			vertex.tangent.z = vert.tangent.z;

			vertex.bitangent.x = vert.bitangent.x;
			vertex.bitangent.y = vert.bitangent.y;
			vertex.bitangent.z = vert.bitangent.z;

			vertsIn.push_back(vertex);
		}

		Buffer vb;
		Buffer ib;
		vb.Initialize(m_dxDev->GetDevice(), VertexBufferDesc<Vertex_POS_UV_NORMAL>{.data = vertsIn });
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

			auto phongMat = std::get<0>(subset.mats);

			std::string diffLook = phongMat.diffuseFilePath.has_value() ? phongMat.diffuseFilePath.value() : defaultDiffuseFilePath;
			std::string specLook = phongMat.specularFilePath.has_value() ? phongMat.specularFilePath.value() : defaultSpecularFilePath;
			std::string opacityLook = phongMat.opacityFilePath.has_value() ? phongMat.opacityFilePath.value() : defaultOpacityFilePath;
			std::string normalLook = phongMat.normalFilePath.has_value() ? phongMat.normalFilePath.value() : defaultNormalFilePath;

			// Create material for this submesh to use
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

	std::unique_ptr<Model> Engine::LoadPBRModel(const AssimpLoader& loader)
	{
		static std::string defaultDiffuseFilePath = "../assets/Textures/Default/defaultdiffuse.jpg";
		static std::string defaultSpecularFilePath = "../assets/Textures/Default/defaultspecular.jpg";
		static std::string defaultOpacityFilePath = "../assets/Textures/Default/defaultopacity.jpg";
		static std::string defaultNormalFilePath = "../assets/Textures/Default/defaultnormal.jpg";

		auto verts = loader.GetVertices();
		auto indices = loader.GetIndices();
		auto subsets = loader.GetSubsets();
		auto mats = loader.GetMaterialsPBR();

		//const std::string directory = filePath.parent_path().string() + "/";

		// Load textures
		for (auto& mat : mats)
		{
			Texture* albedo = nullptr;
			Texture* normal = nullptr;
			Texture* metallicAndRoughness = nullptr;
			Texture* ao = nullptr;
			Texture* emission = nullptr;

			if (mat.albedo.has_value())					albedo = LoadTexture(mat.albedo.value());
			else										albedo = LoadTexture(defaultDiffuseFilePath);	

			if (mat.normal.has_value())					normal = LoadTexture(mat.normal.value(), false);
			else										normal = LoadTexture(defaultNormalFilePath);

			if (mat.metallicAndRoughness.has_value())	metallicAndRoughness = LoadTexture(mat.metallicAndRoughness.value(), false);
			else										metallicAndRoughness = LoadTexture(defaultSpecularFilePath);			// full black, rough/metal = (0, 0)

			if (mat.ao.has_value())						ao = LoadTexture(mat.ao.value(), false);
			else										ao = LoadTexture(defaultSpecularFilePath);

			if (mat.emission.has_value())				emission = LoadTexture(mat.emission.value());
			else										emission = LoadTexture(defaultSpecularFilePath);
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

			vertex.tangent.x = vert.tangent.x;
			vertex.tangent.y = vert.tangent.y;
			vertex.tangent.z = vert.tangent.z;

			vertex.bitangent.x = vert.bitangent.x;
			vertex.bitangent.y = vert.bitangent.y;
			vertex.bitangent.z = vert.bitangent.z;

			vertsIn.push_back(vertex);
		}

		Buffer vb;
		Buffer ib;
		vb.Initialize(m_dxDev->GetDevice(), VertexBufferDesc<Vertex_POS_UV_NORMAL>{.data = vertsIn });
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

			auto pbrMat = std::get<1>(subset.mats);

			std::string albedoLook = pbrMat.albedo.has_value() ? pbrMat.albedo.value() : defaultDiffuseFilePath;
			std::string normalLook = pbrMat.normal.has_value() ? pbrMat.normal.value() : defaultNormalFilePath;
			std::string metallicAndRoughnessLook = pbrMat.metallicAndRoughness.has_value() ? pbrMat.metallicAndRoughness.value() : defaultSpecularFilePath;
			std::string aoLook = pbrMat.ao.has_value() ? pbrMat.ao.value() : defaultSpecularFilePath;
			std::string emissionLook = pbrMat.emission.has_value() ? pbrMat.emission.value() : defaultSpecularFilePath;

			// Create material for this submesh to use
			Material mat;
			mat.Initialize(PBRMaterialData
				{
					.albedo = m_loadedTextures.find(albedoLook)->second.get(),
					.normal = m_loadedTextures.find(normalLook)->second.get(),
					.metallicAndRoughness = m_loadedTextures.find(metallicAndRoughnessLook)->second.get(),
					.ao = m_loadedTextures.find(aoLook)->second.get(),
					.emission = m_loadedTextures.find(emissionLook)->second.get()
				});

			materialsAndMeshes.push_back({ mesh, mat });
		}

		auto model = std::make_unique<Model>();
		model->Initialize(vb, ib, materialsAndMeshes);
		return model;
	}

}


