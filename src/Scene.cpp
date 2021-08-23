#include "pch.h"
#include "Scene.h"
#include "Input.h"
#include "FPCamera.h"
#include "Timer.h"

#include "Graphics/ImGuiRenderer.h"

namespace Gino
{
	Scene::Scene(Engine* engine) :
		m_engine(engine)
	{
		m_engine->SetScene(this);

		m_engine->CreateModel("sponza", "../assets/Models/Sponza_gltf/glTF/Sponza.gltf", true);
		m_engine->CreateModel("pbrSpheres", "../assets/Models/MetalRoughSpheres/glTF/MetalRoughSpheres.gltf", true);
		m_engine->CreateModel("helmet", "../assets/Models/DamagedHelmet/glTF/DamagedHelmet.gltf", true);
		m_engine->CreateModel("ball", "../assets/Models/material_ball/scene.gltf", true);
		m_engine->CreateModel("cerberus", "../assets/Models/cerberus/scene.gltf", true);

		auto e = CreateEntity("Entity1");
		e->AddComponent<ModelType>(m_engine->GetModel("sponza"));
		e->GetComponent<TransformType>()->m_scaling = { 0.07f, 0.07f, 0.07f };

		auto e2 = CreateEntity("Entity2");
		e2->AddComponent<ModelType>(m_engine->GetModel("pbrSpheres"));
		e2->GetComponent<TransformType>()->m_position = { 5.f, 50.f, 0.f };
		e2->GetComponent<TransformType>()->m_rotation = { 90.f, 0.f, 0.f };

		auto e3 = CreateEntity("Entity3");
		e3->AddComponent<ModelType>(m_engine->GetModel("helmet"));
		e3->GetComponent<TransformType>()->m_position = { 50.f, 50.f, 0.f };
		e3->GetComponent<TransformType>()->m_scaling = { 4.f, 4.f, 4.f };
		e3->GetComponent<TransformType>()->m_rotation = { -90.f, 0.f, 0.f };

		auto e4 = CreateEntity("Entity4");
		e4->AddComponent<ModelType>(m_engine->GetModel("ball"));
		e4->GetComponent<TransformType>()->m_position = { 50.f, 7.f, 0.f };
		e4->GetComponent<TransformType>()->m_scaling = { 1.f, 1.f, 1.f };
		e4->GetComponent<TransformType>()->m_rotation = { 90.f, 90.f, 0.f };

		auto e5 = CreateEntity("Entity5");
		e5->AddComponent<ModelType>(m_engine->GetModel("cerberus"));
		e5->GetComponent<TransformType>()->m_position = { 35.f, 50.f, 0.f };
		e5->GetComponent<TransformType>()->m_scaling = { 0.1f, 0.1f, 0.1f };
		e5->GetComponent<TransformType>()->m_rotation = { 90.f, 90.f, 0.f };

		// Non PBR nanosuit models
		//m_engine->CreateModel("nanosuit", "../assets/Models/nanosuit/nanosuit.obj");
		//int counter = 0;
		//for (int x = -10; x < 10; ++x)
		//{
		//	for (int z = -10; z < 10; ++z)
		//	{
		//		auto newE = CreateEntity("ent" + std::to_string((counter++)));
		//		newE->AddComponent<ModelType>(m_engine->GetModel("nanosuit"));
		//		newE->GetComponent<TransformType>()->m_position = { (float)x * 8.f, 0.f, (float)z * 3.f + 5.f };
		//	}
		//}

		//////m_engine->CreateModel("sponza", "../assets/Models/Sponza_new/sponza.fbx");
		////auto e2 = CreateEntity("Entity2");

		////e2->AddComponent<ModelType>(m_engine->GetModel("sponza"));
		////e2->GetComponent<TransformType>()->m_scaling = { 0.07f, 0.07f, 0.07f };
		////e2->GetComponent<TransformType>()->m_position = { 0.f, 0.f, 250.f };


		//auto sphere = CreateEntity("pbrSphere");
		//m_engine->CreateModel("pbrSphere", "../assets/Models/sphere/Sphere.obj");
		//sphere->AddComponent<ModelType>(m_engine->GetModel("pbrSphere"));
		//sphere->GetComponent<TransformType>()->m_position = { -4.f, -4.f, 5.f };
		//sphere->GetComponent<TransformType>()->m_scaling = { 0.05f, 0.05f, 0.05f };




		// Currently no support for adding entities on update and hooking it up to renderer
		// We Finalize Scene to hook the existing entities to the renderers
		// We could make it dynamic by looping through all entities every frame and then sending the new hook to Engine
		//FinalizeScene();
	}

	static float timeElapsed = 0.f;
	static int counter = 0;
	void Scene::Update(float dt)
	{
		Timer clearTime;
		m_modelInstances.clear();
		ImGui::Begin("Frame Statistics");
		ImGui::Text("Scene Render Data Clear %s ms", std::to_string(clearTime.TimeElapsed() * 1000.f).c_str());
		ImGui::End();

		//assert(m_finalized);

		/* Simulate models */
		timeElapsed += dt;
	
		//GetEntity("Entity2")->GetComponent<TransformType>()->m_rotation.y += 45.f * dt;
		//GetEntity("Entity2")->GetComponent<TransformType>()->m_position.y = cosf(timeElapsed) * 40.f;

		//auto newE = CreateEntity("ent" + std::to_string((counter++)));
		//newE->AddComponent<ModelType>(m_engine->GetModel("nanosuit"));
		//newE->GetComponent<TransformType>()->m_position = { (float)counter * 8.f, 0.f, (float)counter * 3.f + 5.f };
		
		ImGui::Begin("Frame Statistics");
		ImGui::Text("Entity Count %i", m_entities.size());
		ImGui::End();

		Timer grabTime;
		// Grab relevant data from entities
		for (const auto& e : m_entities)
		{
			// Grab models and transforms
			if ((e.second.get()->GetActiveComponentBits() & (ComponentType::TransformType | ComponentType::ModelType)) ==
				(ComponentType::TransformType | ComponentType::ModelType))
			{
				// Check if the model already exists in this modelInstances vector
				auto it = std::find_if(m_modelInstances.begin(), m_modelInstances.end(),
					[&e](auto& existingModelInstance)
					{
						return e.second.get()->GetComponent<ModelType>() == existingModelInstance.first;
					});

				// Not found (push a new pair with new transform
				if (it == m_modelInstances.end())
				{
					m_modelInstances.push_back({ e.second.get()->GetComponent<ModelType>(), { e.second.get()->GetComponent<TransformType>() } });
				}
				// Found
				else
				{
					(*it).second.push_back(e.second.get()->GetComponent<TransformType>());
				}

			}
		}
		
		ImGui::Begin("Frame Statistics");
		ImGui::Text("Render Data Grabber %s ms", std::to_string(grabTime.TimeElapsed() * 1000.f).c_str());
		ImGui::End();
	}

	const std::vector<std::pair<Model*, std::vector<Transform*>>>* Scene::GetModelInstances() const
	{
		return &m_modelInstances;
	}

	void Scene::FinalizeScene()
	{
		// Grab relevant data from entities
		for (const auto& e : m_entities)
		{
			// Grab models and transforms
			if ((e.second.get()->GetActiveComponentBits() & (ComponentType::TransformType | ComponentType::ModelType)) ==
				(ComponentType::TransformType | ComponentType::ModelType))
			{
				// Check if the model already exists in this modelInstances vector
				auto it = std::find_if(m_modelInstances.begin(), m_modelInstances.end(), 
					[&e](auto& existingModelInstance) 
					{ 
						return e.second.get()->GetComponent<ModelType>() == existingModelInstance.first;
					});

				// Not found (push a new pair with new transform
				if (it == m_modelInstances.end())
				{
					m_modelInstances.push_back({ e.second.get()->GetComponent<ModelType>(), { e.second.get()->GetComponent<TransformType>() } });
				}
				// Found
				else
				{
					(*it).second.push_back(e.second.get()->GetComponent<TransformType>());
				}

			}
		}


		m_finalized = true;
	}

	Entity* Scene::CreateEntity(const std::string& name)
	{
		auto it = m_entities.find(name);
		if (it != m_entities.cend())
		{
			std::cout << "Entity name already taken!\n";
			assert(false);
		}

		return m_entities.insert({ name, std::make_unique<Entity>() }).first->second.get();
	}
	
	Entity* Scene::GetEntity(const std::string& name)
	{
		auto it = m_entities.find(name);
		if (it == m_entities.end())
		{
			std::cout << "Couldn't find entity with name: '" << name << "'\n";
			assert(false);
		}

		return (*it).second.get();
	}
}


