#include "pch.h"
#include "Entity.h"

namespace Gino
{
	Entity::Entity() :
		m_activeComponentBits(ComponentType::TransformType)
	{
		m_components[ComponentMapper<ComponentType::TransformType>::index] = new Transform;
	}

	Entity::~Entity()
	{
		// Clean up transform component
		auto tr = reinterpret_cast<Transform*>(m_components[ComponentMapper<ComponentType::TransformType>::index]);
		delete tr;
	}
	uint32_t Entity::GetActiveComponentBits() const
	{
		return m_activeComponentBits;
	}
}

