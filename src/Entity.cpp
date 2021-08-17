#include "pch.h"
#include "Entity.h"

namespace Gino
{
	Entity::Entity() :
		m_activeComponentsBits(ComponentType::TransformType)
	{
		m_components[ComponentMapper<ComponentType::TransformType>::index] = new Transform;
	}

	Entity::~Entity()
	{
		// Clean up transform component
		auto tr = reinterpret_cast<Transform*>(m_components[ComponentMapper<ComponentType::TransformType>::index]);
		delete tr;
	}
}

