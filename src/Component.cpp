#include "pch.h"
#include "Component.h"

namespace Gino
{
	Transform::Transform() :
		Component(ComponentType::TransformType)
	{
	}

	DirectX::SimpleMath::Matrix Transform::GetWorldMatrix() const
	{
		auto T = DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
		auto R = DirectX::XMMatrixRotationRollPitchYaw(
			DirectX::XMConvertToRadians(m_rotation.x),
			DirectX::XMConvertToRadians(m_rotation.y), 
			DirectX::XMConvertToRadians(m_rotation.z));
		auto S = DirectX::XMMatrixScalingFromVector(m_scaling);

		return S * R * T;
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

