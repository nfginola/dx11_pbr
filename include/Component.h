#pragma once
#include <d3d11.h>
#include <SimpleMath.h>
#include <array>

namespace Gino
{
	// Enum starts at 1! This affecs indexing and others
	// GetBits in Component is one place for example 

	// It starts at 1 so that we can use it as proper bitflag
	// 1, 2, 4, 8, etc. which signify that the specified component is present
	// We use this enum as bitshift value (<< 1, 2, 3, 4, etc.
	enum ComponentType
	{
		TransformType = 1,
		ModelType
	};
	// NOTE ===== Dont forget to add a ComponentMapper for the new ComponentType (currently in Entity)

	class Component
	{
	public:
		uint32_t GetBit() const;

	protected:
		Component(ComponentType type);
		~Component() = default;

	private:
		ComponentType m_type;
	};

	class Transform : public Component
	{
	public:
		Transform();
		~Transform() = default;

		DirectX::SimpleMath::Matrix GetWorldMatrix() const;

		// Helper functions
		// set, get matrix, etc
	public:
		DirectX::SimpleMath::Vector3 m_position;
		DirectX::SimpleMath::Vector3 m_rotation;
		DirectX::SimpleMath::Vector3 m_scaling = { 1.f, 1.f, 1.f };

	};



}
