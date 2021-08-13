#include "pch.h"
#include "Input.h"

namespace Gino
{
	Input::Input(HWND hwnd) 
	{
		m_keyboard = std::make_unique<DirectX::Keyboard>();

		m_mouse = std::make_unique<DirectX::Mouse>();
		m_mouse->SetWindow(hwnd);
		InitMouse(DirectX::Mouse::Mode::MODE_ABSOLUTE);

	}

	Input::~Input()
	{
	}

	void Input::ProcessKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
	}

	void Input::ProcessMouse(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DirectX::Mouse::ProcessMessage(uMsg, wParam, lParam);

	}
	void Input::Update()
	{
		// We cant update this on ProcessMouse. Otherwise some functionality dont work properly (e.g PRESSED)
		m_mouseState = m_mouse->GetState();
		m_mouseTracker.Update(m_mouseState);

		m_keyboardState = m_keyboard->GetState();
		m_keyboardTracker.Update(m_keyboardState);

		// Get data and make sure to restore mouse state
		SetMouseMode(m_currMouseMode);
		{
			//// Get cursor delta
			//m_mouse->SetMode(DirectX::Mouse::Mode::MODE_RELATIVE);
			//m_screenPosition = { m_mouseState.x, m_mouseState.y };

			// Get mouse screen position of cursor
			m_mouse->SetMode(DirectX::Mouse::Mode::MODE_ABSOLUTE);
			m_currScreenPosition = { m_mouseState.x, m_mouseState.y };

			// Calculate mouse delta
			m_mouseDelta = { m_currScreenPosition.first - m_prevScreenPosition.first, m_currScreenPosition.second - m_prevScreenPosition.second };
		}
		RestorePreviousMouseMode();
		m_prevScreenPosition = m_currScreenPosition;



		// testing ground


		//if (m_keyboardTracker.IsKeyPressed(Gino::Keys::A))
		//	std::cout << "A pressed\n";
		//if (m_keyboardTracker.IsKeyReleased(Gino::Keys::A))
		//	std::cout << "A released\n";


	}
	void Input::InitMouse(DirectX::Mouse::Mode mode)
	{
		m_currMouseMode = mode;
		SetMouseMode(mode);
	}
	
	void Input::ChangeMouseMode(MouseMode mode)
	{
		switch (mode)
		{
		case MouseMode::ABS:
			SetMouseMode(DirectX::Mouse::Mode::MODE_ABSOLUTE);
			break;
		case MouseMode::REL:
			SetMouseMode(DirectX::Mouse::Mode::MODE_RELATIVE);
			break;
		default:
			assert(false);
		}
	}

	bool Input::LMBIsPressed() const
	{
		return m_mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::PRESSED;
	}
	bool Input::LMBIsReleased() const
	{
		return m_mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::RELEASED;
	}
	bool Input::LMBIsDown() const
	{
		return m_mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::HELD;
	}

	bool Input::RMBIsPressed() const
	{
		return m_mouseTracker.rightButton == DirectX::Mouse::ButtonStateTracker::ButtonState::PRESSED;
	}
	bool Input::RMBIsReleased() const
	{
		return m_mouseTracker.rightButton == DirectX::Mouse::ButtonStateTracker::ButtonState::RELEASED;
	}
	bool Input::RMBIsDown() const
	{
		return m_mouseTracker.rightButton == DirectX::Mouse::ButtonStateTracker::ButtonState::HELD;
	}

	bool Input::MMBIsPressed() const
	{
		return m_mouseTracker.middleButton == DirectX::Mouse::ButtonStateTracker::ButtonState::PRESSED;
	}
	bool Input::MMBIsReleased() const
	{
		return m_mouseTracker.middleButton == DirectX::Mouse::ButtonStateTracker::ButtonState::RELEASED;
	}
	bool Input::MMBIsDown() const
	{
		return m_mouseTracker.middleButton == DirectX::Mouse::ButtonStateTracker::ButtonState::HELD;
	}
	
	int Input::GetScrollWheelValue() const
	{
		return m_mouseState.scrollWheelValue;
	}
	const std::pair<int, int>& Input::GetScreenPosition() const
	{
		return m_currScreenPosition;
	}
	const std::pair<int, int>& Input::GetMouseDelta() const
	{
		return m_mouseDelta;
	}

	bool Input::KeyIsPressed(Keys key) const
	{
		return m_keyboardTracker.IsKeyPressed(key);
	}
	bool Input::KeyIsReleased(Keys key) const
	{
		return m_keyboardTracker.IsKeyReleased(key);
	}
	bool Input::KeyIsDown(Keys key) const
	{
		return m_keyboardState.IsKeyDown(key);
	}

	void Input::SetMouseMode(DirectX::Mouse::Mode mode)
	{
		m_prevMouseMode = m_currMouseMode;
		m_currMouseMode = mode;
		m_mouse->SetMode(mode);

		if (mode == DirectX::Mouse::Mode::MODE_RELATIVE)
		{
			m_mouse->SetVisible(false);
		}
	}
	void Input::RestorePreviousMouseMode()
	{
		m_currMouseMode = m_prevMouseMode;
		m_mouse->SetMode(m_prevMouseMode);
	}
}
