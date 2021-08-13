#include "pch.h"
#include "FPCamera.h"

namespace Gino
{
	FPCamera::FPCamera(float aspectRatio, float fovInDegs, float nearPlane, float farPlane, float moveSpeed) :
		m_localUp(s_worldUp),
		m_localRight(s_worldRight),
		m_localForward(s_worldForward),
		m_moveDirectionThisFrame(0.f, 0.f, 0.f),
		m_worldPosition(0.f, 0.f, 0.f),

		// Prior to change
		m_camPitch(0.f),
		m_camYaw(0.f),
		m_mouseSpeed(0.2f),

		m_fovInDegs(fovInDegs),
		m_aspectRatio(aspectRatio),
		m_nearPlane(nearPlane),
		m_farPlane(farPlane),

		m_moveSpeed(moveSpeed)
	{
	}

	void FPCamera::Move(MoveDirection direction)
	{
		switch (direction)
		{
		case MoveDirection::Forward:
			m_moveDirectionThisFrame += m_localForward;
			break;
		case MoveDirection::Backward:
			m_moveDirectionThisFrame += -m_localForward;
			break;
		case MoveDirection::Left:
			m_moveDirectionThisFrame += -m_localRight;
			break;
		case MoveDirection::Right:
			m_moveDirectionThisFrame += m_localRight;
			break;
		case MoveDirection::Up:
			m_moveDirectionThisFrame += m_localUp;
			break;
		case MoveDirection::Down:
			m_moveDirectionThisFrame += -m_localUp;
			break;
		default:
			assert(false);
		}

		// We normalize the vector on Update
	}

	void FPCamera::SetPosition(const DirectX::SimpleMath::Vector3& pos)
	{
		m_worldPosition = pos;
	}

	void FPCamera::RotateCamera(const std::pair<int, int>& mouseDt)
	{
		float deltaYaw = (float)mouseDt.first * m_mouseSpeed;
		float deltaPitch = (float)mouseDt.second * m_mouseSpeed;

		m_camYaw += deltaYaw;
		m_camPitch += deltaPitch;

		// Constrain to avoid gimbal lock
		if (m_camPitch > 89.f)
		{
			m_camPitch = 89.f;
		}
		else if (m_camPitch < -89.f)
		{
			m_camPitch = -89.f;
		}

		std::cout << "dx: " << mouseDt.first << std::endl;
		std::cout << "dy: " << mouseDt.second << std::endl << std::endl;
	}

	DirectX::SimpleMath::Matrix FPCamera::GetViewMatrix() const
	{
		auto lookAtPos = m_worldPosition + m_localForward;
		return DirectX::XMMatrixLookAtLH(m_worldPosition, lookAtPos, s_worldUp);
	}

	DirectX::SimpleMath::Matrix FPCamera::GetProjectionMatrix() const
	{
		return DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_fovInDegs), m_aspectRatio, m_nearPlane, m_farPlane);
	}

	void FPCamera::Update(float dt)
	{
		// Update position
		m_moveDirectionThisFrame.Normalize();
		m_worldPosition += m_moveDirectionThisFrame * m_moveSpeed * dt;
		m_moveDirectionThisFrame = { 0.f, 0.f, 0.f };

	}
}

