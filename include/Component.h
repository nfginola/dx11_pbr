#pragma once
#include <d3d11.h>
#include <SimpleMath.h>
#include <array>
#include <type_traits>

namespace Gino
{
	// Enum starts at 1! This affecs indexing and others
	// GetBits in Component is one place for example 
	enum ComponentType
	{
		TransformType = 1,
		ModelType
	};

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

		// Helper functions
		// set, get matrix, etc
	private:
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector3 rotation;
		DirectX::SimpleMath::Vector3 scaling = { 1.f, 1.f, 1.f };

	};



}
