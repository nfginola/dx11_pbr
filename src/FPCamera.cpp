#include "pch.h"
#include "FPCamera.h"

namespace Gino
{
	FPCamera::FPCamera(float aspectRatio, float fovInDegs, float nearPlane, float farPlane, float defaultMoveSpeed) :
		m_localUp(s_worldUp),
		m_localRight(s_worldRight),
		m_localForward(s_worldForward),
		m_moveDirectionThisFrame(0.f, 0.f, 0.f),
		m_worldPosition(0.f, 2.f, 0.f),

		m_camPitch(90.f),	// We want to be looking parallell to the XZ plane at first!
		m_camYaw(0.f),
		m_mouseSpeed(0.35f),

		m_fovInDegs(fovInDegs),
		m_aspectRatio(aspectRatio),
		m_nearPlane(nearPlane),
		m_farPlane(farPlane),

		m_moveSpeed(defaultMoveSpeed)
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

	void FPCamera::SetMoveSpeed(float moveSpeed)
	{
		m_moveSpeed = moveSpeed;
	}

	void FPCamera::SetMoveSpeed(MoveSpeed speed)
	{
		switch (speed)
		{
		case MoveSpeed::Slow:
			m_moveSpeed = s_moveSpeedSlow;
			break;
		case MoveSpeed::Normal:
			m_moveSpeed = s_moveSpeeNormal;
			break;
		case MoveSpeed::Fast:
			m_moveSpeed = s_moveSpeedFast;
			break;
		}
	}

	const DirectX::SimpleMath::Vector4& FPCamera::GetPosition() const
	{
		return DirectX::SimpleMath::Vector4(m_worldPosition.x, m_worldPosition.y, m_worldPosition.z, 1.f);
	}

	void FPCamera::RotateCamera(const std::pair<int, int>& mouseDt)
	{
		// No need for delta since we consume the WMs generated for this frame
		// We instead supply some multiplier which is applied per WM (applied on the frames total delta)
		float deltaYaw = (float)mouseDt.first * m_mouseSpeed * 0.14f;
		float deltaPitch = (float)mouseDt.second * m_mouseSpeed * 0.14f;

		// Remember that 0, 0 is top left and positive dx is right and positive dy is down
		m_camYaw += deltaYaw;		
		m_camPitch += deltaPitch;

		// Using spherical coordinates
		// We will use pitch as the angle from Y to XZ plane
		// We will use yaw as the angle from X to YZ plane. We need to use the inverted yaw since dragging the mouse to the left applies negative yaw
		
		// Constrain to avoid gimbal lock
		if (m_camPitch > 179.f)
		{
			m_camPitch = 179.f;
		}
		else if (m_camPitch < 1.f)
		{
			m_camPitch = 1.f;
		}
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
		// Update orientation
		
		// Here I use the spherical coordinates
		// We will use pitch as the angle from Y to XZ plane (CCW, thumb on Z pointing Z+)
		// We will use yaw as the angle from X to YZ plane (CCW). We need to use the inverted yaw since dragging the mouse to the left applies negative yaw

		// 90 deg offset since Forward vector is always 90 deg CCW from Right vector
		m_localForward.x = cos(DirectX::XMConvertToRadians(-m_camYaw + 90.f)) * sin(DirectX::XMConvertToRadians(m_camPitch));
		m_localForward.z = sin(DirectX::XMConvertToRadians(-m_camYaw + 90.f)) * sin(DirectX::XMConvertToRadians(m_camPitch));
		m_localForward.y = cos(DirectX::XMConvertToRadians(m_camPitch));
		m_localForward.Normalize();

		m_localRight.x = cos(DirectX::XMConvertToRadians(-m_camYaw));
		m_localRight.z = sin(DirectX::XMConvertToRadians(-m_camYaw));
		m_localRight.Normalize();

		// We wont change local up and down. We will keep them world up and down :) (easier to navigate)
		
		// Update position
		if (m_moveDirectionThisFrame.Length() >= DirectX::g_XMEpsilon[0])
		{
			m_moveDirectionThisFrame.Normalize();
			m_worldPosition += m_moveDirectionThisFrame * m_moveSpeed * dt;

		}

		// Reset move direction
		m_moveDirectionThisFrame = { 0.f, 0.f, 0.f };


		// Debug print
	/*	std::cout << "Yaw: " << m_camYaw << std::endl;
		std::cout << "Local forward || X: " << m_localForward.x << ", Y: " << m_localForward.y << ", Z: " << m_localForward.z << std::endl;
		std::cout << "Local right || X: " << m_localRight.x << ", Y: " << m_localRight.y << ", Z: " << m_localRight.z << std::endl << std::endl;*/
		//std::cout << "Move direction || X: " << m_moveDirectionThisFrame.x << ", Y: " << m_moveDirectionThisFrame.y << ", Z: " << m_moveDirectionThisFrame.z << std::endl;

	}
}

