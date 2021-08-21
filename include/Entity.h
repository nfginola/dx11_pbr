#pragma once
#include "Component.h"

#include "Graphics/Model.h"

namespace Gino
{
	static constexpr int MAX_COMPONENTS = 16;

	// Map component type to real types with some metadata for Entity usage
	template <ComponentType T>
	struct ComponentMapper;

	template <>
	struct ComponentMapper<ComponentType::TransformType>
	{ 
		using type = Transform; 
		static constexpr uint32_t bit = ComponentType::TransformType;
		static constexpr int index = bit - 1;		// Enum starts at 1, so we map the enum directly to array indices
	};

	template <>
	struct ComponentMapper<ComponentType::ModelType>
	{ 
		using type = Model; 
		static constexpr uint32_t bit = ComponentType::ModelType;
		static constexpr int index = bit - 1;
	};



	class Entity
	{
	public:
		Entity();
		~Entity();

		template <ComponentType T>
		void AddComponent(Component* comp);

		template <ComponentType T> 
		auto GetComponent() const;

		uint32_t GetActiveComponentBits() const;

	private:
		std::array<Component*, MAX_COMPONENTS> m_components;

		// Bitflags using ComponentType
		uint32_t m_activeComponentBits;		

	};

	template<ComponentType T>
	inline void Entity::AddComponent(Component* comp)
	{
		// Check that the component given is the right type
		assert((comp->GetBit() & ComponentMapper<T>::bit) == ComponentMapper<T>::bit);

		// Check that the component type doesn't already exist
		assert((m_activeComponentBits & ComponentMapper<T>::bit) != ComponentMapper<T>::bit);

		// Add component since it doesnt exist
		m_components[ComponentMapper<T>::index] = comp;
		m_activeComponentBits |= ComponentMapper<T>::bit;
	}

	template<ComponentType T>
	auto Entity::GetComponent() const
	{
		// Make sure the component exists!
		assert((m_activeComponentBits & ComponentMapper<T>::bit) == ComponentMapper<T>::bit);
		return static_cast<ComponentMapper<T>::type*>(m_components[ComponentMapper<T>::index]);
	}
}


