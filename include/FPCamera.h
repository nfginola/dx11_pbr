#pragma once

#include <d3d11.h>		// Required for SimpleMath
#include <SimpleMath.h>

namespace Gino
{
	enum class MoveDirection
	{
		Forward,
		Backward,
		Left,
		Right,
		Up,
		Down
	};

	class FPCamera
	{
	public:
		FPCamera(float aspectRatio, float fovInDegs, float nearPlane = 0.1f, float farPlane = 1000.f, float moveSpeed = 35.f);
		~FPCamera() = default;

		void Move(MoveDirection direction);
		void SetPosition(const DirectX::SimpleMath::Vector3& pos);

		void RotateCamera(const std::pair<int, int>& mouseDt, float dt);

		DirectX::SimpleMath::Matrix GetViewMatrix() const;
		DirectX::SimpleMath::Matrix GetProjectionMatrix() const;

		// Finalize changes this frame (movement/rotation)
		void Update(float dt);

	private:
		// LH system
		static constexpr DirectX::SimpleMath::Vector3 s_worldUp = { 0.f, 1.f, 0.f };
		static constexpr DirectX::SimpleMath::Vector3 s_worldRight = { 1.f, 0.f, 0.f };
		static constexpr DirectX::SimpleMath::Vector3 s_worldForward = { 0.f, 0.f, 1.f };

		// Normalized local unit vectors in world space
		DirectX::SimpleMath::Vector3 m_localUp;
		DirectX::SimpleMath::Vector3 m_localRight;
		DirectX::SimpleMath::Vector3 m_localForward;

		DirectX::SimpleMath::Vector3 m_moveDirectionThisFrame;
		DirectX::SimpleMath::Vector3 m_worldPosition;

		float m_camPitch;
		float m_camYaw;
		float m_mouseSpeed;

		float m_fovInDegs;
		float m_aspectRatio;
		float m_nearPlane;
		float m_farPlane;

		float m_moveSpeed;
	};
}
