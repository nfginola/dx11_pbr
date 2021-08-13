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
		m_moveDirectionThisFrame.Normalize();
		m_worldPosition += m_moveDirectionThisFrame * m_moveSpeed * dt;
		m_moveDirectionThisFrame = { 0.f, 0.f, 0.f };


	}
}

