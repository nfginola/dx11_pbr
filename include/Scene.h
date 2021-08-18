#pragma once
#include <vector>
#include <utility>
#include "Engine.h"
#include "Entity.h"

namespace Gino
{
	class Scene
	{
	public:
		Scene(Engine* engine);
		~Scene() = default;

		void Update(float dt);

		const std::vector<std::pair<Model*, std::vector<Transform*>>>* GetModelInstances() const;

	private:
		void FinalizeScene();	// Called at the end of scene initialization

		Entity* CreateEntity(const std::string& name);
		Entity* GetEntity(const std::string& name);

	private:
		Engine* m_engine;
		bool m_finalized = false;

		// { model1, vector<Matrix> } 
		// { model2, vector<Matrix> }
		std::vector<std::pair<Model*, std::vector<Transform*>>> m_modelInstances;

		std::unordered_map<std::string, std::unique_ptr<Entity>> m_entities;
	};
}


