#include "pch.h"
#include "Scene.h"
#include "Input.h"

namespace Gino
{
	Scene::Scene(Engine* engine) :
		m_engine(engine)
	{
		m_engine->SetScene(this);

		m_engine->CreateModel("sponza", "../assets/Models/Sponza_new/sponza.obj");
		m_engine->CreateModel("nanosuit", "../assets/Models/nanosuit/nanosuit.obj");
		auto e = CreateEntity("Entity1");
		auto e2 = CreateEntity("Entity2");

		e->AddComponent<ModelType>(m_engine->GetModel("sponza"));
		e->GetComponent<TransformType>()->m_scaling = { 0.07f, 0.07f, 0.07f };

		e2->AddComponent<ModelType>(m_engine->GetModel("sponza"));
		e2->GetComponent<TransformType>()->m_scaling = { 0.07f, 0.07f, 0.07f };
		e2->GetComponent<TransformType>()->m_position = { 0.f, 0.f, 250.f };

		int counter = 0;
		for (int x = -10; x < 10; ++x)
		{
			for (int z = -5; z < 5; ++z)
			{
				auto newE = CreateEntity("ent" + std::to_string((counter++)));
				newE->AddComponent<ModelType>(m_engine->GetModel("nanosuit"));
				newE->GetComponent<TransformType>()->m_position = { (float)x * 3.f, 0.f, (float)z + 6.f };
			}
		}



		FinalizeScene();
	}

	static float timeElapsed = 0.f;
	void Scene::Update(float dt)
	{
		assert(m_finalized);

		/* Simulate models */
		timeElapsed += dt;
	
		GetEntity("Entity2")->GetComponent<TransformType>()->m_rotation.y += 45.f * dt;
		GetEntity("Entity2")->GetComponent<TransformType>()->m_position.y = cosf(timeElapsed) * 40.f;


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


