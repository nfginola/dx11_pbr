#include "pch.h"
#include "Component.h"

namespace Gino
{
	Transform::Transform() :
		Component(ComponentType::TransformType)
	{
	}

	uint32_t Component::GetBit() const
	{
		uint32_t bit = 1 << (m_type - 1);
		return bit;
	}

	Component::Component(ComponentType type) :
		m_type(type)
	{
	}
}

